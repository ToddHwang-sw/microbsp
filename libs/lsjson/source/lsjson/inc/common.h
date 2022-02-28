#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "malloc.h"

/* ---------- TYPE      ------------------- */
#define u8	unsigned char
#define s8	char
#define u16	unsigned short
#define s16	short
#define u32	unsigned int
#define s32	int

#include "mem.h"

#define os_err(fmt,args...)	fprintf(stderr,fmt,##args)
#define os_msg(fmt,args...)	printf(fmt,##args)
#define os_dbg(fmt,args...)	do{ if (json_dbg) os_msg(fmt,##args); }while(0)

#endif /* __COMMON_HEADER__ */
