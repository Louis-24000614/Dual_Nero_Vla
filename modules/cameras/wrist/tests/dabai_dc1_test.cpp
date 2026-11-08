#include "dual_nero/wrist/dabai_dc1.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>

TEST(DabaiDc1, test_capture_when_closed_throws) {
  dual_nero::DabaiDc1 camera;
  EXPECT_FALSE(camera.is_open());
  EXPECT_THROW(camera.capture(std::chrono::milliseconds(1)), std::logic_error);
}

TEST(DabaiDc1, test_close_when_never_opened_is_noop) {
  dual_nero::DabaiDc1 camera;
  camera.close();
  camera.close();
  EXPECT_FALSE(camera.is_open());
}
