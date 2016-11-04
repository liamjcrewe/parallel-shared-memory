# parallel-shared-memory

Coursework completed as part of 'Parallel Programming' final year module for BSc Computer Science at University of Bath.

## Compiling
Running ```make``` will compile to bin/solve.

Other targets are:
* debug (turn warnings and debugging output on)
* balena (command to compile on University of Bath's HPC facility)
* clean (remove compiled code)

## Running
### Basic operation
Run ```bin/solve [problemId] [threads] [precision]```. There are five pre-defined problems. These consist of square two dimensional input arrays of varying dimensions. The contents of these are doubles between 0 and 100 that were all randomly generated The dimensions are:
* Problem 1: 4 x 4
* Problem 2: 5 x 5
* Problem 3: 9 x 9
* Problem 4: 10 x 10
* Problem 5: 40 x 40

### Help
Run ```bin/solve [--help|-h]``` for help.
