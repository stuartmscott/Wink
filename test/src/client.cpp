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
    result.fromIP = kTestUnicastIP;
    result.fromPort = kTestPort;
    result.toIP = kLocalhost;
    result.toPort = kTestPort;
    result.result = true;
    std::ostringstream oss;
    oss << "started ";
    oss << kTestBinary;
    result.message = oss.str();
    mailbox.receiveResults_.push_back(result);
  }

  // Issue request
  Address address(kLocalhost, 0);
  Address destination(kTestUnicastIP, 0);
  std::vector<std::string> args;
  ASSERT_EQ(0, StartMachine(mailbox, address, kTestBinary, destination, args));

  // Check mailbox send
  {
    ASSERT_EQ(1, mailbox.sendArgs_.size());
    const auto arg = mailbox.sendArgs_.at(0);
    ASSERT_EQ(kTestUnicastIP, arg.toIP);
    ASSERT_EQ(kServerPort, arg.toPort);
    ASSERT_EQ("start wink.bin :0", arg.message);
  }

  // Check destination address
  ASSERT_EQ(kTestUnicastIP, destination.ip());
  ASSERT_EQ(kTestPort, destination.port());
}

TEST(ClientTest, StopMachine) {
  MockMailbox mailbox;

  // Set mock send result
  {
    SendResult result = 0;
    mailbox.sendResults_.push_back(result);
  }

  Address address(kTestUnicastIP, kTestPort);
  ASSERT_EQ(0, StopMachine(mailbox, address));

  // Check mailbox send
  ASSERT_EQ(1, mailbox.sendArgs_.size());
  const auto arg = mailbox.sendArgs_.at(0);
  ASSERT_EQ(kTestUnicastIP, arg.toIP);
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

  Address address(kTestUnicastIP, kTestPort);
  SendMessage(mailbox, address, kTestMessage);

  // Check mailbox send
  ASSERT_EQ(1, mailbox.sendArgs_.size());
  const auto arg = mailbox.sendArgs_.at(0);
  ASSERT_EQ(kTestUnicastIP, arg.toIP);
  ASSERT_EQ(kTestPort, arg.toPort);
  ASSERT_EQ(kTestMessage, arg.message);
}

TEST(ClientTest, ReceiveMessage) {
  MockMailbox mailbox;

  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestUnicastIP;
    result.fromPort = kTestPort;
    result.toIP = kTestUnicastIP;
    result.toPort = kTestPort;
    result.message = kTestMessage;
    result.result = true;
    mailbox.receiveResults_.push_back(result);
  }

  Address from;
  Address to;
  std::string message;
  ASSERT_TRUE(ReceiveMessage(mailbox, from, to, message));

  // Check mailbox receive
  ASSERT_EQ(1, mailbox.receiveArgs_.size());
  ASSERT_EQ(kTestUnicastIP, from.ip());
  ASSERT_EQ(kTestPort, from.port());
  ASSERT_EQ(kTestUnicastIP, to.ip());
  ASSERT_EQ(kTestPort, to.port());
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
    result.fromIP = kTestUnicastIP;
    result.fromPort = kServerPort;
    result.result = true;
    std::ostringstream oss;
    oss << "Port,Machine,PID\n";
    oss << kTestPort << ',' << kTestBinary << ',' << kTestPID << '\n';
    result.message = oss.str();
    mailbox.receiveResults_.push_back(result);
  }

  // Issue request
  Address destination(kTestUnicastIP, kServerPort);
  ASSERT_EQ(0, ListMachines(mailbox, destination));

  // Check mailbox send
  {
    ASSERT_EQ(1, mailbox.sendArgs_.size());
    const auto arg = mailbox.sendArgs_.at(0);
    ASSERT_EQ(kTestUnicastIP, arg.toIP);
    ASSERT_EQ(kServerPort, arg.toPort);
    ASSERT_EQ("list", arg.message);
  }

  // Check destination address
  ASSERT_EQ(kTestUnicastIP, destination.ip());
  ASSERT_EQ(kServerPort, destination.port());
}
