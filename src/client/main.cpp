// Copyright 2022-2025 Stuart Scott
#include <Wink/client.h>

#include <algorithm>
#include <cctype>
#include <map>
#include <string>
#include <utility>
#include <vector>

void Usage(std::string name) {
  Info() << name << std::endl;
  Info() << "\tstart [options] <binary> <host>" << std::endl;
  Info() << "\tstop [options] <machine>" << std::endl;
  Info() << "\tsend [options] <machine> <message>" << std::endl;
  Info() << "\tlisten [options] <group>" << std::endl;
  Info() << "\tlist [options] <host>" << std::endl;
  Info() << "\thelp" << std::endl;
}

void Usage() { Usage("Wink"); }

void Help(std::string name, std::string command) {
  if (command == "start") {
    Info() << "Start a new machine." << std::endl;
    Info() << std::endl;
    Info() << "Options;" << std::endl;
    Info() << "\t-a" << std::endl;
    Info() << "\t\tThe address to bind to (default " << kLocalhost << ":<any>)"
           << std::endl;
    Info() << "\t-f" << std::endl;
    Info() << "\t\tFollow the lifecycle of the machine (default false)"
           << std::endl;
    Info() << "Parameters;" << std::endl;
    Info() << "\tbinary" << std::endl;
    Info() << "\t\tThe machine binary to start" << std::endl;
    Info() << "\thost" << std::endl;
    Info() << "\t\tThe host to start the machine on" << std::endl;
    Info() << "Examples;" << std::endl;
    Info() << "\tstart machine.bin" << std::endl;
    Info() << "\t\tStart a new machine on localhost on any available port"
           << std::endl;
    Info() << "\tstart machine.bin :64646" << std::endl;
    Info() << "\t\tStart a new machine on localhost port 64646" << std::endl;
    Info() << "\tstart machine.bin 123.45.67.89" << std::endl;
    Info() << "\t\tStart a new machine on ip 123.45.67.89 on any available port"
           << std::endl;
    Info() << "\tstart machine.bin 123.45.67.89:64646" << std::endl;
    Info() << "\t\tStart a new machine on ip 123.45.67.89 port 64646"
           << std::endl;
  } else if (command == "stop") {
    Info() << "Stop an existing machine." << std::endl;
    Info() << std::endl;
    Info() << "Options;" << std::endl;
    Info() << "\t-a" << std::endl;
    Info() << "\t\tThe address to bind to (default " << kLocalhost << ":<any>)"
           << std::endl;
    Info() << "Parameters;" << std::endl;
    Info() << "\tmachine" << std::endl;
    Info() << "\t\tThe address of the machine to stop" << std::endl;
    Info() << "Examples;" << std::endl;
    Info() << "\tstop 123.45.67.89:64646" << std::endl;
    Info() << "\t\tStop an existing machine on ip 123.45.67.89 port 64646"
           << std::endl;
  } else if (command == "send") {
    Info() << "Sends a message (or messages) to a machine." << std::endl;
    Info() << std::endl;
    Info() << "Options;" << std::endl;
    Info() << "\t-a" << std::endl;
    Info() << "\t\tThe address to bind to (default " << kLocalhost << ":<any>)"
           << std::endl;
    Info() << "\t-r" << std::endl;
    Info() << "\t\tThe number of replies to await (default 0)" << std::endl;
    Info() << "Parameters;" << std::endl;
    Info() << "\tmachine" << std::endl;
    Info() << "\t\tThe address of the machine to send to" << std::endl;
    Info() << "\tmessage" << std::endl;
    Info() << "\t\tThe message (or messages) to send" << std::endl;
    Info() << "Examples;" << std::endl;
    Info() << "\tsend :64646 \"add(2,8)\"" << std::endl;
    Info() << "\t\tSend a message to machine on localhost port 64646"
           << std::endl;
    Info() << "\tsend 123.45.67.89:64646 \"add(2,8)\"" << std::endl;
    Info() << "\t\tSend a message to machine on ip 123.45.67.89 port 64646"
           << std::endl;
    Info() << "\tsend 123.45.67.89:64646 \"add(2,8)\" \"sub(5,3)\""
           << std::endl;
    Info() << "\t\tSend two messages to machine on ip 123.45.67.89 port 64646"
           << std::endl;
  } else if (command == "listen") {
    Info() << "Listen for multicast messages on the given group(s)."
           << std::endl;
    Info() << std::endl;
    Info() << "Options;" << std::endl;
    Info() << "\t-a" << std::endl;
    Info() << "\t\tThe address to bind to (default " << kLocalhost << ":<any>)"
           << std::endl;
    Info() << "Parameters;" << std::endl;
    Info() << "\tgroup" << std::endl;
    Info() << "\t\tThe multicast group to join" << std::endl;
    Info() << "Examples;" << std::endl;
    Info() << "\tlisten 232.3.2.32:32323" << std::endl;
    Info() << "\t\tListens for messages send to the 232.3.2.32:32323 group"
           << std::endl;
  } else if (command == "list") {
    Info() << "List machines running on a host." << std::endl;
    Info() << std::endl;
    Info() << "Options;" << std::endl;
    Info() << "\t-a" << std::endl;
    Info() << "\t\tThe address to bind to (default " << kLocalhost << ":<any>)"
           << std::endl;
    Info() << "Parameters;" << std::endl;
    Info() << "\thost" << std::endl;
    Info() << "\t\tThe host to list the machines from" << std::endl;
    Info() << "Examples;" << std::endl;
    Info() << "\tlist" << std::endl;
    Info() << "\t\tLists the machines running on localhost port 42000"
           << std::endl;
    Info() << "\tlist :64646" << std::endl;
    Info() << "\t\tLists the machines running on localhost port 64646"
           << std::endl;
    Info() << "\tlist 123.45.67.89:64646" << std::endl;
    Info() << "\t\tLists the machines running on ip 123.45.67.89 port 64646"
           << std::endl;
  } else {
    Usage();
  }
}

