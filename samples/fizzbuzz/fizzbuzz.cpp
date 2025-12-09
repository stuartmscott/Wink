// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/mailbox.h>
#include <Wink/socket.h>
#include <Wink/state.h>

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
  UDPSocket socket(address);
  AsyncMailbox mailbox(socket);
  Address parent(argv[3]);
  Machine m(name, mailbox, address, parent);

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
          {"",
           [&](const Address& from, const Address& to, std::istream& args) {
             int n;
             args >> n;
             if (n % 15 == 0) {
               m.Transition("FizzBuzz");
             } else if (n % 5 == 0) {
               m.Transition("Buzz");
             } else if (n % 3 == 0) {
               m.Transition("Fizz");
             } else {
               Info() << n << std::endl;
             }
           }},
      }));

  m.AddState(State(
      // State Name
      "Fizz",
      // Parent State
      "main",
      // On Entry Action
      []() { Info() << "Fizz" << std::endl; },
      // On Exit Action
      []() {},
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "Buzz",
      // Parent State
      "main",
      // On Entry Action
      []() { Info() << "Buzz" << std::endl; },
      // On Exit Action
      []() {},
      // Receivers
      {}));

  m.AddState(State(
      // State Name
      "FizzBuzz",
      // Parent State
      "main",
      // On Entry Action
      []() { Info() << "FizzBuzz" << std::endl; },
      // On Exit Action
      []() {},
      // Receivers
      {}));

  m.Start();
}
