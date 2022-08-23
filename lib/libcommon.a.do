#!/bin/execlineb -S3
multisubstitute {
	importas -sD "ar" AR AR
	importas -sD "ranlib" RANLIB RANLIB
	elglob DEPS "*.c"
}
foreground { redo-ifchange ${DEPS}.o }
foreground { $AR rc $3 ${DEPS}.o }
$RANLIB $3
