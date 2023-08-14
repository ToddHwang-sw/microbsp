HTTPDIR=/etc/httpd

WEBSUBS=js \
	etc \
	img \
	css \
	index.html \
	frame1.html \
	left.html \
	info.html \
	network.html \
	user.html \
	daemon.html \
	update.html

CFLAGS = \
   -I$(DESTDIR)/include \
   -I$(DESTDIR)/include/json-c

CFLAGS += -D_DEBUG_

LIBS	= -lpthread -L$(DESTDIR)/lib -lfcgi -ljson-c

bin_PROGRAMS = server.fcgi

server_fcgi_SOURCES = \
	cgi/server.c \
	cgi/svcproc.c \
	cgi/utils.c

install-exec-hook:
	[ -d $(DESTDIR)/$(HTTPDIR) ] || mkdir -p $(DESTDIR)/$(HTTPDIR)
	for obj in $(WEBSUBS); do cp -rf $(abs_srcdir)/$$obj $(DESTDIR)/$(HTTPDIR) || exit 1; done
