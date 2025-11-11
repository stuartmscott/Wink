// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_SOCKET_H_
#define INCLUDE_WINK_SOCKET_H_

#include <Wink/address.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class Socket {
 public:
  virtual ~Socket() {}
  virtual ssize_t Receive(Address&, char*) = 0;
  virtual bool Send(const Address&, const char*, const ssize_t) = 0;
};

class UDPSocket : public Socket {
 public:
  explicit UDPSocket(Address& address);
  ~UDPSocket() { close(socket_); }
  ssize_t Receive(Address&, char*) override;
  bool Send(const Address&, const char*, const ssize_t) override;

 private:
  int socket_;
  std::mutex send_mutex_;
};

#endif  // INCLUDE_WINK_SOCKET_H_
