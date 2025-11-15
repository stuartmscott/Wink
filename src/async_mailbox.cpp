// Copyright 2022-2025 Stuart Scott
#include <Wink/log.h>
#include <Wink/mailbox.h>
#include <Wink/socket.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <string>

AsyncMailbox::AsyncMailbox(Socket* socket)
    : socket_(socket),
      running_(true),
      receiver_([&]() {
        while (running_) {
          BackgroundReceive();
        }
      }),
      sender_([&]() {
        while (running_) {
          BackgroundSend();
        }
      }) {}

AsyncMailbox::~AsyncMailbox() {
  while (!Flushed()) {
  }
  running_ = false;
  receiver_.join();
  sender_.join();
  delete socket_;
}

bool AsyncMailbox::Receive(Address& from, std::string& message) {
  std::unique_lock lock(incoming_mutex_);
  if (!incoming_condition_.wait_for(lock, kReceiveTimeout, [this] {
        return !incoming_messages_.empty();
      })) {
    return false;
  }

  const auto& in = incoming_messages_.front();
  from = in.address;
  message = in.message;
  incoming_messages_.pop_front();
  return true;
}

void AsyncMailbox::Send(const Address& to, const std::string& message) {
  uint64_t seq_num = 0;
  if (const auto& it = outgoing_seq_nums_.find(to);
      it != outgoing_seq_nums_.end()) {
    it->second++;
    seq_num = it->second;
  } else {
    outgoing_seq_nums_[to] = 0;
  }
  std::scoped_lock lock(outgoing_mutex_);
  outgoing_messages_.emplace_back(std::chrono::system_clock::now(), seq_num, 0,
                                  to, message);
  outgoing_condition_.notify_all();
}

bool AsyncMailbox::Flushed() {
  std::unique_lock lock(outgoing_mutex_);
  return outgoing_condition_.wait_for(
      lock, kSendTimeout, [this] { return outgoing_messages_.empty(); });
}

void AsyncMailbox::BackgroundReceive() {
  Address from;
  size_t length;
  if (!socket_->Receive(from, receive_buffer_, length)) {
    return;
  }
  while (receive_buffer_[length - 1] == '\n') {
    --length;
  }
  if (length < sizeof(uint64_t)) {
    Error() << "Message too small: " << length << std::endl;
    return;
  }

  // Parse sequence number
  uint64_t seq_num;
  std::memcpy(&seq_num, receive_buffer_, sizeof(uint64_t));

  std::string message(receive_buffer_ + sizeof(uint64_t),
                      length - sizeof(uint64_t));

  if (message == "ack") {
    // Remove associated message from outgoing_messages_
    std::scoped_lock lock(outgoing_mutex_);
    for (auto it = outgoing_messages_.begin();
         it != outgoing_messages_.end();) {
      if (it->address == from && it->seq_num == seq_num) {
        outgoing_messages_.erase(it);
        outgoing_condition_.notify_all();
        return;
      } else {
        it++;
      }
    }
  } else {
    // Send acknowledgement
    {
      // receive_buffer_[0:7] alreadycontains seq_num
      receive_buffer_[8] = 'a';
      receive_buffer_[9] = 'c';
      receive_buffer_[10] = 'k';
      if (!socket_->Send(from, receive_buffer_, sizeof(uint64_t) + 3)) {
        Error() << "Failed to acknowledge " << from << ": "
                << std::strerror(errno) << std::endl;
      }
    }

    if (const auto& it = incoming_seq_nums_.find(from);
        it != incoming_seq_nums_.end()) {
      if (seq_num <= it->second) {
        Info() << "Dropping duplicate message: " << seq_num << std ::endl;
        // Drop duplicate packet
        // TODO handle sequence number overflow and wrap around
        return;
      }
    }

    // Save sequence number
    incoming_seq_nums_[from] = seq_num;

    std::scoped_lock lock(incoming_mutex_);
    incoming_messages_.emplace_back(std::chrono::system_clock::now(), seq_num,
                                    0, from, message);
    incoming_condition_.notify_all();
  }
}

void AsyncMailbox::BackgroundSend() {
  std::unique_lock lock(outgoing_mutex_);

  if (!outgoing_condition_.wait_for(
          lock, kSendTimeout, [this] { return !outgoing_messages_.empty(); })) {
    return;
  }

  const auto now = std::chrono::system_clock::now();
  for (auto it = outgoing_messages_.begin(); it != outgoing_messages_.end();) {
    if (it->attempts >= kMaxRetries) {
      Error() << "Failed to deliver to " << it->address << " failed after "
              << it->attempts << " attempts" << std::endl;
      it = outgoing_messages_.erase(it);
      continue;
    }

    const auto deadline = it->time + it->attempts * kReceiveTimeout;
    if (now >= deadline) {
      // TODO move out of outgoing_mutex_ lock
      uint64_t seq_num = it->seq_num;
      std::memcpy(send_buffer_, &seq_num, sizeof(uint64_t));
      const auto length =
          std::min(it->message.length(), kMaxUDPPayload - sizeof(uint64_t));
      std::memcpy(send_buffer_ + sizeof(uint64_t), it->message.c_str(), length);
      uint64_t bytes = length + sizeof(uint64_t);
      if (!socket_->Send(it->address, send_buffer_, bytes)) {
        Error() << "Failed to send to " << it->address << ": "
                << std::strerror(errno) << std::endl;
      }

      it->attempts++;
    }
    it++;
  }
}
