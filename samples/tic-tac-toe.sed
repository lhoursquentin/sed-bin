#!/bin/sed -f

1c \
Press enter to start playing\
\
Controls:\
  a => move left\
  d => move right\
  enter => place your choice

2{
  s/.*/x\
-------\
|?| | |\
-------\
| | | |\
-------\
| | | |\
-------/
  t next
}

s/a//; t left
s/d//; t right
s/^$//; t enter

d

:left
g
s/\(.*\) \([^?]*\)?/\1?\2 /; t next
d

:right
g
s/?\([^ ]*\) / \1?/; t next
d

:enter
g
s/^\(.\)\(.*\)?/\1\2\1/; t check_victory

:placed
s/ /?/; t switch_player_indicator
b draw

:switch_player_indicator
s/^x/o/; t next
s/^o/x/; t next

:next
h
s/^.//
p
d

:win
s/^\(.\)\(.*\)/\2\
\
Winner: \1/
q

:draw
s/^\(.\)\(.*\)/\2\
\
Draw/
q

:check_victory
/\([xo]\)|\1|\1/{
  b win
}

/-------\
|\([xo]\)|.|.|\
-------\
|\1|.|.|\
-------\
|\1|.|.|\
-------/{
  b win
}

/-------\
|.|\([xo]\)|.|\
-------\
|.|\1|.|\
-------\
|.|\1|.|\
-------/{
  b win
}

/-------\
|.|.|\([xo]\)|\
-------\
|.|.|\1|\
-------\
|.|.|\1|\
-------/{
  b win
}

/-------\
|\([xo]\)|.|.|\
-------\
|.|\1|.|\
-------\
|.|.|\1|\
-------/{
  b win
}

/-------\
|.|.|\([xo]\)|\
-------\
|.|\1|.|\
-------\
|\1|.|.|\
-------/{
  b win
}

b placed
