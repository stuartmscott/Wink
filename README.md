# Wink

Wink is a framework for developing Fault Tolerant Systems with Asynchronous Concurrent Independent Hierarchical Finite State Machines.

## Fault Tolerance

> A system is fault-tolerant if it continues working even if something is wrong.
>
> Fault-tolerance cannot be achieved using a single computer – it may fail.
>
> -- *Joe Armstrong*

### Principles

1. Isolated Fault Units - split software into multiple small independent modules that run concurrently to achieve a single objective or fail fast.
2. Supervision Tree - modules can monitor the lifecycles of submodules.
3. Message Passing - modules communicate asynchronously and do not share state.
4. Live Upgrade - modules can be updated without restarting the whole system.
5. Persistent Records - logs are written to storage that can survive reboot.

## State Machine

A State Machine;
- Runs Independently and Concurrently for Fault Isolation.
- Has a Lifecycle that is monitored by its Parent for Error Detection and Resolution.
- Communicates Asynchronously to minimize Latency and maximize Throughput.
- Can be Updated and Restarted without affecting others State Machines.
- Writes Logs to either stdout or the filesystem for Debuggability.
- Has a Hierarchy of States to minimize Code Duplication.
- Is Uniquely Identified by its Name, and Network Address;
  - name consists of;
    - package and executable: `family/Parent` or `family/Child`
    - optional tag: `family/Child#Alice` or `family/Child#Bob`
  - address can either be local, or remote;
    - local: `:<port>` or `localhost:<port>` or `127.0.0.1:<port>`
    - remote: `<ip>:<port>`

### States

Each State consists of a unique Name, an optional Parent, an optional Entry Action, an optional Exit Action, and a set of Receivers.

### Actions

An action is triggered when a state is entered or exited.

### Receivers

A receiver is triggered upon receipt of a matching message.

If the optional empty receiver exists, it is triggered if no other receivers match, else the unhandled message is handled by the parent state. If no parent exists, or the message is not handled by the hierarchy, an error is raised.

### Example

```
#include <iostream>
#include <string>

#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    error() << "Incorrect parameters, expected <address> <parent>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[0]);
  UDPSocket socket;
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, socket, address, parent);

  m.AddState(State(
      // State Name
      "off",
      // Parent State
      "",
      // On Entry Action
      []() { info() << "Switch is OFF\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      {
          {"on", [&](const Address &sender,
                     std::istream &args) { m.Transition("on"); }},
          {"off", [&](const Address &sender,
                      std::istream &args) { m.Transition("off"); }},
      }));

  m.AddState(State(
      // State Name
      "on",
      // Parent State
      "off",
      // On Entry Action
      []() { info() << "Switch is ON\n"
                    << std::flush; },
      // On Exit Action
      []() {},
      // Receivers
      {}));

  m.Start();
}
```

## Lifecycle

When a State Machine spawns another, the parent receives lifecycle messages from the child.

In the `success` case, the parent will receive;

- started - the child indicates it has started and provides the parent with the name of its binary and the address (ip:port) it has bound to.
- pulsed - the child indicates it is still alive by sending a heartbeat message every 10 seconds.
- exited - the child indicates it has terminated.

In the `error` case, the parent will receive;

- started - same as above.
- pulsed - same as above.
- errored - the child indicates it has encountered an error by sending the error message to the parent.
- exited - same as above.

If a parent doesn't receive a heartbeat for 1 minute it will assume the child has failed (maybe the computer crashed, lost power, or the network disconnected - who knows?!).

When a parent is notified that a child has errored, it can chose to do nothing, restart the child, or raise an error. In the last situation, the grandparent will be notified that the parent has errored.

### Example

```
#include <iostream>
#include <string>

#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    error() << "Incorrect parameters, expected <address> <parent>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[0]);
  UDPSocket socket;
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, socket, address, parent);

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() {
        info() << "Parent: OnEntry\n" << std::flush;
        m.Spawn("family/Child");
      },
      // On Exit Action
      []() { info() << "Parent: OnExit\n"
                    << std::flush; },
      // Receivers
      {
          {"started",
           [&](const Address &sender, std::istream &args) {
             std::string child;
             args >> child;
             info() << "Parent: " << sender << ' ' << child << " has started\n"
                    << std::flush;
           }},
          {"pulsed",
           [&](const Address &sender, std::istream &args) {
             std::string child;
             args >> child;
             info() << "Parent: " << sender << ' ' << child << " has pulsed\n"
                    << std::flush;
           }},
          {"errored",
           [&](const Address &sender, std::istream &args) {
             std::string child;
             args >> child;
             std::ostringstream os;
             os << args.rdbuf();
             info() << "Parent: " << sender << ' ' << child
                    << " has errored: " << os.str() << '\n'
                    << std::flush;
           }},
          {"exited",
           [&](const Address &sender, std::istream &args) {
             std::string child;
             args >> child;
             info() << "Parent: " << sender << ' ' << child << " has exited\n"
                    << std::flush;
             m.Transition("main"); // Retry
           }},
      }));

  m.Start();
}
```

