// Copyright 2022-2025 Stuart Scott
#ifndef TEST_INCLUDE_WINKTEST_MAILBOX_H_
#define TEST_INCLUDE_WINKTEST_MAILBOX_H_

#include <Wink/address.h>
#include <Wink/constants.h>
#include <Wink/mailbox.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

typedef bool ReceiveArgs;

struct ReceiveResult {
  std::string fromIP;
  uint16_t fromPort;
  std::string toIP;
  uint16_t toPort;
  std::string message;
  bool result;
};

struct SendArgs {
  std::string toIP;
  uint16_t toPort;
  std::string message;
};

typedef bool SendResult;

class MockMailbox : public Mailbox {
 public:
  MockMailbox() {}
  MockMailbox(const MockMailbox& s) = delete;
  MockMailbox(MockMailbox&& s) = delete;
  ~MockMailbox() {}
  bool Receive(Address& from, Address& to, std::string& message) override;
  void Send(const Address& to, const std::string& message) override;
  bool Flushed() override { return flushed_; }

  std::vector<ReceiveArgs> receiveArgs_;
  std::vector<ReceiveResult> receiveResults_;
  std::vector<SendArgs> sendArgs_;
  std::vector<SendResult> sendResults_;
  bool flushed_ = true;
};

void setup_default_mailbox(MockMailbox& mailbox);
void assert_default_mailbox(MockMailbox& mailbox, Address& parent);

#endif  // TEST_INCLUDE_WINKTEST_MAILBOX_H_
