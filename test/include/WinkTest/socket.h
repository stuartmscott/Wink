// Copyright 2022-2025 Stuart Scott
#ifndef TEST_INCLUDE_WINKTEST_SOCKET_H_
#define TEST_INCLUDE_WINKTEST_SOCKET_H_

#include <Wink/address.h>
#include <Wink/constants.h>
#include <Wink/socket.h>
#include <gtest/gtest.h>

#include <deque>

class MockSocket : public Socket {
 public:
  ssize_t Receive(Address&, char*) override;
  bool Send(const Address&, const char*, const ssize_t) override;
  void Push(const Address&, const char*, const ssize_t);
  bool Pop(Address&, char*, ssize_t&);
  void Await(Address&, char*, ssize_t&);

 private:
  std::mutex mutex_;
  struct Packet {
    Address address_;
    char buffer_[kMaxTestPayload];
    ssize_t length_;
    Packet(const Address address, const char* buffer, const ssize_t length)
        : address_(address), length_(length) {
      std::memcpy(buffer_, buffer, length);
    }
  };
  std::deque<Packet> receive_queue_;
  std::deque<Packet> send_queue_;
};

#endif  // TEST_INCLUDE_WINKTEST_SOCKET_H_
