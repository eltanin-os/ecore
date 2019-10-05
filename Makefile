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
	src/cp\
	src/dirname\
	src/du\
	src/echo\
	src/env\
	src/false\
	src/head\
	src/link\
	src/ln\
	src/mkdir\
	src/mv\
	src/pwd\
	src/rm\
	src/rmdir\
	src/true\
	src/uname\
	src/unlink\
	src/yes

# MAN
MAN1=\
	man/basename.1\
	man/cat.1\
	man/chgrp.1\
	man/chmod.1\
	man/chown.1\
	man/cksum.1\
	man/cp.1\
	man/dirname.1\
	man/du.1\
	man/echo.1\
	man/env.1\
	man/false.1\
	man/head.1\
	man/link.1\
	man/ln.1\
	man/mkdir.1\
	man/mv.1\
	man/pwd.1\
	man/rm.1\
	man/rmdir.1\
	man/true.1\
	man/uname.1\
	man/unlink.1\
	man/yes.1

# LIB SRC
LIBCOMMONSRC=\
	lib/common/cp.c\
	lib/common/estrtovl.c\
	lib/common/path.c\
	lib/common/pathcat.c\
	lib/common/rm.c\
	lib/common/sdup.c\
	lib/common/serr.c\
	lib/common/tmpargv.c

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
	$(CC) $(LDFLAGS) -o $@ $< $(LIB) $(LDLIBS)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -I $(INC) -o $@ -c $<

# LIBRARIES RULES
$(LIBCOMMON): $(LIBCOMMONOBJ)
	$(AR) rc $@ $(LIBCOMMONOBJ)
	$(RANLIB) $@

# RULES
install-man: all
	install -dm 755 $(DESTDIR)/$(MANDIR)/man1
	install -cm 644 $(MAN1) $(DESTDIR)/$(MANDIR)/man1

install: install-man all
	install -dm 755 $(DESTDIR)/$(PREFIX)/bin
	install -cm 755 $(BIN) $(DESTDIR)/$(PREFIX)/bin

clean:
	rm -f $(BIN) $(OBJ) $(LIB)

.PHONY:
	all clean install install-man

