#include "dual_nero/environment/realsense_d435.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>

TEST(RealSenseD435, test_capture_when_closed_throws) {
  dual_nero::RealSenseD435 camera;
  EXPECT_FALSE(camera.is_open());
  EXPECT_THROW(camera.capture(std::chrono::milliseconds(1)), std::logic_error);
}

TEST(RealSenseD435, test_close_when_never_opened_is_noop) {
  dual_nero::RealSenseD435 camera;
  camera.close();
  camera.close();
  EXPECT_FALSE(camera.is_open());
}
