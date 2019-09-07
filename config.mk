PREFIX=    /usr/local
MANPREFIX= $(PREFIX)/share/man

AR     = ar
CC     = cc
RANLIB = ranlib

CFLAGS   = -O0 -g -std=c99 -Wall -Wextra -Werror -pedantic
LDFLAGS  =
LDLIBS   = -ltertium
