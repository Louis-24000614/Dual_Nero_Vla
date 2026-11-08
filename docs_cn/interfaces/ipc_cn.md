# IPC 契约

[English](../../docs/interfaces/ipc.md) | [架构](../design/architecture_cn.md)

**目标设计，尚未实现。** 仓库里还没有程序打开这个套接字或解析这个结构。下面的字段号要等第一版服务发出之后才成为兼容规则。在那之前它们是提案。清单是[路线图](../maintenance/roadmap_cn.md)。

## 概述

调用方是运行 LeRobot 或 VLA 策略的 Python 进程。拥有者是 `dual_nero_server`。传输是承载 Protobuf 消息的 ZeroMQ 套接字，包名为 `dual_nero`。ZeroMQ 负责连接。Protobuf 负责字段号和类型。第一版部署使用本地 IPC 端点 `ipc:///tmp/dual_nero.sock`。没有单独的认证令牌。准入条件是本地套接字访问，加上服务端的下列检查。支持一个控制客户端。第二个提交运动的客户端会被拒绝。状态读取可以共用这条连接。服务是唯一的 CAN 控制帧发送者。

## 生命周期与超时

客户端连接后，可以在 `Enable` 之前调用 `GetHealth` 和 `GetState`。`Enable` 只是请求。服务在允许运动之前仍然检查固件档位、总线健康和故障。只有两条臂都已使能且健康时，`SubmitActionChunk` 才有效。`Stop` 停止轨迹推进并保持。`Disable` 取消使能。`EmergencyStop` 优先于其他调用，任何本地客户端都可以发出。`StreamState` 是服务端推送的最新配对状态。

配置示例里的 `model_watchdog_ms` 默认 250，这不是实测极限。动作块过期或心跳缺失会停止推进。请求的套接字超时是 1.0 秒，除非以后的配置结构改掉它。一次 RPC 超时并不表示机械臂命令已经取消。客户端必须在超时后读取 `GetHealth`。

## 调用

| 调用 | 方向 | 谁可以调用 | 结果 |
| --- | --- | --- | --- |
| `GetState` | Python → C++ | 任意本地客户端 | 最新的 `DualArmState` |
| `StreamState` | C++ → Python | 已订阅的本地客户端 | 连续的 `DualArmState` |
| `SubmitActionChunk` | Python → C++ | 唯一的控制客户端 | 接受并限幅后的 `ActionChunk` |
| `Enable` | Python → C++ | 唯一的控制客户端 | 使能被接受或被拒绝 |
| `Disable` | Python → C++ | 唯一的控制客户端 | 机械臂失能 |
| `Stop` | Python → C++ | 唯一的控制客户端 | 轨迹停止推进 |
| `EmergencyStop` | 本地客户端 → C++ | 任意本地客户端 | 两条臂进入急停 |
| `GetHealth` | Python → C++ | 任意本地客户端 | 固件、总线和故障摘要 |

## 消息

`JointVector.value` 是 `repeated double`，字段号 1。向量本身没有单位，单位由父字段的名字表达。缺省是空列表，对机械臂命令或关节反馈来说这是非法的。校验要求长度为 7，且每个元素有限。新增字段使用新的字段号。已有的 1 号字段保持为 `value`。

| 字段 | 类型 | 单位 | 作用 | 缺省 | 校验 |
| --- | --- | --- | --- | --- | --- |
| `ArmState.position_rad` | `JointVector` | rad | 实测关节位置 | 缺少样本是故障，不是全零 | 长度 7，有限 |
| `ArmState.velocity_rad_s` | `JointVector` | rad/s | 实测关节速度 | 第一帧速度到来前为空 | 出现时长度 7，有限 |
| `ArmState.gripper_position` | `double` | m | AGX 夹爪开口 | `0` 是真实宽度，不是哨兵值 | 有限，范围由夹爪档位检查 |
| `ArmState.fault_code` | `uint32` | — | 厂商故障位 | `0` 表示没有故障位 | 透传解码后的码 |
| `ArmState.monotonic_time_ns` | `int64` | ns | `CLOCK_MONOTONIC` 上的采样时间 | `0` 表示样本尚未打戳 | 第一份样本之后必须为正 |
| `DualArmState.sequence` | `uint64` | — | 配对状态计数 | 第一对之前为 `0` | 每发布一对加一 |
| `DualArmState.left` / `right` | `ArmState` | — | 一条机械臂 | 缺少一条臂则配对无效 | 两条臂都新鲜 |
| `DualArmState.synchronized_time_ns` | `int64` | ns | 配对时间 | 配对无效时为 `0` | 单调 |
| `DualArmState.left_right_skew_ns` | `int64` | ns | 右样本时间减左样本时间 | 两个时间戳相同时为 `0` | 绝对值处于配置的偏差内 |
| `ActionStep.left_position_rad` | `JointVector` | rad | 左臂位置目标 | 空则无效 | 长度 7，有限，处于软件限位内 |
| `ActionStep.right_position_rad` | `JointVector` | rad | 右臂位置目标 | 空则无效 | 长度 7，有限，处于软件限位内 |
| `ActionStep.left_gripper_position` | `double` | m | 左夹爪目标 | 不隐式沿用上一次 | 有限，处于夹爪范围内 |
| `ActionStep.right_gripper_position` | `double` | m | 右夹爪目标 | 不隐式沿用上一次 | 有限，处于夹爪范围内 |
| `ActionStep.execute_at_ns` | `int64` | ns | 期望的单调执行时间 | `0` 无效 | 不得早于配置允许的滞后 |
| `ActionChunk.sequence` | `uint64` | — | 客户端动作块编号 | `0` 可以出现一次 | 每个客户端严格递增 |
| `ActionChunk.valid_until_ns` | `int64` | ns | 动作块过期时间 | `0` 拒绝该块 | 准入时大于服务端的单调当前时间 |
| `ActionChunk.steps` | `repeated ActionStep` | — | 未来目标 | 空块被拒绝 | 每一步通过 `ActionStep` 规则 |

