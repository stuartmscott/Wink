// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <WinkServer/server.h>
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>
#include <gtest/gtest.h>

#include <string>

TEST(ServerTest, Registration) {
  Address server_address(kLocalhost, kServerPort);
  UDPSocket server_socket(server_address);
  AsyncMailbox server_mailbox(server_socket);
  Server server(server_address, server_mailbox);

  std::thread worker{[&server] { server.Serve("../../samples/"); }};

  Address client_address(kLocalhost, 0);
  UDPSocket client_socket(client_address);
  AsyncMailbox client_mailbox(client_socket);

  Address from;
  Address to;
  std::string message;

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 0 Machines
  ASSERT_TRUE(client_mailbox.Receive(from, to, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ(client_address, to);
  ASSERT_EQ("Port,PID,Machine", message);

  // Register Machine
  client_mailbox.Send(server_address, "register useless/Useless 12345");

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 1 Machine
  ASSERT_TRUE(client_mailbox.Receive(from, to, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ(client_address, to);
  ASSERT_EQ("Port,PID,Machine\n" + std::to_string(client_address.port()) +
                ",12345,useless/Useless",
            message);

  // Unregister Machine
  client_mailbox.Send(server_address, "unregister useless/Useless 12345");

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 0 Machines
  ASSERT_TRUE(client_mailbox.Receive(from, to, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ(client_address, to);
  ASSERT_EQ("Port,PID,Machine", message);

  server.Shutdown();
  worker.join();
}

TEST(ServerTest, StartListStop) {
  Address server_address(kLocalhost, kServerPort);
  UDPSocket server_socket(server_address);
  AsyncMailbox server_mailbox(server_socket);
  Server server(server_address, server_mailbox);

  std::thread worker{[&server] { server.Serve("../../samples/"); }};

  Address client_address(kLocalhost, 0);
  UDPSocket client_socket(client_address);
  AsyncMailbox client_mailbox(client_socket);

  Address from;
  Address to;
  std::string message;

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 0 Machines
  ASSERT_TRUE(client_mailbox.Receive(from, to, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ(client_address, to);
  ASSERT_EQ("Port,PID,Machine", message);

  // Start Machine
  client_mailbox.Send(server_address, "start time/After#foobar :42424");

  sleep(1);

  // Assert Machine Started
  ASSERT_TRUE(client_mailbox.Receive(from, to, message));
  ASSERT_EQ(kLocalhost, from.ip());
  ASSERT_EQ(42424, from.port());
  ASSERT_EQ(client_address, to);
  ASSERT_EQ("started time/After#foobar", message);

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 1 Machine
  ASSERT_TRUE(client_mailbox.Receive(from, to, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ(client_address, to);
  const std::string prefix("Port,PID,Machine\n42424,");
  ASSERT_TRUE(message.starts_with(prefix));
  const std::string suffix(",time/After#foobar");
  ASSERT_TRUE(message.ends_with(suffix));

  // Stop Machine
  client_mailbox.Send(server_address, "stop 42424");

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 0 Machines
  ASSERT_TRUE(client_mailbox.Receive(from, to, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ(client_address, to);
  ASSERT_EQ("Port,PID,Machine", message);

  server.Shutdown();
  worker.join();
}
