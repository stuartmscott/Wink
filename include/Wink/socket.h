// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_SOCKET_H_
#define INCLUDE_WINK_SOCKET_H_

#include <Wink/address.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <mutex>

class Socket {
 public:
  virtual ~Socket() {}
  virtual bool Receive(Address&, char*, size_t&) = 0;
  virtual bool Send(const Address&, const char*, const size_t) = 0;
};

class UDPSocket : public Socket {
 public:
  explicit UDPSocket(Address& address);
  ~UDPSocket() { close(socket_); }
  bool Receive(Address&, char*, size_t&) override;
  bool Send(const Address&, const char*, const size_t) override;

 private:
  int socket_ = -1;
  std::mutex send_mutex_;
};

#endif  // INCLUDE_WINK_SOCKET_H_
