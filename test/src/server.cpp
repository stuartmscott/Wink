// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/server/server.h>
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>
#include <gtest/gtest.h>

#include <string>

// Paths assume tests were executed from inside build/test/src
TEST(ServerTest, Registration) {
  Address server_address(kLocalhost, kServerPort);
  AsyncMailbox server_mailbox(new UDPSocket(server_address));
  Server server(server_address, server_mailbox);

  std::thread worker{[&server] { server.Serve("../../samples/"); }};

  Address client_address(kLocalhost, 0);
  AsyncMailbox client_mailbox(new UDPSocket(client_address));

  Address from;
  std::string message;

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 0 Machines
  ASSERT_TRUE(client_mailbox.Receive(from, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ("Port,PID,Machine", message);

  // Register Machine
  client_mailbox.Send(server_address, "register useless/Useless 12345");

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 1 Machine
  ASSERT_TRUE(client_mailbox.Receive(from, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ("Port,PID,Machine\n" + std::to_string(client_address.port()) +
                ",12345,useless/Useless",
            message);

  // Unregister Machine
  client_mailbox.Send(server_address, "unregister useless/Useless 12345");

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 0 Machines
  ASSERT_TRUE(client_mailbox.Receive(from, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ("Port,PID,Machine", message);

  server.Shutdown();
  worker.join();
}

TEST(ServerTest, StartListStop) {
  Address server_address(kLocalhost, kServerPort);
  AsyncMailbox server_mailbox(new UDPSocket(server_address));
  Server server(server_address, server_mailbox);

  std::thread worker{[&server] { server.Serve("../../samples/"); }};

  Address client_address(kLocalhost, 0);
  AsyncMailbox client_mailbox(new UDPSocket(client_address));

  Address from;
  std::string message;

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 0 Machines
  ASSERT_TRUE(client_mailbox.Receive(from, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ("Port,PID,Machine", message);

  // Start Machine
  client_mailbox.Send(server_address, "start time/After#foobar :42424");

  // Assert Machine Started
  ASSERT_TRUE(client_mailbox.Receive(from, message));
  Info() << "Received: " << from << ": " << message << '\n' << std::flush;
  ASSERT_EQ(kLocalhost, from.ip());
  ASSERT_EQ(42424, from.port());
  ASSERT_EQ("started time/After#foobar", message);

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 1 Machine
  ASSERT_TRUE(client_mailbox.Receive(from, message));
  ASSERT_EQ(server_address, from);
  const std::string prefix("Port,PID,Machine\n42424,");
  ASSERT_TRUE(message.starts_with(prefix));
  const std::string suffix(",time/After#foobar");
  ASSERT_TRUE(message.ends_with(suffix));

  // Stop Machine
  client_mailbox.Send(server_address, "stop 42424");

  // List Machines
  client_mailbox.Send(server_address, "list");

  // Assert 0 Machines
  ASSERT_TRUE(client_mailbox.Receive(from, message));
  ASSERT_EQ(server_address, from);
  ASSERT_EQ("Port,PID,Machine", message);

  server.Shutdown();
  worker.join();
}
