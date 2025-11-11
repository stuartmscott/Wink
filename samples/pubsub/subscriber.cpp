// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 3) {
    Error() << "Incorrect parameters, expected <address> <parent> <publisher>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[0]);
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, address, parent);

  Address publisher(argv[3]);

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        Info() << "main: OnEntry\n" << std::flush;
        // Send message to subscribe to publisher
        m.Send(publisher, "subscribe");
        // Schedule message to unsubscribe from publisher after 10s
        m.SendAfter(publisher, "unsubscribe", std::chrono::seconds(10));
        // Schedule message to exit after 15s
        m.SendAfter(address, "exit", std::chrono::seconds(15));
      },
      // On Exit Action
      []() { Info() << "main: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"update",
           [&](const Address& sender, std::istream& args) {
             std::string payload;
             args >> payload;
             Info() << sender << " updated " << name << ": " << payload << '\n'
                    << std::flush;
           }},
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
      }));

  m.Start();
}
