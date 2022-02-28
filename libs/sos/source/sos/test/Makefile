SRCS= 	buff_test.c \
      	thr_test.c \
      	mutex_test.c \
	os_test0.c \
	os_test1.c \
	os_test2.c \
	os_test3.c \
	os_test4.c \
	dbg_test1.c \
	net_c.c \
	net_s.c \
	net_ms.c \
	net_mc.c \
	net_mc2.c

CC=gcc
CFLAGS=-g -I../inc -I../inc/linux 
CFLAGS+=-DPROTO_TCP
LIBS=-L$(DESTDIR)/lib -lsos -lpthread

EXEC=$(SRCS:.c=)
OBJS=$(SRCS:.c=.o)

all: $(SRCS)
	@for a in $(EXEC); do \
		$(CC) $(CFLAGS) $$a.c -o $$a $(LIBS); \
	done

clean:
	@\rm -rf *.o $(EXEC) $(OBJS)
