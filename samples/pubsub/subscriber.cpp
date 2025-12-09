// Copyright 2022-2025 Stuart Scott
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
               "<publisher>"
            << std::endl;
    return -1;
  }

  std::string name(argv[1]);
  Address address(argv[2]);
  UDPSocket socket(address);
  AsyncMailbox mailbox(socket);
  Address parent(argv[3]);
  Machine m(name, mailbox, address, parent);

  Address publisher(argv[4]);

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        Info() << "main: OnEntry" << std::endl;
        // Send message to subscribe to publisher
        m.Send(publisher, "subscribe");
        // Schedule message to unsubscribe from publisher after 10s
        m.SendAfter(publisher, "unsubscribe", std::chrono::seconds(10));
        // Schedule message to exit after 15s
        m.SendAfter(address, "exit", std::chrono::seconds(15));
      },
      // On Exit Action
      []() { Info() << "main: OnExit" << std::endl; },
      // Receivers
      {
          {"update",
           [&](const Address& from, const Address& to, std::istream& args) {
             std::string payload;
             args >> payload;
             Info() << from << " updated " << name << ": " << payload
                    << std::endl;
           }},
      }));

  m.Start();
}
