// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
  if (argc < 5) {
    Error() << "Incorrect parameters, expected <name> <address> <parent> "
               "<interval>"
            << std::endl;
    return -1;
  }

  std::string name(argv[1]);
  Address address(argv[2]);
  Address parent(argv[3]);
  Machine m(name, address, parent);

  const std::chrono::seconds interval(std::stoi(argv[4]));

  std::atomic_bool running = true;

  std::thread worker([&]() {
    while (running) {
      std::this_thread::sleep_for(interval);
      if (running) {
        m.Send(parent, "tick " + name);
      }
    }
  });

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "main: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "main: OnExit" << std::endl; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) {
             running = false;
             worker.join();
           }},
      }));

  m.Start();
}
