# Compiling a sed script

Translate **sed** to **C** and generate a binary that will have the exact same
behavior as a sed script, basically `echo foo | sed 's/foo/bar/'` will be
replaced by `echo foo | ./sed-bin`.

# Quick start

Clone the repo and move inside its directory, you'll need the usual UNIX
core and build utils (sed, libc, C compiler, shell, make).

*Note: this project is currently tested with the GNU libc (2.30), GNU sed (4.5)
and GCC (9.2.1)*

Say you want to compile the following sed script called `binary-add.sed` (see
the `samples` directory):
```sed
s/[[:blank:]]//g
h
: start
s/[01]+/+/g
s/[01]$//g
s/++*/+/g
s/^+*//
s/+*$//
x
s/[01]*\([01]\)+/\1+/g
s/[01]*\([01]\)$/\1/g

t reduce
: reduce
s/0+\([01]\)/\1/; t reduce
s/\([01]\)+0/\1/; t reduce

/^$/d

s/1+1/0/
t handle_carry

p
g
b start

: handle_carry
x
s/^/1+/
s/+$//
x
b reduce
```

Its purpose is to compute binary additions, for instance:

```sh
sh$ echo 1+1+10+101+1 | sed -f binary-add.sed
0
1
0
1
```

Now let's translate it to C, pass the script to the translator named `par.sed`
and write the output to a file named `generated.c`:

```sh
sh$ sed -f par.sed < binary-add.sed > generated.c
```

Compile the generated code:

```sh
sh$ make
cc    -c -o sed-bin.o sed-bin.c
cc    -c -o address.o address.c
cc    -c -o operations.o operations.c
cc    -c -o read.o read.c
cc   sed-bin.o address.o operations.o read.o   -o sed-bin
```

A binary named `sed-bin` has been generated, it should have the exact same
behavior as the sed script:

```sh
sh$ echo 1+1+10+101+1 | ./sed-bin
0
1
0
1
```

That's about it!

Here's a shorter example, compiling `s/foo/bar/`:

```sh
sh$ echo 's/foo/bar/' | ./par.sed > generated.c
sh$ make
cc    -c -o sed-bin.o sed-bin.c
cc   sed-bin.o address.o operations.o read.o   -o sed-bin
sh$ echo foo | ./sed-bin
bar
```

# Why

Not much practical use to this, here are some thoughts:

- Debugging a sed script is hard, one possible way is to run `sed` in gdb,
  but this assumes some familiarity with the implementation. Here the generated
  C code is rather close to the original sed script, which should allow gdb to
  be much easier to use (`make -B CFLAGS=-g` for symbols).
- One might find this useful for obfuscation or maybe to limit the scope of sed?
- Better speed? Since the generated code is specific to a script, one might
  expect it to be much faster than using `sed`, since we can skip parsing,
  walking the AST etc. Though with the current implementation a compiled script
  is roughly 4 times slower than GNU sed, this is mostly due to having to
  compile all regexes each time, instead of once, which I'm still working on.

# Translating the translator

The basic idea of this project is to translate **sed** code to **C** code, to
compile it and have a resulting binary with the same behavior as the original
script.

Now since the translator from sed to C is written is sed, we should be able to
translate the translator, compile it and then be able to use the compiled
version to translate other sed scripts.

Translate the translator (`par.sed`) with itself:

```sh
sh$ ./par.sed < ./par.sed > generated.c
```

```sh
sh$ make
cc    -c -o sed-bin.o sed-bin.c
cc    -c -o address.o address.c
cc    -c -o operations.o operations.c
cc    -c -o read.o read.c
cc   sed-bin.o address.o operations.o read.o   -o sed-bin
```

We now have a binary that should be able to translate sed code, let's try to
translate the translator with it:

```sh
sh$ ./sed-bin < ./par.sed | diff -s generated.c -
Files generated.c and - are identical
```

# Notes

- Some commands are missing (currently c, l, w, r, y), and some features are
  missing as well (empty label jumps, "$" line address, "#n" marker).
  Supporting those is planned.

- The translator does not handle invalid sed scripts, it will just generate
  invalid C code which will probably fail to compile, make sure you can run your
  script with an actual `sed` implementation before attempting to translate it.

- Non POSIX support is currently not planned, if you are using GNU sed, you can
  try to see what is not supported by running your script with the `--posix`
  option. Also check out the [POSIX specification](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/sed.html).

- It is assumed no options are passed to `sed` (`-n` is a common option for
  instance), supporting those is not planned.

- There are some bugs, the C code is very rough around the edges (by that I mean
  dirty and unsafe, for instance allocating everything on the stack without
  checking any overflow), I'm still working on it, but contributions
  (issues/comments/pull requests) are also welcomed.
