// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/machine.h>
#include <Wink/state.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc < 4) {
    Error() << "Incorrect parameters, expected <name> <address> <parent>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[1]);
  Address address(argv[2]);
  Address parent(argv[3]);
  Machine m(name, address, parent);

  m.AddState(State(
      // State Name
      "Parent",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Parent: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Parent: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Parent: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(State(
      // State Name
      "Leaf1",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Leaf1: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf1: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf1: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(State(
      // State Name
      "Child1",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Child1: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Child1: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Child1: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(State(
      // State Name
      "Leaf2",
      // Parent State
      "Child1",
      // On Entry Action
      []() { Info() << "Leaf2: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf2: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf2: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.AddState(State(
      // State Name
      "Leaf3",
      // Parent State
      "Child1",
      // On Entry Action
      []() { Info() << "Leaf3: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf3: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"exit",
           [&](const Address& sender, std::istream& args) { m.Exit(); }},
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf3: " << os.str() << '\n' << std::flush;
           }},
      }));

  m.Start("Child1");
}
