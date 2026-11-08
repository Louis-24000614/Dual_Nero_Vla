# Setup

[中文](../../docs_cn/usage/setup_cn.md) | [Architecture](../design/architecture.md)

**Target design — not implemented.** The commands below prepare a host and name the CAN interfaces. They do not start `dual_nero_server`, because that program is not built yet. See the [roadmap](../maintenance/roadmap.md).

## Prerequisites

Use Ubuntu 22.04 LTS or Ubuntu 24.04 LTS. The first-party C++ standard is C++20, set by the root `CMakeLists.txt`. CMake 3.20 or newer is required. Two official USB-CAN adapters are required for the real arms. Training and inference expect two RTX 3090 GPUs in a process that is separate from CAN control.

## Packages

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

| Package | Role |
| --- | --- |
| SocketCAN | Linux CAN sockets |
| can-utils | `candump` and `cansend` |
| ZeroMQ | Process IPC |
| Protobuf | Versioned IPC messages |
| GoogleTest | Protocol, scaling, and safety tests |

## CAN names

Do not bind the arms to whatever the kernel enumerates as `can0` and `can1`. Pin each USB port or adapter serial number to a stable name.

```text
left arm:  can_left
right arm: can_right
```

The official bitrate used by the vendor SDK is 1 Mbit/s.

```bash
sudo ip link set can_left down 2>/dev/null || true
sudo ip link set can_left type can bitrate 1000000
sudo ip link set can_left up

sudo ip link set can_right down 2>/dev/null || true
sudo ip link set can_right type can bitrate 1000000
sudo ip link set can_right up
```

Check the links, then listen. The first session is read-only. Enable and motion wait until both buses show a continuous status stream and the C++ parser matches `pyAgxArm` on a recorded log.

```bash
ip -details link show can_left
ip -details link show can_right
candump can_left
candump can_right
```

## Configure the build

The root project sets the C++ standard and builds `apps/arm/record_can.cpp` and, when librealsense is installed, `apps/cameras/capture_d435.cpp`. Arm code is under `modules/arm`. Camera code is under `modules/cameras`. It does not compile `dual_nero_server` yet.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
ctest --test-dir build --output-on-failure
```

After `ninja-build` is installed, add `-G Ninja` to the `cmake -S` command.

`cmake --build build` produces `build/apps/record_can`, `build/apps/capture_d435` when librealsense is installed, and `build/apps/capture_dc1` when the Orbbec SDK is present. `ctest` runs the CAN tests and the closed-camera tests. Those tests do not open a live bus or a camera.

Save one D435 color image and depth image after the camera is plugged in:

```bash
./build/apps/capture_d435 /tmp
```

This writes `/tmp/d435_color.ppm` and `/tmp/d435_depth.pgm`. Depth values are millimeters. The program does not open CAN.

Save one Dabai DC1 color image and depth image the same way:

```bash
./build/apps/capture_dc1 /tmp
```

This writes `/tmp/dc1_color.ppm` and `/tmp/dc1_depth.pgm`. The program opens the first device whose name contains `DC1` and does not open CAN.

## Hardware order

When a later phase connects real arms, use this order. Keep the arms unloaded, keep people out of the workspace, and keep a physical emergency stop within reach. Read status first. Then test disable, a single joint at very small amplitude, all joints at low speed, one arm, both arms with a large separation, watchdog and link loss, and only then a teach pendant or a model. Software-only stop is not the emergency stop.

Shadow comparison reads the real bus or an offline log. The C++ driver and `pyAgxArm` must not both send command frames on the same live bus.
