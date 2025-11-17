# Turing

A Machine which, when initialized with a set of quintuples of a [Turing Machine](https://en.wikipedia.org/wiki/Turing_machine), will create a [Universal Turing Machine](https://en.wikipedia.org/wiki/Universal_Turing_machine) capable of computing the same sequence.

## Parameters

### Blank Symbol

A non-whitespace character, such as '_' (Underscore) or '0' (Zero), representing a blank or empty cell on the Tape.

### Quintuples

1. Current State
2. Symbol Read from Tape
3. Symbol Written to Tape
4. Head Movement over Tape
5. Next State

Quintuples are semicolon-separated, with individual elements represented as single characters.

## Determinations and Assumptions

1. The set of states is determined by the set of current states in the quintuples.
2. The initial state is assumed to be the current state of the first quintuple.
3. There is a single final state represented by 'H' (Halt).
4. The alphabet of symbols is determined by the set of symbols read from and written to the tape in the quintuples.
5. The alphabet of symbols does not include any whitespace characters.
6. The Tape is finite at any given time, but will automatically grow at both ends to effectively make it infinite.

## Usage

First, the 'turing/Universal' machine is started with a blank symbol and set of quintuples.

Next, the machine will accept a request and perform the following steps;
1. initialize the Tape to the message contents.
2. run the state machine until reaching the 'Halt' state.
3. send the contents of the Tape back to the address the issued the request.
4. transition back to the initial state to accept the next request.

### Alternating Squares

Generates a repeating sequence of 0_1_0_1_0_1... (until '.' is encountered).

```
$ build/src/Wink start turing/Universal localhost _ "A_0RB;A..NH;B__RC;B..NH;C_1RD;C..NH;D__RA;D..NH"
> localhost:42000 start turing/Universal :0 _ A_0RB;A..NH;B__RC;B..NH;C_1RD;C..NH;D__RA;D..NH
< <address> started turing/Universal

$ build/src/Wink send -r 1 <address> "_______________."
> <address> _______________.
< <address> 0_1_0_1_0_1_0_1.
```

### Busy Beaver

Generates six ones on the tape.

```
$ build/src/Wink start turing/Universal localhost 0 "A01RB;A11LC;B01LA;B11RB;C01LB;C11NH"
> localhost:42000 start turing/Universal :0 0 A01RB;A11LC;B01LA;B11RB;C01LB;C11NH
< <address> started turing/Universal

$ build/src/Wink send -r 1 <address> "000000000000000"
> <address> 000000000000000
< <address> 111111000000000000
```
