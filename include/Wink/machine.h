#ifndef MACHINE_H
#define MACHINE_H

#include <Wink/address.h>
#include <Wink/client.h>
#include <Wink/log.h>
#include <Wink/socket.h>
#include <Wink/state.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>

class Machine {
 public:
  Machine(std::string name, Socket& socket, Address& address, Address& parent)
      : name_(name), socket_(socket), address_(address), parent_(parent) {}
  Machine(const Machine& m) = delete;
  Machine(Machine&& m) = delete;
  Machine& operator=(const Machine& m) = delete;
  Machine& operator=(Machine&& m) = delete;
  ~Machine() {}
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
  void AddState(std::unique_ptr<State>&& state);
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
              const std::chrono::time_point<std::chrono::system_clock> time);
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
  void CheckChildren(
      const std::chrono::time_point<std::chrono::system_clock> now);
  void SendPulse();
  void SendScheduled(
      const std::chrono::time_point<std::chrono::system_clock> now);
  void ReceiveMessage(
      const std::chrono::time_point<std::chrono::system_clock> now);
  void HandleMessage(
      const std::chrono::time_point<std::chrono::system_clock> now);
  void RegisterMachine(const std::string& machine, const int pid);
  void UnregisterMachine();

  std::string name_;
  Socket& socket_;
  Address& address_;
  Address& parent_;
  std::string uid_;
  bool running_ = true;
  Address sender_;
  char buffer_[kMaxPayload];
  std::map<const std::string, const std::unique_ptr<State>> states_;
  std::string current_;
  struct ScheduledMessage {
    const Address& address_;
    const std::string message_;
    const std::chrono::time_point<std::chrono::system_clock> time_;
  };
  std::vector<ScheduledMessage> queue_;
  std::map<const std::string,
           std::pair<const std::string,
                     std::chrono::time_point<std::chrono::system_clock>>>
      spawned_;
};

std::pair<std::string, std::string> ParseMachineName(const std::string& name);

#endif
