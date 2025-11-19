# Time

Demonstrates various time-related use cases.

## After

Send a message after a given delay.

### Usage

```
$ Wink start time/After
```

## At

Send a message at a specific time.

### Usage

```
$ Wink start time/At
```

## StopWatch

Implements a StopWatch for calculating how much time has elapsed between the watch being started and stopped.

This state machine has two states; `idle` and `timing`.

When the machine is in state `idle` and receives;

- idle - the machine transitions back to idle.
- start - the machine records the current time and transitions to timing.
- stop - the machine transitions back to idle.
- exit - the machine exits.

When the machine is in state `timing` and receives;

- stop - the machine records the current time, send the elapsed time to the sender, and transitions back to `idle`.

The state `idle` is the parent of `timing`, so when `timing` receives an `idle`, `start`, or `exit` message, it is handled by the parent state.

### Usage

```
$ Wink start time/StopWatch
$ Wink send <stopwatch> start
$ Wink send <stopwatch> stop

```

## Ticker

Implements a Ticker which sends the spawner a `tick` message periodically.

### Usage

```
$ Wink start time/Ticker 2 # Receive tick every 2 seconds
$ Wink send <ticker> exit
```
