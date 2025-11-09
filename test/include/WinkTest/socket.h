#ifndef SOCKET_TEST_H
#define SOCKET_TEST_H

#include <Wink/address.h>
#include <Wink/constants.h>
#include <Wink/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <vector>

struct BindArgs {
  std::string ip;
  ushort port;
};

struct BindResult {
  std::string ip;
  ushort port;
  int result;
};

struct SetReceiveTimeoutArgs {
  int seconds;
};

typedef int SetReceiveTimeoutResult;

struct ReceiveArgs {
  int length;
  int flags;
};

struct ReceiveResult {
  std::string fromIP;
  ushort fromPort;
  char buffer[kMaxPayload];
  int result;
};

struct SendArgs {
  std::string toIP;
  ushort toPort;
  char buffer[kMaxPayload];
  int length;
  int flags;
};

typedef int SendResult;

class MockSocket : public Socket {
 public:
  MockSocket() {}
  MockSocket(const MockSocket& s) = delete;
  MockSocket(MockSocket&& s) = delete;
  ~MockSocket() {}
  int Bind(Address& self) override;
  int SetReceiveTimeout(const int seconds) override;
  int Receive(Address& from, char* buffer, const int length,
              const int flags = 0) override;
  int Send(const Address& to, const char* buffer, const int length,
           const int flags = 0) override;
  std::vector<BindArgs> bindArgs_;
  std::vector<BindResult> bindResults_;
  std::vector<SetReceiveTimeoutArgs> setReceiveTimeoutArgs_;
  std::vector<SetReceiveTimeoutResult> setReceiveTimeoutResults_;
  std::vector<ReceiveArgs> receiveArgs_;
  std::vector<ReceiveResult> receiveResults_;
  std::vector<SendArgs> sendArgs_;
  std::vector<SendResult> sendResults_;
};

#endif
