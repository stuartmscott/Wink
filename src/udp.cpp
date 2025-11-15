// Copyright 2022-2025 Stuart Scott
#include <Wink/log.h>
#include <Wink/socket.h>

#include <cerrno>
#include <cstring>
#include <string>

UDPSocket::UDPSocket(Address& address)
    : socket_(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
  if (socket_ < 0) {
    throw std::runtime_error(std::string("Failed to open UDP socket: ") +
                             std::strerror(errno));
  }

  // Enable address reuse
  auto on = 1;
  if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
    throw std::runtime_error(
        std::string("Failed to set UDP socket reuse option: ") +
        std::strerror(errno));
  }

  // Bind socket
  struct sockaddr_in sa;
  address.WriteTo(sa);
  socklen_t size = sizeof(struct sockaddr_in);
  if (const auto result = bind(socket_, (struct sockaddr*)&sa, size);
      result < 0) {
    throw std::runtime_error(std::string("Failed to bind UDP socket to ") +
                             address.ToString() + std::string(": ") +
                             std::strerror(errno));
  }

  // Retrieve assigned address
  if (const auto result = getsockname(socket_, (struct sockaddr*)&sa, &size);
      result < 0) {
    throw std::runtime_error(std::string("Failed to get UDP socket name: ") +
                             std::strerror(errno));
  }
  address.ReadFrom(sa);

  // Set receive timeout
  struct timeval tv;
  tv.tv_sec = kReceiveTimeout.count();
  tv.tv_usec = 0;
  if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &tv,
                 sizeof(struct timeval)) < 0) {
    throw std::runtime_error(
        std::string("Failed to set UDP socket receive timeout: ") +
        std::strerror(errno));
  }
}

bool UDPSocket::Receive(Address& from, char* buffer, size_t& length) {
  struct sockaddr_in address;
  socklen_t size = sizeof(struct sockaddr_in);
  const ssize_t result = recvfrom(socket_, buffer, kMaxUDPPayload, 0,
                                  (struct sockaddr*)&address, &size);
  if (result < 0) {
    if (errno != EAGAIN) {
      Error() << "Failed to receive packet: " << std::strerror(errno)
              << std::endl;
    }
    return false;
  }
  from.ReadFrom(address);
  length = result;
  return true;
}

bool UDPSocket::Send(const Address& to, const char* buffer,
                     const size_t length) {
  std::scoped_lock send_lock(send_mutex_);
  struct sockaddr_in address;
  to.WriteTo(address);
  const ssize_t result =
      sendto(socket_, buffer, length, 0, (struct sockaddr*)&address,
             sizeof(struct sockaddr_in));
  return result >= 0;
}
