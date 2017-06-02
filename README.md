# Inner Product

## What is this?

This program is a demonstration of asynchronous use of the OpenMPI library in
C++. The program generates a random matrix of MxN size and computes the dot
product for every combination of rows (except for row i x row i). It will then
perform a serial check to validate the answer.

## Compiling

You can either compile with the makefile or with mpic++ yourself.
```
# With the makefile
$ make

# On your own
$ mpic++ *.cc -o main -g --std=c++11
```

## Running
The number of procs includes the master thread. So if you want to run 4 workers:
```
export NUM_PROCS=5
```

Here is an example of running with that environment variable.
```
# Assuming you've compiled to -o main
$ mpirun -np $NUM_PROCS ./main --rows 500 --columns 16
```

Or without
```
# Assuming you've compiled to -o main
$ mpirun -np 5 ./main --rows 500 --columns 16
```

## Help

```
$ ./main --help
Usage: main [OPTION...]

  -m, --columns=columns      The size of each vector.
  -n, --rows=rows            The number of vectors.
  -v, --verbose              Verbose mode.
  -?, --help                 Give this help list
      --usage                Give a short usage message

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```
