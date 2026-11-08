#include "dual_nero/camera_frame.hpp"

#include <gtest/gtest.h>

TEST(CameraFrame, test_empty_frame_is_inconsistent) {
  const dual_nero::CameraFrame frame;
  EXPECT_FALSE(dual_nero::CameraFrameIsConsistent(frame));
}

TEST(CameraFrame, test_matching_color_and_depth_is_consistent) {
  dual_nero::CameraFrame frame;
  frame.device_name = "D435";
  frame.monotonic_time_ns = 1;
  frame.width = 2;
  frame.height = 2;
  frame.color_rgb.assign(12, 0);
  frame.depth_mm.assign(4, 0);
  EXPECT_TRUE(dual_nero::CameraFrameIsConsistent(frame));
}

TEST(CameraFrame, test_short_depth_is_inconsistent) {
  dual_nero::CameraFrame frame;
  frame.device_name = "D435";
  frame.monotonic_time_ns = 1;
  frame.width = 2;
  frame.height = 2;
  frame.color_rgb.assign(12, 0);
  frame.depth_mm.assign(3, 0);
  EXPECT_FALSE(dual_nero::CameraFrameIsConsistent(frame));
}
