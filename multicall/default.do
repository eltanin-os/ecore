#!/usr/bin/automate -s std-default,std-c
%programs{
	transform{
		${%%glob-edit}
		glob:../src/*.c
		edit:sed -e "s;.*/;;" -e "s;.c$$;;"
	}
}
@program{
	ecore
}
ecore{
	library{
		../lib/libcommon.a
		libtertium
	}
	${%programs}.c.o
}
ecore.c{
	output:$3
	module:std-rules
	rules{
		head
		body
		tail
	}
	head{
		module:std-heredoc
		heredoc{
			#include <tertium/cpu.h>
			#include <tertium/std.h>
			#include "common.h"
			#include "prototypes.h"

			static char *names = "$:{%programs}";

			ctype_status
			main(int argc, char **argv)
			{
				char *s = c_gen_basename(*argv);
				if (!C_STR_CMP("ecore", s)) {
					--argc, ++argv;
					if (argc) s = *argv;
				}
				if (0) {;}
			}
		}
	}
	body{
		module:std-map
		map:$:{%programs}
		apply{
			command:echo "else if (!C_STR_SCMP("${<}", s)) return ${<}_main(argc, argv);"
		}
	}
	tail{
		module:std-heredoc
		heredoc{
			else if (!C_STR_CMP("md5sum", s)) {
				return digest_main(argc, argv);
			} else if (!C_STR_CMP("sha1sum", s)) {
				return digest_main(argc, argv);
			} else if (!C_STR_CMP("sha256sum", s)) {
				return digest_main(argc, argv);
			} else if (!C_STR_CMP("sha512sum", s)) {
				return digest_main(argc, argv);
			} else {
				c_ioq_put(ioq1, names+1);
				c_ioq_flush(ioq1);
			}
		}
	}
}
default.c.o{
	env{
		CPPFLAGS{
			command:echo $CPPFLAGS -I../inc
		}
	}
	../inc/common.h
}
default.c{
	input:../src/${1}
	command:sed "s;^main(;${2}_&;"
	prototypes.h
}
prototype.h{
	module:std-map
	map:$:{%programs}
	apply{
		command:echo "ctype_status ${<}_main(int, char **);"
	}
}
