// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_SOCKET_H_
#define INCLUDE_WINK_SOCKET_H_

#include <Wink/address.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <mutex>

class Socket {
 public:
  virtual ~Socket() {}
  virtual bool Receive(Address&, Address&, char*, size_t&) = 0;
  virtual bool ReceiveMulticast(Address&, Address&, char*, size_t&) = 0;
  virtual bool Send(const Address&, const char*, const size_t) = 0;
};

class UDPSocket : public Socket {
 public:
  explicit UDPSocket(Address& address);
  ~UDPSocket() {
    close(unicast_socket_);
    for (auto& [a, s] : multicast_sockets_) {
      close(s);
    }
  }
  bool Receive(Address&, Address&, char*, size_t&) override;
  bool ReceiveMulticast(Address&, Address&, char*, size_t&) override;
  bool Send(const Address&, const char*, const size_t) override;
  bool JoinGroup(const Address&);
  bool LeaveGroup(const Address&);

 private:
  Address& address_;
  int unicast_socket_ = -1;
  std::map<Address, int> multicast_sockets_;
  std::mutex send_mutex_;
};

#endif  // INCLUDE_WINK_SOCKET_H_
