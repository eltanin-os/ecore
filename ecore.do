#!/bin/rc -e
MAINDIR=$PWD
. $MAINDIR/config.rc
CFILES=$MAINDIR'/'^`{find src -type f -name '*.c' | sort}
redo-always
redo-ifchange $MAINDIR/lib/libcommon.a
fn sigexit {
	rm -Rf $dir
}
dir=`{mktemp -d}
cp $CFILES $dir
for (file in $CFILES) {
	tmp=`{basename $file}
	f=`{basename $file .c | sed 's/-/_/g'}
	sed 's/^main(/'$"f'_&/' <$file >$dir/$tmp
}
cat <<EOF >$dir/ecore.c
#include <tertium/cpu.h>
#include <tertium/std.h>
#include "common.h"

EOF
for (file in $CFILES) {
	f=`{basename $file .c | sed 's/-/_/g'}
	echo 'ctype_status '$"f'_main(int, char **);'
} >> $dir/ecore.c
echo >>$dir/ecore.c
echo 'static char *names = ""\' >>$dir/ecore.c
{ for (file in $CFILES) echo "`{basename $file .c}"'\' } >>$dir/ecore.c
cat <<EOF >>$dir/ecore.c
"\n";

ctype_status
main(int argc, char **argv)
{
	char *s;
	s = c_gen_basename(argv[0]);
	if (!C_STR_CMP("ecore", s)) {
		--argc, ++argv;
		if (argc) s = *argv;
	}
	if (0) ;
EOF
for (file in $CFILES) {
	f=`{basename $file .c | sed 's/-/_/g'}
	printf '\telse if (!C_STR_CMP("'$"f'", s)) return '$"f'_main(argc, argv);\n'
} >> $dir/ecore.c
cat <<EOF >>$dir/ecore.c
	else {
		c_ioq_put(ioq1, names);
		c_ioq_flush(ioq1);
	}
	return 0;
}
EOF
LIBS=$MAINDIR/lib/libcommon.a
$CC $CFLAGS $CPPFLAGS $LDFLAGS -I$MAINDIR/inc -o $3 $dir/*.c $LIBS -ltertium
