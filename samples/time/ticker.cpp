// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
  if (argc < 4) {
    Error() << "Incorrect parameters, expected <address> <parent> <interval>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[0]);
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, address, parent);

  const std::chrono::seconds interval(std::stoi(argv[3]));

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
      []() { Info() << "main: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "main: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) {
             running = false;
             worker.join();
             m.Exit();
           }},
      }));

  m.Start();
}
