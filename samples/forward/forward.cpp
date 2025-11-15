// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 5) {
    Error() << "Incorrect parameters, expected <name> <address> <parent> "
               "<destination>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[1]);
  Address address(argv[2]);
  Address parent(argv[3]);
  Machine m(name, address, parent);

  Address destination(argv[4]);

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "main: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "main: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             m.Send(destination, os.str());
           }},
      }));

  m.Start();
}
