#include <Wink/machine.h>

void Machine::Start(const std::string& initial) {
  if (states_.empty()) {
    // Nothing to do
    return;
  }

  // Bind to port
  if (const auto result = socket_.Bind(address_); result < 0) {
    ::Error() << "Failed to bind to port " << address_.port() << '\n'
              << std::flush;
    return;
  }

  std::ostringstream oss;
  oss << name_;
  oss << '@';
  oss << address_;
  uid_ = oss.str();

  // Set Receive Timeout
  if (const auto result = socket_.SetReceiveTimeout(kReplyTimeout);
      result < 0) {
    ::Error() << "Failed to set receive timeout: " << std::strerror(errno)
              << '\n'
              << std::flush;
    return;
  }

  Info() << uid_ << " started\n" << std::flush;

  // Notify parent of start
  Send(parent_, "started " + name_);

  // Register with server
  RegisterMachine(name_, getpid());

  std::string state = current_;
  if (!initial.empty()) {
    state = initial;
  }
  current_ = "";
  Transition(state);

  // Loop receiving messages
  auto last = std::chrono::system_clock::now();
  while (running_) {
    const auto now = std::chrono::system_clock::now();
    CheckChildren(now);  // Check every loop
    if (now - last > std::chrono::seconds(kPulseInterval)) {
      SendPulse();  // Send every kPulseInterval
      last = now;
    }
    SendScheduled(now);   // Send any scheduled messages
    ReceiveMessage(now);  // Waits up to kReplyTimeout for message
  }
}

void Machine::Exit() {
  Info() << uid_ << " exited\n" << std::flush;

  // Exit current state
  std::string s = current_;
  while (!s.empty()) {
    auto& state = states_.at(s);
    state.on_exit_();
    s = state.parent_;
  }

  std::ostringstream oss;
  oss << "exited ";
  oss << name_;

  // Notify parent of exit
  Send(parent_, oss.str());

  // Unregister with server
  UnregisterMachine();

  // TODO kill all child machines

  on_exit_();
  running_ = false;
  return;
}

void Machine::Error(const std::string& message) {
  Info() << uid_ << " errored: " << message << '\n' << std::flush;

  std::ostringstream oss;
  oss << "errored ";
  oss << name_;
  oss << ' ';
  oss << message;

  // Notify parent of error
  Send(parent_, oss.str());

  Exit();
  return;
}

void Machine::AddState(State state) {
  const auto n = state.name_;
  if (current_.empty()) {
    current_ = n;
  }
  states_.emplace(n, state);
}

void Machine::Transition(const std::string& state) {
  Info() << uid_ << " transitioned: " << current_ << " to " << state << '\n'
         << std::flush;
  auto current_lineage = StateLineage(current_);
  auto next_lineage = StateLineage(state);

  std::reverse(current_lineage.begin(), current_lineage.end());
  std::reverse(next_lineage.begin(), next_lineage.end());
  auto current_it = current_lineage.begin();
  auto next_it = next_lineage.begin();
  while (current_it != current_lineage.end() && next_it != next_lineage.end()) {
    if (*current_it == *next_it) {
      current_it = current_lineage.erase(current_it);
      next_it = next_lineage.erase(next_it);
    } else
      break;
  }

  std::reverse(current_lineage.begin(), current_lineage.end());
  // Exit current state hierarchy
  for (const auto& s : current_lineage) {
    states_.at(s).on_exit_();
  }

  current_ = state;

  // Enter new state hierarchy
  for (const auto& s : next_lineage) {
    states_.at(s).on_enter_();
  }
}

void Machine::Send(const Address& to, const std::string& message) {
  Info() << uid_ << " > " << to << ' ' << message << '\n' << std::flush;
  if (const auto result =
          socket_.Send(to, message.c_str(), message.length() + 1);
      result < 0) {
    ::Error() << uid_ << ": Failed to send packet: " << std::strerror(errno)
              << '\n'
              << std::flush;
  }
}

void Machine::SendAt(
    const Address& to, const std::string& message,
    const std::chrono::time_point<std::chrono::system_clock> time) {
  queue_.push_back(ScheduledMessage{to, message, time});
}

void Machine::SendAfter(const Address& to, const std::string& message,
                        const std::chrono::seconds delay) {
  auto time = std::chrono::system_clock::now();
  time += delay;
  SendAt(to, message, time);
}

void Machine::Spawn(const std::string& machine) {
  std::vector<std::string> args;
  Spawn(machine, args);
}

void Machine::Spawn(const std::string& machine,
                    const std::vector<std::string>& args) {
  const Address destination(address_.ip(), 0);
  Spawn(machine, destination, args);
}

void Machine::Spawn(const std::string& machine, const Address& destination) {
  std::vector<std::string> args;
  Spawn(machine, destination, args);
}

