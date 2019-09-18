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
MAN1=

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
	install -csm 755 $(BIN) $(DESTDIR)/$(PREFIX)/bin

clean:
	rm -f $(BIN) $(OBJ) $(LIB)

.PHONY:
	all clean install

