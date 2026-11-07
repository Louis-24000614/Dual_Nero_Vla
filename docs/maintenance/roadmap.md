# Roadmap

[中文](../../docs_cn/maintenance/roadmap_cn.md) | [Architecture](../design/architecture.md)

The runtime is **Target design — not implemented.** Items below are the executable checklist. Checking an item requires the verification stated in that phase, not a document edit alone.

## Phase A — read-only driver

- [ ] Pin `can_left` and `can_right`.
- [ ] Implement `CanTransport`.
- [ ] Record raw CAN frames.
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
- [ ] Capture the cameras.
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

## Non-goals for the first version

MIT torque mode stays closed. Unverified high-speed CPV stays closed. A firmware mismatch is not bypassed. Multiple clients do not control the arms at once. A model does not send raw CAN frames. A software button is not the emergency stop. The project does not claim a real-time class or a safety rating before the hardware phases pass.

## Current verification

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
ctest --test-dir build --output-on-failure
```

Expected result today: configuration succeeds, the build has no targets, and ctest finds no tests.
