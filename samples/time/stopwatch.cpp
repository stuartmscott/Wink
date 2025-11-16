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

  std::chrono::time_point<std::chrono::system_clock> start;

  m.AddState(State(
      // State Name
      "idle",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "StopWatch is IDLE" << std::endl; },
      // On Exit Action
      []() {},
      // Receivers
      {
          {"idle", [&](const Address& sender,
                       std::istream& args) { m.Transition("idle"); }},
          {"start",
           [&](const Address& sender, std::istream& args) {
             start = std::chrono::system_clock::now();
             m.Transition("timing");
           }},
          {"stop", [&](const Address& sender,
                       std::istream& args) { m.Transition("idle"); }},
      }));

  m.AddState(State(
      // State Name
      "timing",
      // Parent State
      "idle",
      // On Entry Action
      []() { Info() << "StopWatch is TIMING" << std::endl; },
      // On Exit Action
      []() {},
      // Receivers
      {
          {"stop",
           [&](const Address& sender, std::istream& args) {
             const auto now = std::chrono::system_clock::now();
             const auto delta =
                 std::chrono::floor<std::chrono::seconds>(now - start).count();
             std::ostringstream oss;
             oss << "elapsed ";
             oss << delta;
             oss << " seconds";
             m.Send(sender, oss.str());
             m.Transition("idle");
           }},
      }));

  m.Start();
}
