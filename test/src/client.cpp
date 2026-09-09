// Copyright 2022-2025 Stuart Scott
#include <Wink/client.h>
#include <Wink/log.h>
#include <WinkTest/constants.h>
#include <WinkTest/mailbox.h>
#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace Wink::Test {

TEST(ClientTest, StartMachine) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result{true};
    mailbox.sendResults_.push_back(result);
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = TestUnicastIP;
    result.fromPort = TestPort;
    result.toIP = Localhost;
    result.toPort = TestPort;
    result.result = true;
    std::ostringstream oss;
    oss << "started ";
    oss << TestBinary;
    result.message = oss.str();
    mailbox.receiveResults_.push_back(result);
  }

  // Issue request
  Address address(Localhost, 0);
  Address destination(TestUnicastIP, 0);
  std::vector<std::string> args;
  ASSERT_EQ(0, StartMachine(mailbox, address, TestBinary, destination, args));

  // Check mailbox send
  {
    ASSERT_EQ(1, mailbox.sendArgs_.size());
    const auto arg{mailbox.sendArgs_.at(0)};
    ASSERT_EQ(TestUnicastIP, arg.toIP);
    ASSERT_EQ(ServerPort, arg.toPort);
    ASSERT_EQ("start wink.bin :0", arg.message);
  }

  // Check destination address
  ASSERT_EQ(TestUnicastIP, destination.ip());
  ASSERT_EQ(TestPort, destination.port());
}

TEST(ClientTest, StopMachine) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result{0};
    mailbox.sendResults_.push_back(result);
  }

  Address address(TestUnicastIP, TestPort);
  ASSERT_EQ(0, StopMachine(mailbox, address));

  // Check mailbox send
  ASSERT_EQ(1, mailbox.sendArgs_.size());
  const auto arg{mailbox.sendArgs_.at(0)};
  ASSERT_EQ(TestUnicastIP, arg.toIP);
  ASSERT_EQ(ServerPort, arg.toPort);
  ASSERT_EQ("stop 42424", arg.message);
}

TEST(ClientTest, SendMessage) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result{0};
    mailbox.sendResults_.push_back(result);
  }

  Address address(TestUnicastIP, TestPort);
  SendMessage(mailbox, address, TestMessage);

  // Check mailbox send
  ASSERT_EQ(1, mailbox.sendArgs_.size());
  const auto arg{mailbox.sendArgs_.at(0)};
  ASSERT_EQ(TestUnicastIP, arg.toIP);
  ASSERT_EQ(TestPort, arg.toPort);
  ASSERT_EQ(TestMessage, arg.message);
}

TEST(ClientTest, ReceiveMessage) {
  MockMailbox mailbox;

  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = TestUnicastIP;
    result.fromPort = TestPort;
    result.toIP = TestUnicastIP;
    result.toPort = TestPort;
    result.message = TestMessage;
    result.result = true;
    mailbox.receiveResults_.push_back(result);
  }

  Address from;
  Address to;
  std::string message;
  ASSERT_TRUE(ReceiveMessage(mailbox, from, to, message));

  // Check mailbox receive
  ASSERT_EQ(1, mailbox.receiveArgs_.size());
  ASSERT_EQ(TestUnicastIP, from.ip());
  ASSERT_EQ(TestPort, from.port());
  ASSERT_EQ(TestUnicastIP, to.ip());
  ASSERT_EQ(TestPort, to.port());
  ASSERT_EQ(TestMessage, message);
}

TEST(ClientTest, ListMachines) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result{0};
    mailbox.sendResults_.push_back(result);
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = TestUnicastIP;
    result.fromPort = ServerPort;
    result.result = true;
    std::ostringstream oss;
    oss << "Port,Machine,PID\n";
    oss << TestPort << ',' << TestBinary << ',' << TestPID << '\n';
    result.message = oss.str();
    mailbox.receiveResults_.push_back(result);
  }

  // Issue request
  Address destination(TestUnicastIP, ServerPort);
  ASSERT_EQ(0, ListMachines(mailbox, destination));

  // Check mailbox send
  {
    ASSERT_EQ(1, mailbox.sendArgs_.size());
    const auto arg{mailbox.sendArgs_.at(0)};
    ASSERT_EQ(TestUnicastIP, arg.toIP);
    ASSERT_EQ(ServerPort, arg.toPort);
    ASSERT_EQ("list", arg.message);
  }

  // Check destination address
  ASSERT_EQ(TestUnicastIP, destination.ip());
  ASSERT_EQ(ServerPort, destination.port());
}

};  // namespace Wink::Test
