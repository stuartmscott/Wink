#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 4) {
    Error()
        << "Incorrect parameters, expected <address> <parent> <destination>\n"
        << std::flush;
    return -1;
  }

  std::string name(argv[0]);
  UDPSocket socket;
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, socket, address, parent);

  Address destination(argv[3]);

  m.AddState(std::make_unique<State>(
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
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             m.Send(destination, os.str());
           }},
      }));

  m.Start();
}
