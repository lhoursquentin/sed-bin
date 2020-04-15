#!/bin/sed -f

# The first line of the hold space is used for temporary storage in this script,
# never use it to store data longer than a single command.
# Second line will act as an id to create unique variable names, though this
# line should be located from the bottom.

1{
  x
  s/^/\
reg_x/
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
s|^#|//|; t comment
s/^b[[:blank:]]*\([^;}][^[:blank:];}]*\)/goto \1;\n/; t label_cmds
s/^t[[:blank:]]*\([^;}][^[:blank:];}]*\)/if (status.sub_success) { status.sub_success = false; goto \1; }\n/; t label_cmds
# semi-colon needed since declarations cannot directly follow a label in C
s/^:[[:blank:]]*\([^;}][^[:blank:];}]*\)/\1:;\n/; t label_cmds
s/^r[[:blank:]]*//; t r_cmd
s/^s//; t s_cmd
s/^y//; t y_cmd
s/^[hHgGlpPqx]/&(\&status);\
/
t single_char_cmd
s/^=/equal(\&status);\
/
t single_char_cmd
s/^d/{ &(\&status); continue; }\
/
t single_char_cmd
s/^D/if (&(\&status) == CONTINUE) continue;\
/
t single_char_cmd
s/^[Nn]/if (&(\&status) == BREAK) break;\
/
t single_char_cmd

s/^\([aci]\)[[:blank:]]*\\$/\1/; t aci_cmds

# TODO missing cmds
# w, l

: address_check
s|^/|&|; t addr_regex
s|^\\\(.\)|\1|; t addr_regex
s/^[0-9]/&/; t addr_number
s/^\$//; t addr_last_line
s/^./invalid command: &/
t fail
s/^/Missing command/
t fail

: comment
p
d

: single_char_cmd
P
s/.*\
//
t start
s/^/single char cmd cleanup: /
b fail

: addr_last_line
x
# work on the hold, if second address, do not add a newline (we've already built
# the start of the C code on a new line)
/^[^rn]/s/$/\
/
x
# add address followed by a newline, the pattern should never have more than one
# newline
s/^/status.last_line_nb\n/
# save address to hold and strip it from pattern, only leaving rest of the line
H
s/^.*\n//
x
# back to hold
# remove H call newline and the very last line which is the rest of the current
# line.
# include address type at the very top (n).
s/^\([rn]*\)\(.*\)\
\(.*\)\
.*/\1n\2\3/
t valid_s_or_addr_parsing
b fail

: addr_number
x
# work on the hold, if second address, do not add a newline (we've already built
# the start of the C code on a new line)
/^[^rn]/s/$/\
/
x
# save address to hold and strip it from pattern, only leaving rest of the line
H
s/^[0-9]*//
x
# back to hold
# remove H call newline and the rest of the line (only keep the number), also
# include address type at the very top (n).
s/^\([rn]*\)\(.*\)\
\([0-9][0-9]*\).*/\1n\2\3/
t valid_s_or_addr_parsing
b fail

: addr_regex
x
s/^/r/
t regex_start_process

