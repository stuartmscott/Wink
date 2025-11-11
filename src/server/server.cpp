// Copyright 2022-2025 Stuart Scott
#include <Wink/server/server.h>

#include <string>
#include <vector>

int Server::Serve(const std::string& directory) {
  running_ = true;
  if (!log_.empty()) {
    if (const auto result = LogToFile(log_, "server"); result < 0) {
      Error() << "Failed to setup logging\n" << std::flush;
      return -1;
    }
  }

  Info() << "Directory: " << directory << '\n';

  Info() << "Address: " << address_ << '\n' << std::flush;

  Address sender;
  std::string message;
  while (running_) {
    if (!mailbox_.Receive(sender, message)) {
      continue;
    }
    Info() << "< " << sender << ' ' << message << '\n' << std::flush;

    std::istringstream iss(message);
    std::string command;
    iss >> command;
    if (command == "start") {
      std::string name;
      iss >> name;

      // start <name>
      Address destination;
      // start <name> <port>
      if (iss.good()) {
        iss >> destination;
      }
      destination.set_ip(address_.ip());

      if (const auto port = destination.port(); port > 0) {
        // Stop existing machine on requested port (if any).
        Stop(port);
      }

      // Resolve file path.
      // ie. <directory>/<name>
      std::filesystem::path filepath(directory);
      filepath /= ParseMachineName(name).first;

      // Create parameter list
      std::vector<std::string> parameters;

      // First parameter is name of the machine, optionally including tag.
      // This will also become the log file name (if enabled).
      // ie. <name>, <name>#<tag>
      parameters.push_back(name);

      // Second parameter is the address of the machine.
      parameters.push_back(destination.ToString());

      // Third parameter is the address of the spawner.
      parameters.push_back(sender.ToString());

      // Remaining parameters are given by spawner.
      while (iss.good()) {
        std::string p;
        iss >> p;
        parameters.push_back(p);
      }

      if (const auto result = Start(filepath.string(), parameters);
          result < 0) {
        Error() << "Failed to start process\n" << std::flush;
        return result;
      }
    } else if (command == "stop") {
      ushort port;
      iss >> port;
      if (const auto result = Stop(port); result < 0) {
        Error() << "Failed to stop process\n" << std::flush;
        return result;
      }
    } else if (command == "register") {
      std::string machine;
      iss >> machine;
      int pid;
      iss >> pid;
      machines_.insert(std::pair<ushort, std::string>(sender.port(), machine));
      pids_.insert(std::pair<ushort, int>(sender.port(), pid));
    } else if (command == "unregister") {
      if (const auto it = pids_.find(sender.port()); it != pids_.end()) {
        // remove port from machines and pids maps
        machines_.erase(sender.port());
        pids_.erase(sender.port());
      } else {
        Error() << "Unrecognized port " << sender.port() << '\n' << std::flush;
      }
    } else if (command == "list") {
      SendMessage(mailbox_, sender, List());
    } else {
      Error() << "Failed to parse " << message << '\n' << std::flush;
    }
  }

  while (!mailbox_.Flushed()) {
  }

  return 0;
}

int Server::Start(const std::string& binary,
                  const std::vector<std::string>& parameters) {
  // Fork child process
  pid_t pid;
  switch (pid = fork()) {
    case -1:
      Error() << "Failed to fork process: " << std::strerror(errno) << '\n'
              << std::flush;
      return -1;
    case 0: {
      // Child runs machine process
      if (!log_.empty()) {
        std::string s(parameters.at(0));
        std::replace(s.begin(), s.end(), '/', '_');
        std::replace(s.begin(), s.end(), '#', '_');
        if (const auto result = LogToFile(log_, s); result < 0) {
          Error() << "Failed to setup logging\n" << std::flush;
          return -1;
        }
      }

      const auto length = parameters.size();
      const auto args = new char*[length + 1];
      args[length] = NULL;

      uint i = 0;
      for (const auto& p : parameters) {
        const auto l = p.length() + 1;
        const auto c = p.c_str();
        auto a = new char[l];
        strncpy(a, c, l);
        args[i] = a;
        i++;
      }

      const auto result = execv(binary.c_str(), args);

      for (i = 0; i < length; i++) {
        delete[] args[i];
      }
      delete[] args;

      if (result < 0) {
        Error() << "Failed to execute binary: " << parameters.at(0) << ": "
                << binary << ": " << result << ": " << std::strerror(errno)
                << '\n'
                << std::flush;
        return result;
      }
      _exit(0);
    }
    default:
      // Parent does nothing
      Info() << "Forked: " << pid << '\n' << std::flush;
  }
  return pid;
}

int Server::Stop(int port) {
  int pid = 0;
  if (const auto it = pids_.find(port); it != pids_.end()) {
    pid = it->second;
  }
  if (pid > 0) {
    if (const auto result = kill(pid, SIGTERM); result < 0) {
      return result;
    }
    // remove port from machines and pids maps
    machines_.erase(port);
    pids_.erase(port);
  }
  return pid;
}

std::string Server::List() {
  std::ostringstream oss;
  oss << "Port,PID,Machine\n";
  for (const auto& [k, v] : machines_) {
    oss << k << ',' << pids_[k] << ',' << v << '\n';
  }
  return oss.str();
}

void Server::Shutdown() { running_ = false; }
