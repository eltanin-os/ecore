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
case -- $1 {
".*\.[1ch]" {
	exit 0
}
"all" {
	redo-ifchange src/${PROGS}
}
"clean" {
	backtick targets { redo-targets }
	importas -isu targets targets
	rm -f $targets
}
"install" {
	if { redo-ifchange all install-man }
	if { install -dm 755 "${DESTDIR}${PREFIX}${BINDIR}" }
	if { ln -s digest "${DESTDIR}${PREFIX}${BINDIR}/md5sum" }
	if { ln -s digest "${DESTDIR}${PREFIX}${BINDIR}/sha1sum" }
	if { ln -s digest "${DESTDIR}${PREFIX}${BINDIR}/sha256sum" }
	if { ln -s digest "${DESTDIR}${PREFIX}${BINDIR}/sha512sum" }
	install -cm 755 src/$PROGS "${DESTDIR}${PREFIX}${BINDIR}"
}
"install-ecore" {
	if { redo-ifchange ecore install-man }
	if { install -dm 755 "${DESTDIR}${PREFIX}${BINDIR}" }
	if { install -cm 755 ecore "${DESTDIR}${PREFIX}${BINDIR}" }
	if { ln -s ecore "${DESTDIR}${PREFIX}${BINDIR}/md5sum" }
	if { ln -s ecore "${DESTDIR}${PREFIX}${BINDIR}/sha1sum" }
	if { ln -s ecore "${DESTDIR}${PREFIX}${BINDIR}/sha256sum" }
	if { ln -s ecore "${DESTDIR}${PREFIX}${BINDIR}/sha512sum" }
	forx -E prog { $PROGS } ln -s ecore "${DESTDIR}${PREFIX}${BINDIR}/${prog}"
}
"install-man" {
	if { install -dm 755 "${DESTDIR}${PREFIX}${MANDIR}/man1" }
	install -cm 644 $MANPAGES "${DESTDIR}${PREFIX}${MANDIR}/man1"
}
}
foreground {
	fdmove 1 2
	echo no rule for $1
}
exit 1
