#!/bin/sed -f

:start
H

s/^b//; t b_cmd
s/^/invalid command: /
b fail

: b_cmd
# assume current line is saved at the bottom of the hold space

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


s/^\([; ]*\)*//g
t nop
: nop

/^$/n

b start

n
: fail
s/^/FAIL - /
q