```
#include <iostream>
#include <sstream>
#include <string>

#include <Wink/address.h>
#include <Wink/log.h>
#include <Wink/machine.h>
#include <Wink/state.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    error() << "Incorrect parameters, expected <address> <parent>\n"
            << std::flush;
    return -1;
  }

  std::string name(argv[0]);
  UDPSocket socket;
  Address address(argv[1]);
  Address parent(argv[2]);
  Machine m(name, socket, address, parent);

  m.AddState(State(
      // State Name
      "main",
      // Parent State
      "",
      // On Entry Action
      [&]() { m.Error("AHHHHH"); },
      // On Exit Action
      []() {},
      // Receivers
      {}));

  m.Start();
}
```

```
Directory: build/samples/                                                       # Directory of Binaries
Address: 127.0.0.1:42000                                                        # Server Listening Address
< 127.0.0.1:50498 start family/Parent :0                                        # Server Receives Client Request
Forked: 41972                                                                   # Process Created
127.0.0.1:56950 family/Parent started                                           # Parent Starts
127.0.0.1:56950 family/Parent > 127.0.0.1:50498 started family/Parent           # Parent Sends Lifecycle Event to Client
127.0.0.1:56950 family/Parent > 127.0.0.1:42000 register family/Parent 41972    # Parent Registers with Server
Parent: OnEntry                                                                 # Parent Enters `main` State
127.0.0.1:56950 family/Parent > 127.0.0.1:42000 start family/Child :0           # Parent Issues Spawn Request
< 127.0.0.1:56950 register family/Parent 41972                                  # Server Receives Parent Registration
< 127.0.0.1:56950 start family/Child :0                                         # Server Receives Parent Spawn Request
Forked: 41973                                                                   # Process Created
127.0.0.1:64701 family/Child started                                            # Child Starts
127.0.0.1:64701 family/Child > 127.0.0.1:56950 started family/Child             # Child Sends Lifecycle Event to Parent
127.0.0.1:64701 family/Child > 127.0.0.1:42000 register family/Child 41973      # Child Registers with Server
127.0.0.1:64701 family/Child errored: AHHHHH                                    # Child Triggers an Error
127.0.0.1:56950 family/Parent < 127.0.0.1:64701 started family/Child            # Parent Receives Child Lifecycle Event
127.0.0.1:64701 family/Child > 127.0.0.1:56950 errored family/Child AHHHHH      # Child Sends Lifecycle Event to Parent
127.0.0.1:64701 family/Child exited                                             # Child Exits
127.0.0.1:64701 family/Child > 127.0.0.1:56950 exited family/Child              # Child Sends Lifecycle Event to Parent
< 127.0.0.1:64701 register family/Child 41973                                   # Server Receives Parent Registration
Parent: 127.0.0.1:64701  has started                                            # Parent Logs Child Lifecycle Event
127.0.0.1:64701 family/Child > 127.0.0.1:42000 unregister                       # Child Unregisters with Server
127.0.0.1:56950 family/Parent < 127.0.0.1:64701 errored family/Child AHHHHH     # Parent Receives Child Lifecycle Event
Parent: 127.0.0.1:64701 family/Child has errored:  AHHHHH                       # Parent Logs Child Lifecycle Event
127.0.0.1:56950 family/Parent < 127.0.0.1:64701 exited family/Child             # Parent Receives Child Lifecycle Event
< 127.0.0.1:64701 unregister                                                    # Server Receives Child Unregistration
Parent: 127.0.0.1:64701 family/Child has exited                                 # Parent Logs Child Lifecycle Event
Parent: OnExit                                                                  # Parent Exits `main` State
```

## Communication

Machines communicate via asynchronous message passing using UDP.

Each machine maintains a sequence counter for each recipient it to sends, the current value of which is included in each message sent, and incremented afterwards.

### Reliability

UDP is fast but unreliable, providing no guarantees that a packet is delivered. To somewhat remedy this, recipients respond with an acknowledgement upon receipt of a message, and senders will retry failed messages.

Consider the scenario:
- Machine A sends Message M to Machine B.
- If B receives M, it responds with Acknowledgement K containing M's sequence number.
- If A does not receive K within 10 seconds, it will resend M, up to 3 times.
- If B receives M and sends K, but A does not receive K it will resend M. B will ignore the duplicate M but will resend K.

## Repository Layout

 - include: header files
 - samples: code samples
 - src: source code files
 - test: test code files

## Build

```cmake
cmake -S . -B build
cmake --build build
```

## Test

```
(cd build/test/src && ctest)
```

## Docker

```
docker build . -t wink:latest
```

# Usage

## Client
```
./build/src/Wink
```

### Start

Starts a new machine from a binary.

```
./build/src/Wink start [options] <name>
./build/src/Wink start [options] <name> :<port>
./build/src/Wink start [options] <name> <ip>:<port>
```

### Stop

Stops an existing machine.

```
./build/src/Wink stop [options] :<port>
./build/src/Wink stop [options] <ip>:<port>
```

### Send

Sends a message to a machine.

```
./build/src/Wink send [options] :<port> <message>
./build/src/Wink send [options] <ip>:<port> <message>
```

### List

List existing machines running on a server.

```
./build/src/Wink list [options]
./build/src/Wink list [options] :<port>
./build/src/Wink list [options] <ip>:<port>
```

### Help

```
./build/src/Wink help
./build/src/Wink help <command>
```

## Server

Starts the WinkServer serving from the given directory.

```
./build/src/WinkServer serve <directory>
```

### Help

```
./build/src/WinkServer help
./build/src/WinkServer help <command>
```

---

Inspired by @winksaville
