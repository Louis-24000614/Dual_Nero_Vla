import ast

from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from moveit_configs_utils import MoveItConfigsBuilder

ALL_ARM_TYPES = ["nero"]
ALL_EFFECTOR_TYPES = ["none", "agx_gripper"]


def declare_common_args():
    return [
        DeclareLaunchArgument(
            "namespace",
            default_value="",
            description="ROS namespace for this arm instance (e.g. arm1).",
        ),
        DeclareLaunchArgument(
            "arm_type", default_value="nero",
            choices=ALL_ARM_TYPES, description="Arm type.",
        ),
        DeclareLaunchArgument(
            "effector_type", default_value="none",
            choices=ALL_EFFECTOR_TYPES, description="Effector type.",
        ),
        DeclareLaunchArgument(
            "tcp_offset",
            default_value="[0.0, 0.0, 0.0, 0.0, 0.0, 0.0]",
            description="TCP offset [x, y, z, rx, ry, rz] in meters/radians.",
        ),
        DeclareLaunchArgument(
            "follow",
            default_value="false",
            choices=["true", "false"],
            description="Follow real arm state. "
            "true: move_group subscribes to feedback_topic; "
            "false: subscribes to control_topic (mock hardware).",
        ),
        DeclareLaunchArgument(
            "feedback_topic",
            default_value="feedback/joint_states",
            description="Joint states feedback topic (used when follow:=true).",
        ),
        DeclareLaunchArgument(
            "control_topic",
            default_value="control/joint_states",
            description="Joint states control topic (used when follow:=false, and for ros2_control_node).",
        ),
    ]


def _select_profile(effector_type: str) -> str:
    if effector_type == "agx_gripper":
        return "gripper"
    return "none"


def build_moveit_config(context):
    arm_type = LaunchConfiguration("arm_type").perform(context)
    effector_type = LaunchConfiguration("effector_type").perform(context)
    tcp_offset = ast.literal_eval(
        LaunchConfiguration("tcp_offset").perform(context)
    )

    profile = _select_profile(effector_type)
    urdf_mappings = {
        "arm_type": arm_type,
        "effector_type": effector_type,
        "tcp_offset_xyz": f"{tcp_offset[0]} {tcp_offset[1]} {tcp_offset[2]}",
        "tcp_offset_rpy": f"{tcp_offset[3]} {tcp_offset[4]} {tcp_offset[5]}",
    }
    srdf_mappings = {
        "arm_type": arm_type,
        "effector_type": effector_type,
    }

    moveit_config = (
        MoveItConfigsBuilder("agx_arm", package_name="agx_arm_moveit")
        .robot_description(file_path="config/agx_arm.urdf.xacro", mappings=urdf_mappings)
        .robot_description_semantic(
            file_path="config/agx_arm.srdf.xacro", mappings=srdf_mappings
        )
        .robot_description_kinematics(file_path="config/kinematics.yaml")
        .joint_limits(file_path="config/joint_limits.yaml")
        .sensors_3d(file_path="config/sensors_3d.yaml")
        .trajectory_execution(file_path=f"config/moveit_controllers_{profile}.yaml")
        .to_moveit_configs()
    )

    if arm_type == "nero":
        moveit_config.trajectory_execution[
            "moveit_simple_controller_manager"
        ]["arm_controller"]["joints"] = [
            "joint1", "joint2", "joint3", "joint4",
            "joint5", "joint6", "joint7",
        ]

    return moveit_config