void Machine::Spawn(const std::string& machine, const Address& destination,
                    const std::vector<std::string>& args) {
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
  const auto s = oss.str();
  Send(server, s);
}

void Machine::CheckChildren(
    const std::chrono::time_point<std::chrono::system_clock> now) {
  std::vector<std::string> dead;
  for (const auto& [k, v] : spawned_) {
    if (const auto d = now - v.second;
        d > std::chrono::seconds(kHeartbeatTimeout)) {
      dead.push_back(k);
    }
  }

  for (const auto& c : dead) {
    if (const auto& it = spawned_.find(c); it != spawned_.end()) {
      const auto k = it->first;
      const auto v = it->second;

      std::istringstream iss(k);
      iss >> address_;

      {
        // Send errored message
        std::ostringstream oss;
        oss << "errored ";
        oss << v.first;
        oss << " heartbeat timeout";
        const auto s = oss.str();
        s.copy(buffer_, s.size());
        buffer_[s.size()] = 0;
        HandleMessage(now);
      }

      {
        // Send exited message
        std::ostringstream oss;
        oss << "exited ";
        oss << v.first;
        const auto s = oss.str();
        s.copy(buffer_, s.size());
        buffer_[s.size()] = 0;
        HandleMessage(now);
      }
    }
  }
}

void Machine::SendPulse() { Send(parent_, "pulsed " + name_); }

void Machine::SendScheduled(
    const std::chrono::time_point<std::chrono::system_clock> now) {
  std::vector<ScheduledMessage> q;
  q.swap(queue_);
  for (const auto& e : q) {
    if (e.time_ < now) {
      Send(e.address_, e.message_);
    } else {
      queue_.push_back(e);
    }
  }
}

void Machine::ReceiveMessage(
    const std::chrono::time_point<std::chrono::system_clock> now) {
  if (const auto result = socket_.Receive(sender_, buffer_, kMaxPayload);
      result < 0) {
    if (errno == EAGAIN) {
      return;
    }
    ::Error() << uid_ << ": Failed to receive packet: " << std::strerror(errno)
              << '\n'
              << std::flush;

    return;
  }
  HandleMessage(now);
}

void Machine::HandleMessage(
    const std::chrono::time_point<std::chrono::system_clock> now) {
  Info() << uid_ << " < " << sender_ << ' ' << buffer_ << '\n' << std::flush;
  std::istringstream iss(buffer_);
  std::string m;
  iss >> m;

  // Supervision
  std::ostringstream oss;
  oss << sender_;
  const auto key = oss.str();
  if (m == "started") {
    // Start tracking spawned health
    std::string name;
    iss >> name;
    spawned_.emplace(key, std::make_pair(name, now));
    // Seek back to start of name to make it available to the message handler.
    iss.seekg(-name.length(), std::ios_base::cur);
  } else if (m == "exited") {
    spawned_.erase(key);
  } else if (m == "pulsed") {
    if (auto it = spawned_.find(key); it != spawned_.end()) {
      it->second.second = now;
    }
  }

  auto s = current_;
  while (!s.empty()) {
    if (const auto it = states_.find(s); it != states_.end()) {
      const auto rs = it->second.receivers_;
      if (const auto i = rs.find(m); i != rs.end()) {
        i->second(sender_, iss);
        return;
      } else if (const auto i = rs.find(""); i != rs.end()) {
        std::istringstream iss(buffer_);
        i->second(sender_, iss);
        return;
      } else {
        // Message not handled by state, try parent
        s = it->second.parent_;
      }
    } else {
      ::Error() << uid_ << ": No such state: " << s << '\n' << std::flush;
      Error("Unrecognized state: " + s);
    }
  }
  // Message not handled by hierarchy
  ::Error() << uid_ << ": Failed to handle message\n" << std::flush;
  Error("Unhandled message: " + m);
}

void Machine::RegisterMachine(const std::string& machine, const int pid) {
  std::ostringstream oss;
  oss << "register ";
  oss << machine;
  oss << ' ';
  oss << pid;
  Address server(address_.ip(), kServerPort);
  Send(server, oss.str());
}

void Machine::UnregisterMachine() {
  Address server(address_.ip(), kServerPort);
  Send(server, "unregister");
}

std::vector<std::string> Machine::StateLineage(const std::string& state) {
  std::string s(state);
  std::vector<std::string> lineage;
  while (!s.empty()) {
    lineage.push_back(s);
    s = states_.at(s).parent_;
  }
  return lineage;
}

// Parse machine name into binary (directory/file), and optional tag.
std::pair<std::string, std::string> ParseMachineName(const std::string& name) {
  std::string binary = name;
  std::string tag;
  if (const auto i = name.find('#'); i != std::string::npos) {
    binary = name.substr(0, i);
    tag = name.substr(i);
  }
  return {binary, tag};
}
