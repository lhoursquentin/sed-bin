#!/bin/sed -f

# The first line of the hold space is used for temporary storage in this script,
# never use it to store data longer than a single command.

1{
  x
  s/^/\
/
  x
}

: start

# remove ; and spaces
s/^[; ][; ]*//g
t start

# If empty line, read the next one
/^$/{
  n
  b start
}

# curly braces need to be printed, and then removed
s/^[{}]/&\
/
t curly_bracket_found
# if this last check failed then we can assume the next chars will be an actual
# command
b cmd_check

: curly_bracket_found
# print and remove
P
s/^..//
t start

: cmd_check
s/^b[[:blank:]]*//; t b_cmd
s/^s//; t s_cmd

: address_check
s|^/|&|; t addr_regex
s|^\\\(.\)|\1|; t addr_regex
s/^[0-9]/&/; t addr_number
s/^./invalid command: &/
t fail
s/^/Missing command/
t fail

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
# If we are processing the second address in a range, we want to avoid adding a
# newline since we have the beginning of the C code for this range at the bottom
# of the hold.
/^[rn]r/!s/$/\
/
s/$/"/
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
t addr_regex_handle_end

# Found end of single regex addr, a second address might follow
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
/^r[^rn]/{
  # single address, we need to check if another one follows

  x
  s/^[[:blank:]]*,[[:blank:]]*\([^[:blank:]]\)/\1/
  x
  t append_comma
  b regex_close_function

  : append_comma
  s/$/, /
  x
  t address_check
}

: regex_close_function
# close C function call + add ";" if not an address
s/$/)/
/^s/s/$/;/
x
# push remaining current line on hold
H
# clean hold
# 1st line is hold unrelated to the current processing (except for the leading
# command name)
# 2rd line is the C code that we need to print, we'll swap it last
# 3rth line is the rest of the line on which the s cmd was
g
s/^\(.*\)\
\(.*\)\
\(.*\)$/\1\
\3\
\2/

# save result to hold
h
# get rid of everything except C code (which is last), and print it, this is
# also where we actually complete the name and fixed args of the function.
# The very top of the hold contains the info needed to generate the correct
# function name
s/^\([^[:space:]]*\).*\
/\1(status, /
s/^[nr].*/if (&)/
p
# clean the C code from the hold
g
s/^\(.*\)\
.*/\1/
h
# hold still contains current line, we need to remove it from there and also
# have it in the pattern space
s/.*\
//
x
s/\(.*\)\
.*/\1/
# reset sub success value
t valid_hold_reorder
: valid_hold_reorder

# clean temp chars at the top of the hold
s/^[^[:space:]]*//
x
t start
b fail

: b_cmd

# eat what we will process right now, what remains is the future current line
s/^[^[:blank:];}][^[:blank:];}]*/goto &;\
/
t valid_b_parsing
s/^/b command parsing: /
b fail

: valid_b_parsing
P
s/.*\
//
t start
s/^/b cleanup: /
b fail

n
: fail
s/^/FAIL - /
q
