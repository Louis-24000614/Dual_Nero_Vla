# Roadmap

[中文](../../docs_cn/maintenance/roadmap_cn.md) | [Architecture](../design/architecture.md)

The runtime is **Target design — not implemented.** Items below are the executable checklist. Checking an item requires the verification stated in that phase, not a document edit alone.

## Phase A — read-only driver

- [ ] Pin `can_left` and `can_right`.
- [x] Implement `CanTransport`.
- [x] Record raw CAN frames with `record_can`.
- [ ] Identify the firmware profile.
- [ ] Parse joint feedback for both arms.
- [ ] Compare each parsed field with `pyAgxArm`.

Acceptance: 30 minutes without a disconnect, with no unexplained frame, and with parsed state matching the official SDK.

## Phase B — one-arm safe motion

- [ ] Implement enable and disable.
- [ ] Implement position commands.
- [ ] Implement software limits.
- [ ] Implement the watchdog.
- [ ] Implement emergency stop.
- [ ] Run a small, slow motion on one real arm.

Acceptance: dropping the client, stopping the heartbeat, or unplugging CAN enters the documented safe state.

## Phase C — dual-arm service

- [ ] Receive each bus on its own thread.
- [ ] Synchronize the two arm states.
- [ ] Submit a dual-arm action atomically.
- [ ] Apply the basic workspace box.
- [ ] Serve the [IPC contract](../interfaces/ipc.md).

Acceptance: a stale sample on one arm stops advance for both arms.

## Phase D — demonstration capture

- [ ] Read both teach pendants in the C++ process.
- [ ] Capture the global RealSense D435 and the wrist Orbbec Dabai DC1. The first episodes use one arm and its wrist camera.
- [ ] Align timestamps.
- [ ] Implement the LeRobot `Robot` adapter.
- [ ] Implement the LeRobot `Teleoperator` adapter.
- [ ] Record, replay, and inspect 10 episodes.

Acceptance: replay preserves the order of both arms, both grippers, and the image events without a visible time shift.

## Phase E — policy deployment

- [ ] Train an ACT baseline.
- [ ] Buffer action chunks.
- [ ] Roll closed-loop predictions.
- [ ] Measure on-robot success.
- [ ] Connect SmolVLA.
- [ ] Later, connect OpenVLA-OFT.

## Task milestones

These milestones sit on phases A–E. The first dataset and the first policy use one arm.

- [ ] Milestone 1: one Nero grasps a teapot. Observations are that arm's joints, its wrist Dabai DC1, and the global RealSense D435.
- [ ] Milestone 2: the two arms grasp separate objects. Each wrist uses a Dabai DC1. The global camera remains the D435.
- [ ] Milestone 3: the two arms pour tea. This waits until milestone 2 is reliable.

## Non-goals for the first version

MIT torque mode stays closed. Unverified high-speed CPV stays closed. A firmware mismatch is not bypassed. Multiple clients do not control the arms at once. A model does not send raw CAN frames. A software button is not the emergency stop. The project does not claim a real-time class or a safety rating before the hardware phases pass.

## Current verification

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected result today: configuration succeeds, `can_transport_test` passes five closed-socket and missing-interface cases, and `record_can` is built. Those tests do not open a live bus.
