#include "dual_nero/wrist/dabai_dc1.hpp"

#include <libobsensor/ObSensor.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
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

std::string OrbbecMessage(const ob::Error& error) {
  const char* message = error.getMessage();
  return message == nullptr ? "Orbbec SDK error" : message;
}

bool IsDc1Name(const char* raw_name) {
  if (raw_name == nullptr) {
    return false;
  }
  const std::string name(raw_name);
  return name.find("DC1") != std::string::npos || name.find("dc1") != std::string::npos;
}

std::uint16_t DepthSampleToMillimeters(std::uint16_t sample, float scale) {
  const float millimeters = static_cast<float>(sample) * scale;
  if (!std::isfinite(millimeters) || millimeters <= 0.f) {
    return 0;
  }
  const long rounded = std::lround(millimeters);
  return static_cast<std::uint16_t>(std::min(rounded, 65535L));
}

}  // namespace

struct DabaiDc1::Impl {
  std::shared_ptr<ob::Pipeline> pipe;
  bool started{false};
  std::string device_name;
};

DabaiDc1::DabaiDc1() : impl_(std::make_unique<Impl>()) {}

DabaiDc1::~DabaiDc1() { close(); }

void DabaiDc1::open() {
  if (impl_->started) {
    throw std::logic_error("Dabai DC1 is already open");
  }

  ob::Context context;
  const std::shared_ptr<ob::DeviceList> devices = context.queryDeviceList();
  if (!devices || devices->deviceCount() == 0) {
    throw std::runtime_error("no Orbbec Dabai DC1 is connected");
  }

  int found = -1;
  std::string name;
  std::string serial;
  for (std::uint32_t index = 0; index < devices->deviceCount(); ++index) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    const char* raw_name = devices->name(index);
#pragma GCC diagnostic pop
    if (!IsDc1Name(raw_name)) {
      continue;
    }
    const char* raw_serial = devices->serialNumber(index);
    name = raw_name;
    serial = raw_serial == nullptr ? "" : raw_serial;
    found = static_cast<int>(index);
    break;
  }
  if (found < 0) {
    throw std::runtime_error("no Orbbec Dabai DC1 is connected");
  }

  try {
    const std::shared_ptr<ob::Device> device =
        devices->getDevice(static_cast<std::uint32_t>(found));
    auto config = std::make_shared<ob::Config>();
    config->enableVideoStream(OB_STREAM_COLOR, 640, 480, 30, OB_FORMAT_RGB);
    config->enableVideoStream(OB_STREAM_DEPTH, 640, 480, 30, OB_FORMAT_Y16);
    config->setAlignMode(ALIGN_D2C_SW_MODE);
    impl_->pipe = std::make_shared<ob::Pipeline>(device);
    impl_->pipe->start(config);
  } catch (const ob::Error& error) {
    impl_->pipe.reset();
    throw std::runtime_error(OrbbecMessage(error));
  }
  impl_->device_name = serial.empty() ? name : name + " " + serial;
  impl_->started = true;
}

void DabaiDc1::close() noexcept {
  if (!impl_ || !impl_->started) {
    return;
  }
  try {
    impl_->pipe->stop();
  } catch (...) {
  }
  impl_->started = false;
}

bool DabaiDc1::is_open() const noexcept { return impl_ && impl_->started; }

CameraFrame DabaiDc1::capture(std::chrono::milliseconds timeout) {
  if (!is_open()) {
    throw std::logic_error("Dabai DC1 is closed");
  }
  if (timeout.count() <= 0) {
    throw std::invalid_argument("Dabai DC1 capture timeout must be positive");
  }

  std::shared_ptr<ob::FrameSet> frames;
  try {
    frames = impl_->pipe->waitForFrames(static_cast<std::uint32_t>(timeout.count()));
  } catch (const ob::Error& error) {
    throw std::runtime_error(OrbbecMessage(error));
  }
  if (!frames) {
    throw std::runtime_error("Dabai DC1 frame timed out");
  }

  const std::shared_ptr<ob::ColorFrame> color = frames->colorFrame();
  const std::shared_ptr<ob::DepthFrame> depth = frames->depthFrame();
  if (!color || !depth) {
    throw std::runtime_error("Dabai DC1 frame is missing color or depth");
  }
  if (color->format() != OB_FORMAT_RGB || depth->format() != OB_FORMAT_Y16) {
    throw std::runtime_error("Dabai DC1 frame format is not RGB and Y16");
  }

  CameraFrame frame;
  frame.device_name = impl_->device_name;
  frame.monotonic_time_ns = MonotonicTimeNs();
  frame.width = static_cast<int>(color->width());
  frame.height = static_cast<int>(color->height());
  if (depth->width() != color->width() || depth->height() != color->height()) {
    throw std::runtime_error("Dabai DC1 color and depth sizes differ");
  }

  const std::size_t pixels =
      static_cast<std::size_t>(frame.width) * static_cast<std::size_t>(frame.height);
  if (color->dataSize() != pixels * 3 || depth->dataSize() != pixels * sizeof(std::uint16_t)) {
    throw std::runtime_error("Dabai DC1 frame size does not match its resolution");
  }
  const float scale = depth->getValueScale();
  if (!std::isfinite(scale) || scale <= 0.f) {
    throw std::runtime_error("Dabai DC1 depth scale is invalid");
  }

  const auto* color_bytes = static_cast<const std::uint8_t*>(color->data());
  const auto* depth_values = static_cast<const std::uint16_t*>(depth->data());
  frame.color_rgb.assign(color_bytes, color_bytes + pixels * 3);
  frame.depth_mm.resize(pixels);
  for (std::size_t i = 0; i < pixels; ++i) {
    frame.depth_mm[i] = DepthSampleToMillimeters(depth_values[i], scale);
  }
  return frame;
}

}  // namespace dual_nero
