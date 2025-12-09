// Copyright 2022-2025 Stuart Scott
#include <Wink/client.h>

#include <string>
#include <vector>

int StartMachine(Mailbox& mailbox, const Address address,
                 const std::string machine, Address& destination,
                 const std::vector<std::string> args, const bool follow) {
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
  Address to;
  std::string message;
  if (!ReceiveMessage(mailbox, destination, to, message)) {
    Error() << "Failed to receive \"started\" message: " << std::strerror(errno)
            << std::endl;
    return -1;
  }
  Info() << to << " < " << destination << ' ' << message << std::endl;
  std::istringstream iss(message);
  std::string command;
  iss >> command;
  if (command != "started") {
    Error() << "Incorrect message received. Expected: \"started\", "
            << "Got: \"" << command << '"' << std::endl;
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
            << "\", Got: \"" << bin_res << '"' << std::endl;
    return -1;
  }

  Address from;
  while (follow) {
    if (!mailbox.Receive(from, to, message)) {
      continue;
    }
    Info() << to << " < " << from << ' ' << message << std::endl;
    std::istringstream iss(message);
    std::string command;
    iss >> command;
    if (command == "exited") {
      break;
    }
  }
  return 0;
}

int StopMachine(Mailbox& mailbox, const Address address) {
  Address server(address.ip(), kServerPort);
  std::ostringstream oss;
  oss << "stop ";
  oss << address.port();
  SendMessage(mailbox, server, oss.str());
  return 0;
}

void SendMessage(Mailbox& mailbox, const Address to,
                 const std::string message) {
  Info() << "> " << to << ' ' << message << std::endl;
  mailbox.Send(to, message);
}

void SendMessages(Mailbox& mailbox, const Address to,
                  const std::vector<std::string> messages) {
  for (const auto& m : messages) {
    SendMessage(mailbox, to, m);
  }
}

bool ReceiveMessage(Mailbox& mailbox, Address& from, Address& to,
                    std::string& message) {
  bool success = false;
  for (uint8_t i = 0; i < kMaxRetries && !success; i++) {
    success = mailbox.Receive(from, to, message);
  }
  return success;
}

int ListMachines(Mailbox& mailbox, const Address server) {
  // Send Request
  SendMessage(mailbox, server, "list");

  // Recieve Reply
  Address from;
  Address to;
  std::string message;
  if (!ReceiveMessage(mailbox, from, to, message)) {
    Error() << "Failed to receive \"list\" message: " << std::strerror(errno)
            << std::endl;
    return -1;
  }
  Info() << to << " < " << from << ' ' << message << std::endl;
  return 0;
}
