#ifndef SOCKET_H
#define SOCKET_H

#include <Wink/address.h>
#include <Wink/constants.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>

class Socket {
 public:
  Socket() {}
  Socket(const Socket& s) = delete;
  Socket(Socket&& s) = delete;
  Socket& operator=(const Socket& s) = delete;
  Socket& operator=(Socket&& s) = delete;
  virtual ~Socket() = default;
  virtual int Bind(Address& address) = 0;
  virtual int SetReceiveTimeout(const int seconds) = 0;
  virtual int Receive(Address& from, char* buffer, const int length,
                      const int flags = 0) = 0;
  virtual int Send(const Address& to, const char* buffer, const int length,
                   const int flags = 0) = 0;
};

class UDPSocket : public Socket {
 public:
  UDPSocket();
  UDPSocket(const UDPSocket& s) = delete;
  UDPSocket(UDPSocket&& s) = delete;
  UDPSocket& operator=(const UDPSocket& s) = delete;
  UDPSocket& operator=(UDPSocket&& s) = delete;
  ~UDPSocket();
  int Bind(Address& addr) override;
  int SetReceiveTimeout(const int seconds) override;
  int Receive(Address& from, char* buffer, const int length,
              const int flags = 0) override;
  int Send(const Address& to, const char* buffer, const int length,
           const int flags = 0) override;

 private:
  int socket_;
};

#endif
