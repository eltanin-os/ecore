#!/bin/execlineb -S3
backtick PROGS { pipeline { find src -type f -name  "*.c" } sed -e "s;.c$;;" -e "s;src/;;" }
multisubstitute {
	importas -D "" DESTDIR DESTDIR
	importas -D "/usr/local" PREFIX PREFIX
	importas -D "/bin" BINDIR BINDIR
	importas -D "/share/man" MANDIR MANDIR
	importas -isu PROGS PROGS
	elglob MANPAGES "man/*"
}
ifelse { test "${1}" = "all" } {
	redo-ifchange src/${PROGS}
}
ifelse { test "${1}" = "clean" } {
	backtick targets { redo-targets }
	importas -isu targets targets
	rm -f $targets
}
ifelse { test "${1}" = "install" } {
	foreground { redo-ifchange all install-man }
	foreground { install -dm 755 "${DESTDIR}${PREFIX}${BINDIR}" }
	install -cm 755 src/$PROGS "${DESTDIR}${PREFIX}${BINDIR}"
}
ifelse { test "${1}" = "install-ecore" } {
	foreground { redo-ifchange ecore install-man }
	foreground { install -dm 755 "${DESTDIR}${PREFIX}${BINDIR}" }
	foreground { install -cm 755 ecore "${DESTDIR}${PREFIX}${BINDIR}" }
	forx -E prog { $PROGS } ln -s ecore "${DESTDIR}${PREFIX}${BINDIR}/${prog}"
}
ifelse { test "${1}" = "install-man" } {
	foreground { install -dm 755 "${DESTDIR}${PREFIX}${MANDIR}/man1" }
	install -cm 644 $MANPAGES "${DESTDIR}${PREFIX}${MANDIR}/man1"
}
exit 0
