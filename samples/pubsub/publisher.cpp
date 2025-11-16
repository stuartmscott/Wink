// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <iostream>
#include <set>
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

  std::set<Address> subscribers;

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() { Info() << "Publisher: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Publisher: OnExit" << std::endl; },
      // Receivers
      {
          {"subscribe",
           [&](const Address& sender, std::istream& args) {
             if (subscribers.insert(sender).second) {
               Info() << "Publisher: subscribed: " << sender << std::endl;
             }
           }},
          {"unsubscribe",
           [&](const Address& sender, std::istream& args) {
             if (subscribers.erase(sender)) {
               Info() << "Publisher: unsubscribed: " << sender << std::endl;
             }
           }},
          {"publish",
           [&](const Address& sender, std::istream& args) {
             std::string payload;
             args >> payload;
             Info() << "Publisher: publish " << payload << std::endl;
             std::ostringstream os;
             os << "update ";
             os << payload;
             const auto message = os.str();
             for (const auto& s : subscribers) {
               m.Send(s, message);
             }
           }},
      }));

  m.Start();
}
