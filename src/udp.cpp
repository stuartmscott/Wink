// Copyright 2022-2025 Stuart Scott
#include <Wink/log.h>
#include <Wink/socket.h>

#include <cerrno>
#include <cstring>
#include <string>

UDPSocket::UDPSocket(Address& address)
    : address_(address),
      unicast_socket_(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
  if (unicast_socket_ < 0) {
    throw std::runtime_error(
        std::string("Failed to open UDP unicast socket: ") +
        std::strerror(errno));
  }

  // Enable address reuse on unicast socket
  auto on = 1;
  if (setsockopt(unicast_socket_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) <
      0) {
    throw std::runtime_error(
        std::string("Failed to set UDP unicast socket reuse option: ") +
        std::strerror(errno));
  }

  // Bind unicast socket
  sockaddr_in unicast_address = {};
  address.WriteTo(unicast_address);
  socklen_t size = sizeof(struct sockaddr_in);
  if (const auto result =
          bind(unicast_socket_, (struct sockaddr*)&unicast_address, size);
      result < 0) {
    throw std::runtime_error(
        std::string("Failed to bind UDP unicast socket to ") +
        address.ToString() + std::string(": ") + std::strerror(errno));
  }

  // Retrieve assigned unicast address
  if (const auto result = getsockname(
          unicast_socket_, (struct sockaddr*)&unicast_address, &size);
      result < 0) {
    throw std::runtime_error(
        std::string("Failed to get UDP unicast socket name: ") +
        std::strerror(errno));
  }
  address.ReadFrom(unicast_address);

  // Set receive timeout on unicast socket
  timeval tv = {};
  tv.tv_sec = kReceiveTimeout.count();
  tv.tv_usec = 0;
  if (setsockopt(unicast_socket_, SOL_SOCKET, SO_RCVTIMEO, &tv,
                 sizeof(struct timeval)) < 0) {
    throw std::runtime_error(
        std::string("Failed to set UDP unicast socket receive timeout: ") +
        std::strerror(errno));
  }

  // Set multicast interface on unicast socket
  in_addr interface = {};
  interface.s_addr = address.ToInetAddr();
  if (setsockopt(unicast_socket_, IPPROTO_IP, IP_MULTICAST_IF, &interface,
                 sizeof(interface)) < 0) {
    throw std::runtime_error(
        std::string("Failed to set UDP unicast socket multicast interace: ") +
        std::strerror(errno));
  }
}

bool UDPSocket::Receive(Address& from, Address& to, char* buffer,
                        size_t& length) {
  sockaddr_in address = {};
  socklen_t size = sizeof(struct sockaddr_in);
  const ssize_t result = recvfrom(unicast_socket_, buffer, kMaxUDPPayload, 0,
                                  (struct sockaddr*)&address, &size);
  if (result <= 0) {
    if (errno != EAGAIN) {
      Error() << "Failed to receive unicast packet: " << std::strerror(errno)
              << std::endl;
    }
    return false;
  }
  from.ReadFrom(address);
  to = address_;
  length = result;
  return true;
}

bool UDPSocket::ReceiveMulticast(Address& from, Address& to, char* buffer,
                                 size_t& length) {
  for (const auto& [group, multicast_socket] : multicast_sockets_) {
    sockaddr_in address = {};
    socklen_t size = sizeof(struct sockaddr_in);
    const ssize_t result = recvfrom(multicast_socket, buffer, kMaxUDPPayload, 0,
                                    (struct sockaddr*)&address, &size);
    if (result <= 0) {
      if (errno != EAGAIN) {
        Error() << "Failed to receive multicast packet from " << group << ": "
                << std::strerror(errno) << std::endl;
      }
      continue;
    }
    from.ReadFrom(address);
    to = group;
    length = result;
    return true;
  }
  return false;
}

bool UDPSocket::Send(const Address& to, const char* buffer,
                     const size_t length) {
  std::scoped_lock send_lock(send_mutex_);
  sockaddr_in address = {};
  to.WriteTo(address);
  const ssize_t result =
      sendto(unicast_socket_, buffer, length, 0, (struct sockaddr*)&address,
             sizeof(struct sockaddr_in));
  return result >= 0;
}

bool UDPSocket::JoinGroup(const Address& group) {
  if (!group.IsMulticast()) {
    Error() << "IP is not a multicast group: " << group.ip() << std::endl;
    return false;
  }

  const int multicast_socket(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
  if (multicast_socket < 0) {
    Error() << "Failed to open UDP multicast socket: " << std::strerror(errno)
            << std::endl;
    return false;
  }

  // Enable address reuse on multicast socket
  auto on = 1;
  if (setsockopt(multicast_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) <
      0) {
    Error() << "Failed to set UDP multicast socket reuse option: "
            << std::strerror(errno) << std::endl;
    close(multicast_socket);
    return false;
  }

  // Bind multicast socket
  sockaddr_in multicast_address = {};
  multicast_address.sin_family = AF_INET;
  multicast_address.sin_port = htons(group.port());
  multicast_address.sin_addr.s_addr = INADDR_ANY;
  if (const auto result =
          bind(multicast_socket, (struct sockaddr*)&multicast_address,
               sizeof(struct sockaddr_in));
      result < 0) {
    Error() << "Failed to bind UDP multicast socket to 0.0.0.0:" << group.port()
            << ": " << std::strerror(errno) << std::endl;
    close(multicast_socket);
    return false;
  }

  // Set receive timeout on unicast socket
  timeval tv = {};
  tv.tv_sec = kReceiveTimeout.count();
  tv.tv_usec = 0;
  if (setsockopt(multicast_socket, SOL_SOCKET, SO_RCVTIMEO, &tv,
                 sizeof(struct timeval)) < 0) {
    Error() << "Failed to set UDP multicast socket receive timeout: "
            << std::strerror(errno) << std::endl;
    close(multicast_socket);
    return false;
  }

  // Join multicast group
  ip_mreq group_address = {};
  group_address.imr_multiaddr.s_addr = group.ToInetAddr();
  group_address.imr_interface.s_addr = address_.ToInetAddr();
  if (setsockopt(multicast_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                 &group_address, sizeof(group_address)) < 0) {
    Error() << "Failed to join UDP multicast group: " << std::strerror(errno)
            << std::endl;
    close(multicast_socket);
    return false;
  }

  multicast_sockets_[group] = multicast_socket;
  return true;
}

bool UDPSocket::LeaveGroup(const Address& group) {
  if (const auto it = multicast_sockets_.find(group);
      it != multicast_sockets_.end()) {
    const auto multicast_socket = it->second;
    multicast_sockets_.erase(it);

    // Leave multicast group
    ip_mreq group_address = {};
    group_address.imr_multiaddr.s_addr = group.ToInetAddr();
    group_address.imr_interface.s_addr = address_.ToInetAddr();
    if (setsockopt(multicast_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                   &group_address, sizeof(group_address)) < 0) {
      Error() << "Failed to leave UDP multicast group: " << std::strerror(errno)
              << std::endl;
      close(multicast_socket);
      return false;
    }

    close(multicast_socket);
    return true;
  }
  return false;
}
