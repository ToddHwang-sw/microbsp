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
	$(LFLDIR)/lib/buffer.c \
	$(LFLDIR)/lib/cuse_lowlevel.c \
	$(LFLDIR)/lib/fuse.c \
	$(LFLDIR)/lib/fuse_log.c \
	$(LFLDIR)/lib/fuse_loop.c \
	$(LFLDIR)/lib/fuse_loop_mt.c \
	$(LFLDIR)/lib/fuse_lowlevel.c \
	$(LFLDIR)/lib/fuse_opt.c \
	$(LFLDIR)/lib/fuse_signals.c \
	$(LFLDIR)/lib/helper.c \
	$(LFLDIR)/lib/modules/iconv.c \
	$(LFLDIR)/lib/modules/subdir.c \
	$(LFLDIR)/lib/mount.c \
	$(LFLDIR)/lib/mount_util.c

TRANSPORT_SOURCES = \
	$(TOP)/transport.cpp \
	$(TOP)/udp.cpp \
	$(TOP)/udp6.cpp \
	$(TOP)/raw.cpp \
	$(TOP)/sk.cpp

fusemount_SOURCES = \
	$(LFLDIR)/util/fusermount.c \
	$(TOP)/malloc.cpp \
	$(LIBFUSE_SOURCES)

lpps_SOURCES = \
	$(TOP)/lpps.cpp \
	$(TOP)/malloc.cpp \
	$(TOP)/plist.cpp \
	$(TOP)/sig.cpp \
	$(TOP)/conf.cpp \
	$(TOP)/fhandle.cpp \
	$(JSON)/src/json.cpp \
	$(TRANSPORT_SOURCES) \
	$(LIBFUSE_SOURCES)

lppsnm_SOURCES = \
	$(TOP)/lppsnm.cpp \
	$(TRANSPORT_SOURCES)

install-exec-hook:
	[   -d $(DESTDIR)/etc           ] || mkdir -p $(DESTDIR)/etc
	[ ! -f $(DESTDIR)/etc/lpps.conf ] || \rm -rf $(DESTDIR)/etc/lpps.conf 
	$(INSTALL_DATA) $(TOP)/lpps.conf $(DESTDIR)/etc/lpps.conf
