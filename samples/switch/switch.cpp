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
      "off",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Switch is OFF\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      {
          {"on", [&](const Address& sender,
                     std::istream& args) { m.Transition("on"); }},
          {"off", [&](const Address& sender,
                      std::istream& args) { m.Transition("off"); }},
      }));

  m.AddState(State(
      // State Name
      "on",
      // Parent State
      "off",
      // On Entry Action
      []() { Info() << "Switch is ON\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      {}));

  m.Start();
}
