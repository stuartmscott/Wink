// Copyright 2022-2025 Stuart Scott
#include <Wink/client.h>
#include <Wink/log.h>
#include <WinkTest/constants.h>
#include <WinkTest/mailbox.h>
#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(ClientTest, StartMachine) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result = true;
    mailbox.sendResults_.push_back(result);
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestIP;
    result.fromPort = kTestPort;
    result.result = true;
    std::ostringstream oss;
    oss << "started ";
    oss << kTestBinary;
    result.message = oss.str();
    mailbox.receiveResults_.push_back(result);
  }

  // Issue request
  Address address(kLocalhost, 0);
  Address destination(kTestIP, 0);
  std::vector<std::string> args;
  ASSERT_EQ(0, StartMachine(mailbox, address, kTestBinary, destination, args));

  // Check mailbox send
  {
    ASSERT_EQ(1, mailbox.sendArgs_.size());
    const auto arg = mailbox.sendArgs_.at(0);
    ASSERT_EQ(kTestIP, arg.toIP);
    ASSERT_EQ(kServerPort, arg.toPort);
    ASSERT_EQ("start wink.bin :0", arg.message);
  }

  // Check destination address
  ASSERT_EQ(kTestIP, destination.ip());
  ASSERT_EQ(kTestPort, destination.port());
}

TEST(ClientTest, StopMachine) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result = 0;
    mailbox.sendResults_.push_back(result);
  }

  Address address(kTestIP, kTestPort);
  ASSERT_EQ(0, StopMachine(mailbox, address));

  // Check mailbox send
  ASSERT_EQ(1, mailbox.sendArgs_.size());
  const auto arg = mailbox.sendArgs_.at(0);
  ASSERT_EQ(kTestIP, arg.toIP);
  ASSERT_EQ(kServerPort, arg.toPort);
  ASSERT_EQ("stop 42424", arg.message);
}

TEST(ClientTest, SendMessage) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result = 0;
    mailbox.sendResults_.push_back(result);
  }

  Address address(kTestIP, kTestPort);
  SendMessage(mailbox, address, kTestMessage);

  // Check mailbox send
  ASSERT_EQ(1, mailbox.sendArgs_.size());
  const auto arg = mailbox.sendArgs_.at(0);
  ASSERT_EQ(kTestIP, arg.toIP);
  ASSERT_EQ(kTestPort, arg.toPort);
  ASSERT_EQ(kTestMessage, arg.message);
}

TEST(ClientTest, ReceiveMessage) {
  MockMailbox mailbox;

  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestIP;
    result.fromPort = kTestPort;
    result.message = kTestMessage;
    result.result = true;
    mailbox.receiveResults_.push_back(result);
  }

  Address address;
  std::string message;
  ASSERT_TRUE(ReceiveMessage(mailbox, address, message));

  // Check mailbox receive
  ASSERT_EQ(1, mailbox.receiveArgs_.size());
  ASSERT_EQ(kTestIP, address.ip());
  ASSERT_EQ(kTestPort, address.port());
  ASSERT_EQ(kTestMessage, message);
}

TEST(ClientTest, ListMachines) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result = 0;
    mailbox.sendResults_.push_back(result);
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestIP;
    result.fromPort = kServerPort;
    result.result = true;
    std::ostringstream oss;
    oss << "Port,Machine,PID\n";
    oss << kTestPort << ',' << kTestBinary << ',' << kTestPID << '\n';
    result.message = oss.str();
    mailbox.receiveResults_.push_back(result);
  }

  // Issue request
  Address destination(kTestIP, kServerPort);
  ASSERT_EQ(0, ListMachines(mailbox, destination));

  // Check mailbox send
  {
    ASSERT_EQ(1, mailbox.sendArgs_.size());
    const auto arg = mailbox.sendArgs_.at(0);
    ASSERT_EQ(kTestIP, arg.toIP);
    ASSERT_EQ(kServerPort, arg.toPort);
    ASSERT_EQ("list", arg.message);
  }

  // Check destination address
  ASSERT_EQ(kTestIP, destination.ip());
  ASSERT_EQ(kServerPort, destination.port());
}
