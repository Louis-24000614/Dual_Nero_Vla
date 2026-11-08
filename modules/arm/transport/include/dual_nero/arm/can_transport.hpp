#pragma once

#include <linux/can.h>

#include <chrono>
#include <optional>
#include <string>

namespace dual_nero {

// Arm transport. Owns one classic CAN socket for a single SocketCAN interface.
// This object does not decode Nero fields and does not open a camera.
// The caller brings the interface up and sets the bitrate before open().
// open() blocks only while the socket is created and bound. On failure the
// transport stays closed and throws std::system_error.
// receive() blocks until one raw frame arrives or the timeout expires.
// send() and receive() throw std::logic_error when the transport is closed.
class CanTransport {
 public:
  explicit CanTransport(std::string interface_name);
  ~CanTransport();

  CanTransport(const CanTransport&) = delete;
  CanTransport& operator=(const CanTransport&) = delete;

  void open();
  void close() noexcept;
  bool is_open() const noexcept;

  void send(const can_frame& frame);
  std::optional<can_frame> receive(std::chrono::milliseconds timeout);

 private:
  std::string interface_name_;
  int socket_fd_{-1};
};

}  // namespace dual_nero
