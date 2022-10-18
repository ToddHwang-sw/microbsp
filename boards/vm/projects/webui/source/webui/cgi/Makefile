CGIBIN=/usr/sbin

EXEC=server.fcgi
OBJS=utils.o \
     svcproc.o \
     server.o

CFLAGS += -I$(destination)/include \
	  -I$(destination)/include/json-c
LIBS += -L$(destination)/lib -lfcgi -ljson-c

all: $(EXEC)

%.o: %.c
	@echo Compilie $< to $@
	@$(CC) -o $@ -c $^ $(CFLAGS) $(LDFLAGS) 

%.fcgi: $(OBJS)
	@echo Linking $(OBJS) to $@
	@$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)
	@$(STRIP) $@

install: $(EXEC)
	cp -rf $(EXEC) $(destination)$(CGIBIN)

clean:
	\rm -rf *.fcgi *.o
