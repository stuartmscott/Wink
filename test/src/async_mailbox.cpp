// Copyright 2022-2025 Stuart Scott
#include <Wink/constants.h>
#include <Wink/mailbox.h>
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>
#include <WinkTest/utils.h>
#include <gtest/gtest.h>

#include <string>

namespace Wink::Test {

TEST(AsyncMailboxTest, Timeout) {
  Address address(Localhost, 0);
  UDPSocket socket(address);
  AsyncMailbox mailbox(socket);

  Address from;
  Address to;
  std::string message;
  ASSERT_FALSE(mailbox.Receive(from, to, message));
}

TEST(AsyncMailboxTest, UnicastDelivery_Thread) {
  Address receiver_address(Localhost, 0);
  UDPSocket receiver_socket(receiver_address);
  AsyncMailbox receiver_mailbox(receiver_socket);

  std::thread worker{[receiver_address] {
    Address sender_address(Localhost, 0);
    UDPSocket sender_socket(sender_address);
    AsyncMailbox sender_mailbox(sender_socket);
    sender_mailbox.Send(receiver_address, TestMessage);
    while (!sender_mailbox.Flushed()) {
    }
  }};

  Address from;
  Address to;
  std::string message;
  bool success{false};
  for (uint8_t i{0}; i < MaxRetries && !success; ++i) {
    success = receiver_mailbox.Receive(from, to, message);
  }
  ASSERT_TRUE(success);
  ASSERT_EQ(TestMessage, message);
  worker.join();
}

TEST(AsyncMailboxTest, UnicastDelivery_Process) {
  Address receiver_address(Localhost, 0);
  UDPSocket receiver_socket(receiver_address);
  AsyncMailbox receiver_mailbox(receiver_socket);

  // Fork child process
  pid_t pid{fork()};

  if (pid == 0) {
    Address sender_address(Localhost, 0);
    UDPSocket sender_socket(sender_address);
    AsyncMailbox sender_mailbox(sender_socket);
    sender_mailbox.Send(receiver_address, TestMessage);
    while (!sender_mailbox.Flushed()) {
    }
    _exit(0);
  } else {
    ASSERT_GT(pid, 0);
    Address from;
    Address to;
    std::string message;
    bool success{false};
    for (uint8_t i{0}; i < MaxRetries && !success; ++i) {
      success = receiver_mailbox.Receive(from, to, message);
    }
    ASSERT_TRUE(success);
    ASSERT_EQ(TestMessage, message);
  }
}

TEST(AsyncMailboxTest, UnicastAcknowledgement) {
  MockSocket sender_socket;
  AsyncMailbox sender_mailbox(sender_socket);
  Address sender_address(Localhost, 0);

  MockSocket receiver_socket;
  AsyncMailbox receiver_mailbox(receiver_socket);
  Address receiver_address(Localhost, 0);

  sender_mailbox.Send(receiver_address, TestMessage);

  // Outgoing Message
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    sender_socket.Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(TestPacketLength, length);
    ASSERT_ARRAY_EQ(length, TestPacket, buffer);
  }

  receiver_socket.Push(sender_address, receiver_address, &TestPacket[0],
                       TestPacketLength);

  // Incoming Message
  {
    Address from;
    Address to;
    std::string message;
    ASSERT_TRUE(receiver_mailbox.Receive(from, to, message));
    ASSERT_EQ(sender_address.ip(), from.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(TestMessage, message);
  }

  // Outgoing Ack
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    receiver_socket.Await(to, buffer, length);
    ASSERT_EQ(sender_address.ip(), to.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(TestAckLength, length);
    ASSERT_ARRAY_EQ(length, TestAck, buffer);
  }

  sender_socket.Push(receiver_address, sender_address, &TestAck[0],
                     TestAckLength);

  // Incoming Ack
  {
    // Ensure message is removed from sender_mailbox's outgoing queue
    ASSERT_TRUE(sender_mailbox.Flushed());
  }
}

TEST(AsyncMailboxTest, UnicastRetry_DroppedMessage) {
  MockSocket sender_socket;
  AsyncMailbox sender_mailbox(sender_socket);
  Address sender_address(Localhost, 0);

  MockSocket receiver_socket;
  AsyncMailbox receiver_mailbox(receiver_socket);
  Address receiver_address(Localhost, 0);

  sender_mailbox.Send(receiver_address, TestMessage);

  // Outgoing Message
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    sender_socket.Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(TestPacketLength, length);
    ASSERT_ARRAY_EQ(length, TestPacket, buffer);
  }

  // Incoming Message NOT Received

