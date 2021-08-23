# crisp3

Yet another sequel to the barely-working
[crisp](https://github.com/nonk123/crisp) project.

crisp3 can't interpret any Lisp yet. It is simply a garbage collector
with a C API for allocating new objects.

## Building

* `make` builds the main executable, which, in its current state, is
  completely useless and exits with a 0 status.
* `make tests` runs the currently defined tests for various components
  of the interpreter. They come in .c files with a `main` function and
  a `test_` prefix. Plain and simple.
* `make clean` deletes the compiled object-files and executables.
