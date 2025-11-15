// Copyright 2022-2025 Stuart Scott
#ifndef INCLUDE_WINK_STATE_H_
#define INCLUDE_WINK_STATE_H_

#include <functional>
#include <map>
#include <string>

typedef std::function<void()> Trigger;
typedef std::function<void(const Address&, std::istream&)> Receiver;
typedef std::map<const std::string, Receiver> ReceiverMap;

class State {
 public:
  State(const std::string name, const std::string parent,
        const Trigger on_enter, const Trigger on_exit,
        const ReceiverMap receivers)
      : name_(name),
        parent_(parent),
        on_enter_(on_enter),
        on_exit_(on_exit),
        receivers_(receivers) {}
  State(const State& s)
      : name_(s.name_),
        parent_(s.parent_),
        on_enter_(s.on_enter_),
        on_exit_(s.on_exit_),
        receivers_(s.receivers_) {}
  State(State&& s)
      : name_(s.name_),
        parent_(s.parent_),
        on_enter_(s.on_enter_),
        on_exit_(s.on_exit_),
        receivers_(s.receivers_) {}
  State& operator=(const State& s) = delete;
  State& operator=(State&& s) = delete;
  ~State() {}

  const std::string name_ = "";
  const std::string parent_ = "";
  const Trigger on_enter_;
  const Trigger on_exit_;
  const ReceiverMap receivers_;
};

#endif  // INCLUDE_WINK_STATE_H_
