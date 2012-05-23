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
of partitioning a set of terminals. Our implementation follows this structure.
It is a red-black ordering of the problem.

The current Makefile is hardcoded. I don't expect portability to be an issue, 
since the code is essentially flat C. This will be the last thing we'll worry 
about.

You might want to consider using Neal's proximal library on kona.

TODO
====
* Write some unit tests.
* Make `connect` override-able or at least user-supplied.

Some notes
==========
I tried using this code to interface with our OPSP solver. It doesn't quite work. 
The issue is with the following idea in ADMM (or DR splitting, for that matter)

    x^{k+1} = prox_f(z^k - u^k)

The prox function of f can be specified in a `C` library (assuming we know what f 
looks like). The problem with creating a graph admm library where the use specifies 
the prox function for each vertex in the graph is that each vertex could have a 
different prox function. Each of these functions has to be written *by hand* in order
to be passed to our vertices.

This would not be a problem in `C` or `C++` if we could create functions programmatically,
but since we can't (since functions are not first-class objects), the library is pretty
useless. It is possible to create global objects and global indices inside the functions
to access the parameters, but that's the kind of programming we've all been taught to avoid.

For fun, it's possible to write a compiler to read a network specification and *emit* the
`C` code, since we are technically talking about a simulation, but that feels like
using a sledgehammer where we didn't need it in the first place.

A piece of `C` code like this is useful for sensor nodes and hooking them up, but
it's not general purpose enough for anybody else to really use in their own programming
projects. They might as well write their own ADMM code.

The goal was to save others the trouble of writing ADMM code. They would simply provide
the definitions for

    prox_{f_i}
    prox_{g_i}
    
for all vertices i, specify how these things were connected, and call `admm_solve` or something.

It's not that easy when the f's and g's are similar (say, linear functions) but require parameters.

I can't write a general purpose library for a function that takes any kind of parameters. I need
users to supply the parameters and then just pass me the resulting function. But `C` can't do this!

Guess I'll pass a `void *`...

Inline
======
Some of the code segments are very short, but they don't get inlined properly by the compilers.
Not sure if there's a workaround for this.

Notes about library
===================
We should be responsible for passing a vector `v` to the prox function and responsible for 
updating the primal and dual variables through the updated / modified `v`. You'll get a
`float *` along with an `int` to describe the length.