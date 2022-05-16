#!/bin/rc -e
redo-ifchange ../inc/common.h $2
$CC $CFLAGS $CPPFLAGS -I../inc -o $3 -c $2
