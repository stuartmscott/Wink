// Copyright 2022-2025 Stuart Scott
#include <Wink/log.h>
#include <WinkTest/constants.h>
#include <WinkTest/mailbox.h>

#include <cstring>
#include <string>

bool MockMailbox::Receive(Address& from, Address& to, std::string& message) {
  const auto index = receiveArgs_.size();
  receiveArgs_.push_back(true);
  if (index >= receiveResults_.size()) {
    Error() << "Unexpected call to Receive" << std::endl;
  }
  const auto result = receiveResults_.at(index);
  from.set_ip(result.fromIP);
  from.set_port(result.fromPort);
  to.set_ip(result.toIP);
  to.set_port(result.toPort);
  message = result.message;
  return result.result;
}

void MockMailbox::Send(const Address& to, const std::string& message) {
  const auto index = sendArgs_.size();
  SendArgs args;
  args.toIP = to.ip();
  args.toPort = to.port();
  args.message = message;
  sendArgs_.push_back(args);
  if (index >= sendResults_.size()) {
    Error() << "Unexpected call to Send" << std::endl;
  }
}

void setup_default_mailbox(MockMailbox& mailbox) {
  // Set mock send result
  {
    SendResult result = true;
    mailbox.sendResults_.push_back(result);
    mailbox.sendResults_.push_back(result);
    mailbox.sendResults_.push_back(result);
    mailbox.sendResults_.push_back(result);
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestUnicastIP;
    result.fromPort = kTestPort;
    result.toIP = kTestUnicastIP;
    result.toPort = kTestPort;
    result.result = true;
    std::ostringstream oss;
    oss << "started ";
    oss << kTestBinary;
    result.message = oss.str();
    mailbox.receiveResults_.push_back(result);
  }
}

void assert_default_mailbox(MockMailbox& mailbox, Address& parent) {
  // Check mailbox send
  {
    ASSERT_EQ(4, mailbox.sendArgs_.size());
    // Send Started Message to Spawner
    {
      const auto arg0 = mailbox.sendArgs_.at(0);
      ASSERT_EQ(parent.ip(), arg0.toIP);
      ASSERT_EQ(parent.port(), arg0.toPort);
      ASSERT_EQ(std::string("started test/Test"), arg0.message);
    }
    // Register Machine
    {
      const auto arg1 = mailbox.sendArgs_.at(1);
      ASSERT_EQ(kLocalhost, arg1.toIP);
      ASSERT_EQ(kServerPort, arg1.toPort);
      ASSERT_TRUE(arg1.message.starts_with("register test/Test "));
    }
    // Send Exited Message to Spawner
    {
      const auto arg2 = mailbox.sendArgs_.at(2);
      ASSERT_EQ(parent.ip(), arg2.toIP);
      ASSERT_EQ(parent.port(), arg2.toPort);
      ASSERT_EQ(std::string("exited test/Test"), arg2.message);
    }
    // Unregister Machine
    {
      const auto arg3 = mailbox.sendArgs_.at(3);
      ASSERT_EQ(kLocalhost, arg3.toIP);
      ASSERT_EQ(kServerPort, arg3.toPort);
      ASSERT_EQ(std::string("unregister"), arg3.message);
    }
  }
}
