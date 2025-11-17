// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_MACHINE_H_
#define INCLUDE_WINK_MACHINE_H_

#include <Wink/address.h>
#include <Wink/client.h>
#include <Wink/log.h>
#include <Wink/mailbox.h>
#include <Wink/state.h>
#include <unistd.h>

#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

static std::atomic_bool got_sigterm = false;
void SignalHandler(int signal);

class Machine {
 public:
  Machine(std::string name, Address& address, Address& parent)
      : Machine(name, new AsyncMailbox(new UDPSocket(address)), address,
                parent) {}
  Machine(std::string name, Mailbox* mailbox, Address& address, Address& parent)
      : name_(name), mailbox_(mailbox), address_(address), parent_(parent) {
    std::signal(SIGTERM, SignalHandler);
  }
  Machine(const Machine& m) = delete;
  Machine(Machine&& m) = delete;
  Machine& operator=(const Machine& m) = delete;
  Machine& operator=(Machine&& m) = delete;
  ~Machine() { delete mailbox_; }
  /**
   * Returns this Machine's Unique Identifier.
   */
  std::string UID() const { return uid_; }
  /**
   * Begins execution of the state machine and sends a
   * 'started' message to the machine which spawned this machine.
   *
   * Parameter initial sets the state in which the state machine
   * will start. If empty, the state machine will start in the
   * first state that was added.
   */
  void Start(const std::string& initial = "");
  /**
   * Immediately ceases execution of the state machine and sends an
   * 'exited' message to the machine which spawned this machine.
   */
  void Exit();
  /**
   * Immediately ceases execution of the state machine and sends an
   * 'errored' message and the given error message to the machine which
   * spawned this machine.
   */
  void Error(const std::string& message);
  /**
   * Adds the given state to this state machine.
   */
  void AddState(State state);
  /**
   * Transitions the state machine to the given state.
   */
  void Transition(const std::string& state);
  /**
   * Transmits a message to the given address.
   */
  void Send(const Address& to, const std::string& message);
  /**
   * Sends the given address the message at the given time.
   */
  void SendAt(const Address& to, const std::string& message,
              const std::chrono::system_clock::time_point time);
  /**
   * Sends the given address the message after the given delay.
   */
  void SendAfter(const Address& to, const std::string& message,
                 const std::chrono::seconds delay);
  /**
   * Spawns a new state machine.
   */
  void Spawn(const std::string& machine);
  /**
   * Spawns a new state machine with the given arguments.
   */
  void Spawn(const std::string& machine, const std::vector<std::string>& args);
  /**
   * Spawns a new state machine at the destination address.
   */
  void Spawn(const std::string& machine, const Address& destination);
  /**
   * Spawns a new state machine at the destination address with the given
   * arguments.
   */
  void Spawn(const std::string& machine, const Address& destination,
             const std::vector<std::string>& args);

  std::function<void()> on_exit_ = []() { _exit(0); };

 private:
  void CheckChildren(const std::chrono::system_clock::time_point now);
  void SendPulse();
  void SendScheduled(const std::chrono::system_clock::time_point now);
  void ReceiveMessage(const std::chrono::system_clock::time_point now);
  void HandleMessage(const std::chrono::system_clock::time_point now,
                     const Address& from, const std::string& message);
  void RegisterMachine(const std::string& machine, const int pid);
  void UnregisterMachine();
  std::vector<std::string> StateLineage(const std::string& state);

  std::string name_ = "";
  Mailbox* mailbox_;
  Address& address_;
  Address& parent_;
  std::string uid_ = "";
  std::atomic_bool running_ = true;
  std::map<const std::string, State> states_;
  std::string current_ = "";
  std::string error_message_ = "";
  struct ScheduledMessage {
    const Address& address;
    const std::string message;
    const std::chrono::system_clock::time_point time;
  };
  std::vector<ScheduledMessage> queue_;
  std::map<const std::string,
           std::pair<const std::string, std::chrono::system_clock::time_point>>
      spawned_;
};

void PruneLineage(std::vector<std::string>&, std::vector<std::string>&);

std::pair<std::string, std::string> ParseMachineName(const std::string& name);

#endif  // INCLUDE_WINK_MACHINE_H_
