# Compiling a sed script

Translate **sed** to **C** and generate a binary that will have the exact same
behavior as a sed script, basically `echo foo | sed s/foo/bar/` will be
replaced by `echo foo | ./sed-bin`.

# Table of contents

* [Compiling a sed script](#Compiling-a-sed-script)
* [Quick start](#Quick-start)
  * [Setup](#Setup)
  * [How to use](#How-to-use)
  * [Quick step-by-step](#Quick-step-by-step)
  * [Full walk-through with a bigger script](#Full-walk-through-with-a-bigger-script)
* [How it works](#How-it-works)
  * [Some generated code](#Some-generated-code)
* [Why](#Why)
* [Translating the translator](#Translating-the-translator)
* [Notes](#Notes)

# Quick start

## Setup
Clone the repo and move inside its directory, you'll need the usual UNIX
core and build utils (sed, libc, C compiler, shell, make).

*Note: this project is currently tested with the GNU libc (2.30), GNU sed (4.5)
and GCC (9.2.1)*

## How to use

### Quick step-by-step

Let's take a simple example:
```bash
sh$ echo foo | sed s/foo/bar/
bar
```
Assuming you want to compile the statement above, you can use the small
`compile` shell script wrapping up the C translation and compilation steps:
```bash
sh$ echo s/foo/bar/ | ./compile
+ cat
+ ./par.sed
+ make
cc    -c -o sed-bin.o sed-bin.c
cc    -c -o address.o address.c
cc    -c -o operations.o operations.c
cc    -c -o read.o read.c
cc   sed-bin.o address.o operations.o read.o   -o sed-bin
Compiled sed script available: ./sed-bin
```

Almost done, once the generated C code is compiled you can use the resulting
`sed-bin` binary in place of `sed s/foo/bar/`:
```bash
sh$ echo foo | ./sed-bin
bar
```

That's about it!

### Full walk-through with a bigger script

Say you want to compile the following sed script
[samples/generate-table-of-contents](./samples/generate-table-of-contents),
which is used to generate the table of contents of this project's README.

```sed
#!/bin/sed -f

# ignore code blocks
/^```/,//d

# no need to index ourselves
/^# Table of contents/d

# found header
/^#/{
  # save our line and first work on the actual URI
  h
  # strip leading blanks
  s/^#*[[:blank:]]*//
  s/[[:blank:]]/-/g
  # punctuation and anything funky gets lost
  s/[^-[:alnum:]]//g
  # swap with hold and work on the displayed title
  x
  # get rid of last leading # and potential white spaces
  s/^\(#\)*#[[:blank:]]*/\1/
  # the remaining leading # (if any) will be used for indentation
  s/#/  /g
  # prepare the first half of the markdown
  s/\( *\)\(.*\)/\1* [\2](#/
  # append the link kept and remove the newline
  G
  s/\(.*\)[[:space:]]\(.*\)/\1\2)/p
}
d
```

Let's use the translator [par.sed](./par.sed) which is a big sed script
translating other sed scripts to C code, redirect the output to a file named
`generated.c`. Another file with some declarations called `generated-init.c`
will be created by the translator automatically. You'll need those two files to
generate a working binary.

```sh
sh$ sed -f par.sed < samples/generate-table-of-contents > generated.c
```

Now we're ready to compile the generated code (you'll note that for simplicity
and readability the generated code is mostly functions calls, the actual C code
doing the work is not generated):

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
sh$ ./sed-bin < README.md
* [Compiling a sed script](#Compiling-a-sed-script)
* [Quick start](#Quick-start)
  * [Setup](#Setup)
  * [How to use](#How-to-use)
  * [Quick step-by-step](#Quick-step-by-step)
  * [Full walk-through with a bigger script](#Full-walk-through-with-a-bigger-script)
* [How it works](#How-it-works)
  * [Some generated code](#Some-generated-code)
* [Why](#Why)
* [Translating the translator](#Translating-the-translator)
* [Notes](#Notes)
```

Some other example sed scripts are available in the [samples](./samples)
directory:
- [tic-tac-toe game](./samples/tic-tac-toe.sed)
- [binary addition](./samples/binary-add.sed)
- [par.sed (sed to C translator)](./par.sed)

# How it works

## Some generated code
The translator [par.sed](./par.sed) (which is written in sed itself) converts
sed commands calls to valid C code:

```bash
sh$ echo y/o/u/ | sed -f ./par.sed
```

Will output:

```c
y(&status, "o", "u");
```

The actual logic to handle `y` (and most other commands) is not generated, we
just need to translate the sed syntax to valid C code, which here stays fairly
readable.

Let's look at one more example:

```sed
/foo/{
  p;x
}
```

Translates to:
```c
static Regex reg_1 = {.compiled = false, .str = "foo"};
if (addr_r(&status, &reg_1))
{

p(&status);
x(&status);

}
```

# Why

Not much practical use to this, here are some thoughts:

- Debugging a sed script is hard, one possible way is to run `sed` in gdb,
  but this assumes some familiarity with the implementation. Here the generated
  C code is rather close to the original sed script, which should allow gdb to
  be easier to use (`make -B CFLAGS=-g` for symbols).
- Might be useful for obfuscation or maybe to limit the scope of sed? Resulting
  binaries are usually smaller than a full `sed` binary as well.
- Better speed? Since the generated code is specific to a script, one might
  expect it to be much faster than using `sed`, since we can skip parsing,
  walking the AST etc. I didn't do any serious measurements yet, but so far it
  seems slightly faster than GNU sed (around 20% faster to translate the
  translator for instance).

# Translating the translator

The basic idea of this project is to translate **sed** code to **C** code, to
compile it and have a resulting binary with the same behavior as the original
script.

Now since the translator from sed to C is written in sed, we should be able to
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

Generated code is identical, which means that at this point we have a standalone
binary that is able to translate other sed scripts to C and that we no longer
need another sed implementation as a starting point to make the translation.

# Notes

- Missing/incomplete features (supporting/fixing those is planned):
  - leading `#n` marker is not handled
  - with 2 addresses, the `c` command will be executed every time for each
  matching line instead of only once when leaving the range.

- The translator does not handle invalid sed scripts, it will just generate
  invalid C code which will probably fail to compile, make sure you can run your
  script with an actual `sed` implementation before attempting to translate it.

- Non POSIX support is currently not planned, if you are using GNU sed, you can
  try to see what is not supported by running your script with the `--posix`
  option. Also check out the [POSIX specification](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/sed.html).

- It is assumed no options are passed to `sed` (`-n` is a common option for
  instance), supporting those is not planned for the moment.

- There are some bugs, the C code is very rough around the edges (by that I mean
  dirty and unsafe, for instance allocating everything on the stack without
  checking any overflow), I'm still working on it, but contributions
  (issues/comments/pull requests) are also welcomed.
