#!/bin/rc -e
DEPS=`{find $MAINDIR/lib -type f -name '*.c'}^'.o'
redo-ifchange $DEPS
$AR rc $3 $DEPS
$RANLIB $3
