// Copyright 2025 Stuart Scott
#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/mailbox.h>
#include <Wink/socket.h>
#include <Wink/state.h>
#include <tape.h>

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

typedef std::tuple<char, char, char, char> quadruple;

int main(int argc, char** argv) {
  if (argc < 6) {
    Error() << "Incorrect parameters, expected <name> <address> <parent> "
               "<blank> <quintuples>"
            << std::endl;
    return -1;
  }

  std::string name(argv[1]);
  Address address(argv[2]);
  UDPSocket socket(address);
  AsyncMailbox mailbox(socket);
  Address parent(argv[3]);
  std::string quintuples(argv[5]);

  Machine m(name, mailbox, address, parent);
  Tape tape(argv[4][0]);

  Address requester;

  // Split quintuples by semicolon, insert into map of state to vector of
  // tuple of read, write, move, and next.
  std::map<char, std::vector<quadruple>> states;
  std::string token;
  std::istringstream iss(quintuples);
  while (std::getline(iss, token, ';')) {
    char current = token[0];
    char read = token[1];
    char write = token[2];
    char move = token[3];
    char next = token[4];
    const auto& it = states.find(current);
    if (it == states.end()) {
      states.emplace(current,
                     std::vector<quadruple>{{read, write, move, next}});
    } else {
      it->second.emplace_back(read, write, move, next);
    }
  }

  char first_state = 0;

  // Iterate map, adding states to machine
  for (const auto& [k, v] : states) {
    if (!first_state) {
      first_state = k;
    }

    ReceiverMap receivers;
    for (const auto& [r, w, d, n] : v) {
      receivers.emplace(
          std::string{r},
          [&m, &tape, w, d, n](const Address& from, const Address& to,
                               std::istream& args) {
            tape.Write(w);
            tape.Move(d);
            m.Transition(std::string{n});
            Info() << "Tape: " << tape.ToString() << std::endl;
          });
    }

    m.AddState(State(
        // State Name
        std::string{k},
        // Parent State
        "",
        // On Entry Action
        [&m, &address, &tape, k]() {
          Info() << k << ": OnEntry" << std::endl;
          m.Send(address, std::string{tape.Read()});
        },
        // On Exit Action
        [k]() { Info() << k << ": OnExit" << std::endl; },
        // Receivers
        receivers));
  }

  // Add initial state to record requester and initialize tape
  m.AddState(State(
      // State Name
      "Initial",
      // Parent State
      "",
      // On Entry Action
      []() { Info() << "Initial: OnEntry" << std::endl; },
      // On Exit Action
      []() { Info() << "Initial: OnExit" << std::endl; },
      // Receivers
      {{"exit",
        [&](const Address& from, const Address& to, std::istream& args) {}},
       {"", [&](const Address& from, const Address& to, std::istream& args) {
          requester = from;
          tape.Assign(args);
          m.Transition(std::string{first_state});
        }}}));

  // Add final state to respond to requester and reset machine
  m.AddState(State(
      // State Name
      "H",
      // Parent State
      "",
      // On Entry Action
      [&m, &requester, &tape]() {
        Info() << "H: OnEntry" << std::endl;
        m.Send(requester, tape.ToString());
        m.Transition("Initial");
      },
      // On Exit Action
      []() { Info() << "H: OnExit" << std::endl; },
      // Receivers
      {}));

  m.Start("Initial");
}
