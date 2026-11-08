#include "dual_nero/arm/can_transport.hpp"

#include <linux/can.h>

#include <chrono>
#include <cstdint>
#include <iostream>

namespace {

std::int64_t MonotonicTimeNs() {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

}  // namespace

// Read-only dump of raw SocketCAN frames. This program never sends a frame.
int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: record_can <interface>\n";
    return 2;
  }

  dual_nero::CanTransport transport(argv[1]);
  try {
    transport.open();
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }

  while (true) {
    const std::optional<can_frame> frame = transport.receive(std::chrono::milliseconds(1000));
    if (!frame.has_value()) {
      continue;
    }
    const bool is_error = (frame->can_id & CAN_ERR_FLAG) != 0;
    std::cout << MonotonicTimeNs() << ' ' << (is_error ? "error" : "data") << ' ' << std::hex
              << (frame->can_id & CAN_EFF_MASK) << std::dec << ' ' << static_cast<int>(frame->can_dlc);
    for (int i = 0; i < frame->can_dlc && i < CAN_MAX_DLEN; ++i) {
      std::cout << ' ' << std::hex << static_cast<int>(frame->data[i]) << std::dec;
    }
    std::cout << '\n';
  }
}
