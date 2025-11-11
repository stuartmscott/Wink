// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
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
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, address, parent);

  m.AddState(State(
      // State Name
      "Leaf",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Leaf: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
      }));

  m.Start();
}
