// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
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
  Address parent(argv[3]);
  Machine m(name, address, parent);

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        Info() << "Parent: OnEntry" << std::endl;
        // Spawn two identical children, differentiated by a tag
        m.Spawn("family/Child#Alice");
        m.Spawn("family/Child#Bob");
      },
      // On Exit Action
      []() { Info() << "Parent: OnExit" << std::endl; },
      // Receivers
      {
          {"started",
           [&](const Address& sender, std::istream& args) {
             std::string child;
             args >> child;
             Info() << "Parent: " << sender << ' ' << child << " has started"
                    << std::endl;
           }},
          {"pulsed",
           [&](const Address& sender, std::istream& args) {
             std::string child;
             args >> child;
             Info() << "Parent: " << sender << ' ' << child << " has pulsed"
                    << std::endl;
           }},
          {"errored",
           [&](const Address& sender, std::istream& args) {
             std::string child;
             args >> child;
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Parent: " << sender << ' ' << child
                    << " has errored: " << os.str() << std::endl;
           }},
          {"exited",
           [&](const Address& sender, std::istream& args) {
             std::string child;
             args >> child;
             Info() << "Parent: " << sender << ' ' << child << " has exited"
                    << std::endl;
             m.Spawn(child);  // Retry
           }},
      }));

  m.Start();
}
