#ifndef STATE_H
#define STATE_H

#include <functional>
#include <map>

typedef std::function<void()> Trigger;
typedef std::function<void(const Address&, std::istream&)> Receiver;

class State {
 public:
  State(const std::string name, const std::string parent,
        const Trigger on_enter, const Trigger on_exit,
        const std::map<const std::string, Receiver> receivers)
      : name_(name),
        parent_(parent),
        on_enter_(on_enter),
        on_exit_(on_exit),
        receivers_(receivers) {}
  State(const State& s) = delete;
  State(State&& s) = delete;
  State& operator=(const State& m) = delete;
  State& operator=(State&& m) = delete;
  ~State() {}

  const std::string name_;
  const std::string parent_;
  const Trigger on_enter_;
  const Trigger on_exit_;
  const std::map<const std::string, Receiver> receivers_;
};

#endif
