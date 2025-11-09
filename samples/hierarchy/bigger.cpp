#include <Wink/address.h>
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
      "Parent",
      // Parent State
      "",
      // On Entry Action
      []() {},
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Parent: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf1",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Leaf1 On Entry\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{
          {"Test",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf1: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Child1",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Child1 On Entry\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{
          {"Test",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Child1: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf2",
      // Parent State
      "Child1",
      // On Entry Action
      []() { Info() << "Leaf2 On Entry\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{
          {"Test",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf2: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf3",
      // Parent State
      "Child1",
      // On Entry Action
      []() { Info() << "Leaf3 On Entry\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{
          {"Test",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf3: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.Start("Child1");
}
