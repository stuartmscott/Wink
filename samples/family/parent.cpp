#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <iostream>
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

  m.AddState(std::make_unique<State>(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        Info() << "Parent: OnEntry\n" << std::flush;
        // Spawn two identical children, differentiated by a tag
        m.Spawn("family/Child:Alice");
        m.Spawn("family/Child:Bob");
      },
      // On Exit Action
      []() { Info() << "Parent: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{
          {"started",
           [&](const Address& sender, std::istream& args) {
             std::string child;
             args >> child;
             Info() << "Parent: " << sender << ' ' << child << " has started\n"
                    << std::flush;
           }},
          {"pulsed",
           [&](const Address& sender, std::istream& args) {
             std::string child;
             args >> child;
             Info() << "Parent: " << sender << ' ' << child << " has pulsed\n"
                    << std::flush;
           }},
          {"errored",
           [&](const Address& sender, std::istream& args) {
             std::string child;
             args >> child;
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Parent: " << sender << ' ' << child
                    << " has errored: " << os.str() << '\n'
                    << std::flush;
           }},
          {"exited",
           [&](const Address& sender, std::istream& args) {
             std::string child;
             args >> child;
             Info() << "Parent: " << sender << ' ' << child << " has exited\n"
                    << std::flush;
             m.Spawn(child);  // Retry
           }},
      }));

  m.Start();
}
