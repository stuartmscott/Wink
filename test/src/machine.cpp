// Copyright 2022-2025 Stuart Scott
#include <Wink/machine.h>
#include <WinkTest/constants.h>
#include <WinkTest/mailbox.h>
#include <gtest/gtest.h>

#include <string>
#include <thread>
#include <vector>

TEST(MachineTest, UID) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  setup_default_mailbox(*mailbox);
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, mailbox, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&m]() { m.Exit(); },
      // On Exit Action
      []() {},
      // Receivers
      {}));
  m.Start();

  ASSERT_EQ("test/Test@127.0.0.1:42002", m.UID());
}

TEST(MachineTest, UID_Tag) {
  std::string name("test/Test#Tag");
  auto mailbox = new MockMailbox();
  setup_default_mailbox(*mailbox);
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, mailbox, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&m]() { m.Exit(); },
      // On Exit Action
      []() {},
      // Receivers
      {}));
  m.Start();

  ASSERT_EQ("test/Test#Tag@127.0.0.1:42002", m.UID());
}

TEST(MachineTest, Start) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  setup_default_mailbox(*mailbox);
  Address address(":42002");
  Address parent(":42001");

  int mainOnEntry = 0;
  int mainOnExit = 0;

  Machine m(name, mailbox, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&m, &mainOnEntry]() {
        mainOnEntry++;
        m.Exit();
      },
      // On Exit Action
      [&mainOnExit]() { mainOnExit++; },
      // Receivers
      {}));
  m.Start();

  ASSERT_EQ("test/Test@127.0.0.1:42002", m.UID());

  assert_default_mailbox(*mailbox, parent);

  // Initial State Transitions
  ASSERT_EQ(1, mainOnEntry);
  ASSERT_EQ(1, mainOnExit);
}

TEST(MachineTest, Start_InitialState) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  setup_default_mailbox(*mailbox);
  Address address(":42002");
  Address parent(":42001");

  int firstOnEntry = 0;
  int firstOnExit = 0;
  int secondOnEntry = 0;
  int secondOnExit = 0;

  Machine m(name, mailbox, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(State(
      // State Name
      "first",
      // Parent State
      "",
      // On Entry Action
      [&m, &firstOnEntry]() {
        firstOnEntry++;
        Info() << "firstOnEntry: " << firstOnEntry << '\n' << std::flush;
        m.Exit();
      },
      // On Exit Action
      [&firstOnExit]() {
        firstOnExit++;
        Info() << "firstOnExit: " << firstOnExit << '\n' << std::flush;
      },
      // Receivers
      {}));
  m.AddState(State(
      // State Name
      "second",
      // Parent State
      "",
      // On Entry Action
      [&m, &secondOnEntry]() {
        secondOnEntry++;
        Info() << "secondOnEntry: " << secondOnEntry << '\n' << std::flush;
        m.Transition("first");
      },
      // On Exit Action
      [&secondOnExit]() {
        secondOnExit++;
        Info() << "secondOnExit: " << secondOnExit << '\n' << std::flush;
      },
      // Receivers
      {}));
  m.Start("second");

  ASSERT_EQ("test/Test@127.0.0.1:42002", m.UID());

  assert_default_mailbox(*mailbox, parent);

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
  auto mailbox = new MockMailbox();
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  mailbox->sendResults_.push_back(result);
  mailbox->sendResults_.push_back(result);

  Machine m(name, mailbox, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.Exit();

  // Check mailbox send
  ASSERT_EQ(2, mailbox->sendArgs_.size());
  // Send Exited Message to Spawner
  const auto arg0 = mailbox->sendArgs_.at(0);
  ASSERT_EQ(parent.ip(), arg0.toIP);
  ASSERT_EQ(parent.port(), arg0.toPort);
  ASSERT_EQ(std::string("exited test/Test"), arg0.message);
  // Unregister Machine
  const auto arg1 = mailbox->sendArgs_.at(1);
  ASSERT_EQ(kLocalhost, arg1.toIP);
  ASSERT_EQ(kServerPort, arg1.toPort);
  ASSERT_EQ(std::string("unregister"), arg1.message);
}

TEST(MachineTest, Error) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  mailbox->sendResults_.push_back(result);
  mailbox->sendResults_.push_back(result);
  mailbox->sendResults_.push_back(result);

  Machine m(name, mailbox, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.Error("AHHHH");

  // Check mailbox send
  ASSERT_EQ(3, mailbox->sendArgs_.size());
  // Send Errored Message to Spawner
  const auto arg0 = mailbox->sendArgs_.at(0);
  ASSERT_EQ(parent.ip(), arg0.toIP);
  ASSERT_EQ(parent.port(), arg0.toPort);
  ASSERT_EQ(std::string("errored test/Test AHHHH"), arg0.message);
  // Send Exited Message to Spawner
  const auto arg1 = mailbox->sendArgs_.at(1);
  ASSERT_EQ(parent.ip(), arg1.toIP);
  ASSERT_EQ(parent.port(), arg1.toPort);
  ASSERT_EQ(std::string("exited test/Test"), arg1.message);
  // Unregister Machine
  const auto arg2 = mailbox->sendArgs_.at(2);
  ASSERT_EQ(kLocalhost, arg2.toIP);
  ASSERT_EQ(kServerPort, arg2.toPort);
  ASSERT_EQ(std::string("unregister"), arg2.message);
}

TEST(MachineTest, AddState) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  mailbox->sendResults_.push_back(result);

  Machine m(name, mailbox, address, parent);
  // TODO Test state is added to the vector of states
  // TODO Test first state becomes initial default
}

