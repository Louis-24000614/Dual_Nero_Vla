# IPC contract

[中文](../../docs_cn/interfaces/ipc_cn.md) | [Architecture](../design/architecture.md)

**Target design — not implemented.** Nothing in the repository opens this socket or parses this schema. Field numbers below become compatibility rules only when the first server ships. Until then they are a proposal. The checklist is the [roadmap](../maintenance/roadmap.md).

## Overview

The caller is the Python process that runs LeRobot or a VLA policy. The owner is `dual_nero_server`. The transport is a ZeroMQ socket carrying Protobuf messages in package `dual_nero`. ZeroMQ owns the connection. Protobuf owns the field numbers and types. The first deployment uses a local IPC endpoint, `ipc:///tmp/dual_nero.sock`. There is no separate authentication token. Admission is local-socket access plus the server-side checks below. One controlling client is supported. A second client that submits motion is rejected. Status reads may share the connection. The server is the only CAN command sender.

## Lifecycle and timeouts

The client connects, then may call `GetHealth` and `GetState` before `Enable`. `Enable` is a request. The server still checks firmware profile, bus health, and faults before the arms are allowed to move. `SubmitActionChunk` is valid only while both arms are enabled and healthy. `Stop` stops trajectory advance and holds. `Disable` removes enable. `EmergencyStop` has priority over every other call and may be issued by any local client. `StreamState` is a server push of the latest paired state.

`model_watchdog_ms` in the configuration sample defaults to 250 and is not a measured limit. An expired chunk or a missed heartbeat stops advance. The socket `timeout` on a request is 1.0 second unless a later configuration schema changes it. A timed-out RPC does not mean the arm command was canceled. The client must read `GetHealth` after a timeout.

## Calls

| Call | Direction | Who may call | Result |
| --- | --- | --- | --- |
| `GetState` | Python → C++ | Any local client | Latest `DualArmState` |
| `StreamState` | C++ → Python | Subscribed local client | Continuous `DualArmState` |
| `SubmitActionChunk` | Python → C++ | The single controlling client | Accepted, limited `ActionChunk` |
| `Enable` | Python → C++ | The single controlling client | Enable accepted or a refusal |
| `Disable` | Python → C++ | The single controlling client | Arms disabled |
| `Stop` | Python → C++ | The single controlling client | Trajectory advance stopped |
| `EmergencyStop` | local client → C++ | Any local client | Both arms in emergency stop |
| `GetHealth` | Python → C++ | Any local client | Firmware, bus, and fault summary |

## Messages

`JointVector.value` is `repeated double`, field 1. The vector itself has no unit. The parent field names the unit. Absence is an empty list, which is invalid for an arm command or a reported joint sample. Validation requires length 7, and every element finite. Adding a field uses a new number. Existing number 1 stays `value`.

| Field | Type | Unit | Purpose | Absence | Validation |
| --- | --- | --- | --- | --- | --- |
| `ArmState.position_rad` | `JointVector` | rad | Measured joint position | Missing sample is a fault, not zeros | Length 7, finite |
| `ArmState.velocity_rad_s` | `JointVector` | rad/s | Measured joint velocity | Empty until the first velocity frame | Length 7 when present, finite |
| `ArmState.gripper_position` | `double` | m | AGX gripper opening | `0` is a real width, not a sentinel | Finite, range checked by the gripper profile |
| `ArmState.fault_code` | `uint32` | — | Vendor fault bits | `0` means no fault bit is set | Passthrough of the decoded code |
| `ArmState.monotonic_time_ns` | `int64` | ns | Sample time on `CLOCK_MONOTONIC` | `0` means the sample was never stamped | Must be positive after the first sample |
| `DualArmState.sequence` | `uint64` | — | Paired-state counter | `0` before the first pair | Increases by one per published pair |
| `DualArmState.left` / `right` | `ArmState` | — | One arm | Missing arm marks the pair invalid | Both arms fresh |
| `DualArmState.synchronized_time_ns` | `int64` | ns | Pair time | `0` if the pair is invalid | Monotonic |
| `DualArmState.left_right_skew_ns` | `int64` | ns | Right sample time minus left sample time | `0` when both stamps match | Absolute value within the configured skew |
| `ActionStep.left_position_rad` | `JointVector` | rad | Left position target | Invalid if empty | Length 7, finite, inside software limits |
| `ActionStep.right_position_rad` | `JointVector` | rad | Right position target | Invalid if empty | Length 7, finite, inside software limits |
| `ActionStep.left_gripper_position` | `double` | m | Left gripper target | No implicit previous value | Finite, inside gripper range |
| `ActionStep.right_gripper_position` | `double` | m | Right gripper target | No implicit previous value | Finite, inside gripper range |
| `ActionStep.execute_at_ns` | `int64` | ns | Desired monotonic execution time | `0` is invalid | Not in the past beyond the configured lateness |
| `ActionChunk.sequence` | `uint64` | — | Client chunk id | `0` is allowed once | Strictly increasing per client |
| `ActionChunk.valid_until_ns` | `int64` | ns | Chunk expiry | `0` rejects the chunk | Greater than the server's monotonic now at admission |
| `ActionChunk.steps` | `repeated ActionStep` | — | Future targets | Empty chunk is rejected | Each step passes the `ActionStep` rules |

Protobuf `repeated` does not enforce length 7. The server checks the length after decode.

## Schema

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

## Examples

A valid position step uses seven finite joint targets in radians and gripper widths in meters.

```text
ActionChunk sequence=1 valid_until_ns=<future monotonic ns>
  ActionStep execute_at_ns=<future monotonic ns>
    left_position_rad  = [0, 0, 0, 0, 0, 0, 0]
    right_position_rad = [0, 0, 0, 0, 0, 0, 0]
    left_gripper_position = 0.05
    right_gripper_position = 0.05
```

Success returns the chunk the server kept after limiting, not the raw request. LeRobot must store that accepted chunk.

A rejection uses a stable `event_code` and does not move the arms.

| Condition | `event_code` |
| --- | --- |
| Wrong joint length, NaN, or infinity | `ipc.rejected.invalid_argument` |
| Sequence not increasing, or chunk expired | `ipc.rejected.stale` |
| Second motion client | `ipc.rejected.busy` |
| Firmware profile mismatch or bus fault | `ipc.rejected.unavailable` |
| Safety limit or emergency stop | `ipc.rejected.safety` |

## Compatibility

Compatible evolution adds optional fields with new numbers and documented defaults. Readers accept older messages that omit the new fields. Renaming or reusing a field number is a breaking change and needs a new package identity plus a migration note. The product version and this `dual_nero` package version move independently.

## Python client shape

The LeRobot adapter stays thin. `get_observation` maps the accepted C++ state into LeRobot keys. `send_action` returns the action the server applied. The C++ program `capture_d435` produces the RealSense D435 frame stored later under `global`. `capture_dc1` reads the first connected Orbbec Dabai DC1. `left_wrist` and `right_wrist` are the later names for the two wrist cameras and are not assigned yet. The first teapot-grasp episodes store one arm, that wrist image, and the global image. The dataset stores `observation.state` as the measured arm state, `action` as the teach-pendant or model target, the camera images, and `task` as the language instruction. The stored action is the limited command returned by C++, not only the original Python request.

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
