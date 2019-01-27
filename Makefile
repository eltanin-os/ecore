include config.mk

.SUFFIXES:
.SUFFIXES: .o .c

INC= inc

# SOURCE
BIN=\
	src/cat\
	src/cksum\
	src/echo

LIB=
OBJ= $(BIN:=.o)
SRC= $(BIN:=.c)

# VAR RULES
all: $(BIN)

$(BIN): $(LIB) $(@:=.o)
$(OBJ): $(HDR) config.mk

# SUFFIX RULES
.o:
	$(CC) $(LDFLAGS) -o $@ $< $(LDLIBS)

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -I $(INC) -o $@ -c $<

install: all
	install -dm 755 $(DESTDIR)/$(PREFIX)/bin
	install -csm 755 $(BIN) $(DESTDIR)/$(PREFIX)/bin

clean:
	rm -f $(BIN) $(OBJ)

.PHONY:
	all clean install

