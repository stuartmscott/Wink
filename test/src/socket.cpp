// Copyright 2022-2025 Stuart Scott
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>

#include <thread>

bool MockSocket::Receive(Address& from, char* buffer, size_t& length) {
  std::scoped_lock lock(mutex_);
  if (receive_queue_.empty()) {
    return false;
  }
  const auto p = receive_queue_.front();
  from = p.address;
  std::memcpy(buffer, p.buffer, p.length);
  length = p.length;
  receive_queue_.pop_front();
  return true;
}

void MockSocket::Push(const Address& from, const char* buffer,
                      const size_t length) {
  std::scoped_lock lock(mutex_);
  receive_queue_.emplace_back(from, buffer, length);
}

bool MockSocket::Send(const Address& to, const char* buffer,
                      const size_t length) {
  std::scoped_lock lock(mutex_);
  send_queue_.emplace_back(to, buffer, length);
  return true;
}

bool MockSocket::Pop(Address& to, char* buffer, size_t& length) {
  std::scoped_lock lock(mutex_);
  if (send_queue_.empty()) {
    return false;
  }
  const auto p = send_queue_.front();
  to = p.address;
  std::memcpy(buffer, p.buffer, p.length);
  length = p.length;
  send_queue_.pop_front();
  return true;
}

void MockSocket::Await(Address& to, char* buffer, size_t& length) {
  while (!Pop(to, buffer, length)) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
