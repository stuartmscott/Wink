#include <Wink/machine.h>
#include <WinkTest/constants.h>
#include <WinkTest/socket.h>
#include <gtest/gtest.h>

#include <thread>

TEST(MachineTest, UID) {
  // TODO Basic
  // TODO Tag
  // TODO Version
}

TEST(MachineTest, Start) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");
  // Set mock bind result
  {
    BindResult result;
    result.ip = kLocalhost;
    result.port = 42002;
    result.result = 0;
    socket.bindResults_.push_back(result);
  }
  // Set mock setreceivetimeout result
  {
    SetReceiveTimeoutResult result = 0;
    socket.setReceiveTimeoutResults_.push_back(result);
  }
  // Set mock send result
  {
    SendResult result = 0;
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestIP;
    result.fromPort = kTestPort;
    result.result = 0;
    std::ostringstream oss;
    oss << "started ";
    oss << kTestBinary;
    strcpy(result.buffer, oss.str().c_str());
    socket.receiveResults_.push_back(result);
  }

  int mainOnEntry = 0;
  int mainOnExit = 0;

  Machine m(name, socket, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(std::make_unique<State>(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        mainOnEntry++;
        m.Exit();
      },
      // On Exit Action
      [&]() { mainOnExit++; },
      // Receivers
      std::map<const std::string, Receiver>{}));
  m.Start();

  ASSERT_EQ("test/Test@127.0.0.1:42002", m.UID());

  // Bind call
  {
    ASSERT_EQ(1, socket.bindArgs_.size());
  }

  // Check socket receive timeout
  {
    ASSERT_EQ(1, socket.setReceiveTimeoutArgs_.size());
    const auto arg = socket.setReceiveTimeoutArgs_.at(0);
    ASSERT_EQ(kReplyTimeout, arg.seconds);
  }

  // Check socket send
  {
    ASSERT_EQ(4, socket.sendArgs_.size());
    // Send Started Message to Spawner
    {
      const auto arg0 = socket.sendArgs_.at(0);
      ASSERT_EQ(parent.ip(), arg0.toIP);
      ASSERT_EQ(parent.port(), arg0.toPort);
      ASSERT_EQ(std::string("started test/Test"), std::string(arg0.buffer));
      ASSERT_EQ(18, arg0.length);
    }
    // Register Machine
    {
      const auto arg1 = socket.sendArgs_.at(1);
      ASSERT_EQ(kLocalhost, arg1.toIP);
      ASSERT_EQ(kServerPort, arg1.toPort);
      std::string message(arg1.buffer);
      ASSERT_TRUE(message.starts_with("register test/Test "));
    }
    // Send Exited Message to Spawner
    {
      const auto arg2 = socket.sendArgs_.at(2);
      ASSERT_EQ(parent.ip(), arg2.toIP);
      ASSERT_EQ(parent.port(), arg2.toPort);
      ASSERT_EQ(std::string("exited test/Test"), std::string(arg2.buffer));
      ASSERT_EQ(17, arg2.length);
    }
    // Unregister Machine
    {
      const auto arg3 = socket.sendArgs_.at(3);
      ASSERT_EQ(kLocalhost, arg3.toIP);
      ASSERT_EQ(kServerPort, arg3.toPort);
      ASSERT_EQ(std::string("unregister"), std::string(arg3.buffer));
      ASSERT_EQ(11, arg3.length);
    }
  }
  // Initial State Transitions
  ASSERT_EQ(1, mainOnEntry);
  ASSERT_EQ(1, mainOnExit);
}

