# 模块

[English](../../docs/develop/modules.md) | [架构](../design/architecture_cn.md)

**目标设计，服务尚未实现。** `CanTransport` 已经编译，只搬运 SocketCAN 原始帧，不解析 Nero 字段。线上字段只写在 [IPC 契约](../interfaces/ipc_cn.md) 里。交付清单是[路线图](../maintenance/roadmap_cn.md)。风格规则在[代码风格](code_style_cn.md)。

## 目录

产品树按模块增长。根目录 `CMakeLists.txt` 固定语言标准，并编译 `dual_nero_can_transport` 和只读工具 `record_can`。

```text
CMakeLists.txt
config/dual_nero.example.yaml          示例，尚未加载
proto/dual_nero.proto                   IPC 契约源文件
modules/can_transport/
modules/firmware_profile/
modules/nero_protocol/
modules/nero_driver/
modules/state_synchronizer/
modules/command_buffer/
modules/safety_supervisor/
modules/dual_nero_server/
apps/                                   探测、只读、使能、回零、记录
python/dual_nero_client/
python/lerobot_robot_dual_nero/
tests/protocol/  tests/safety/  tests/synchronization/  tests/vcan/
tools/                                  activate_can.sh、create_vcan.sh、inspect_log.py
```

每个模块目录包含 `CMakeLists.txt`、`include/dual_nero/` 下的公开头文件，以及 `src/` 下的私有源文件。`AgileX/` 不进入这个布局。

## 固件与单位

内部数值使用弧度、弧度每秒、米、牛·米，以及纳秒单调时钟。角度的度、协议整数和线上缩放只留在 `NeroProtocol` 内。边界值要有测试。

公开 Python SDK 把 Nero 分为 `default`、`v111`、`v112`、`v120` 和 `v121`。启动时先只读连接，读取固件版本，映射到 `NeroFirmwareProfile`，不能精确匹配时拒绝运动。一个档位不会静默充当另一个档位的后备。Piper 报文不能代替 Nero 报文。

`pyAgxArm` 是 LGPL-3.0-only。复制或改编它的源码时必须保留版权和许可证声明。参考页是 `AgileX/pyAgxArm/docs/` 下的固件说明、首次 CAN 指南和 CAN 模块手册。

## 职责

`CanTransport` 打开一个 SocketCAN 接口并收发原始帧。它使用 `PF_CAN`、`SOCK_RAW` 和 `CAN_RAW`，用 `SIOCGIFINDEX` 绑定，用 `poll` 或 `epoll` 等待。它检查短读、短写、`ENETDOWN`、`ENOBUFS`、设备拔出和 bus-off，记录错误帧，并在析构时关闭文件描述符。它不会吞掉未知失败后继续使用过期状态。总线断开会报告给安全监控器。

`NeroProtocol` 是某个固件档位的纯编码器和解析器。它没有线程、套接字或进程级全局状态。编码路径拒绝超范围、NaN 和无穷大，缩放前检查物理单位，缩放后检查整数溢出，并且结果确定。

`NeroDriver` 拥有一条机械臂、一个 `CanTransport` 和一个 `NeroProtocol`。它负责这条臂的连接、使能、失能和急停。接收循环独立于模型推理。发送循环不消费未经检查的网络载荷。

`StateSynchronizer` 用 `CLOCK_MONOTONIC` 或 `std::chrono::steady_clock` 给每个反馈样本打时间戳，保存每条臂最新的完整样本，并且只在左右时间差处于配置窗口内时发布 `DualArmState`。数据集写入有效配对，同时保留原始时间戳。在测得真实反馈频率之前，初始窗口取一个控制周期。

`command_buffer` 接受宽度为 16 的动作块：左臂七个关节、左夹爪、右臂七个关节、右夹爪。它检查形状，并要求每个值有限。把模型输出还原成国际单位制发生在 Python，或发生在单独的一步转换里，不发生在 CAN 编码器内部。每个时间步检查位置、速度和加速度。执行跟随单调时钟。更新的动作块按配置替换尚未执行的尾部。过期后缓冲停止向前推进，不无限重复最后一段轨迹。

