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
  bool Receive(Address&, Address&, char*, size_t&) override;
  bool ReceiveMulticast(Address&, Address&, char*, size_t&) override;
  bool Send(const Address&, const char*, const size_t) override;
  void Push(const Address&, const Address&, const char*, const size_t);
  void PushMulticast(const Address&, const Address&, const char*, const size_t);
  bool Pop(Address&, char*, size_t&);
  void Await(Address&, char*, size_t&);

 private:
  std::mutex mutex_;
  struct Packet {
    Address from;
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    Packet(const Address f, const Address t, const char* b, const size_t l)
        : from(f), to(t), length(l) {
      std::memcpy(buffer, b, l);
    }
  };
  std::deque<Packet> receive_queue_;
  std::deque<Packet> receive_multicast_queue_;
  std::deque<Packet> send_queue_;
};

#endif  // TEST_INCLUDE_WINKTEST_SOCKET_H_
