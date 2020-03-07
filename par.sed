#!/bin/sed -f

:start
H

s/^b//; t b_cmd
s/^s//; t s_cmd
s/^/invalid command: /
b fail

# assume current line is saved at the bottom of the hold space

: s_cmd

x
# insert start of s C code at the bottom of the hold
# the leading number indicates the number of delimiters we've encountered so far
s/$/\
0s(status, "/; t s_cmd_insert_c_start; : s_cmd_insert_c_start
x

: s_cmd_eat_next

# Case where we only have our delimiter on the line, meaning there's a newline
# in the s command
/^.$/{
  x
  # insert litteral \n in C code
  s/$/\\n/
  x
  # read next line and remove automatic newline between delim and next line
  N
  s/^\(.\)\
/\1/
  t s_cmd_eat_next
}

s/^\(.\)\1/\1/
t valid_delim_eaten
s/^\(.\)\\\1/\1/
t valid_escaped_delim_eaten

# {delim}{char} -> {char}{delim}
s/^\(.\)\(.\)/\2\1/
H
# get rid of eaten char
s/.//
x
# On the bottom line we have: {char eaten}{delim}{rest of line}, we just want to
# append the {char eaten} to the end of the line before the last, the one
# containing the C code under construction.
s/\(.*\)\
\(.\).*/\1\2/
x
t s_cmd_eat_next

: valid_escaped_delim_eaten
# TODO
b s_cmd_eat_next

: valid_delim_eaten
x

# TODO handle s cmd options
# FIXME this is flaky, the 1 might match something unrelated might want to try
# [^<newline>]*$ instead of .*, assuming it works

# Found second delim
s/^\(.*\
\)1\(.*\)$/\1\2", 0);/
t valid_s_parsing

# Found first delim
s/^\(.*\
\)0\(.*\)$/\11\2", "/
x
t s_cmd_eat_next

: valid_s_parsing
x
# push remaining current line on hold
H
# clean hold
# 1st line is hold unrelated to s
# 2nd line is initially saved line that we didn't use, we can get rid of it
# 3rd line is the C code that we need to print, we'll swap it last
# 4rth line is the rest of the line on which the s cmd was, we need to remove
#   the leading delimiter from it.
g
s/^\(.*\)\
.*\
\(.*\)\
.\(.*\)$/\1\
\3\
\2/

# save result to hold
h
# get rid of everything except C code (which is last), and print it
s/.*\
//p
# clean the C code from the hold
g
s/\(.*\)\
.*/\1/
h
# hold still contains current line, we need to remove it from there and also
# have it in the pattern space
s/.*\
//
x
s/\(.*\)\
.*/\1/
x
t valid_cleanup
b fail

: b_cmd

# eat what we will process right now, what remains is the future current line
s/^[^;][^;]*//
t valid_b_parsing
s/^/b command parsing: /
b fail

: valid_b_parsing
# save future current line
H
# restore full hold pattern with current line + future current line
g
# swap current line and future current line (c-f -> f-c)
s/\(.*\)\
\(.*\)\
\(.*\)/\1\
\3\
\2/
t valid_b_internal_swap
s/^/internal b command swap: /
b fail
: valid_b_internal_swap

# save full and strip current line to have a clean hold
h
s/\(.*\)\
.*/\1/
t valid_b_internal_hold_cleaning
s/^/internal b hold cleaning: /
b fail
: valid_b_internal_hold_cleaning

# swap and now actually work on the current line
x
s/.*\
b *\([^;][^;]*\).*/goto \1;/p
t valid_b_translation
s/^/b translation: /
b fail
: valid_b_translation
# current context is still dirty
# remove first saved line and work on partial line
# hold has future current line at the bottom
g
# only keep future current line in pattern, which becomes new current line
s/.*\
\(.*\)/\1/
t valid_cleanup
s/^/b cleanup: /
b fail

: valid_cleanup

s/^\([; ]*\)*//g
t separator_and_spaces_removed
: separator_and_spaces_removed

/^$/n

b start

n
: fail
s/^/FAIL - /
q