TEST(MachineTest, Transition) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, mailbox, address, parent);

  int firstOnEntry = 0;
  int firstOnExit = 0;
  int secondOnEntry = 0;
  int secondOnExit = 0;

  m.AddState(State(
      // State Name
      "first",
      // Parent State
      "",
      // On Entry Action
      [&firstOnEntry]() { firstOnEntry++; },
      // On Exit Action
      [&firstOnExit]() { firstOnExit++; },
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "second",
      // Parent State
      "",
      // On Entry Action
      [&secondOnEntry]() { secondOnEntry++; },
      // On Exit Action
      [&secondOnExit]() { secondOnExit++; },
      // Receivers
      {}));

  m.Transition("second");

  ASSERT_EQ(0, firstOnEntry);
  ASSERT_EQ(1, firstOnExit);
  ASSERT_EQ(1, secondOnEntry);
  ASSERT_EQ(0, secondOnExit);
}

TEST(MachineTest, Transition_Hierarchy) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, mailbox, address, parent);

  std::vector<int> entries;
  std::vector<int> exits;

  m.AddState(State(
      // State Name
      "first",
      // Parent State
      "",
      // On Entry Action
      [&entries]() { entries.push_back(1); },
      // On Exit Action
      [&exits]() { exits.push_back(1); },
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "second",
      // Parent State
      "first",
      // On Entry Action
      [&entries]() { entries.push_back(2); },
      // On Exit Action
      [&exits]() { exits.push_back(2); },
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "third",
      // Parent State
      "first",
      // On Entry Action
      [&entries]() { entries.push_back(3); },
      // On Exit Action
      [&exits]() { exits.push_back(3); },
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "forth",
      // Parent State
      "second",
      // On Entry Action
      [&entries]() { entries.push_back(4); },
      // On Exit Action
      [&exits]() { exits.push_back(4); },
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "fifth",
      // Parent State
      "second",
      // On Entry Action
      [&entries]() { entries.push_back(5); },
      // On Exit Action
      [&exits]() { exits.push_back(5); },
      // Receivers
      {}));

  m.Transition("second");

  // Although 'first' is the initial state, the
  // machine hasn't started yet so it wasn't entered.

  ASSERT_EQ(std::vector<int>{2}, entries);
  ASSERT_EQ(std::vector<int>{}, exits);

  entries.clear();
  exits.clear();

  m.Transition("forth");

  ASSERT_EQ(std::vector<int>{4}, entries);
  ASSERT_EQ(std::vector<int>{}, exits);

  entries.clear();
  exits.clear();

  m.Transition("fifth");

  ASSERT_EQ(std::vector<int>{5}, entries);
  ASSERT_EQ(std::vector<int>{4}, exits);

  entries.clear();
  exits.clear();

  m.Transition("third");

  ASSERT_EQ(std::vector<int>{3}, entries);
  ASSERT_EQ((std::vector<int>{5, 2}), exits);

  entries.clear();
  exits.clear();

  m.Transition("second");

  ASSERT_EQ(std::vector<int>{2}, entries);
  ASSERT_EQ(std::vector<int>{3}, exits);

  entries.clear();
  exits.clear();

  m.Transition("first");

  ASSERT_EQ(std::vector<int>{}, entries);
  ASSERT_EQ(std::vector<int>{2}, exits);
}

