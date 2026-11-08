#pragma once

#include "dual_nero/camera_frame.hpp"

#include <chrono>
#include <memory>
#include <string>

namespace dual_nero {

// Environment camera. Owns one RealSense pipeline for the scene D435.
// This object does not open SocketCAN and does not read a wrist camera.
// open() blocks while the device is selected and the pipeline starts.
// capture() blocks until an aligned color and depth pair arrives or the timeout expires.
// A missing D435 throws std::runtime_error and leaves the object closed.
// capture() before open() throws std::logic_error.
class RealSenseD435 {
 public:
  RealSenseD435();
  ~RealSenseD435();

  RealSenseD435(const RealSenseD435&) = delete;
  RealSenseD435& operator=(const RealSenseD435&) = delete;

  void open();
  void close() noexcept;
  bool is_open() const noexcept;

  CameraFrame capture(std::chrono::milliseconds timeout);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace dual_nero
