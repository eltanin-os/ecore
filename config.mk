PREFIX=    /usr/local
MANPREFIX= $(PREFIX)/share/man

AR     = ar
CC     = ecc
RANLIB = ranlib

CFLAGS   = -Os -std=c99 -Wall -pedantic
LDFLAGS  = -static
LDLIBS   = lib/libc.a