Protobuf 的 `repeated` 不强制长度为 7。服务在解码后检查长度。

## 结构

```proto
syntax = "proto3";

package dual_nero;

message JointVector {
  repeated double value = 1;
}

message ArmState {
  JointVector position_rad = 1;
  JointVector velocity_rad_s = 2;
  double gripper_position = 3;
  uint32 fault_code = 4;
  int64 monotonic_time_ns = 5;
}

message DualArmState {
  uint64 sequence = 1;
  ArmState left = 2;
  ArmState right = 3;
  int64 synchronized_time_ns = 4;
  int64 left_right_skew_ns = 5;
}

message ActionStep {
  JointVector left_position_rad = 1;
  JointVector right_position_rad = 2;
  double left_gripper_position = 3;
  double right_gripper_position = 4;
  int64 execute_at_ns = 5;
}

message ActionChunk {
  uint64 sequence = 1;
  int64 valid_until_ns = 2;
  repeated ActionStep steps = 3;
}
```

## 示例

一个合法的位置步使用七个有限的关节目标，单位是弧度，夹爪宽度单位是米。

```text
ActionChunk sequence=1 valid_until_ns=<未来的单调时间 ns>
  ActionStep execute_at_ns=<未来的单调时间 ns>
    left_position_rad  = [0, 0, 0, 0, 0, 0, 0]
    right_position_rad = [0, 0, 0, 0, 0, 0, 0]
    left_gripper_position = 0.05
    right_gripper_position = 0.05
```

成功时返回服务限幅后保留的动作块，不是原始请求。LeRobot 必须保存这份被接受的动作块。

拒绝使用稳定的 `event_code`，并且不移动机械臂。

| 条件 | `event_code` |
| --- | --- |
| 关节长度错误、NaN 或无穷大 | `ipc.rejected.invalid_argument` |
| 序号没有递增，或动作块已过期 | `ipc.rejected.stale` |
| 第二个运动客户端 | `ipc.rejected.busy` |
| 固件档位不匹配或总线故障 | `ipc.rejected.unavailable` |
| 安全限位或急停 | `ipc.rejected.safety` |

## 兼容

兼容的演化用新字段号增加可选字段，并写明默认值。读取方接受省略新字段的旧消息。重命名或复用字段号是破坏性变更，需要新的包标识和迁移说明。产品版本和这个 `dual_nero` 包的版本各自变化。

## Python 客户端形状

LeRobot 适配层保持很薄。`get_observation` 把 C++ 接受的状态映射成 LeRobot 键。`send_action` 返回服务实际应用的动作。C++ 程序 `capture_d435` 产生 RealSense D435 的图像，以后记在 `global` 下面。`capture_dc1` 读取第一台连上的 Orbbec Dabai DC1。`left_wrist` 和 `right_wrist` 是以后两只腕部相机的名字，现在还没有分配左右。抓茶壶的第一批数据保存一条臂、该腕图像和全局图像。数据集把 `observation.state` 存为实测机械臂状态，把 `action` 存为示教器或模型目标，同时保存相机图像，`task` 是语言指令。存下的动作是 C++ 返回的限幅命令，不只是 Python 的原始请求。

```python
def get_observation(self) -> dict[str, float | object]:
    state = self.client.get_state()
    return {
        "left.joint_1.pos": state.left.position[0],
        "left.gripper.pos": state.left.gripper,
        "right.joint_1.pos": state.right.position[0],
        "right.gripper.pos": state.right.gripper,
        "global": self.global_camera.read(),
        "left_wrist": self.left_wrist_camera.read(),
        "right_wrist": self.right_wrist_camera.read(),
    }

def send_action(self, action: dict[str, float]) -> dict[str, float]:
    return self.client.submit_action(action)
```
