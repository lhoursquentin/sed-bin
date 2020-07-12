FreeBSD 12.1

When using the "y" command, it seems like square bracket balancing is required.
They should instead be treated like any other character the "y" cmd context, as
they are already at the time of doing the actual character replacement:

```
sh$ echo | sed 'y/[/x/'
sed: 1: "y/[/x/": unbalanced brackets ([])
sh$ echo | sed 'y/[]/xy/'
sed: 1: "y/[]/xy/": unbalanced brackets ([])
sh$ echo | sed 'y/[a]/xyz/'

sh$ echo '][a' | sed 'y/[a]/xyz/'
zxy
```
