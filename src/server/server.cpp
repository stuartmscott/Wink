// Copyright 2022-2025 Stuart Scott
#include <WinkServer/server.h>

#include <string>
#include <vector>

int Server::Serve(const std::string& directory) {
  running_ = true;
  if (!log_.empty()) {
    if (const auto result = LogToFile(log_, "server"); result < 0) {
      Error() << "Failed to setup logging" << std::endl;
      return -1;
    }
  }

  Info() << "Directory: " << directory << std::endl;
  Info() << "Address: " << address_ << std::endl;

  Address sender;
  std::string message;
  while (running_) {
    if (!mailbox_.Receive(sender, message)) {
      continue;
    }
    Info() << "< " << sender << ' ' << message << std::endl;

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

      // TODO handle in worker threat
      {
        if (const auto port = destination.port(); port > 0) {
          // Stop existing machine on requested port (if any).
          Stop(port);
        }

        // Resolve file path.
        // ie. <directory>/<name>
        std::filesystem::path filepath(directory);
        filepath /= ParseMachineName(name).first;
        filepath = std::filesystem::absolute(filepath);

        Info() << "Current Path: " << std::filesystem::current_path()
               << std::endl;
        Info() << "Binary Path: " << filepath << std::endl;

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
        std::string parameter;
        while (iss >> parameter) {
          parameters.push_back(parameter);
        }

        if (const auto result = Start(filepath.string(), parameters);
            result < 0) {
          Error() << "Failed to start process" << std::endl;
          return result;
        }
      }
    } else if (command == "stop") {
      uint16_t port;
      iss >> port;

      // TODO handle in worker threat
      {
        if (const auto result = Stop(port); result < 0) {
          Error() << "Failed to stop process" << std::endl;
          return result;
        }
      }
    } else if (command == "register") {
      std::string machine;
      iss >> machine;
      int pid;
      iss >> pid;
      // TODO secure with mutex
      {
        machines_.emplace(sender.port(), machine);
        pids_.emplace(sender.port(), pid);
      }
    } else if (command == "unregister") {
      // TODO secure with mutex
      {
        if (const auto it = pids_.find(sender.port()); it != pids_.end()) {
          // remove port from machines and pids maps
          machines_.erase(sender.port());
          pids_.erase(sender.port());
        } else {
          Error() << "Unrecognized port " << sender.port() << std::endl;
        }
      }
    } else if (command == "list") {
      // TODO handle in worker threat
      {
        SendMessage(mailbox_, sender, List());
      }
    } else {
      Error() << "Failed to parse " << message << std::endl;
    }
  }

  while (!mailbox_.Flushed()) {
  }

  return 0;
}

pid_t Server::Start(const std::string& binary,
                    const std::vector<std::string>& parameters) {
  // Fork child process
  pid_t pid;
  switch (pid = fork()) {
    case -1:
      Error() << "Failed to fork process: " << std::strerror(errno)
              << std::endl;
      return -1;
    case 0: {
      // Child runs machine process
      if (!log_.empty()) {
        std::string s(parameters.at(0));
        std::replace(s.begin(), s.end(), '/', '_');
        std::replace(s.begin(), s.end(), '#', '_');
        if (const auto result = LogToFile(log_, s); result < 0) {
          Error() << "Failed to setup logging" << std::endl;
          return -1;
        }
      }

      const auto length = parameters.size() + 2;
      Info() << "Allocating char*[" << length << ']' << std::endl;
      const auto args = new char*[length];

      // Binary name
      {
        auto b = binary.c_str();
        auto l = binary.length() + 1;
        Info() << "Allocating char[" << l << "] for " << binary << std::endl;
        auto a = new char[l];
        strncpy(a, b, l);
        args[0] = a;
      }

      // Parameters
      {
        uint32_t i = 1;
        for (const auto& p : parameters) {
          const auto l = p.length() + 1;
          const auto c = p.c_str();
          Info() << "Allocating char[" << l << "] for " << p << std::endl;
          auto a = new char[l];
          strncpy(a, c, l);
          args[i] = a;
          i++;
        }
      }

      // End pointer
      {
        args[length - 1] = NULL;
      }

      const auto result = execv(binary.c_str(), args);

      if (result < 0) {
        Error() << "Failed to execute binary: " << parameters.at(0) << ": "
                << binary << ": " << result << ": " << std::strerror(errno)
                << std::endl;
      }
      _exit(result);
    }
    default:
      // Parent does nothing
      Info() << "Forked: " << pid << std::endl;
  }
  return pid;
}

pid_t Server::Stop(uint16_t port) {
  int pid = 0;
  if (const auto it = pids_.find(port); it != pids_.end()) {
    pid = it->second;
  }
  if (pid > 0) {
    Info() << "Killing: " << port << " : " << pid << std::endl;
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
