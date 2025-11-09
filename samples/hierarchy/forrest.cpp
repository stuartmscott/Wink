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
      "Leaf1", "", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf1: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Parent1", "", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Parent1: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Leaf2", "Parent1", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf2: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Child2", "Parent1",
      []() { Info() << "Child2 On Entry\n"
                    << std::flush; }, []() {},
      std::map<const std::string, Receiver>{
          {"Test",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Child2: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Leaf4", "Child2", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf4: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Leaf5", "Child2", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf5: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Parent2", "", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Parent2: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Child3", "Parent2",
      []() { Info() << "Child3 On Entry\n"
                    << std::flush; }, []() {},
      std::map<const std::string, Receiver>{
          {"Test",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Child3: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Leaf6", "Child3", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf6: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Child4", "Parent2",
      []() { Info() << "Child4 On Entry\n"
                    << std::flush; }, []() {},
      std::map<const std::string, Receiver>{
          {"Test",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Child4: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Leaf7", "Child4", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf7: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Parent3", "", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Parent3: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(std::make_unique<State>(
      "Leaf3", "Parent3", []() {}, []() {},
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf3: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.Start();
}