TEST(MachineTest, Send) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  mailbox->sendResults_.push_back(result);

  Machine m(name, mailbox, address, parent);

  Address destination(":42003");
  m.Send(destination, kTestMessage);

  // Check mailbox send
  ASSERT_EQ(1, mailbox->sendArgs_.size());
  const auto arg = mailbox->sendArgs_.at(0);
  ASSERT_EQ(destination.ip(), arg.toIP);
  ASSERT_EQ(destination.port(), arg.toPort);
  ASSERT_EQ(std::string(kTestMessage), arg.message);
}

TEST(MachineTest, SendAt) {
  std::string name("test/Test");
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      []() {},
      // On Exit Action
      []() {},
      // Receivers
      {
          {"exit",
           [&m](const Address& sender, std::istream& args) { m.Exit(); }},
      }));
  std::thread worker([&]() { m.Start(); });

  auto time = std::chrono::system_clock::now();
  time += std::chrono::seconds(1);
  m.SendAt(address, "exit", time);

  worker.join();
}

TEST(MachineTest, SendAfter) {
  std::string name("test/Test");
  Address address(":42002");
  Address parent(":42001");

  Machine m(name, address, parent);
  // Override exit
  m.on_exit_ = []() {};
  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      []() {},
      // On Exit Action
      []() {},
      // Receivers
      {
          {"exit",
           [&m](const Address& sender, std::istream& args) { m.Exit(); }},
      }));
  std::thread worker([&]() { m.Start(); });

  m.SendAfter(address, "exit", std::chrono::seconds(1));

  worker.join();
}

TEST(MachineTest, Spawn_Local) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  mailbox->sendResults_.push_back(result);

  Machine m(name, mailbox, address, parent);
  m.Spawn("useless/Useless");

  // Check mailbox send
  ASSERT_EQ(1, mailbox->sendArgs_.size());
  const auto arg = mailbox->sendArgs_.at(0);
  ASSERT_EQ(kLocalhost, arg.toIP);
  ASSERT_EQ(kServerPort, arg.toPort);
  ASSERT_EQ(std::string("start useless/Useless :0"), arg.message);
}

TEST(MachineTest, Spawn_Remote) {
  std::string name("test/Test");
  auto mailbox = new MockMailbox();
  Address address(":42002");
  Address parent(":42001");

  // Set mock send result
  SendResult result = 0;
  mailbox->sendResults_.push_back(result);

  Machine m(name, mailbox, address, parent);
  Address destination(kTestIP, kTestPort);
  m.Spawn("useless/Useless", destination);

  // Check mailbox send
  ASSERT_EQ(1, mailbox->sendArgs_.size());
  const auto arg = mailbox->sendArgs_.at(0);
  ASSERT_EQ(kTestIP, arg.toIP);
  ASSERT_EQ(kServerPort, arg.toPort);
  ASSERT_EQ(std::string("start useless/Useless :42424"), arg.message);
}
