#include "dual_nero/environment/realsense_d435.hpp"

#include <librealsense2/rs.hpp>

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace dual_nero {
namespace {

std::int64_t MonotonicTimeNs() {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

bool IsD435(const rs2::device& device) {
  if (!device.supports(RS2_CAMERA_INFO_NAME)) {
    return false;
  }
  const std::string name = device.get_info(RS2_CAMERA_INFO_NAME);
  return name.find("D435") != std::string::npos;
}

}  // namespace

struct RealSenseD435::Impl {
  rs2::pipeline pipe;
  rs2::align align{RS2_STREAM_COLOR};
  bool started{false};
  std::string device_name;
};

RealSenseD435::RealSenseD435() : impl_(std::make_unique<Impl>()) {}

RealSenseD435::~RealSenseD435() { close(); }

void RealSenseD435::open() {
  if (impl_->started) {
    throw std::logic_error("RealSense D435 is already open");
  }

  rs2::context context;
  std::string serial;
  std::string name;
  for (rs2::device device : context.query_devices()) {
    if (!IsD435(device) || !device.supports(RS2_CAMERA_INFO_SERIAL_NUMBER)) {
      continue;
    }
    serial = device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
    name = device.get_info(RS2_CAMERA_INFO_NAME);
    break;
  }
  if (serial.empty()) {
    throw std::runtime_error("no RealSense D435 is connected");
  }

  rs2::config config;
  config.enable_device(serial);
  config.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_RGB8, 30);
  config.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
  try {
    impl_->pipe.start(config);
  } catch (const rs2::error& error) {
    throw std::runtime_error(error.what());
  }
  impl_->device_name = name + " " + serial;
  impl_->started = true;
}

void RealSenseD435::close() noexcept {
  if (!impl_ || !impl_->started) {
    return;
  }
  try {
    impl_->pipe.stop();
  } catch (...) {
  }
  impl_->started = false;
}

bool RealSenseD435::is_open() const noexcept { return impl_ && impl_->started; }

CameraFrame RealSenseD435::capture(std::chrono::milliseconds timeout) {
  if (!is_open()) {
    throw std::logic_error("RealSense D435 is closed");
  }
  if (timeout.count() <= 0) {
    throw std::invalid_argument("RealSense capture timeout must be positive");
  }

  rs2::frameset frames;
  try {
    frames = impl_->pipe.wait_for_frames(static_cast<unsigned int>(timeout.count()));
    frames = impl_->align.process(frames);
  } catch (const rs2::error& error) {
    throw std::runtime_error(error.what());
  }

  const rs2::video_frame color = frames.get_color_frame();
  const rs2::depth_frame depth = frames.get_depth_frame();
  if (!color || !depth) {
    throw std::runtime_error("RealSense D435 frame is missing color or depth");
  }

  CameraFrame frame;
  frame.device_name = impl_->device_name;
  frame.monotonic_time_ns = MonotonicTimeNs();
  frame.width = color.get_width();
  frame.height = color.get_height();
  if (depth.get_width() != frame.width || depth.get_height() != frame.height) {
    throw std::runtime_error("RealSense D435 color and depth sizes differ");
  }

  const std::size_t pixels = static_cast<std::size_t>(frame.width) * static_cast<std::size_t>(frame.height);
  const auto* color_bytes = static_cast<const std::uint8_t*>(color.get_data());
  const auto* depth_values = static_cast<const std::uint16_t*>(depth.get_data());
  frame.color_rgb.assign(color_bytes, color_bytes + pixels * 3);
  frame.depth_mm.assign(depth_values, depth_values + pixels);
  return frame;
}

}  // namespace dual_nero
