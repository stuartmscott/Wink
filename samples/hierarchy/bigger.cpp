// Copyright 2022-2025 Stuart Scott
#include <Wink/address.h>
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
      "Parent",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Parent: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Parent: OnExit" << std::endl; },
      // Receivers
      {
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Parent: " << os.str() << std::endl;
           }},
      }));

  m.AddState(State(
      // State Name
      "Leaf1",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Leaf1: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Leaf1: OnExit" << std::endl; },
      // Receivers
      {
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf1: " << os.str() << std::endl;
           }},
      }));

  m.AddState(State(
      // State Name
      "Child1",
      // Parent State
      "Parent",
      // On Entry Action
      []() { Info() << "Child1: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Child1: OnExit" << std::endl; },
      // Receivers
      {
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Child1: " << os.str() << std::endl;
           }},
      }));

  m.AddState(State(
      // State Name
      "Leaf2",
      // Parent State
      "Child1",
      // On Entry Action
      []() { Info() << "Leaf2: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Leaf2: OnExit" << std::endl; },
      // Receivers
      {
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf2: " << os.str() << std::endl;
           }},
      }));

  m.AddState(State(
      // State Name
      "Leaf3",
      // Parent State
      "Child1",
      // On Entry Action
      []() { Info() << "Leaf3: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Leaf3: OnExit" << std::endl; },
      // Receivers
      {
          {"",
           [&](const Address& sender, std::istream& args) {
             std::ostringstream os;
             os << args.rdbuf();
             Info() << "Leaf3: " << os.str() << std::endl;
           }},
      }));

  m.Start("Child1");
}
