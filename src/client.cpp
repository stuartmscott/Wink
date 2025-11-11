// Copyright 2022-2025 Stuart Scott
#include <Wink/client.h>

#include <string>
#include <vector>

int StartMachine(Mailbox& mailbox, Address& address, const std::string& machine,
                 Address& destination, const std::vector<std::string> args,
                 const bool follow) {
  Info() << "Address: " << address << '\n' << std::flush;

  // Send Request
  Address server(destination.ip(), kServerPort);
  std::ostringstream oss;
  oss << "start ";
  oss << machine;
  oss << " :";
  oss << destination.port();
  for (const auto& a : args) {
    oss << ' ';
    oss << a;
  }
  const auto request = oss.str();
  SendMessage(mailbox, server, request);

  // Recieve Reply
  std::string message;
  if (!ReceiveMessage(mailbox, destination, message)) {
    Error() << "Failed to receive \"started\" message: " << std::strerror(errno)
            << '\n'
            << std::flush;
    return -1;
  }
  Info() << "< " << destination << ' ' << message << '\n' << std::flush;
  std::istringstream iss(message);
  std::string command;
  iss >> command;
  if (command != "started") {
    Error() << "Incorrect message received. Expected: \"started\", "
            << "Got: \"" << command << "\"\n"
            << std::flush;
    return -1;
  }
  auto name_req = ParseMachineName(machine);
  auto bin_req = get<0>(name_req);
  std::string n;
  iss >> n;
  auto name_res = ParseMachineName(n);
  auto bin_res = get<0>(name_req);
  if (bin_res != bin_req) {
    Error() << "Incorrect machine binary started. Expected: \"" << bin_req
            << "\", Got: \"" << bin_res << "\"\n"
            << std::flush;
    return -1;
  }

  while (follow) {
    if (!mailbox.Receive(destination, message)) {
      continue;
    }
    Info() << "< " << destination << ' ' << message << '\n' << std::flush;
    std::istringstream iss(message);
    std::string command;
    iss >> command;
    if (command == "exited") {
      break;
    }
  }
  return 0;
}

int StopMachine(Mailbox& mailbox, const Address& address) {
  Address server(address.ip(), kServerPort);
  std::ostringstream oss;
  oss << "stop ";
  oss << address.port();
  SendMessage(mailbox, server, oss.str());
  return 0;
}

void SendMessage(Mailbox& mailbox, const Address& to,
                 const std::string& message) {
  Info() << "> " << to << ' ' << message << '\n' << std::flush;
  mailbox.Send(to, message);
}

bool ReceiveMessage(Mailbox& mailbox, Address& from, std::string& message) {
  bool success = false;
  for (int i = 0; i < kMaxRetries && !success; i++) {
    success = mailbox.Receive(from, message);
  }
  return success;
}

int ListMachines(Mailbox& mailbox, const Address& server) {
  // Send Request
  SendMessage(mailbox, server, "list");

  // Recieve Reply
  Address from;
  std::string message;
  if (!ReceiveMessage(mailbox, from, message)) {
    Error() << "Failed to receive \"list\" message: " << std::strerror(errno)
            << '\n'
            << std::flush;
    return -1;
  }
  Info() << "< " << from << ' ' << message << '\n' << std::flush;
  return 0;
}
