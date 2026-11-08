#include "dual_nero/camera_frame.hpp"
#include "dual_nero/environment/realsense_d435.hpp"

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void WriteColorPpm(const std::string& path, const dual_nero::CameraFrame& frame) {
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    throw std::runtime_error("cannot write " + path);
  }
  out << "P6\n" << frame.width << " " << frame.height << "\n255\n";
  out.write(reinterpret_cast<const char*>(frame.color_rgb.data()),
            static_cast<std::streamsize>(frame.color_rgb.size()));
}

void WriteDepthPgm(const std::string& path, const dual_nero::CameraFrame& frame) {
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    throw std::runtime_error("cannot write " + path);
  }
  out << "P5\n" << frame.width << " " << frame.height << "\n65535\n";
  for (const std::uint16_t sample : frame.depth_mm) {
    const unsigned char bytes[2] = {static_cast<unsigned char>(sample >> 8),
                                    static_cast<unsigned char>(sample & 0xff)};
    out.write(reinterpret_cast<const char*>(bytes), 2);
  }
}

}  // namespace

// Saves one aligned color and depth frame from a D435. This program does not open CAN.
int main(int argc, char** argv) {
  const std::string directory = argc > 1 ? argv[1] : ".";
  dual_nero::RealSenseD435 camera;
  try {
    camera.open();
    const dual_nero::CameraFrame frame = camera.capture(std::chrono::milliseconds(5000));
    if (!dual_nero::CameraFrameIsConsistent(frame)) {
      throw std::runtime_error("captured D435 frame is inconsistent");
    }
    WriteColorPpm(directory + "/d435_color.ppm", frame);
    WriteDepthPgm(directory + "/d435_depth.pgm", frame);
    std::cout << frame.device_name << " " << frame.width << "x" << frame.height << " "
              << frame.monotonic_time_ns << "\n";
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << "\n";
    return 1;
  }
  return 0;
}
