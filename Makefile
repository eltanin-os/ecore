include config.mk

.SUFFIXES:
.SUFFIXES: .o .c

INC= inc
HDR=\
	inc/common.h

# BIN
BIN=\
	src/basename\
	src/cat\
	src/chgrp\
	src/chmod\
	src/chown\
	src/cksum\
	src/cmp\
	src/cp\
	src/digest\
	src/dirname\
	src/du\
	src/echo\
	src/env\
	src/false\
	src/head\
	src/install\
	src/link\
	src/ln\
	src/ls\
	src/mkdir\
	src/mktemp\
	src/mv\
	src/printf\
	src/pwd\
	src/rm\
	src/rmdir\
	src/tail\
	src/true\
	src/uname\
	src/unlink\
	src/wc\
	src/yes

# MAN
MAN1=\
	man/basename.1\
	man/cat.1\
	man/chgrp.1\
	man/chmod.1\
	man/chown.1\
	man/cksum.1\
	man/cmp.1\
	man/cp.1\
	man/digest.1\
	man/dirname.1\
	man/du.1\
	man/echo.1\
	man/env.1\
	man/false.1\
	man/head.1\
	man/link.1\
	man/ln.1\
	man/ls.1\
	man/mkdir.1\
	man/mktemp.1\
	man/mv.1\
	man/printf.1\
	man/pwd.1\
	man/rm.1\
	man/rmdir.1\
	man/true.1\
	man/uname.1\
	man/unlink.1\
	man/wc.1\
	man/yes.1

# LIB SRC
LIBCOMMONSRC=\
	lib/common/cp.c\
	lib/common/estrtovl.c\
	lib/common/mkpath.c\
	lib/common/path.c\
	lib/common/pathcat.c\
	lib/common/pdb.c\
	lib/common/rm.c\
	lib/common/serr.c\
	lib/common/strtomode.c\
	lib/common/tmpargv.c\
	lib/common/yesno.c

# LIB PATH
LIBCOMMON= lib/libcommon.a

# LIB OBJS
LIBCOMMONOBJ= $(LIBCOMMONSRC:.c=.o)

# ALL
LIB= $(LIBCOMMON)
OBJ= $(BIN:=.o) $(LIBCOMMONOBJ)
SRC= $(BIN:=.c)

# VAR RULES
all: $(BIN)

$(BIN): $(LIB) $(@:=.o)
$(OBJ): $(HDR) config.mk

# SUFFIX RULES
.o:
	$(CC) $(LDFLAGS) -o $@ $< $(LIB) -ltertium

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -I $(INC) -o $@ -c $<

# LIBRARIES RULES
$(LIBCOMMON): $(LIBCOMMONOBJ)
	$(AR) rc $@ $(LIBCOMMONOBJ)
	$(RANLIB) $@

# RULES
ecore: build/ecore.sh $(LIB) $(SRC)
	build/ecore.sh $(SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -I $(INC) -o $@ tmpbuild/*.c $(LIB) -ltertium
	rm -Rf tmpbuild

install-man:
	install -dm 755 $(DESTDIR)/$(MANDIR)/man1
	install -cm 644 $(MAN1) $(DESTDIR)/$(MANDIR)/man1

install: install-man all
	install -dm 755 $(DESTDIR)/$(BINDIR)
	install -cm 755 $(BIN) $(DESTDIR)/$(BINDIR)

install-ecore: ecore install-man
	install -dm 755 $(DESTDIR)/$(BINDIR)
	install -cm 755 ecore $(DESTDIR)/$(BINDIR)
	for f in $(BIN); do\
		ln -s ecore $(DESTDIR)/$(BINDIR)/$$(basename $$f);\
	done

clean:
	rm -Rf tmpbuild
	rm -f ecore $(BIN) $(OBJ) $(LIB)

.PHONY:
	all ecore clean install install-ecore install-man

