# 日志

[English](../../docs/debug/logging.md) | [架构](../design/architecture_cn.md)

**目标设计，尚未实现。** 现在没有运行目录，也没有就绪事件。第一版实现必须先发出一条启动或就绪记录，然后才记录其他状态变化。清单是[路线图](../maintenance/roadmap_cn.md)。

## 一次运行要记下什么

每条运行记录包含 git commit、构建类型、两台 Nero 的固件版本、`can_left` 和 `can_right`，以及配置文件哈希。它保留原始反馈时间戳和配对后的双臂时间戳。它保留模型原始动作、安全限幅后的动作，以及实际发出的 CAN 命令。故障码、超时和急停事件属于同一次记录。

## 记录如何写出

控制日志使用有界异步队列。队列满时丢弃或合并日志，不用磁盘写入阻塞 CAN 线程。部署方选择文件名、轮转方式，以及读者在重启后如何继续读取。C++ 进程仍然是生产者。

机器可读记录是 UTF-8 JSON Lines。每个对象包含 RFC3339 UTC 的 `timestamp`，`level`（`trace`、`debug`、`info`、`warn` 或 `error`），`source`，所属服务名，稳定的 `event_code`，存在运行时的 `run_id`，给人读的 `message`，以及有界的 `fields`。`node_path` 和 `correlation_id` 可选。日志不包含密钥、相机画面、提示词或无界的外部载荷。

读者从当前这次运行开始，查找第一条就绪事件或第一条失败事件。服务真正创建路径之后，后续页面才写具体路径。
