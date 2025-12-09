// Copyright 2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/mailbox.h>
#include <Wink/socket.h>
#include <Wink/state.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 5) {
    Error() << "Incorrect parameters, expected <name> <address> <parent> "
               "<multicast>"
            << std::endl;
    return -1;
  }

  std::string name(argv[1]);
  Address address(argv[2]);
  UDPSocket socket(address);
  AsyncMailbox mailbox(socket);
  Address parent(argv[3]);
  Address multicast(argv[4]);
  Machine m(name, mailbox, address, parent);

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        Info() << "main: OnEntry" << std::endl;
        // Schedule message to be sent to self after 10s
        m.SendAfter(address, "multicast", std::chrono::seconds(10));
      },
      // On Exit Action
      []() { Info() << "main: OnExit" << std::endl; },
      // Receivers
      {
          {"multicast",
           [&](const Address& from, const Address& to, std::istream& args) {
             m.Send(multicast, "hello");
             m.Exit();
           }},
      }));

  m.Start();
}
