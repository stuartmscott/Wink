// Copyright 2022-2025 Stuart Scott
#include <Wink/constants.h>
#include <Wink/mailbox.h>
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>
#include <WinkTest/utils.h>
#include <gtest/gtest.h>

#include <string>

TEST(AsyncMailboxTest, Delivery_Thread) {
  Address receiver_address(kLocalhost, 0);
  AsyncMailbox receiver_mailbox(new UDPSocket(receiver_address));

  std::thread worker{[receiver_address] {
    // Thread sends message after delay
    sleep(1);
    Address sender_address(kLocalhost, 0);
    AsyncMailbox sender_mailbox(new UDPSocket(sender_address));
    sender_mailbox.Send(receiver_address, kTestMessage);
    while (!sender_mailbox.Flushed()) {
    }
  }};

  Address from;
  std::string message;
  bool success = false;
  for (uint8_t i = 0; i < kMaxRetries && !success; i++) {
    success = receiver_mailbox.Receive(from, message);
  }
  ASSERT_TRUE(success);
  ASSERT_EQ(kTestMessage, message);
  worker.join();
}

TEST(AsyncMailboxTest, Delivery_Process) {
  Address receiver_address(kLocalhost, 0);
  AsyncMailbox receiver_mailbox(new UDPSocket(receiver_address));

  // Fork child process
  pid_t pid = fork();

  if (pid == 0) {
    // Child sends message after delay
    sleep(1);
    Address sender_address(kLocalhost, 0);
    AsyncMailbox sender_mailbox(new UDPSocket(sender_address));
    sender_mailbox.Send(receiver_address, kTestMessage);
    while (!sender_mailbox.Flushed()) {
    }
    _exit(0);
  } else {
    ASSERT_GT(pid, 0);
    Address from;
    std::string message;
    bool success = false;
    for (uint8_t i = 0; i < kMaxRetries && !success; i++) {
      success = receiver_mailbox.Receive(from, message);
    }
    ASSERT_TRUE(success);
    ASSERT_EQ(kTestMessage, message);
  }
}

TEST(AsyncMailboxTest, Timeout) {
  Address address(kLocalhost, 0);
  AsyncMailbox mailbox(new UDPSocket(address));

  Address from;
  std::string message;
  ASSERT_FALSE(mailbox.Receive(from, message));
}

TEST(AsyncMailboxTest, Acknowledgement) {
  auto sender_socket = new MockSocket();
  AsyncMailbox sender_mailbox(sender_socket);
  Address sender_address(kLocalhost, 0);

  auto receiver_socket = new MockSocket();
  AsyncMailbox receiver_mailbox(receiver_socket);
  Address receiver_address(kLocalhost, 0);

  sender_mailbox.Send(receiver_address, kTestMessage);

  // Outgoing Message
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    sender_socket->Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(kTestPacketLength, length);
    ASSERT_ARRAY_EQ(length, kTestPacket, buffer);
  }

  receiver_socket->Push(sender_address, &kTestPacket[0], kTestPacketLength);

  // Incoming Message
  {
    Address from;
    std::string message;
    ASSERT_TRUE(receiver_mailbox.Receive(from, message));
    ASSERT_EQ(sender_address.ip(), from.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(kTestMessage, message);
  }

  // Outgoing Ack
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    receiver_socket->Await(to, buffer, length);
    ASSERT_EQ(sender_address.ip(), to.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(kTestAckLength, length);
    ASSERT_ARRAY_EQ(length, kTestAck, buffer);
  }

  sender_socket->Push(receiver_address, &kTestAck[0], kTestAckLength);

  // Incoming Ack
  {
    // Ensure message is removed from sender_mailbox's outgoing queue
    ASSERT_TRUE(sender_mailbox.Flushed());
  }
}

