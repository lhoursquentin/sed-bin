#!/bin/sed -f
s/^s\(.\)\([^\1]*\)\1\([^\1]*\)\1$/s(status, "\2", "\3")/
s/^[ghGHpx]$/&(status)/
s/$/;/
