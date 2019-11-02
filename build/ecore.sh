#!/bin/sh
mkdir tmpbuild
tmpdir='tmpbuild'
__delete='true'
clean() {
	$__delete && rm -Rf $tmpdir
}
trap clean 0
cat <<EOF > $tmpdir/ecore.c
#include <tertium/cpu.h>
#include <tertium/std.h>

#include "common.h"
#include "names.h"
#include "prototypes.h"

#define STRCMP(a, b) c_mem_cmp((a), sizeof((a)) - 1, (b))

int
main(int argc, char **argv) {
	char *s = c_gen_basename(sdup(*argv));
	if (!STRCMP("ecore", s)) {
		--argc, ++argv;
		if (argc) s = *argv;
	}
	if (0) {
		;
	}
EOF
echo 'char *names = ""\' > $tmpdir/names.h
for f in $@; do
	p="$(basename $f .c)"
	sed "s/\(^main(.*)\)/${p}_\1/" < $f > $tmpdir/$p.c
	echo "int ${p}_main(int, char **);" >> $tmpdir/prototypes.h
	cat <<EOF >>$tmpdir/ecore.c
	else if (!STRCMP("$p", s)) {
		return ${p}_main(argc, argv);
	}
EOF
	echo "\"$p \"\\" >> $tmpdir/names.h
done
echo '"\n";' >> $tmpdir/names.h
cat <<EOF >> $tmpdir/ecore.c
	else {
		c_ioq_put(ioq1, names);
		c_ioq_flush(ioq1);
	}
	return 0;
}
EOF
__delete='false'
