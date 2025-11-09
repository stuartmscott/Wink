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
      []() {},
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{
          {"",
           [&](const Address& sender, std::istream& args) {
             int n;
             args >> n;
             if (n % 15 == 0) {
               m.Transition("FizzBuzz");
             } else if (n % 5 == 0) {
               m.Transition("Buzz");
             } else if (n % 3 == 0) {
               m.Transition("Fizz");
             } else {
               Info() << n << '\n' << std::flush;
             }
           }},
      }));

  m.AddState(std::make_unique<State>(
      // State Name
      "Fizz",
      // Parent State
      "main",
      // On Entry Action
      []() { Info() << "Fizz\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "Buzz",
      // Parent State
      "main",
      // On Entry Action
      []() { Info() << "Buzz\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.AddState(std::make_unique<State>(
      // State Name
      "FizzBuzz",
      // Parent State
      "main",
      // On Entry Action
      []() { Info() << "FizzBuzz\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      std::map<const std::string, Receiver>{}));

  m.Start();
}
