#!/bin/sed -f

# Example usage:
# sh$ $ echo 1 + 10 + 0 + 11  | sed -f ./samples/binary-add.sed
# 0
# 1
# 1

# bc version:
# sh$ echo 'ibase=2; obase=2; 1 + 10 + 0 + 11' | bc
# 110

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
