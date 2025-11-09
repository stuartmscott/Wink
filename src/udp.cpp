#include <Wink/log.h>
#include <Wink/socket.h>
#include <sys/time.h>

UDPSocket::UDPSocket() : socket_(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
  if (socket_ < 0) {
    Error() << "Failed to create socket: " << std::strerror(errno) << '\n'
            << std::flush;
  }
  auto on = 1;
  if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
    Error() << "Failed to set reuse option: " << std::strerror(errno) << '\n'
            << std::flush;
  }
}

UDPSocket::~UDPSocket() { close(socket_); }

int UDPSocket::SetReceiveTimeout(const int seconds) {
  if (socket_ < 0) {
    return -1;
  }
  struct timeval tv;
  tv.tv_sec = seconds;
  tv.tv_usec = 0;
  return setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &tv,
                    sizeof(struct timeval));
}

int UDPSocket::Bind(Address& address) {
  if (socket_ < 0) {
    return -1;
  }
  struct sockaddr_in sa;
  address.WriteTo(sa);
  socklen_t size = sizeof(struct sockaddr_in);
  if (const auto result = bind(socket_, (struct sockaddr*)&sa, size);
      result < 0) {
    return result;
  }
  if (const auto result = getsockname(socket_, (struct sockaddr*)&sa, &size);
      result < 0) {
    return result;
  }
  address.ReadFrom(sa);
  return 0;
}

int UDPSocket::Receive(Address& from, char* buffer, const int length,
                       const int flags) {
  if (socket_ < 0) {
    return -1;
  }
  struct sockaddr_in address;
  socklen_t size = sizeof(struct sockaddr_in);
  int result = recvfrom(socket_, buffer, length, flags,
                        (struct sockaddr*)&address, &size);
  if (result < 0) {
    return result;
  }
  from.ReadFrom(address);
  buffer[result] = 0;
  if (result > 0 && buffer[result - 1] == '\n') {
    buffer[result - 1] = 0;
  }
  return 0;
}

int UDPSocket::Send(const Address& to, const char* buffer, const int length,
                    const int flags) {
  if (socket_ < 0) {
    return -1;
  }
  struct sockaddr_in address;
  to.WriteTo(address);
  socklen_t size = sizeof(struct sockaddr_in);
  return sendto(socket_, buffer, std::min(length, (int)kMaxPayload), flags,
                (struct sockaddr*)&address, size);
}
