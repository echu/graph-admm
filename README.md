graph-admm
==========

A simple framework to perform ADMM on graphs of low-degree.

Before running `make`, you must build the shared library in the folder
"gadmm."

The `main.c` file is just an example of how to build a binary / process that
mimics an embedded controller we use to hook up a network.

Running `make` will build the library under the directory `lib` and compile
the example in the directory `bin`.

This is the source directory for gadmm--a shared C library used to perform
ADMM on graph-based problems. In particular, it is designed to solve problems
of the form

  minimize f(x) + I_C(y) subject to x = y

where I_C(y) is the indicator function over the set C = { y | sum(y) = 0 }. (I
think. I need to double-check.)

The Makefile will produce a .a file (static library) which you can then use in
your code. The API requires that the solve() and objective() functions be
passed in to the node structures.

In our paper, device and nets have the same priority: they are just two ways
of partitioning a set of terminals. Our implementation is slightly different:
nets exist virtually (they exist on a device). A terminal has knowledge of
which net it is a part of.

The current Makefile is hardcoded. I don't expect portability to be an issue, 
since the code is essentially flat C. This will be the last thing we'll worry 
about.

You might want to consider using Neal's proximal library on kona.

TODO
====
* Write some unit tests.
* Make `connect` override-able or at least user-supplied.