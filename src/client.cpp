#include <Wink/client.h>

int StartMachine(Socket& socket, Address& address, const std::string& machine,
                 Address& destination, const std::vector<std::string> args,
                 const bool follow) {
  // Bind to address
  if (const auto result = socket.Bind(address); result < 0) {
    Error() << "Failed to bind to address: " << address << " : "
            << std::strerror(errno) << '\n'
            << std::flush;
    return result;
  }

  Info() << "Address: " << address << '\n' << std::flush;

  // Set Receive Timeout
  if (const auto result = socket.SetReceiveTimeout(kHeartbeatTimeout);
      result < 0) {
    Error() << "Failed to set receive timeout: " << std::strerror(errno) << '\n'
            << std::flush;
    return result;
  }

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
  if (const auto result = SendMessage(socket, server, request); result < 0) {
    return result;
  }

  // Recieve Reply
  char buffer[kMaxPayload];
  if (const auto result = socket.Receive(destination, buffer, kMaxPayload);
      result < 0) {
    Error() << "Failed to receive packet: " << std::strerror(errno) << '\n'
            << std::flush;
    return result;
  }
  Info() << "< " << destination << ' ' << buffer << '\n' << std::flush;
  std::istringstream iss(buffer);
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
    if (const auto result = socket.Receive(destination, buffer, kMaxPayload);
        result < 0) {
      if (errno == EAGAIN) {
        Info() << "< " << destination << " errored " << machine
               << " heartbeat timeout\n";
        Info() << "< " << destination << " exited " << machine << '\n'
               << std::flush;
        return -1;
      }
      Error() << "Failed to receive packet: " << std::strerror(errno) << '\n'
              << std::flush;
      return result;
    }
    Info() << "< " << destination << ' ' << buffer << '\n' << std::flush;
    std::istringstream iss(buffer);
    std::string command;
    iss >> command;
    if (command == "exited") {
      break;
    }
  }
  return 0;
}

int StopMachine(Socket& socket, const Address& address) {
  Address server(address.ip(), kServerPort);
  std::ostringstream oss;
  oss << "stop ";
  oss << address.port();
  return SendMessage(socket, server, oss.str());
}

int SendMessage(Socket& socket, const Address& address, const std::string& m) {
  Info() << "> " << address << ' ' << m << '\n' << std::flush;
  if (const auto result = socket.Send(address, m.c_str(), m.length() + 1);
      result < 0) {
    Error() << "Failed to send packet: " << std::strerror(errno) << '\n'
            << std::flush;
    return result;
  }
  return 0;
}

int ListMachines(Socket& socket, const Address& server) {
  Address address;

  // Bind to address
  if (const auto result = socket.Bind(address); result < 0) {
    Error() << "Failed to bind to address: " << address << " : "
            << std::strerror(errno) << '\n'
            << std::flush;
    return result;
  }

  Info() << "Address: " << address << '\n' << std::flush;

  // Set Receive Timeout
  if (const auto result = socket.SetReceiveTimeout(kHeartbeatTimeout);
      result < 0) {
    Error() << "Failed to set receive timeout: " << std::strerror(errno) << '\n'
            << std::flush;
    return result;
  }

  // Send Request
  if (const auto result = SendMessage(socket, server, "list"); result < 0) {
    return result;
  }

  // Recieve Reply
  Address from;
  char buffer[kMaxPayload];
  if (const auto result = socket.Receive(from, buffer, kMaxPayload);
      result < 0) {
    Error() << "Failed to receive packet: " << std::strerror(errno) << '\n'
            << std::flush;
    return result;
  }
  Info() << "< " << from << ' ' << buffer << '\n' << std::flush;
  return 0;
}
