#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 3) {
    Error() << "Incorrect parameters, expected <address> <parent>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[0]);
  UDPSocket socket;
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, socket, address, parent);

  m.AddState(State(
      // State Name
      "Parent",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Parent: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Parent: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"goto",
           [&](const Address& sender, std::istream& args) {
             std::string s;
             args >> s;
             m.Transition(s);
           }},
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
      }));

  m.AddState(State(
      // State Name
      "Leaf1",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Leaf1: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf1: OnExit\n"
                    << std::flush; },
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "Leaf2",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Leaf2: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf2: OnExit\n"
                    << std::flush; },
      // Receivers
      {}));

  m.Start("Leaf2");
}
