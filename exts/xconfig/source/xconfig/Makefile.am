
##
## scew library 
##
##
## common library 
##

CXXFLAGS += \
   -g \
   -I$(abs_srcdir)/scew \
   -I$(abs_srcdir)/expat \
   -I$(abs_srcdir)/mem \
   -I$(abs_srcdir)/json \
   -I$(abs_srcdir)/common \
   -I$(abs_srcdir)/minhttp \
   -I$(abs_srcdir)/daemon \
   -I$(abs_srcdir)/storage \
   -DHAVE_EXPAT_CONFIG_H \
   -DXCONFIG_PATCH \
   -fPIC \
   -Wall

CXXFLAGS += -D_DEBUG_

LIBS	+= -lpthread

bin_PROGRAMS = xcfgd xcrc

xcfgd_SOURCES = \
	daemon/xcfgd.cpp \
	daemon/xml.cpp \
	daemon/timer.cpp \
	daemon/shellproc.cpp \
	storage/storage.cpp \
	scew/attribute.cpp \
	scew/base64.cpp \
	scew/element.cpp \
	scew/error.cpp \
	scew/list.cpp \
	scew/dmt.cpp \
	scew/config_xml.cpp \
	scew/parser.cpp \
	scew/str.cpp \
	scew/tree.cpp \
	scew/writer.cpp \
	scew/xerror.cpp \
	scew/xhandler.cpp \
	scew/xparser.cpp \
	scew/xprint.cpp \
	expat/xmlparse.cpp \
	expat/xmlrole.cpp \
	expat/xmltok.cpp \
	mem/malloc.cpp \
	mem/xmlmem.cpp \
	json/dump.cpp \
	json/error.cpp \
	json/hashtable.cpp \
	json/hashtable_seed.cpp \
	json/load.cpp \
	json/memory.cpp \
	json/pack_unpack.cpp \
	json/strbuffer.cpp \
	json/strconv.cpp \
	json/utf.cpp \
	json/value.cpp \
	minhttp/internal.cpp \
	minhttp/base64.cpp \
	minhttp/basicauth.cpp \
	minhttp/connection.cpp \
	minhttp/digestauth.cpp \
	minhttp/postprocessor.cpp \
	minhttp/response.cpp \
	minhttp/reason_phrase.cpp \
	minhttp/daemon.cpp \
	minhttp/tsearch.cpp \
	minhttp/md5.cpp \
	minhttp/memorypool.cpp \
	minhttp/mhd_compat.cpp \
	minhttp/mhd_itc.cpp \
	minhttp/mhd_mono_clock.cpp \
	minhttp/mhd_sockets.cpp \
	minhttp/mhd_str.cpp \
	minhttp/mhd_threads.cpp \
	minhttp/sysfdsetsize.cpp

xcrc_SOURCES = \
	crc/crc32.cpp

install-exec-hook:
	(cd $(DESTDIR)$(bindir); ([ -f xcfgd.fact ] || $(LN_S) xcfgd xcfgd.fact))
	(cd $(DESTDIR)$(bindir); ([ -f xcfgd.hw   ] || $(LN_S) xcfgd xcfgd.hw))
	(cd $(DESTDIR)$(bindir); ([ -f xcfgd.cfg  ] || $(LN_S) xcfgd xcfgd.cfg))
