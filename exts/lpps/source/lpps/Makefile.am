TOP    = $(top_srcdir)
JSON   = $(TOP)/json
LFDIR  = $(TOP)/libfuse
LFLDIR = $(LFDIR)/libfuse
LFMT = fusemount
LFDM = lpps
NETM = lppsnm

CXXDEBUG = -g -O0
##CXXDEBUG += -DUSE_GLIBC_MALLOC

CXXFLAGS += \
   $(CXXDEBUG) \
   -I$(LFLDIR)/include \
   -I$(LFLDIR)/lib \
   -I$(LFDIR)/inc \
   -I$(JSON)/inc \
   -fPIC \
   -Wall \
   -D_REENTRANT \
   -DHAVE_CONFIG_H \
   -Wno-sign-compare \
   -Wmissing-declarations \
   -Wwrite-strings \
   -fno-strict-aliasing \
   -pthread \
   -DEXTRA_WORKAROUND \
   -DMEMORY_TRACK \
   -D_QNXAPP_ \
   -D_FILE_OFFSET_BITS=64 \
   -DFUSE_USE_VERSION=31 \
   -DFUSERMOUNT_DIR=\".\" \
   -DFUSERMOUNT_PROG=\"$(LFMT)\" \
   -DFUSE_CONF=\"$(LFDIR)/inc\"

CXXFLAGS += -D_DEBUG_
CFLAGS   += $(CXXFLAGS)

LIBS	 += -lpthread -ldl

bin_PROGRAMS = $(LFDM) $(LFMT) $(NETM)

LIBFUSE_SOURCES = \
	libfuse/libfuse/lib/buffer.c \
	libfuse/libfuse/lib/cuse_lowlevel.c \
	libfuse/libfuse/lib/fuse.c \
	libfuse/libfuse/lib/fuse_log.c \
	libfuse/libfuse/lib/fuse_loop.c \
	libfuse/libfuse/lib/fuse_loop_mt.c \
	libfuse/libfuse/lib/fuse_lowlevel.c \
	libfuse/libfuse/lib/fuse_opt.c \
	libfuse/libfuse/lib/fuse_signals.c \
	libfuse/libfuse/lib/helper.c \
	libfuse/libfuse/lib/modules/iconv.c \
	libfuse/libfuse/lib/modules/subdir.c \
	libfuse/libfuse/lib/mount.c \
	libfuse/libfuse/lib/mount_util.c

TRANSPORT_SOURCES = \
	transport.cpp \
	udp.cpp \
	udp6.cpp \
	raw.cpp \
	sk.cpp

fusemount_SOURCES = \
	libfuse/libfuse/util/fusermount.c \
	malloc.cpp \
	$(LIBFUSE_SOURCES)

lpps_SOURCES = \
	lpps.cpp \
	malloc.cpp \
	plist.cpp \
	sig.cpp \
	conf.cpp \
	fhandle.cpp \
	json/src/json.cpp \
	$(TRANSPORT_SOURCES) \
	$(LIBFUSE_SOURCES)

lppsnm_SOURCES = \
	lppsnm.cpp \
	$(TRANSPORT_SOURCES)

install-exec-hook:
	[   -d $(DESTDIR)/etc           ] || mkdir -p $(DESTDIR)/etc
	[ ! -f $(DESTDIR)/etc/lpps.conf ] || \rm -rf $(DESTDIR)/etc/lpps.conf 
	$(INSTALL_DATA) $(TOP)/lpps.conf $(DESTDIR)/etc/lpps.conf