TEST(AsyncMailboxTest, Retry_DroppedMessage) {
  auto sender_socket = new MockSocket();
  AsyncMailbox sender_mailbox(sender_socket);
  Address sender_address(kLocalhost, 0);

  auto receiver_socket = new MockSocket();
  AsyncMailbox receiver_mailbox(receiver_socket);
  Address receiver_address(kLocalhost, 0);

  sender_mailbox.Send(receiver_address, kTestMessage);

  // Outgoing Message
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    sender_socket->Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(kTestPacketLength, length);
    ASSERT_ARRAY_EQ(length, kTestPacket, buffer);
  }

  // Incoming Message NOT Received

  // Retry Message
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    sender_socket->Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(kTestPacketLength, length);
    ASSERT_ARRAY_EQ(length, kTestPacket, buffer);
  }

  receiver_socket->Push(sender_address, &kTestPacket[0], kTestPacketLength);

  // Incoming Message
  {
    Address from;
    std::string message;
    ASSERT_TRUE(receiver_mailbox.Receive(from, message));
    ASSERT_EQ(sender_address.ip(), from.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(kTestMessage, message);
  }

  // Outgoing Ack
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    receiver_socket->Await(to, buffer, length);
    ASSERT_EQ(sender_address.ip(), to.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(kTestAckLength, length);
    ASSERT_ARRAY_EQ(length, kTestAck, buffer);
  }

  sender_socket->Push(receiver_address, &kTestAck[0], kTestAckLength);

  // Incoming Ack
  {
    // Ensure message is removed from sender_mailbox's outgoing queue
    ASSERT_TRUE(sender_mailbox.Flushed());
  }
}

TEST(AsyncMailboxTest, Retry_DroppedAck) {
  auto sender_socket = new MockSocket();
  AsyncMailbox sender_mailbox(sender_socket);
  Address sender_address(kLocalhost, 0);

  auto receiver_socket = new MockSocket();
  AsyncMailbox receiver_mailbox(receiver_socket);
  Address receiver_address(kLocalhost, 0);

  sender_mailbox.Send(receiver_address, kTestMessage);

  // Outgoing Message
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    sender_socket->Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(kTestPacketLength, length);
    ASSERT_ARRAY_EQ(length, kTestPacket, buffer);
  }

  receiver_socket->Push(sender_address, &kTestPacket[0], kTestPacketLength);

  // Incoming Message
  {
    Address from;
    std::string message;
    ASSERT_TRUE(receiver_mailbox.Receive(from, message));
    ASSERT_EQ(sender_address.ip(), from.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(kTestMessage, message);
  }

  // Outgoing Ack
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    receiver_socket->Await(to, buffer, length);
    ASSERT_EQ(sender_address.ip(), to.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(kTestAckLength, length);
    ASSERT_ARRAY_EQ(length, kTestAck, buffer);
  }

  // Incoming Ack NOT Received

  // Retry Message
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    sender_socket->Await(to, buffer, length);
    ASSERT_EQ(receiver_address, to);
    ASSERT_EQ(kTestPacketLength, length);
    ASSERT_ARRAY_EQ(length, kTestPacket, buffer);
  }

  receiver_socket->Push(sender_address, &kTestPacket[0], kTestPacketLength);

  // Incoming Message Dropped
  {
    Address from;
    std::string message;
    ASSERT_FALSE(receiver_mailbox.Receive(from, message));
  }

  // Outgoing Ack
  {
    Address to;
    char buffer[kMaxTestPayload];
    size_t length;
    receiver_socket->Await(to, buffer, length);
    ASSERT_EQ(sender_address.ip(), to.ip());
    // Sender's port is not bound, so dont assert value
    ASSERT_EQ(kTestAckLength, length);
    ASSERT_ARRAY_EQ(length, kTestAck, buffer);
  }

  sender_socket->Push(receiver_address, &kTestAck[0], kTestAckLength);

  // Incoming Ack
  {
    // Ensure message is removed from sender_mailbox's outgoing queue
    ASSERT_TRUE(sender_mailbox.Flushed());
  }
}
