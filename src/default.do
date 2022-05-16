#!/bin/rc -e
if (~ $1 *.c) exit
redo-ifchange $2.c.o ../lib/libcommon.a
$CC $LDFLAGS -o $3 $2.c.o ../lib/libcommon.a -ltertium
