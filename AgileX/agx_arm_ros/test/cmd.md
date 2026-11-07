## Nero Command Test

```bash
ros2 topic pub /control/move_j sensor_msgs/msg/JointState "$(cat test/nero/test_move_j.yaml)" -1
```

```bash
ros2 topic pub /control/move_p geometry_msgs/msg/PoseStamped "$(cat test/nero/test_move_p.yaml)" -1
```

## Gripper Control

```bash
# Gripper control (width: 0.05m, force: 1.0N)
ros2 topic pub /control/joint_states sensor_msgs/msg/JointState "$(cat test/gripper/test_gripper_joint_states.yaml)" -1
```

```bash
# Arm + Gripper combo control
ros2 topic pub /control/joint_states sensor_msgs/msg/JointState "$(cat test/gripper/test_arm_gripper_joint_states.yaml)" -1
```

```bash
# Arm + Hand combo control
ros2 topic pub /control/joint_states sensor_msgs/msg/JointState "$(cat test/hand/test_arm_hand_joint_states.yaml)" -1
```
