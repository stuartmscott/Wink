// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
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
  Address parent(argv[3]);
  Machine m(name, address, parent);

  m.AddState(State(
      // State Name
      "off",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Switch is OFF" << std::endl; },
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
      []() { Info() << "Switch is ON" << std::endl; },
      // On Exit Action
      []() {},
      // Receivers
      {}));

  m.Start();
}