int main(int argc, char** argv) {
  if (argc <= 0) {
    Usage();
    return 0;
  }
  std::string name(argv[0]);
  if (argc == 1) {
    Usage(name);
    return 0;
  }

  std::string command(argv[1]);

  std::map<std::string, std::string> options;
  std::vector<std::string> parameters;
  for (int i = 2; i < argc; ++i) {
    if (argv[i][0] == '-' && i + 1 < argc) {
      options.insert(
          std::make_pair(std::string(argv[i]), std::string(argv[i + 1])));
      i++;
    } else {
      parameters.push_back(std::string(argv[i]));
    }
  }

  if (command == "start") {
    Address address(kLocalhost, 0);
    bool follow = false;

    // Parse Options
    for (const auto& [k, v] : options) {
      if (k == "-a") {
        std::stringstream ss(v);
        ss >> address;
      } else if (k == "-f") {
        std::stringstream ss(v);
        std::string f;
        ss >> f;
        std::transform(f.begin(), f.end(), f.begin(), ::tolower);
        follow = (f == "1" || f == "true");
      } else {
        Error() << "Option " << k << ":" << v << " not supported" << std::endl;
      }
    }

    Address destination(kLocalhost, 0);
    std::vector<std::string> args;
    auto count = parameters.size();
    if (count == 0) {
      Error() << "Missing <binary> parameter" << std::endl;
      return -1;
    }
    std::string binary(parameters.at(0));
    if (count > 1) {
      destination.FromString(parameters.at(1));
      for (uint32_t i = 2; i < count; i++) {
        args.push_back(parameters.at(i));
      }
    }

    UDPSocket socket(address);
    AsyncMailbox mailbox(socket);
    return StartMachine(mailbox, address, binary, destination, args, follow);
  } else if (command == "stop") {
    Address address(kLocalhost, 0);

    // Parse Options
    for (const auto& [k, v] : options) {
      if (k == "-a") {
        std::stringstream ss(v);
        ss >> address;
      } else {
        Error() << "Option " << k << ":" << v << " not supported" << std::endl;
      }
    }

    Address destination(kLocalhost, 0);
    switch (parameters.size()) {
      case 0:
        Error() << "Missing <machine> parameter" << std::endl;
        return -1;
      case 1: {
        std::istringstream ss(parameters.at(0));
        ss >> destination;
      } break;
      default:
        Error() << "Too many parameters" << std::endl;
        return -1;
    }

    UDPSocket socket(address);
    AsyncMailbox mailbox(socket);
    return StopMachine(mailbox, destination);
  } else if (command == "send") {
    Address address(kLocalhost, 0);
    uint32_t replies = 0;
    // Parse Options
    for (const auto& [k, v] : options) {
      if (k == "-a") {
        std::stringstream ss(v);
        ss >> address;
      } else if (k == "-r") {
        std::stringstream ss(v);
        ss >> replies;
      } else {
        Error() << "Option " << k << ":" << v << " not supported" << std::endl;
      }
    }

    Address destination(kLocalhost, 0);
    std::vector<std::string> messages;
    switch (parameters.size()) {
      case 0:
        Error() << "Missing <machine> parameter" << std::endl;
        return -1;
      case 1:
        Error() << "Missing <message> parameter" << std::endl;
        return -1;
      default: {
        const auto s = parameters.at(0);
        std::istringstream ss(s);
        ss >> destination;
        messages.assign(parameters.begin() + 1, parameters.end());
      } break;
    }

    UDPSocket socket(address);
    AsyncMailbox mailbox(socket);
    SendMessages(mailbox, destination, messages);

    Address from;
    Address to;
    std::string reply;
    for (uint32_t r = 0; r < replies;) {
      if (ReceiveMessage(mailbox, from, to, reply)) {
        Info() << to << " < " << from << ' ' << reply << std::endl;
        r++;
      }
    }
    return 0;
  } else if (command == "listen") {
    Address address(kLocalhost, 0);

    // Parse Options
    for (const auto& [k, v] : options) {
      if (k == "-a") {
        std::stringstream ss(v);
        ss >> address;
      } else {
        Error() << "Option " << k << ":" << v << " not supported" << std::endl;
      }
    }

    UDPSocket socket(address);
    switch (parameters.size()) {
      case 0:
        Error() << "Missing <group> parameter" << std::endl;
        return -1;
      default: {
        for (const auto& s : parameters) {
          Address group;
          std::istringstream ss(s);
          ss >> group;
          if (group.IsMulticast()) {
            socket.JoinGroup(group);
          } else {
            Error() << "Address " << group << " is not a multicast group"
                    << std::endl;
            return -1;
          }
        }
      } break;
    }

    AsyncMailbox mailbox(socket);
    Address from;
    Address to;
    std::string reply;
    while (true) {
      if (ReceiveMessage(mailbox, from, to, reply)) {
        Info() << to << " < " << from << ' ' << reply << std::endl;
      }
    }
    return 0;
  } else if (command == "list") {
    Address address(kLocalhost, 0);

    // Parse Options
    for (const auto& [k, v] : options) {
      if (k == "-a") {
        std::stringstream ss(v);
        ss >> address;
      } else {
        Error() << "Option " << k << ":" << v << " not supported" << std::endl;
      }
    }

    Address destination(kLocalhost, kServerPort);
    switch (parameters.size()) {
      case 0:
        break;
      case 1: {
        const auto s = parameters.at(0);
        std::istringstream ss(s);
        ss >> destination;
      } break;
      default:
        Error() << "Too many parameters" << std::endl;
        return -1;
    }

    UDPSocket socket(address);
    AsyncMailbox mailbox(socket);
    return ListMachines(mailbox, destination);
  } else if (command == "help") {
    if (argc < 3) {
      Usage();
    } else {
      Help(name, std::string(argv[2]));
    }
  } else {
    Usage(name);
  }
  return 0;
}