安全监控器是强制边界，并且独立于模型。它拒绝 NaN、无穷大、错误宽度、非递增序号、过期命令和未知控制模式。软件关节限位位于机械极限之内。每个周期有最大位置增量、速度和加速度。一条臂状态过期、一条臂故障、CAN bus-off、设备拔出，或反馈与命令偏差过大，都会停止双臂。急停优先于任何模型命令。第一版工作空间保护是一个保守的盒子，避免一条臂进入另一条臂的区域。基于网格的碰撞、与桌面的距离、抓持物体后的外形和线缆区域以后再做。看门狗覆盖模型命令、Python 心跳、每条臂的反馈、CAN 总线和动作执行。优先的超时反应是停止推进并保持安全位置。保持动作仍须在真实负载和夹爪条件下验证。

如果示教器使用 CAN，由同一个 C++ 进程读取，以便共用一个时钟。一条样本保留原始关节值、标定后的目标、采样时间、夹爪状态、映射版本、是否经过限幅，以及实际发出的命令。示教器与机械臂之间的映射是配置，不是散落的常量。

## 公开头文件草图

下面的声明是预期的所有权边界。它们还没有编译，第一份模块落地之前方法列表可以改变。单位是弧度和米。`open` 和 `connect` 在套接字或机械臂会话建立期间阻塞。`receive` 阻塞到收到一帧或超时。`close`、`disconnect`、`stop`、`disable` 和 `emergency_stop` 不抛出异常。`open`、`send` 或 `enable` 失败时把系统或协议错误交给调用方，而不是让对象看起来仍然处于已连接状态。

```cpp
namespace dual_nero {
constexpr std::size_t kArmDof = 7;
using JointArray = std::array<double, kArmDof>;

class CanTransport {
 public:
  explicit CanTransport(std::string interface_name);
  void open();
  void close() noexcept;
  void send(const can_frame& frame);
  std::optional<can_frame> receive(std::chrono::milliseconds timeout);
};

class NeroProtocol {
 public:
  explicit NeroProtocol(NeroFirmwareProfile profile);
  std::vector<can_frame> encode_joint_position_command(const JointArray& position_rad) const;
  void consume_feedback_frame(const can_frame& frame, ArmState& state) const;
};
}  // namespace dual_nero
```

## 配置示例

示例里的频率和超时是占位值，不是已经测得的 Nero 极限。

```yaml
server:
  state_publish_hz: 50
  command_hz: 100
  ipc_endpoint: "ipc:///tmp/dual_nero.sock"
  model_watchdog_ms: 250
left_arm:
  can_interface: "can_left"
  firmware_profile: "v121"
  gripper_type: "agx_gripper"
right_arm:
  can_interface: "can_right"
  firmware_profile: "v121"
  gripper_type: "agx_gripper"
safety:
  require_both_arms_healthy: true
  max_state_age_ms: 100
  max_left_right_skew_ms: 20
  reject_nan: true
  reject_expired_commands: true
```

## 验证

不连接机械臂的单元测试覆盖反馈解析、命令编码、符号、零值、最小、最大、溢出、NaN、无穷大、固件差异、限位、速度、超时和左右配对。vcan 测试加载 `vcan` 模块，创建 `vcan_left` 和 `vcan_right`，由模拟 Nero 节点应答，然后再使用真实适配器。影子模式在只读采集上比较 C++ 解析和 `pyAgxArm`。真机验收顺序见[环境与接通](../usage/setup_cn.md)。

```bash
sudo modprobe vcan
sudo ip link add dev vcan_left type vcan
sudo ip link add dev vcan_right type vcan
sudo ip link set vcan_left up
sudo ip link set vcan_right up
```