  // Retry Message
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    sender_socket.Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(TestPacketLength, length);
    ASSERT_ARRAY_EQ(length, TestPacket, buffer);
  }

  receiver_socket.Push(sender_address, receiver_address, &TestPacket[0],
                       TestPacketLength);

  // Incoming Message
  {
    Address from;
    Address to;
    std::string message;
    ASSERT_TRUE(receiver_mailbox.Receive(from, to, message));
    ASSERT_EQ(sender_address.ip(), from.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(TestMessage, message);
  }

  // Outgoing Ack
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    receiver_socket.Await(to, buffer, length);
    ASSERT_EQ(sender_address.ip(), to.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(TestAckLength, length);
    ASSERT_ARRAY_EQ(length, TestAck, buffer);
  }

  sender_socket.Push(receiver_address, sender_address, &TestAck[0],
                     TestAckLength);

  // Incoming Ack
  {
    // Ensure message is removed from sender_mailbox's outgoing queue
    ASSERT_TRUE(sender_mailbox.Flushed());
  }
}

TEST(AsyncMailboxTest, UnicastRetry_DroppedAck) {
  MockSocket sender_socket;
  AsyncMailbox sender_mailbox(sender_socket);
  Address sender_address(Localhost, 0);

  MockSocket receiver_socket;
  AsyncMailbox receiver_mailbox(receiver_socket);
  Address receiver_address(Localhost, 0);

  sender_mailbox.Send(receiver_address, TestMessage);

  // Outgoing Message
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    sender_socket.Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(TestPacketLength, length);
    ASSERT_ARRAY_EQ(length, TestPacket, buffer);
  }

  receiver_socket.Push(sender_address, receiver_address, &TestPacket[0],
                       TestPacketLength);

  // Incoming Message
  {
    Address from;
    Address to;
    std::string message;
    ASSERT_TRUE(receiver_mailbox.Receive(from, to, message));
    ASSERT_EQ(sender_address.ip(), from.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(TestMessage, message);
  }

  // Outgoing Ack
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    receiver_socket.Await(to, buffer, length);
    ASSERT_EQ(sender_address.ip(), to.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(TestAckLength, length);
    ASSERT_ARRAY_EQ(length, TestAck, buffer);
  }

  // Incoming Ack NOT Received

  // Retry Message
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    sender_socket.Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(TestPacketLength, length);
    ASSERT_ARRAY_EQ(length, TestPacket, buffer);
  }

  receiver_socket.Push(sender_address, receiver_address, &TestPacket[0],
                       TestPacketLength);

  // Incoming Message Dropped
  {
    Address from;
    Address to;
    std::string message;
    ASSERT_FALSE(receiver_mailbox.Receive(from, to, message));
  }

  // Outgoing Ack
  {
    Address to;
    char buffer[MaxTestPayload];
    size_t length;
    receiver_socket.Await(to, buffer, length);
    ASSERT_EQ(sender_address.ip(), to.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(TestAckLength, length);
    ASSERT_ARRAY_EQ(length, TestAck, buffer);
  }

  sender_socket.Push(receiver_address, sender_address, &TestAck[0],
                     TestAckLength);

  // Incoming Ack
  {
    // Ensure message is removed from sender_mailbox's outgoing queue
    ASSERT_TRUE(sender_mailbox.Flushed());
  }
}

TEST(AsyncMailboxTest, MulticastDelivery_Thread) {
  Address receiver_address(Localhost, 0);
  UDPSocket receiver_socket(receiver_address);
  Address multicast_address(TestMulticastIP, TestPort);
  ASSERT_TRUE(receiver_socket.JoinGroup(multicast_address));
  AsyncMailbox receiver_mailbox(receiver_socket);

  std::thread worker{[multicast_address] {
    Address sender_address(Localhost, 0);
    UDPSocket sender_socket(sender_address);
    AsyncMailbox sender_mailbox(sender_socket);
    sender_mailbox.Send(multicast_address, TestMessage);
    while (!sender_mailbox.Flushed()) {
    }
  }};

  Address from;
  Address to;
  std::string message;
  bool success{false};
  for (uint8_t i{0}; i < MaxRetries && !success; ++i) {
    success = receiver_mailbox.Receive(from, to, message);
  }
  ASSERT_TRUE(success);
  ASSERT_EQ(multicast_address, to);
  ASSERT_EQ(TestMessage, message);
  worker.join();
}

TEST(AsyncMailboxTest, MulticastDelivery_Process) {
  Address receiver_address(Localhost, 0);
  UDPSocket receiver_socket(receiver_address);
  Address multicast_address(TestMulticastIP, TestPort);
  ASSERT_TRUE(receiver_socket.JoinGroup(multicast_address));
  AsyncMailbox receiver_mailbox(receiver_socket);

  // Fork child process
  pid_t pid{fork()};

  if (pid == 0) {
    Address sender_address(Localhost, 0);
    UDPSocket sender_socket(sender_address);
    AsyncMailbox sender_mailbox(sender_socket);
    sender_mailbox.Send(multicast_address, TestMessage);
    while (!sender_mailbox.Flushed()) {
    }
    _exit(0);
  } else {
    ASSERT_GT(pid, 0);
    Address from;
    Address to;
    std::string message;
    bool success{false};
    for (uint8_t i{0}; i < MaxRetries && !success; ++i) {
      success = receiver_mailbox.Receive(from, to, message);
    }
    ASSERT_TRUE(success);
    ASSERT_EQ(multicast_address, to);
    ASSERT_EQ(TestMessage, message);
  }
}

};  // namespace Wink::Test
