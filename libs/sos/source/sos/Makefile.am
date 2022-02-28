## -DOS_DEBUG
AM_CPPFLAGS=-DOS_DEBUG

##
## sos libraries...
noinst_LIBRARIES=libsos.a

libsos_a_CFLAGS=\
	-I$(abs_srcdir)/inc \
	-I$(abs_srcdir)/inc/linux

libsos_a_SOURCES=\
	src/list.c \
	src/res.c \
	src/mem.c \
	src/malloc.c \
	src/dbg.c \
	src/errno.c \
	src/init.c \
	src/msgq.c \
	src/buff.c \
	src/qbuff.c \
	src/thread.c \
	src/var.c \
	src/time.c \
	src/sel.c \
	src/net.c \
	src/linux/os_specific.c \
	src/linux/proto.c

install-exec-am:
	@[ ! -d $(DESTDIR)/lib ] || cp $(noinst_LIBRARIES) $(DESTDIR)/lib
	@[ ! -d $(DESTDIR)/include ] || cp -rf $(abs_srcdir)/inc/* $(DESTDIR)/include
