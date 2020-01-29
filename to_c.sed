#!/bin/sed -f
s/^s/s/
t s_cleaning
: start_replacing
s/^s\(.\)\(.*\)\1\(.*\)\1g/s(status, "\2", "\3", S_OPT_G)/
t add_semi
s/^s\(.\)\(.*\)\1\(.*\)\1/s(status, "\2", "\3", 0)/
t add_semi
s/^[ghGHpx]$/&(status)/
t add_semi
s/^t \(.*\)/if (status->sub_success) { status->sub_success = false; goto \1; }/
t end
s/^b /goto /
t add_semi
s/^:[[:blank:]]*\(.*\)/\1:/
t end

: add_semi
s/^[^#].*/&;/

b end

: s_cleaning

s/\\/\\\\/g
s/"/\\"/g
t start_replacing
b start_replacing

: end
