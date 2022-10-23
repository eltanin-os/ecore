#!/bin/execlineb -S3
backtick tmpdir { mktemp -d }
backtick PROGS { pipeline { find src -type f -name  "*.c" } pipeline { sed -e "s;.c$;;" -e "s;src/;;" } sort }
multisubstitute {
	importas -sD "cc" CC CC
	importas -sD "-O0 -g -std=c99 -Wall -Wextra -pedantic" CFLAGS CFLAGS
	importas -sD "" CPPFLAGS CPPFLAGS
	importas -sD "" LDFLAGS LDFLAGS
	importas -iu tmpdir tmpdir
	importas -isu PROGS PROGS
}
foreground { redo-ifchange lib/libcommon.a }
foreground {
	foreground {
		forx -E file { $PROGS }
		foreground {
			redirfd -r 0 src/${file}.c
			redirfd -w 1 ${tmpdir}/${file}.c
			sed "s;^main(;${file}_&;"
		}
		redirfd -a 1 ${tmpdir}/prototypes.h
		echo "ctype_status ${file}_main(int, char **);"
	}
	foreground {
		backtick -E PROGNAMES { echo "\" ${PROGS}\"" }
		redirfd -w 1 ${tmpdir}/ecore.c
		heredoc 0
"#include <tertium/cpu.h>
#include <tertium/std.h>
#include \"common.h\"
#include \"prototypes.h\"

static char *names = \"\"
${PROGNAMES}
\"\\n\";

ctype_status
main(int argc, char **argv)
{
	char *s;
	s = c_gen_basename(argv[0]);
	if (!C_STR_CMP(\"ecore\", s)) {
		--argc, ++argv;
		if (argc) s = *argv;
	}
	if (0) ;
"
		cat
	}
	foreground {
		forx -E file { $PROGS }
		redirfd -a 1 ${tmpdir}/ecore.c
		echo "\telse if (!C_STR_SCMP(\"${file}\", s)) return ${file}_main(argc, argv);"
	}
	foreground {
		redirfd -a 1 ${tmpdir}/ecore.c
		heredoc 0
"	else if (!C_STR_CMP(\"md5sum\", s)) {
		return digest_main(argc, argv);
	} else if (!C_STR_CMP(\"sha1sum\", s)) {
		return digest_main(argc, argv);
	} else if (!C_STR_CMP(\"sha256sum\", s)) {
		return digest_main(argc, argv);
	} else if (!C_STR_CMP(\"sha512sum\", s)) {
		return digest_main(argc, argv);
	} else {
		c_ioq_put(ioq1, names+1);
		c_ioq_flush(ioq1);
	}
	return 0;
}
"
		cat
	}
	elglob C "${tmpdir}/*.c"
	$CC $CFLAGS $CPPFLAGS $LDFLAGS -Iinc -o $3 $C lib/libcommon.a -ltertium
}
rm -Rf $tmpdir
