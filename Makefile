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
	src/cksum\
	src/dirname\
	src/du\
	src/echo\
	src/env\
	src/false\
	src/head\
	src/link\
	src/pwd\
	src/true\
	src/unlink\
	src/yes

# LIB SRC
LIBCOMMONSRC=\
	lib/common/dir.c\
	lib/common/estrtovl.c

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
install: all
	install -dm 755 $(DESTDIR)/$(PREFIX)/bin
	install -csm 755 $(BIN) $(DESTDIR)/$(PREFIX)/bin

clean:
	rm -f $(BIN) $(OBJ) $(LIB)

.PHONY:
	all clean install

