// Copyright 2022-2025 Stuart Scott
#include <Wink/machine.h>

#include <string>
#include <utility>
#include <vector>

void SignalHandler(int signal) {
  Info() << "Received signal: " << signal << std::endl;
  got_sigterm = true;
}

void Machine::Start(const std::string& initial) {
  std::ostringstream oss;
  oss << name_;
  oss << '@';
  oss << address_;
  uid_ = oss.str();

  Info() << uid_ << " started" << std::endl;

  // Notify parent of start
  Send(parent_, "started " + name_);

  // Register with server
  RegisterMachine(name_, getpid());

  if (!states_.empty()) {
    std::string state = current_;
    if (!initial.empty()) {
      state = initial;
    }
    current_ = "";
    Transition(state);

    // Loop receiving messages
    auto last = std::chrono::system_clock::now();
    while (running_ && !got_sigterm) {
      const auto now = std::chrono::system_clock::now();
      CheckChildren(now);  // Check every loop
      if (now - last > kPulseInterval) {
        SendPulse();  // Send every kPulseInterval
        last = now;
      }
      SendScheduled(now);   // Send any scheduled messages
      ReceiveMessage(now);  // Waits up to kReceiveTimeout for message
    }

    // Exit current state
    std::string s = current_;
    while (!s.empty()) {
      auto& state = states_.at(s);
      state.on_exit_();
      s = state.parent_;
    }
  }

  if (!error_message_.empty()) {
    std::ostringstream oss;
    oss << "errored ";
    oss << name_;
    oss << ' ';
    oss << error_message_;

    // Notify parent of error
    Send(parent_, oss.str());
  }

  // Notify parent of exit
  Send(parent_, "exited " + name_);

  // Unregister with server
  UnregisterMachine();

  // Tell all spawned to exit
  for (const auto& [k, v] : spawned_) {
    Address address;
    std::istringstream iss(k);
    iss >> address;
    Address server(address.ip(), kServerPort);
    Send(server, "stop " + std::to_string(address.port()));
  }

  while (!mailbox_->Flushed()) {
  }

  on_exit_();
}

void Machine::Exit() {
  Info() << uid_ << " exited" << std::endl;
  running_ = false;
  return;
}

void Machine::Error(const std::string& message) {
  Info() << uid_ << " errored: " << message << std::endl;
  error_message_ = message;

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
  Info() << uid_;
  if (current_.empty()) {
    Info() << " transitioned to ";
  } else {
    Info() << " transitioned from " << current_ << " to ";
  }
  Info() << state << std::endl;
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
    } else {
      break;
    }
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
  Info() << uid_ << " > " << to << ' ' << message << std::endl;
  mailbox_->Send(to, message);
}

void Machine::SendAt(const Address& to, const std::string& message,
                     const std::chrono::system_clock::time_point time) {
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

void Machine::CheckChildren(const std::chrono::system_clock::time_point now) {
  std::vector<std::string> dead;
  for (const auto& [k, v] : spawned_) {
    if (const auto d = now - v.second; d > kHeartbeatTimeout) {
      dead.push_back(k);
    }
  }

  for (const auto& c : dead) {
    if (const auto& it = spawned_.find(c); it != spawned_.end()) {
      const auto k = it->first;
      const auto v = it->second;

      Address address;
      std::istringstream iss(k);
      iss >> address;

      {
        // Send errored message
        std::ostringstream oss;
        oss << "errored ";
        oss << v.first;
        oss << " heartbeat timeout";
        HandleMessage(now, address, oss.str());
      }

      {
        // Send exited message
        std::ostringstream oss;
        oss << "exited ";
        oss << v.first;
        HandleMessage(now, address, oss.str());
      }
    }
  }
}

void Machine::SendPulse() { Send(parent_, "pulsed " + name_); }

void Machine::SendScheduled(const std::chrono::system_clock::time_point now) {
  std::vector<ScheduledMessage> q;
  q.swap(queue_);
  for (const auto& e : q) {
    if (e.time < now) {
      Send(e.address, e.message);
    } else {
      queue_.push_back(e);
    }
  }
}

void Machine::ReceiveMessage(const std::chrono::system_clock::time_point now) {
  Address sender;
  std::string message;
  if (mailbox_->Receive(sender, message)) {
    HandleMessage(now, sender, message);
  }
}

void Machine::HandleMessage(const std::chrono::system_clock::time_point now,
                            const Address& from, const std::string& message) {
  Info() << uid_ << " < " << from << ' ' << message << std::endl;

  std::istringstream iss(message);
  std::string t;
  iss >> t;

  // Supervision
  std::ostringstream oss;
  oss << from;
  const auto key = oss.str();
  if (t == "exit") {
    Exit();
  } else if (t == "started") {
    // Start tracking spawned health
    std::string name;
    iss >> name;
    spawned_.emplace(key, std::make_pair(name, now));
    // Seek back to start of name to make it available to the message handler.
    iss.seekg(-name.length(), std::ios_base::cur);
  } else if (t == "exited") {
    spawned_.erase(key);
  } else if (t == "pulsed") {
    if (auto it = spawned_.find(key); it != spawned_.end()) {
      it->second.second = now;
    }
  }

  // Receivers
  auto s = current_;
  while (!s.empty()) {
    if (const auto it = states_.find(s); it != states_.end()) {
      const auto rs = it->second.receivers_;
      if (const auto i = rs.find(t); i != rs.end()) {
        i->second(from, iss);
        return;
      } else if (const auto i = rs.find(""); i != rs.end()) {
        std::istringstream iss(message);
        i->second(from, iss);
        return;
      } else {
        // Message not handled by state, try parent
        s = it->second.parent_;
      }
    } else {
      ::Error() << uid_ << ": No such state: " << s << std::endl;
      Error("Unrecognized state: " + s);
    }
  }
  if (t != "exit") {
    // Message not handled by hierarchy
    ::Error() << uid_ << ": Failed to handle message" << std::endl;
    Error("Unhandled message: " + t);
  }
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
