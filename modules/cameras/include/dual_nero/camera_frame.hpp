#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dual_nero {

// One color image and one depth image from one camera.
// The environment D435 reader and the wrist DC1 readers both fill this struct.
// color_rgb is RGB8, row-major, size width * height * 3.
// depth_mm is depth in millimeters, row-major, size width * height.
// monotonic_time_ns is stamped when the frame is copied out of the camera SDK.
struct CameraFrame {
  std::string device_name;
  std::int64_t monotonic_time_ns{0};
  int width{0};
  int height{0};
  std::vector<std::uint8_t> color_rgb;
  std::vector<std::uint16_t> depth_mm;
};

inline bool CameraFrameIsConsistent(const CameraFrame& frame) {
  if (frame.width <= 0 || frame.height <= 0 || frame.device_name.empty()) {
    return false;
  }
  const std::size_t pixels = static_cast<std::size_t>(frame.width) * static_cast<std::size_t>(frame.height);
  return frame.color_rgb.size() == pixels * 3 && frame.depth_mm.size() == pixels && frame.monotonic_time_ns > 0;
}

}  // namespace dual_nero
