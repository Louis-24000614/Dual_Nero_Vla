# 环境与接通

[English](../../docs/usage/setup.md) | [架构](../design/architecture_cn.md)

**目标设计，尚未实现。** 下面的命令用来准备主机并命名 CAN 接口。它们不会启动 `dual_nero_server`，因为这个程序还没有被编译。见[路线图](../maintenance/roadmap_cn.md)。

## 前置条件

使用 Ubuntu 22.04 LTS 或 Ubuntu 24.04 LTS。第一方 C++ 标准是 C++20，由根目录 `CMakeLists.txt` 设定。需要 CMake 3.20 或更高版本。真实机械臂需要两个官方 USB-CAN 适配器。训练和推理使用两张 RTX 3090，并且与 CAN 控制进程分开。

## 软件包

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  can-utils \
  iproute2 \
  libzmq3-dev \
  cppzmq-dev \
  libprotobuf-dev \
  protobuf-compiler \
  libgtest-dev
```

| 软件包 | 作用 |
| --- | --- |
| SocketCAN | Linux CAN 套接字 |
| can-utils | `candump` 与 `cansend` |
| ZeroMQ | 进程间通信 |
| Protobuf | 带版本的 IPC 消息 |
| GoogleTest | 协议、缩放和安全逻辑测试 |

## CAN 名称

不要把机械臂绑在内核枚举出来的 `can0` 和 `can1` 上。按 USB 端口或适配器序列号固定名称。

```text
左臂：can_left
右臂：can_right
```

厂商 SDK 使用的波特率是 1 Mbit/s。

```bash
sudo ip link set can_left down 2>/dev/null || true
sudo ip link set can_left type can bitrate 1000000
sudo ip link set can_left up

sudo ip link set can_right down 2>/dev/null || true
sudo ip link set can_right type can bitrate 1000000
sudo ip link set can_right up
```

先看链路，再监听。第一次只读。两条总线都持续出现状态帧，并且 C++ 解析与 `pyAgxArm` 在同一条记录上一致之后，才进入使能和运动。

```bash
ip -details link show can_left
ip -details link show can_right
candump can_left
candump can_right
```

## 配置构建

根工程目前只设定 C++ 标准，不编译服务。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
ctest --test-dir build --output-on-failure
```

安装 `ninja-build` 之后，可以在 `cmake -S` 命令上加上 `-G Ninja`。

`cmake --build build` 在没有目标时成功。`ctest` 会报告没有发现测试。这是当前结果，不是机械臂测试通过。

## 真机顺序

后续阶段连接真机时按这个顺序。机械臂卸掉负载，人员离开工作区，物理急停放在随手可及的地方。先读状态，再测试失能、单关节极小幅度、全部关节低速、单臂、保持较大间距的双臂、看门狗和链路断开，最后才接入示教器或模型。软件按钮不是急停。

影子对比读取真实总线或离线日志。C++ 驱动和 `pyAgxArm` 不得同时向同一条真实总线发送控制帧。
