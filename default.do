#!/bin/execlineb -S3
backtick PROGS { pipeline { find src -type f -name  "*.c" } sed -e "s;.c$;;" -e "s;src/;;" }
multisubstitute {
	importas -D "/usr/local" DESTDIR DESTDIR
	importas -D "/include" INCDIR INCDIR
	importas -D "/bin" BINDIR BINDIR
	importas -D "/lib" LIBDIR LIBDIR
	importas -D "/share/man" MANDIR MANDIR
	importas -is PROGS PROGS
	elglob MANPAGES "man/*"
}
ifelse { test "${1}" = "all" } {
	redo-ifchange src/${PROGS}
}
ifelse { test "${1}" = "clean" } {
	backtick targets { redo-targets }
	importas -isu targets targets
	rm -Rf $targets
}
ifelse { test "${1}" = "install" } {
	foreground { redo-ifalways }
	foreground { redo-ifchange install-man }
	foreground { install -dm 755 "${DESTDIR}${BINDIR}" }
	install -cm 755 $PROGS "${DESTDIR}${BINDIR}"
}
ifelse { test "${1}" = "install-ecore" } {
	foreground { redo-ifalways }
	foreground { redo-ifchange ecore install-man }
	foreground { install -dm 755 "${DESTDIR}${BINDIR}" }
	foreground { install -cm 755 ecore "${DESTDIR}${BINDIR}" }
	forx -E prog { $PROGS } ln -s ecore "${DESTDIR}${BINDIR}/${prog}"
}
ifelse { test "${1}" = "install-man" } {
	foreground { redo-ifalways }
	foreground { install -dm 755 "${DESTDIR}${MANDIR}/man1" }
	install -cm 644 $MANPAGES "${DESTDIR}${MANDIR}/man1"
}
exit 0
