#pragma once

#include "dual_nero/camera_frame.hpp"

#include <chrono>
#include <memory>
#include <string>

namespace dual_nero {

// Wrist camera. Owns one Orbbec pipeline for the first connected Dabai DC1.
// This object does not open SocketCAN and does not read the scene D435.
// open() blocks while the device is selected and the pipeline starts.
// capture() blocks until an aligned color and depth pair arrives or the timeout expires.
// A missing DC1 throws std::runtime_error and leaves the object closed.
// capture() before open() throws std::logic_error.
class DabaiDc1 {
 public:
  DabaiDc1();
  ~DabaiDc1();

  DabaiDc1(const DabaiDc1&) = delete;
  DabaiDc1& operator=(const DabaiDc1&) = delete;

  void open();
  void close() noexcept;
  bool is_open() const noexcept;

  CameraFrame capture(std::chrono::milliseconds timeout);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace dual_nero
