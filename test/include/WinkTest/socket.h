// Copyright 2022-2025 Stuart Scott
#ifndef TEST_INCLUDE_WINKTEST_SOCKET_H_
#define TEST_INCLUDE_WINKTEST_SOCKET_H_

#include <Wink/address.h>
#include <Wink/constants.h>
#include <Wink/socket.h>
#include <gtest/gtest.h>

#include <cstring>
#include <deque>

class MockSocket : public Socket {
 public:
  bool Receive(Address&, char*, size_t&) override;
  bool Send(const Address&, const char*, const size_t) override;
  void Push(const Address&, const char*, const size_t);
  bool Pop(Address&, char*, size_t&);
  void Await(Address&, char*, size_t&);

 private:
  std::mutex mutex_;
  struct Packet {
    Address address;
    char buffer[kMaxTestPayload];
    size_t length;
    Packet(const Address a, const char* b, const size_t l)
        : address(), length(l) {
      std::memcpy(buffer, b, l);
    }
  };
  std::deque<Packet> receive_queue_;
  std::deque<Packet> send_queue_;
};

#endif  // TEST_INCLUDE_WINKTEST_SOCKET_H_
