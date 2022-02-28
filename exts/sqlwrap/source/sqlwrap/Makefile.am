##	-DOS_DEBUG \
##
AM_CPPFLAGS=\
	-DSQLITE_SINGLE \
	-I$(DESTDIR)/include \
	-I$(DESTDIR)/include/linux

##
## sqlwrapper libraries...
noinst_LIBRARIES=libsqlwrap.a

libsqlwrap_a_CFLAGS=\
	-I$(abs_srcdir)/lib/inc \
	-I$(DESTDIR)/include

libsqlwrap_a_SOURCES=\
	lib/src/dblib_net.c \
	lib/src/dbprintf.c \
	lib/src/pstor.c

##
## applicatins - server and client 

COMMON_CFLAGS=$(libsqlwrap_a_CFLAGS)
COMMON_LDFLAGS=-L. -L$(DESTDIR)/lib

LIBS=-lsos -lsqlwrap -lsqlite3 -lpthread

bin_PROGRAMS=dbcli
dbcli_CFLAGS=$(COMMON_CFLAGS)
dbcli_LDFLAGS=$(COMMON_LDFLAGS)
dbcli_LDADD=$(LIBS)
dbcli_SOURCES=tests/dbcli.c

bin_PROGRAMS+=dbsvr
dbsvr_CFLAGS=$(COMMON_CFLAGS)
dbsvr_LDFLAGS=$(COMMON_LDFLAGS)
dbsvr_LDADD=$(LIBS)
dbsvr_SOURCES=tests/dbsvr.c

install-exec-hook:
