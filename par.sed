#!/bin/sed -f

# The first line of the hold space is used for temporary storage in this script,
# never use it to store data longer than a single command.

: start
H

s/^b//; t b_cmd
s/^s//; t s_cmd
s|^/|&|; t addr_regex
s|^\\\(.\)|\1|; t addr_regex
s/^[0-9]/&/; t addr_number
s/^/invalid command: /
b fail

# assume current line is saved at the bottom of the hold space

# For addresses layout is as follows:

: addr_number
b fail

: addr_regex
x
s/^/r/
t regex_start_process

: s_cmd

x
# at the top of the hold, track the number of delimiters encountered:
# s/foo -> s0 but s/foo/bar -> s1
s/^/s0/

t regex_start_process

: regex_start_process
# insert start of s C code at the bottom of the hold (we omit the name of the
# function since we don't know here if we are currently processing the s command
# or a regex address)
s/$/\
(status, "/
# reset sub success value
t regex_insert_c_start
: regex_insert_c_start
x

: regex_eat_next

# Case where we only have our delimiter on the line, meaning there's a newline
# in the s command
/^.\\$/{
  # remove escape
  s/\\$//
  x
  # insert literal \n in C code
  s/$/\\n/
  x
  # read next line and remove automatic newline between delim and next line
  N
  s/^\(.\)\
/\1/
  t regex_eat_next
}

s/^\(.\)\1/\1/
t regex_valid_delim_eaten
# case of escaped delimiter s/bin\/bash/bin\/sh/, in that case we just remove
# the backslash and process the char as any non delimiter one.
s/^\(.\)\\\1/\1\1/
t regex_save_escaped_delim
# literal double quotes and backslashes must be escaped in the C code
s/^\(.\)\([\"]\)/\1\\\2/
t regex_save_char_with_escape

b regex_save_char

: regex_save_char_with_escape

# {delim}{escape+char} -> {escape+char}{delim}
s/^\(.\)\(..\)/\2\1/
H
# get rid of escape + eaten char
s/..//
x
# On the bottom line we have: {escape + char eaten}{delim}{rest of line}, we
# just want to append the {char eaten} to the end of the line before the last,
# the one containing the C code under construction.
s/\(.*\)\
\(..\).*/\1\2/
x

t regex_eat_next

: regex_save_char

# {delim}{char} -> {char}{delim}
s/^\(.\)\(.\)/\2\1/
# we can skip the swap if it was an escaped delim since \1 == \2
: regex_save_escaped_delim
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
t regex_eat_next

: regex_valid_delim_eaten
x

# Found end of second regex addr, swap chars since we insert from the beginning
s/^r\([nr]\)\(.*\
\)\(.*\)$/\1r\2\3"/
t valid_regex_parsing

s/^r\([^nr].*\
\)\(.*\)$/r\1\2"/
t addr_regex_handle_end

# Found second delim for the s cmd
s/^s1\(.*\
\)\(.*\)$/s\1\2", 0/
t s_cmd_handle_options

# Found first delim for the s cmd
s/^s0\(.*\
\)\(.*\)$/s1\1\2", "/
x
t regex_eat_next

: addr_regex_handle_end
s/^r/addr_regex/
x
# remove delim, we don't need to keep it anymore
s/.//
x
t valid_regex_parsing

# POSIX specifies s valid opts are: g, <nth occurence>, w <file> and p
# TODO handle all s options
: s_cmd_handle_options
# At this point we don't know yet if there are any options
x
# remove delim, we don't need to keep it anymore
s/.//
t s_cmd_eat_options

# could do something shorter with y/[gp]/[GP]/
: s_cmd_eat_options
s/^g/G/
t s_cmd_add_prefix_opt
s/^p/P/
t s_cmd_add_prefix_opt

x
b valid_regex_parsing

: s_cmd_add_prefix_opt
# save to hold and remove processed option from pattern
H
s/.//
x
# process and clean saved line
s/\(.*\)\
\(.\).*/\1 | S_OPT_\2/
x
t s_cmd_eat_options

: valid_regex_parsing
s/$/);/
x
# push remaining current line on hold
H
# clean hold
# 1st line is hold unrelated to the current processing (except for the leading
# command name)
# 2nd line is initially saved line that we didn't use, we can get rid of it
# 3rd line is the C code that we need to print, we'll swap it last
# 4rth line is the rest of the line on which the s cmd was
g
s/^\(.*\)\
.*\
\(.*\)\
\(.*\)$/\1\
\3\
\2/

# save result to hold
h
# get rid of everything except C code (which is last), and print it.
# the leading chars represent the name of the function
s/^\([^[:space:]]*\).*\
/\1/p
# clean the C code from the hold and the leading function name
g
s/^[^[:space:]]*\(.*\)\
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
s/^[^;}][^;}]*//
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
b *\([^;}][^;}]*\).*/goto \1;/p
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
