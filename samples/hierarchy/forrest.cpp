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

  auto goto_receiver = [&](const Address& sender, std::istream& args) {
    std::string s;
    args >> s;
    m.Transition(s);
  };
  auto exit_receiver = [&](const Address& sender, std::istream& args) {
    m.Exit();
  };

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf1",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Leaf1: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf1: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{
          {"goto", goto_receiver},
          {"exit", exit_receiver},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Parent1",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Parent1: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Parent1: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{
          {"goto", goto_receiver},
          {"exit", exit_receiver},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf2",
      // Parent State
      "Parent1",
      // On Entry Action
      []() { Info() << "Leaf2: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf2: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Child2",
      // Parent State
      "Parent1",
      // On Entry Action
      []() { Info() << "Child2: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Child2: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf4",
      // Parent State
      "Child2",
      // On Entry Action
      []() { Info() << "Leaf4: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf4: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf5",
      // Parent State
      "Child2",
      // On Entry Action
      []() { Info() << "Leaf5: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf5: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Parent2",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Parent2: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Parent2: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{
          {"goto", goto_receiver},
          {"exit", exit_receiver},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Child3",
      // Parent State
      "Parent2",
      // On Entry Action
      []() { Info() << "Child3: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Child3: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf6",
      // Parent State
      "Child3",
      // On Entry Action
      []() { Info() << "Leaf6: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf6: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Child4",
      // Parent State
      "Parent2",
      // On Entry Action
      []() { Info() << "Child4: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Child4: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf7",
      // Parent State
      "Child4",
      // On Entry Action
      []() { Info() << "Leaf7: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf7: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Parent3",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Parent3: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Parent3: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{
          {"goto", goto_receiver},
          {"exit", exit_receiver},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Leaf3",
      // Parent State
      "Parent3",
      // On Entry Action
      []() { Info() << "Leaf3: OnEntry\n"
                    << std::flush; },
      // On Exit Action
      []() { Info() << "Leaf3: OnExit\n"
                    << std::flush; },
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.Start();
}
