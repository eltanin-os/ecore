#!/usr/bin/automate -s std-default,std-c
@program{
	transform{
		${%%glob-edit}
		glob:src/*.c
		edit:sed "s;.c$$;;"
	}
}
$@program{
	library{
		lib/libcommon.a
		tertium
	}
}
@manpage{
	transform{
		module:std-glob
		glob:man/*
	}
}
default.c.o{
	env{
		CPPFLAGS{
			command:echo $CPPFLAGS -Iinc
		}
	}
	inc/common.h
}
lib/libcommon.a{
	transform{
		${%%glob-edit}
		glob:lib/*.c
		edit:sed "s;$$;.o;"
	}
	${default.a}
}
