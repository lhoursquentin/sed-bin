FreeBSD 12.1

# Issue

When adding leading text with `s/^/some text/`, if the pattern space contains
multiple lines with the first one being empty then the pattern space remaining
lines are deleted (case 2 below).

One liner reproducer:
```
sh$ echo content | sed 'H; g; s/^/preceding/'
preceding

sh$
```

# 1 - Non empty first line case

```sed
s/.*/1st line\
2nd line\
3rd line/

l
i \
-----

s/^/some preceding text /

l
```

```
sh$ echo | sed -nf non-empty-first-line.sed
1st line$
2nd line$
3rd line$
-----
some preceding text 1st line$
2nd line$
3rd line$
```

# 2 - Empty first line case

```sed
s/.*/\
2nd line\
3rd line/

l
i \
-----

s/^/some preceding text /

l
```

```
sh$ echo | sed -nf empty-first-line.sed
$
2nd line$
3rd line$
-----
some preceding text $
$
```

# 3 - Empty first line case workaround

```sed
s/.*/\
2nd line\
3rd line/

l
i \
-----

s/.*/some preceding text &/

l
```

```
sh$ echo | sed -nf empty-first-line-workaround.sed
$
2nd line$
3rd line$
-----
some preceding text $
2nd line$
3rd line$
```

