# Logging

[中文](../../docs_cn/debug/logging_cn.md) | [Architecture](../design/architecture.md)

**Target design — not implemented.** There is no run directory and no ready event yet. The first implementation has to emit one startup or ready record before any other state change. The checklist is the [roadmap](../maintenance/roadmap.md).

## What a run record contains

Each run record names the git commit, the build type, both Nero firmware versions, `can_left` and `can_right`, and the configuration-file hash. It keeps raw feedback timestamps and the paired dual-arm timestamp. It keeps the raw model action, the action after the safety limits, and the CAN command that was actually sent. Fault codes, timeouts, and emergency-stop events are part of the same record.

## How records are written

Control logs use a bounded asynchronous queue. A full queue drops or coalesces log records. It does not block a CAN thread on disk. The deployment owner chooses the file name, rotation, and how a reader resumes after a restart. The C++ process remains the producer.

Machine-readable records are UTF-8 JSON Lines. Each object has an RFC3339 UTC `timestamp`, a `level` of `trace`, `debug`, `info`, `warn`, or `error`, a `source`, the owning service name, a stable `event_code`, a `run_id` when a run exists, a human `message`, and bounded `fields`. `node_path` and `correlation_id` are optional. The log does not contain secrets, camera frames, prompts, or unbounded provider payloads.

A reader starts at the current run and looks for the first ready event or the first failure event. Later pages can name the path only after the server creates it.
