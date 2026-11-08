# Modules

[中文](../../docs_cn/develop/modules_cn.md) | [Architecture](../design/architecture.md)

**Target design — the server is not implemented.** `CanTransport` is compiled and moves raw SocketCAN frames only. It does not decode Nero fields. Wire fields live only in the [IPC contract](../interfaces/ipc.md). The delivery checklist is the [roadmap](../maintenance/roadmap.md). Style rules are in [code style](code_style.md).

## Layout

The product tree grows by module. The root `CMakeLists.txt` pins the language standard and builds `dual_nero_can_transport`, the read-only `record_can` tool, and, when librealsense is installed, the D435 library plus `capture_d435`.

The compiled tree has two device groups. `modules/arm` owns SocketCAN. `modules/cameras` owns images. A camera library is not linked into an arm tool.

```text
modules/arm/transport/                 raw SocketCAN frames
modules/cameras/                       shared CameraFrame
modules/cameras/environment/           scene RealSense D435
modules/cameras/wrist/                 wrist Orbbec Dabai DC1
apps/arm/record_can.cpp                read-only CAN recorder
apps/cameras/capture_d435.cpp          one D435 color and depth pair
apps/cameras/capture_dc1.cpp           one DC1 color and depth pair
```

These owners are described below and are not directories yet: `firmware_profile`, `nero_protocol`, `nero_driver`, `state_synchronizer`, `command_buffer`, `safety_supervisor`, and `dual_nero_server`.

Each existing module directory contains `CMakeLists.txt`, public headers under `include/dual_nero/`, and private sources under `src/`. `AgileX/` stays outside this layout.

## Firmware and units

Internal values use radians, radians per second, meters, newton-meters, and a monotonic clock in nanoseconds. Degrees, protocol integers, and wire scaling stay inside `NeroProtocol`. Boundary values have tests.

The public Python SDK distinguishes Nero profiles `default`, `v111`, `v112`, `v120`, and `v121`. Startup connects read-only, reads the firmware version, maps it to `NeroFirmwareProfile`, and refuses motion when the match is not exact. A profile is never a silent fallback for another profile. Piper frames are not a substitute for Nero frames.

`pyAgxArm` is LGPL-3.0-only. Copying or adapting its source requires keeping the copyright and license notices. The reference pages are the firmware notes, the first-time CAN guide, and the CAN module manual under `AgileX/pyAgxArm/docs/`.

## Responsibilities

`CanTransport` lives in `modules/arm/transport`. It opens one SocketCAN interface and sends or receives raw frames. It uses `PF_CAN`, `SOCK_RAW`, and `CAN_RAW`, binds with `SIOCGIFINDEX`, and waits with `poll` or `epoll`. It checks short reads, short writes, `ENETDOWN`, `ENOBUFS`, unplug, and bus-off, records error frames, and closes the descriptor in the destructor. It does not catch an unknown failure and continue with a stale state. Bus loss is reported to the safety supervisor.

`NeroProtocol` is a pure encoder and decoder for one firmware profile. It has no thread, socket, or process-global state. Encode paths reject out-of-range values, NaN, and infinity, check the physical unit before scaling, check integer overflow after scaling, and are deterministic.

`NeroDriver` owns one arm side, one `CanTransport`, and one `NeroProtocol`. It connects, enables, disables, and emergency-stops that arm. Its receive loop is independent of model inference. Its send loop does not consume an unchecked network payload.

`StateSynchronizer` stamps each feedback sample with `CLOCK_MONOTONIC` or `std::chrono::steady_clock`, stores the latest complete sample of each arm, and publishes a `DualArmState` only while the left-right skew is inside the configured window. A dataset writer stores valid pairs and also stores the original timestamps. The first window is one control period until the measured feedback rate replaces that guess.

`command_buffer` accepts an action chunk whose width is 16: seven left joints, the left gripper, seven right joints, and the right gripper. It checks the shape and that every value is finite. Denormalizing a model output into SI units happens in Python or in one explicit conversion step, not inside the CAN encoder. Each step is checked for position, speed, and acceleration. Execution follows the monotonic clock. A newer chunk replaces the unexecuted tail according to configuration. After expiry the buffer stops advancing. It does not repeat the last trajectory forever.

The safety supervisor is mandatory and independent of the model. It rejects NaN, infinity, a bad width, a non-monotonic sequence, an expired command, and an unknown control mode. Software joint limits are inside the mechanical limits. Each cycle has a maximum position step, speed, and acceleration. One stale arm, one fault, CAN bus-off, an unplug, or a large feedback-to-command error stops both arms. Emergency stop outranks every model command. The first workspace guard is a conservative box that keeps each arm out of the other arm's region. Mesh collision, table distance, grasped-object hulls, and cable zones are later work. Watchdogs cover the model command, the Python heartbeat, each arm's feedback, the CAN bus, and action execution. The preferred timeout reaction is to stop advancing and hold a safe position. The hold behavior still has to be checked with the real load and the gripper.

Teach pendants, if they speak CAN, are read by the same C++ process so they share one clock. A sample keeps the raw joint values, the calibrated target, the sample time, the gripper state, the mapping version, whether a limit was applied, and the command actually sent. A mapping between a pendant and an arm is configuration, not scattered constants.

## Public header sketch

`CanTransport` in the sketch is the class compiled in `modules/arm/transport`. It moves one raw `can_frame`. `NeroProtocol` is still a sketch and is not compiled. Units for that sketch are radians and meters. `open` and `connect` block while the socket or the arm session is established. `receive` blocks until a frame arrives or the timeout expires. `close`, `disconnect`, `stop`, `disable`, and `emergency_stop` do not throw. A failed `open`, `send`, or `enable` reports the system or protocol error to the caller instead of leaving the object looking connected.

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

## Configuration sample

Frequencies and timeouts in this sample are placeholders. They are not measured Nero limits.

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

## Verification

Unit tests, with no arm attached, cover feedback parsing, command encoding, sign, zero, minimum, maximum, overflow, NaN, infinity, firmware differences, limits, speed, timeout, and left-right pairing. vcan tests load the `vcan` module, create `vcan_left` and `vcan_right`, and answer with a simulated Nero node before any real adapter is used. Shadow mode compares the C++ parse with `pyAgxArm` on a read-only capture. Hardware acceptance follows the order in [setup](../usage/setup.md).

```bash
sudo modprobe vcan
sudo ip link add dev vcan_left type vcan
sudo ip link add dev vcan_right type vcan
sudo ip link set vcan_left up
sudo ip link set vcan_right up
```