TEST(MachineTest, Start_InitialState) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");
  // Set mock bind result
  {
    BindResult result;
    result.ip = kLocalhost;
    result.port = 42002;
    result.result = 0;
    socket.bindResults_.push_back(result);
  }
  // Set mock setreceivetimeout result
  {
    SetReceiveTimeoutResult result = 0;
    socket.setReceiveTimeoutResults_.push_back(result);
  }
  // Set mock send result
  {
    SendResult result = 0;
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
    socket.sendResults_.push_back(result);
  }
  // Set mock receive result
  {
    ReceiveResult result;
    result.fromIP = kTestIP;
    result.fromPort = kTestPort;
    result.result = 0;
    std::ostringstream oss;
    oss << "started ";
    oss << kTestBinary;
    strcpy(result.buffer, oss.str().c_str());
    socket.receiveResults_.push_back(result);
  }

  int firstOnEntry = 0;
  int firstOnExit = 0;
  int secondOnEntry = 0;
  int secondOnExit = 0;

  Machine m(name, socket, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(std::make_unique<State>(
      // State Name
      "first",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        firstOnEntry++;
        Info() << "firstOnEntry: " << firstOnEntry << '\n' << std::flush;
        m.Exit();
      },
      // On Exit Action
      [&]() {
        firstOnExit++;
        Info() << "firstOnExit: " << firstOnExit << '\n' << std::flush;
      },
      // Receivers
      std::map<const std::string, Receiver>{}));
  m.AddState(std::make_unique<State>(
      // State Name
      "second",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        secondOnEntry++;
        Info() << "secondOnEntry: " << secondOnEntry << '\n' << std::flush;
        m.Transition("first");
      },
      // On Exit Action
      [&]() {
        secondOnExit++;
        Info() << "secondOnExit: " << secondOnExit << '\n' << std::flush;
      },
      // Receivers
      std::map<const std::string, Receiver>{}));
  m.Start("second");

  ASSERT_EQ("test/Test@127.0.0.1:42002", m.UID());

  // Bind call
  {
    ASSERT_EQ(1, socket.bindArgs_.size());
  }

  // Check socket receive timeout
  {
    ASSERT_EQ(1, socket.setReceiveTimeoutArgs_.size());
    const auto arg = socket.setReceiveTimeoutArgs_.at(0);
    ASSERT_EQ(kReplyTimeout, arg.seconds);
  }

  // Check socket send
  {
    ASSERT_EQ(4, socket.sendArgs_.size());
    // Send Started Message to Spawner
    {
      const auto arg0 = socket.sendArgs_.at(0);
      ASSERT_EQ(parent.ip(), arg0.toIP);
      ASSERT_EQ(parent.port(), arg0.toPort);
      ASSERT_EQ(std::string("started test/Test"), std::string(arg0.buffer));
      ASSERT_EQ(18, arg0.length);
    }
    // Register Machine
    {
      const auto arg1 = socket.sendArgs_.at(1);
      ASSERT_EQ(kLocalhost, arg1.toIP);
      ASSERT_EQ(kServerPort, arg1.toPort);
      std::string message(arg1.buffer);
      ASSERT_TRUE(message.starts_with("register test/Test "));
    }
    // Send Exited Message to Spawner
    {
      const auto arg2 = socket.sendArgs_.at(2);
      ASSERT_EQ(parent.ip(), arg2.toIP);
      ASSERT_EQ(parent.port(), arg2.toPort);
      ASSERT_EQ(std::string("exited test/Test"), std::string(arg2.buffer));
      ASSERT_EQ(17, arg2.length);
    }
    // Unregister Machine
    {
      const auto arg3 = socket.sendArgs_.at(3);
      ASSERT_EQ(kLocalhost, arg3.toIP);
      ASSERT_EQ(kServerPort, arg3.toPort);
      ASSERT_EQ(std::string("unregister"), std::string(arg3.buffer));
      ASSERT_EQ(11, arg3.length);
    }
  }
  // Initial State Transitions
  ASSERT_EQ(1, firstOnEntry);
  ASSERT_EQ(1, firstOnExit);
  ASSERT_EQ(1, secondOnEntry);
  ASSERT_EQ(1, secondOnExit);
}

TEST(MachineTest, Start_Heartbeat) {
  // Repeatedly
  // - sendPulseToSpawner
  // - checkHealthOfSpawned
  // - receiveMessage
}

TEST(MachineTest, Exit) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  socket.sendResults_.push_back(result);
  socket.sendResults_.push_back(result);

  Machine m(name, socket, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.Exit();

  // Check socket send
  ASSERT_EQ(2, socket.sendArgs_.size());
  // Send Exited Message to Spawner
  const auto arg0 = socket.sendArgs_.at(0);
  ASSERT_EQ(parent.ip(), arg0.toIP);
  ASSERT_EQ(parent.port(), arg0.toPort);
  ASSERT_EQ(std::string("exited test/Test"), std::string(arg0.buffer));
  ASSERT_EQ(17, arg0.length);
  // Unregister Machine
  const auto arg1 = socket.sendArgs_.at(1);
  ASSERT_EQ(kLocalhost, arg1.toIP);
  ASSERT_EQ(kServerPort, arg1.toPort);
  ASSERT_EQ(std::string("unregister"), std::string(arg1.buffer));
  ASSERT_EQ(11, arg1.length);
}

