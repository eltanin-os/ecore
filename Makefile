include config.mk

.SUFFIXES:
.SUFFIXES: .o .c

INC= inc
HDR=\
	inc/ec.h

# SOURCE
BIN=\
	src/cat\
	src/cksum\
	src/echo\
	src/yes

LIBEC=lib/libec.a
LIBECSRC=\
	lib/ec/err.c

LIBECOBJ= $(LIBECSRC:.c=.o)

LIB= $(LIBEC)
OBJ= $(BIN:=.o)
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

# LIB
$(LIBEC): $(LIBECOBJ)
	$(AR) rc $@ $?
	$(RANLIB) $@

# RULES
install: all
	install -dm 755 $(DESTDIR)/$(PREFIX)/bin
	install -csm 755 $(BIN) $(DESTDIR)/$(PREFIX)/bin

clean:
	rm -f $(BIN) $(OBJ) $(LIB)

.PHONY:
	all clean install

