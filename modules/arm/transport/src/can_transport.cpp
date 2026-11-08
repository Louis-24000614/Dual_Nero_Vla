#include "dual_nero/arm/can_transport.hpp"

#include <linux/can/error.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <stdexcept>
#include <system_error>

namespace dual_nero {
namespace {

[[noreturn]] void ThrowErrno(int err, const char* what) {
  throw std::system_error(err, std::generic_category(), what);
}

}  // namespace

CanTransport::CanTransport(std::string interface_name) : interface_name_(std::move(interface_name)) {}

CanTransport::~CanTransport() { close(); }

void CanTransport::open() {
  if (socket_fd_ >= 0) {
    throw std::logic_error("CAN transport is already open");
  }
  if (interface_name_.empty() || interface_name_.size() >= IFNAMSIZ) {
    throw std::invalid_argument("CAN interface name is empty or does not fit IFNAMSIZ");
  }

  const int fd = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fd < 0) {
    ThrowErrno(errno, "socket");
  }

  try {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
      ThrowErrno(errno, "fcntl");
    }

    const int recv_own = 0;
    const int loopback = 0;
    const int err_mask = CAN_ERR_MASK;
    if (::setsockopt(fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own, sizeof(recv_own)) < 0 ||
        ::setsockopt(fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback)) < 0 ||
        ::setsockopt(fd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask)) < 0) {
      ThrowErrno(errno, "setsockopt");
    }

    ifreq ifr{};
    std::strncpy(ifr.ifr_name, interface_name_.c_str(), IFNAMSIZ - 1);
    if (::ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
      ThrowErrno(errno, "SIOCGIFINDEX");
    }
    const int ifindex = ifr.ifr_ifindex;
    if (::ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
      ThrowErrno(errno, "SIOCGIFFLAGS");
    }
    if ((ifr.ifr_flags & IFF_UP) == 0) {
      ThrowErrno(ENETDOWN, "CAN interface is down");
    }

    sockaddr_can addr{};
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifindex;
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
      ThrowErrno(errno, "bind");
    }
  } catch (...) {
    ::close(fd);
    throw;
  }

  socket_fd_ = fd;
}

void CanTransport::close() noexcept {
  if (socket_fd_ < 0) {
    return;
  }
  ::close(socket_fd_);
  socket_fd_ = -1;
}

bool CanTransport::is_open() const noexcept { return socket_fd_ >= 0; }

void CanTransport::send(const can_frame& frame) {
  if (socket_fd_ < 0) {
    throw std::logic_error("CAN transport is closed");
  }
  if (frame.can_dlc > CAN_MAX_DLEN) {
    throw std::invalid_argument("CAN frame dlc is greater than 8");
  }
  const ssize_t written = ::write(socket_fd_, &frame, sizeof(frame));
  if (written < 0) {
    ThrowErrno(errno, "write");
  }
  if (static_cast<std::size_t>(written) != sizeof(frame)) {
    ThrowErrno(EIO, "short CAN write");
  }
}

std::optional<can_frame> CanTransport::receive(std::chrono::milliseconds timeout) {
  if (socket_fd_ < 0) {
    throw std::logic_error("CAN transport is closed");
  }
  if (timeout.count() < 0) {
    throw std::invalid_argument("CAN receive timeout is negative");
  }

  pollfd pfd{};
  pfd.fd = socket_fd_;
  pfd.events = POLLIN;
  const int wait_ms = timeout.count() > INT_MAX ? INT_MAX : static_cast<int>(timeout.count());
  const int polled = ::poll(&pfd, 1, wait_ms);
  if (polled == 0) {
    return std::nullopt;
  }
  if (polled < 0) {
    ThrowErrno(errno, "poll");
  }

  can_frame frame{};
  const ssize_t nread = ::read(socket_fd_, &frame, sizeof(frame));
  if (nread < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return std::nullopt;
    }
    ThrowErrno(errno, "read");
  }
  if (static_cast<std::size_t>(nread) != sizeof(frame)) {
    ThrowErrno(EIO, "short CAN read");
  }
  return frame;
}

}  // namespace dual_nero
