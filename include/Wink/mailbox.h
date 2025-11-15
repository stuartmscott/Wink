// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_MAILBOX_H_
#define INCLUDE_WINK_MAILBOX_H_

#include <Wink/address.h>
#include <Wink/constants.h>
#include <Wink/socket.h>

#include <condition_variable>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class Mailbox {
 public:
  Mailbox() {}
  Mailbox(const Mailbox&) = delete;
  Mailbox(Mailbox&&) = delete;
  Mailbox& operator=(const Mailbox&) = delete;
  Mailbox& operator=(Mailbox&&) = delete;
  virtual ~Mailbox() {}
  virtual bool Receive(Address& from, std::string& message) = 0;
  virtual void Send(const Address& to, const std::string& message) = 0;
  virtual bool Flushed() = 0;
};

class AsyncMailbox : public Mailbox {
 public:
  explicit AsyncMailbox(Socket* socket);
  AsyncMailbox(const AsyncMailbox&) = delete;
  AsyncMailbox(AsyncMailbox&&) = delete;
  AsyncMailbox& operator=(const AsyncMailbox&) = delete;
  AsyncMailbox& operator=(AsyncMailbox&&) = delete;
  ~AsyncMailbox();
  bool Receive(Address& from, std::string& message) override;
  void Send(const Address& to, const std::string& message) override;
  bool Flushed() override;

 private:
  void BackgroundReceive();
  void BackgroundSend();
  struct QueuedMessage {
    std::chrono::system_clock::time_point time;
    uint64_t seq_num;
    uint8_t attempts;
    Address address;
    std::string message;
  };
  Socket* socket_;
  char receive_buffer_[kMaxUDPPayload];
  char send_buffer_[kMaxUDPPayload];
  std::mutex incoming_mutex_;
  std::mutex outgoing_mutex_;
  std::condition_variable incoming_condition_;
  std::condition_variable outgoing_condition_;
  std::deque<QueuedMessage> incoming_messages_;
  std::deque<QueuedMessage> outgoing_messages_;
  std::map<const Address, uint64_t> incoming_seq_nums_;
  std::map<const Address, uint64_t> outgoing_seq_nums_;
  std::atomic_bool running_ = true;
  std::thread receiver_;
  std::thread sender_;
};

#endif  // INCLUDE_WINK_MAILBOX_H_
