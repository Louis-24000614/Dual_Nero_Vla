# pyAgxArm User Guide

> `pyAgxArm` is a Python SDK for the AgileX Nero arm and the two-finger AGX gripper. It supports CAN communication, status reading, motion control, and gripper control.

## Table of Contents

- [Switch to 中文](#pyagxarm-使用说明)
- [Introduction](#pyagxarm-user-guide)
- [Environment](#environment)
- [Documentation](#documentation)
- [Install](#install)
- [Communication Setup](#communication-setup)
- [Quick Start](#quick-start)
- [Notes](#notes)
- [Contact](#contact)


### Environment

- Ubuntu: `18.04 / 20.04 / 22.04 / 24.04`
- Python: `3.6` and above (compatible up to `3.14`)

### Documentation

| Topic | Link |
| --- | --- |
| GitHub Discussions | [All](https://github.com/agilexrobotics/pyAgxArm/discussions) · [Announcements](https://github.com/agilexrobotics/pyAgxArm/discussions/categories/announcements) · [General](https://github.com/agilexrobotics/pyAgxArm/discussions/categories/general) · [Q&A](https://github.com/agilexrobotics/pyAgxArm/discussions/categories/q-a) |
| ROS | [agx_arm_ros](https://github.com/agilexrobotics/agx_arm_ros) |
| Nero API | [docs/nero/nero_api.md](./docs/nero/nero_api.md#nero-api-documentation) |
| AgxGripper API | [docs/effector/agx_gripper/agx_gripper_api.md](./docs/effector/agx_gripper/agx_gripper_api.md#agxgripper-api-documentation) |
| CAN module manual | [docs/can_user.md](./docs/can_user.md#can-module-manual) |
| Nero first-time CAN guide | [docs/nero/first_time_user_guide_can.md](./docs/nero/first_time_user_guide_can.md#nero-first-time-user-guide-can) |
| Ubuntu 24.04 pip guide | [docs/ubuntu_24_04_pip_install.md](./docs/ubuntu_24_04_pip_install.md#ubuntu-2404-pip-installation-guide) |
| Changelog | [CHANGELOG.md](./CHANGELOG.md#changelog) |
| Demos | [pyAgxArm/demos](./pyAgxArm/demos) |

### Install

```shell
pip3 install python-can
```

`python-can` should be newer than `3.3.4`.

Then install `pyAgxArm`. The recommended way is to install directly from GitHub:

```shell
pip3 install "git+https://github.com/agilexrobotics/pyAgxArm.git"
```

You can also clone the repository and install it locally:

```shell
git clone https://github.com/agilexrobotics/pyAgxArm.git
cd pyAgxArm
pip3 install .
```

Ubuntu 24.04 users can also refer to:
[docs/ubuntu_24_04_pip_install.md](./docs/ubuntu_24_04_pip_install.md#ubuntu-2404-pip-installation-guide)

### Communication Setup

See:
[docs/can_user.md](./docs/can_user.md#can-module-manual)

### Quick Start

This quick-start example uses Linux SocketCAN: `interface="socketcan"`, `channel="can0"`.

Prerequisites before running:

- Activate CAN first (for example: `sudo ip link set can0 up type can bitrate 1000000`)
- You can also use the shell scripts in [CAN module manual - Activate a Single CAN Module](./docs/can_user.md#2-activate-a-single-can-module)

```python
import time
from pyAgxArm import create_agx_arm_config, AgxArmFactory, ArmModel, NeroFW

# Nero firmware: <= 1.10 → DEFAULT; 1.11 → V111; 1.12 → V112; 1.20 → V120; >= 1.21 → V121.
cfg = create_agx_arm_config(
    robot=ArmModel.NERO,
    firmeware_version=NeroFW.DEFAULT,
    interface="socketcan",
    channel="can0",
)
robot = AgxArmFactory.create_arm(cfg)
robot.connect()

while True:
    ja = robot.get_joint_angles()
    if ja is not None:
        print(ja.msg)
        print(ja.hz, ja.timestamp)
    time.sleep(0.005)
```

### Notes

- Activate CAN first and configure the correct bitrate before reading or controlling the arm.
- MIT single-joint control is an advanced feature; improper use may damage the robot.

### Contact

- [GitHub Discussions](https://github.com/agilexrobotics/pyAgxArm/discussions) (announcements, Q&A, ideas, and community discussion)
- [GitHub Issues](https://github.com/agilexrobotics/pyAgxArm/issues) (bugs, feature requests, documentation, and other tracked items)
- Email: [support@agilex.ai](mailto:support@agilex.ai) (firmware, host software, tool packages, and after-sales support)
- Discord: <https://discord.gg/wrKYTxwDBd> (real-time chat and community help)

---

# pyAgxArm 使用说明

> `pyAgxArm` 是 AgileX Nero 机械臂与两指 AGX 夹爪的 Python SDK，支持 CAN 通信、状态读取、运动控制和夹爪控制。

## 目录

- [切换到 English](#pyagxarm-user-guide)
- [简介](#pyagxarm-使用说明)
- [环境支持](#环境支持)
- [文档入口](#文档入口)
- [安装方法](#安装方法)
- [通信激活](#通信激活)
- [快速开始](#快速开始)
- [注意事项](#注意事项)
- [联系我们](#联系我们)

## 环境支持

- Ubuntu：`18.04 / 20.04 / 22.04 / 24.04`
- Python：`3.6` 及以上（目前适配至 `3.14`）

## 文档入口

| 说明 | 文档 |
| --- | --- |
| GitHub 讨论区 | [全部](https://github.com/agilexrobotics/pyAgxArm/discussions) · [公告](https://github.com/agilexrobotics/pyAgxArm/discussions/categories/announcements) · [综合](https://github.com/agilexrobotics/pyAgxArm/discussions/categories/general) · [问答](https://github.com/agilexrobotics/pyAgxArm/discussions/categories/q-a) |
| ROS | [agx_arm_ros](https://github.com/agilexrobotics/agx_arm_ros) |
| Nero API | [docs/nero/nero_api.md](./docs/nero/nero_api.md#nero-机械臂-api-使用文档) |
| AgxGripper API | [docs/effector/agx_gripper/agx_gripper_api.md](./docs/effector/agx_gripper/agx_gripper_api.md#agxgripper-夹爪-api-使用文档) |
| CAN 模块手册 | [docs/can_user.md](./docs/can_user.md#can-模块使用手册) |
| Nero 首次使用 CAN 指南 | [docs/nero/first_time_user_guide_can.md](./docs/nero/first_time_user_guide_can.md#nero-首次使用指南can) |
| Ubuntu 24.04 pip 安装说明 | [docs/ubuntu_24_04_pip_install.md](./docs/ubuntu_24_04_pip_install.md#ubuntu-2404-安装第三方-pip-包的方法) |
| 更新日志 | [CHANGELOG.md](./CHANGELOG.md#更新日志) |
| 示例代码 | [pyAgxArm/demos](./pyAgxArm/demos) |

## 安装方法

```shell
pip3 install python-can
```

`python-can` 版本应高于 `3.3.4`。

然后再安装 `pyAgxArm`。推荐直接通过 GitHub 安装：

```shell
pip3 install "git+https://github.com/agilexrobotics/pyAgxArm.git"
```

也可以克隆源码后在本地安装：

```shell
git clone https://github.com/agilexrobotics/pyAgxArm.git
cd pyAgxArm
pip3 install .
```

Ubuntu 24.04 可参考：
[docs/ubuntu_24_04_pip_install.md](./docs/ubuntu_24_04_pip_install.md#ubuntu-2404-安装第三方-pip-包的方法)

## 通信激活

详见：
[docs/can_user.md](./docs/can_user.md#can-模块使用手册)

## 快速开始

本节快速示例使用 Linux SocketCAN：`interface="socketcan"`，`channel="can0"`。

运行前前置条件：

- 先激活 CAN（例如：`sudo ip link set can0 up type can bitrate 1000000`）
- 也可使用脚本，见 [CAN 模块手册 - 激活单个 CAN 模块](./docs/can_user.md#2-激活单个-can-模块)

```python
import time
from pyAgxArm import create_agx_arm_config, AgxArmFactory, ArmModel, NeroFW

# Nero 固件：≤ 1.10 选 DEFAULT；1.11 选 V111；1.12 选 V112；1.20 选 V120；≥ 1.21 选 V121。
cfg = create_agx_arm_config(
    robot=ArmModel.NERO,
    firmeware_version=NeroFW.DEFAULT,
    interface="socketcan",
    channel="can0",
)
robot = AgxArmFactory.create_arm(cfg)
robot.connect()

while True:
    ja = robot.get_joint_angles()
    if ja is not None:
        print(ja.msg)
        print(ja.hz, ja.timestamp)
    time.sleep(0.005)
```

## 注意事项

- 使用 CAN 协议时，需要先激活 CAN 设备并设置正确波特率。
- MIT 单关节控制属于高级功能，使用不当可能损坏机械臂。

## 联系我们

- [GitHub 讨论区](https://github.com/agilexrobotics/pyAgxArm/discussions)（公告、问答、想法交流与社区讨论）
- [GitHub Issues](https://github.com/agilexrobotics/pyAgxArm/issues)（缺陷、功能建议、文档等问题反馈与跟踪）
- 邮箱：[support@agilex.ai](mailto:support@agilex.ai)（固件、上位机、工具包及售后支持等）
- Discord：<https://discord.gg/wrKYTxwDBd>（实时聊天与社区互助）
