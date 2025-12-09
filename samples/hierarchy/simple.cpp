// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/mailbox.h>
#include <Wink/socket.h>
#include <Wink/state.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 4) {
    Error() << "Incorrect parameters, expected <name> <address> <parent>"
            << std::endl;
    return -1;
  }

  std::string name(argv[1]);
  Address address(argv[2]);
  UDPSocket socket(address);
  AsyncMailbox mailbox(socket);
  Address parent(argv[3]);
  Machine m(name, mailbox, address, parent);

  m.AddState(State(
      // State Name
      "Parent",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Parent: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Parent: OnExit" << std::endl; },
      // Receivers
      {
          {"goto",
           [&](const Address& from, const Address& to, std::istream& args) {
             std::string s;
             args >> s;
             m.Transition(s);
           }},
      }));

  m.AddState(State(
      // State Name
      "Leaf1",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Leaf1: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Leaf1: OnExit" << std::endl; },
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "Leaf2",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Leaf2: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Leaf2: OnExit" << std::endl; },
      // Receivers
      {}));

  m.Start("Leaf2");
}
