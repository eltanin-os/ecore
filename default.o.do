#!/bin/execlineb -S3
multisubstitute {
	importas -sD "cc" CC CC
	importas -sD "-O0 -g -std=c99 -Wall -Wextra -pedantic" CFLAGS CFLAGS
	importas -sD "" CPPFLAGS CPPFLAGS
}
if { redo-ifchange ../inc/common.h $2 }
$CC $CFLAGS $CPPFLAGS -I../inc -o $3 -c $2
