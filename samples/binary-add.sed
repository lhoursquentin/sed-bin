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