TEST(MachineTest, Error) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  socket.sendResults_.push_back(result);
  socket.sendResults_.push_back(result);
  socket.sendResults_.push_back(result);

  Machine m(name, socket, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.Error("AHHHH");

  // Check socket send
  ASSERT_EQ(3, socket.sendArgs_.size());
  // Send Errored Message to Spawner
  const auto arg0 = socket.sendArgs_.at(0);
  ASSERT_EQ(parent.ip(), arg0.toIP);
  ASSERT_EQ(parent.port(), arg0.toPort);
  ASSERT_EQ(std::string("errored test/Test AHHHH"), std::string(arg0.buffer));
  ASSERT_EQ(24, arg0.length);
  // Send Exited Message to Spawner
  const auto arg1 = socket.sendArgs_.at(1);
  ASSERT_EQ(parent.ip(), arg1.toIP);
  ASSERT_EQ(parent.port(), arg1.toPort);
  ASSERT_EQ(std::string("exited test/Test"), std::string(arg1.buffer));
  ASSERT_EQ(17, arg1.length);
  // Unregister Machine
  const auto arg2 = socket.sendArgs_.at(2);
  ASSERT_EQ(kLocalhost, arg2.toIP);
  ASSERT_EQ(kServerPort, arg2.toPort);
  ASSERT_EQ(std::string("unregister"), std::string(arg2.buffer));
  ASSERT_EQ(11, arg2.length);
}

TEST(MachineTest, AddState) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  socket.sendResults_.push_back(result);

  Machine m(name, socket, address, parent);
  // TODO Test state is added to the vector of states
  // TODO Test first state becomes initial default
}

TEST(MachineTest, Transition) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, socket, address, parent);

  int firstOnEntry = 0;
  int firstOnExit = 0;
  int secondOnEntry = 0;
  int secondOnExit = 0;

  m.AddState(std::make_unique<State>(
      // State Name
      "first",
      // Parent State
      "",
      // On Entry Action
      [&]() { firstOnEntry++; },
      // On Exit Action
      [&]() { firstOnExit++; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "second",
      // Parent State
      "",
      // On Entry Action
      [&]() { secondOnEntry++; },
      // On Exit Action
      [&]() { secondOnExit++; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.Transition("second");

  ASSERT_EQ(0, firstOnEntry);
  ASSERT_EQ(1, firstOnExit);
  ASSERT_EQ(1, secondOnEntry);
  ASSERT_EQ(0, secondOnExit);
}

TEST(MachineTest, Send) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  socket.sendResults_.push_back(result);

  Machine m(name, socket, address, parent);

  Address destination(":42003");
  m.Send(destination, kTestMessage);

  // Check socket send
  ASSERT_EQ(1, socket.sendArgs_.size());
  const auto arg = socket.sendArgs_.at(0);
  ASSERT_EQ(destination.ip(), arg.toIP);
  ASSERT_EQ(destination.port(), arg.toPort);
  ASSERT_EQ(std::string(kTestMessage), std::string(arg.buffer));
  ASSERT_EQ(10, arg.length);
}

TEST(MachineTest, SendAt) {
  std::string name("test/Test");
  UDPSocket socket;
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, socket, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(std::make_unique<State>(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      []() {},
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
      }));
  std::thread worker([&]() { m.Start(); });

  auto time = std::chrono::system_clock::now();
  time += std::chrono::seconds(1);
  m.SendAt(address, "exit", time);

  worker.join();
}

TEST(MachineTest, SendAfter) {
  std::string name("test/Test");
  UDPSocket socket;
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, socket, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(std::make_unique<State>(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      []() {},
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
      }));
  std::thread worker([&]() { m.Start(); });

  m.SendAfter(address, "exit", std::chrono::seconds(1));

  worker.join();
}

TEST(MachineTest, Spawn_Local) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  socket.sendResults_.push_back(result);

  Machine m(name, socket, address, parent);
  m.Spawn("useless/Useless");

  // Check socket send
  ASSERT_EQ(1, socket.sendArgs_.size());
  const auto arg = socket.sendArgs_.at(0);
  ASSERT_EQ(kLocalhost, arg.toIP);
  ASSERT_EQ(kServerPort, arg.toPort);
  ASSERT_EQ(std::string("start useless/Useless :0"), std::string(arg.buffer));
  ASSERT_EQ(25, arg.length);
}

TEST(MachineTest, Spawn_Remote) {
  std::string name("test/Test");
  MockSocket socket;
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  socket.sendResults_.push_back(result);

  Machine m(name, socket, address, parent);
  Address destination(kTestIP, kTestPort);
  m.Spawn("useless/Useless", destination);

  // Check socket send
  ASSERT_EQ(1, socket.sendArgs_.size());
  const auto arg = socket.sendArgs_.at(0);
  ASSERT_EQ(kTestIP, arg.toIP);
  ASSERT_EQ(kServerPort, arg.toPort);
  ASSERT_EQ(std::string("start useless/Useless :42424"),
            std::string(arg.buffer));
  ASSERT_EQ(29, arg.length);
}
