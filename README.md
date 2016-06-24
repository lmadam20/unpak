unpak
================================
Copyright (c) 2016 Leon Adam
(see file 'LICENSE' for licensing information)

Extracts files from Quake's PAK archives. (Only tested with Quake 1)

Usage
-------------
`./unpak <.pak file> <target directory>`

Compiling
-------------
Manually with GCC: `gcc -o unpak pak.* main.c`
...or use `make` (the executeable is placed in bin/ then)

TODO
-------------
* Make folder creation more portable