// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <chrono>
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
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        Info() << "main: OnEntry" << std::endl;
        // Schedule message to be sent to self at the start of the next minute
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::ceil<std::chrono::minutes>(now);
        m.SendAt(address, "exit", time);
      },
      // On Exit Action
      []() { Info() << "main: OnExit" << std::endl; },
      // Receivers
      {}));

  m.Start();
}
