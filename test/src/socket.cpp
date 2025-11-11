// Copyright 2022-2025 Stuart Scott
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>

#include <thread>

ssize_t MockSocket::Receive(Address& from, char* buffer) {
  std::scoped_lock lock(mutex_);
  if (receive_queue_.empty()) {
    return -1;
  }
  const auto p = receive_queue_.front();
  from = p.address_;
  std::memcpy(buffer, p.buffer_, p.length_);
  receive_queue_.pop_front();
  return p.length_;
}

void MockSocket::Push(const Address& from, const char* buffer,
                      const ssize_t length) {
  std::scoped_lock lock(mutex_);
  receive_queue_.emplace_back(from, buffer, length);
}

bool MockSocket::Send(const Address& to, const char* buffer,
                      const ssize_t length) {
  std::scoped_lock lock(mutex_);
  send_queue_.emplace_back(to, buffer, length);
  return true;
}

bool MockSocket::Pop(Address& to, char* buffer, ssize_t& length) {
  std::scoped_lock lock(mutex_);
  if (send_queue_.empty()) {
    return false;
  }
  const auto p = send_queue_.front();
  to = p.address_;
  std::memcpy(buffer, p.buffer_, p.length_);
  length = p.length_;
  send_queue_.pop_front();
  return true;
}

void MockSocket::Await(Address& to, char* buffer, ssize_t& length) {
  while (!Pop(to, buffer, length)) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
