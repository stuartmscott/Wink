// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <chrono>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 4) {
    Error() << "Incorrect parameters, expected <name> <address> <parent>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[1]);
  Address address(argv[2]);
  Address parent(argv[3]);
  Machine m(name, address, parent);

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        Info() << "main: OnEntry\n" << std::flush;
        // Schedule message to be sent to self after 10s
        m.SendAfter(address, "exit", std::chrono::seconds(10));
      },
      // On Exit Action
      []() { Info() << "main: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
      }));

  m.Start();
}