: r_cmd
s/["\]/\\&/g
s/.*/r(\&status, "&");/
n
b start

: s_cmd
# s cmd needs a scope since for the case:
#   /foo/s/bar/baz/
#   if addr("foo") static reg = ...; s(reg);
# Here static ends up alone in the if, which is no good, so we add a scope:
#   if addr("foo") { static reg = ...; s(reg); }
# This issue cannot happen with addresses since they cannot be chained without
# brackets: /foo//bar/p -> invalid but /foo/{/bar/p} -> valid and not an issue.
i \
{

x
# at the top of the hold, track the number of delimiters encountered:
# s/foo -> s0 but s/foo/bar -> s1
s/^/s0/

t regex_start_process

: y_cmd
x
s/^/y0/
t regex_start_process

: regex_start_process
# insert start of s C code at the bottom of the hold (we omit the name of the
# function since we don't know here if we are currently processing the s command
# or a regex address)
# If we are processing the second address in a range, we want to avoid adding a
# newline since we have the beginning of the C code for this range at the bottom
# of the hold.
/^.[^rn]/s/$/\n/
s/$/\n/

# check if this is an empty pattern, in which case we want to use the last one

/^[rs]/{
  x
  /^\(.\)\1/{
    s//\1/
    x
    s/\n$/status.last_regex/
    t regex_valid_delim_eaten
  }
  x
}
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

/^\(.\)\1/{
  s//\1/
  x
  s/$/"/
  t regex_valid_delim_eaten
}

# case of escaped delimiter s/bin\/bash/bin\/sh/, in that case we just remove
# the backslash and process the char as any non delimiter one.
s/^\(.\)\\\1/\1\
\1/
t regex_save_char
s/^\(.\)\\n/\\n\
\1/
t regex_save_char
s/^\(.\)\(\\\\\)/\2\2\
\1/
t regex_save_char
# TODO, backslash should be removed when no op, example \j -> j
# Literal double quotes and backslashes must be escaped in the C code
s/^\(.\)\([\"]\)/\\\2\
\1/
t regex_save_char

# Default case, normal character
s/^\(.\)\(.\)/\2\
\1/
t regex_save_char

: regex_save_char
H
# get rid of eaten char and newline
s/.*\
//
x
# On the bottom line we have: {chars eaten}<newline>{delim}{rest of line}, we
# just want to append the {chars eaten} to the end of the line before the last,
# the one containing the C code under construction.
s/\(.*\)\
\(.*\)\
.*/\1\2/
x
t regex_eat_next

: regex_valid_delim_eaten

# Found second delim for the s cmd
s/^s1\(.*\)$/s\1, 0/
t s_cmd_handle_options

# case of regex closing a range: swap chars since we insert from the beginning
s/^r\([rn]\)/\1r/

/^y/b skip_regex_creation
# At this point if we do not have a string on the last line then that means
# we're in the last_regex case, skip regex creation
/"$/!b skip_regex_creation
s/\(.*\)\
\(.*\)\
\(.*\)\
\(.*\)/\1\
\2x\
\3\&\2\
static Regex \2 = {.compiled = false, .str = \4};/
# save current line we are working on
G
# save everything to hold
h
# only keep regex declaration and print it
s/.*\n\(.*\)\n.*/\1/p
# restore everything
g
# cleanup line we were working on
s/.*\n//
x
# get rid of regex declaration and saved current line
s/\(.*\)\n.*\n.*/\1/
: skip_regex_creation
s/y1/y/

# Found first delim for the s/y cmd
/^[sy]0/{
  s/^\([sy]\)0\(.*\)$/\11\2, "/
  x
  t regex_eat_next
}

x
# remove delim, we don't need to keep it anymore
s/.//
x
t valid_s_or_addr_parsing

# POSIX specifies s valid opts are: g, <nth occurence>, w <file> and p
# TODO handle all s options
: s_cmd_handle_options
# At this point we don't know yet if there are any options
x
# remove delim, we don't need to keep it anymore
s/.//
t s_cmd_eat_options

# could do something shorter with y/gp/GP/
: s_cmd_eat_options
s/^g/G/
t s_cmd_add_prefix_opt
s/^p/P/
t s_cmd_add_prefix_opt

x
b valid_s_or_addr_parsing

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

: valid_s_or_addr_parsing
/^[rn]/{
  # range, we'll append the LINE macro which will act as an unique id (this is
  # "better" than COUNTER (since LINE is standardized) as long as we can
  # guarantee that we'll never generate two range calls on the same line, that's
  # why using the "=" command is not an option)
  /^.[rn]/{
    s/$/, __LINE__/
    t s_or_addr_close_function
  }
  # single address, we need to check if another one follows
  x
  s/^[[:blank:]]*,[[:blank:]]*\([^[:blank:]]\)/\1/
  x
  t append_comma
  b s_or_addr_close_function

  : append_comma
  s/$/, /
  x
  t address_check
}

: s_or_addr_close_function
# close C function call + add ";" if not an address
s/$/)/
/^[sy]/s/$/;/
/^s/s/$/ }/
x
# negative address
/^[[:blank:]]*!/{
  s///
  x
  # invert result with xor, unfortunately C and sed negation are on the opposite
  # side of the operand, so we'll do with that for now.
  s/$/ ^ true/
  x
}
# push remaining current line on hold
H
# clean hold
# 1st line is hold unrelated to the current processing (except for the leading
# command name)
# 2nd line is the C code that we need to print, we'll swap it last
# 3rd line is the rest of the line on which the s cmd was
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
/\1(\&status, /
s/^[nr].*/if (addr_&)/
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

: label_cmds
P
s/.*\
//
t start
s/^/label cmds cleanup: /
b fail

: aci_cmds
N
s/\\$//
t aci_cmds
# remove first newline
s/\n//
# "\n" -> '\n'
s/\\n/\n/g
# \<any char> -> <any char>
s/\\\(.\)/\1/g
# quotes and backslashes must be escaped for the C
s/[\"]/\\&/g
# '\n' -> "\n" for the C
s/\n/\\n/g
s/^i\(.*\)/i("\1");/
s/^a\(.*\)/a(\&status, "\1");/
s/^c\(.*\)/{ c(\&status, "\1"); continue; }/
n
b start

: fail
s/^/FAIL - /
q
