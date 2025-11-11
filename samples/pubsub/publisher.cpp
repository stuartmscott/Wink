#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <iostream>
#include <set>
#include <string>

int main(int argc, char** argv) {
  if (argc < 3) {
    Error() << "Incorrect parameters, expected <address> <parent>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[0]);
  UDPSocket socket;
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, socket, address, parent);

  std::set<Address> subscribers;

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() { Info() << "Publisher: OnEntry\n"
                     << std::flush; },
      // On Exit Action
      []() { Info() << "Publisher: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"subscribe",
           [&](const Address& sender, std::istream& args) {
             if (subscribers.insert(sender).second) {
               Info() << "Publisher: subscribed: " << sender << '\n'
                      << std::flush;
             }
           }},
          {"unsubscribe",
           [&](const Address& sender, std::istream& args) {
             if (subscribers.erase(sender)) {
               Info() << "Publisher: unsubscribed: " << sender << '\n'
                      << std::flush;
             }
           }},
          {"publish",
           [&](const Address& sender, std::istream& args) {
             std::string payload;
             args >> payload;
             Info() << "Publisher: publish " << payload << '\n' << std::flush;
             std::ostringstream os;
             os << "update ";
             os << payload;
             const auto message = os.str();
             for (const auto& s : subscribers) {
               m.Send(s, message);
             }
           }},
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
      }));

  m.Start();
}
