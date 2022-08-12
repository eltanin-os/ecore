#!/bin/execlineb -S3
multisubstitute {
	importas -D "cc" CC CC
	importas -sD "" LDFLAGS LDFLAGS
}
foreground { redo-ifchange ${2}.c.o ../lib/libcommon.a }
$CC $LDFLAGS -o $3 ${2}.c.o ../lib/libcommon.a -ltertium
