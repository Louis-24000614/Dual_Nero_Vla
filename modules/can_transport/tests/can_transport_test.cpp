#include "dual_nero/can_transport.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>
#include <string>

TEST(CanTransport, test_receive_when_closed_throws) {
  dual_nero::CanTransport transport("can0");
  EXPECT_FALSE(transport.is_open());
  EXPECT_THROW(transport.receive(std::chrono::milliseconds(1)), std::logic_error);
}

TEST(CanTransport, test_send_when_closed_throws) {
  dual_nero::CanTransport transport("can0");
  can_frame frame{};
  EXPECT_THROW(transport.send(frame), std::logic_error);
}

TEST(CanTransport, test_close_when_never_opened_is_noop) {
  dual_nero::CanTransport transport("can0");
  transport.close();
  transport.close();
  EXPECT_FALSE(transport.is_open());
}

TEST(CanTransport, test_open_when_interface_missing_throws) {
  dual_nero::CanTransport transport("nosuchcan0");
  EXPECT_THROW(transport.open(), std::system_error);
  EXPECT_FALSE(transport.is_open());
}

TEST(CanTransport, test_open_when_name_empty_throws) {
  dual_nero::CanTransport transport("");
  EXPECT_THROW(transport.open(), std::invalid_argument);
  EXPECT_FALSE(transport.is_open());
}
