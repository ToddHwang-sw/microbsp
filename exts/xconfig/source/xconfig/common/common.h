#ifndef __COMMON_HEADER__
#define __COMMON_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

/* LOGLEVEL */
enum {
	LOGLEVEL_SILENT = 0,
	LOGLEVEL_DEBUG,
	LOGLEVEL_SUB,
	LOGLEVEL_TIMER,
	LOGLEVEL_MEM,
	LOGLEVEL_NUM
};
#define LOGLEVEL_DEFAULT    LOGLEVEL_DEBUG

extern int sys_loglevel; /* system loglevel */
extern void logfunc(const char *fmt, ...);

#ifndef __STANDALONE__
#define ERR(fmt,args ...)    logfunc("ERR:%-20s:%-5d" fmt,__func__,__LINE__, ## args)
#define MSG(fmt,args ...)    logfunc("MSG:%-20s:%-5d" fmt,__func__,__LINE__, ## args)
#define __DBG(fmt,args ...)  logfunc("DBG:%-20s:%-5d" fmt,__func__,__LINE__, ## args)
#else
#define ERR(fmt,args ...)    printf("ERR:%-20s:%-5d" fmt,__func__,__LINE__, ## args)
#define MSG(fmt,args ...)    printf("MSG:%-20s:%-5d" fmt,__func__,__LINE__, ## args)
#define __DBG(fmt,args ...)  printf("DBG:%-20s:%-5d" fmt,__func__,__LINE__, ## args)
#endif
#define _DBG(fmt,args ...)   printf("DBG:%-20s:%-5d" fmt,__func__,__LINE__, ## args)
#define DBG(fmt,args ...)    { \
		if (sys_loglevel >= LOGLEVEL_DEBUG) __DBG(fmt, ## args); \
}
#define TIMDBG(fmt,args ...) { \
		if (sys_loglevel >= LOGLEVEL_TIMER) _DBG(fmt, ## args); \
}
#define MEMDBG(fmt,args ...) { \
		if (sys_loglevel >= LOGLEVEL_MEM) _DBG(fmt, ## args); \
}
#define SDBG(fmt,args ...) { \
		if (sys_loglevel >= LOGLEVEL_SUB) __DBG(fmt, ## args); \
}

/* PORT FILE under /var/tmp */
#define PORT_FILE   "port.info"

/* structure of XML packet */
/* Actually payload will be maintained from JSON string */
struct jxmlpkt {
	struct {
		#define JPKT_MAGIC 0xab1ed00d
		unsigned int magic;
		unsigned short command;
		unsigned short length;
		unsigned char dummy[16];
	} h; /* intentionally 16 bytes */
	unsigned char jpld[0]; /* json payload*/
} __attribute__((packed));

/* max of "signed short" */
#define MAXPKTLEN   ((short)(0x7fff-sizeof(struct jxmlpkt)))

/* client info */
struct client_info {
	int used;               /* used flag */
	int done;               /* terminated */
	int csock;              /* client socket */
	struct sockaddr_in paddr; /* client peer address */
	int paddrlen;             /* peer address length */
	#define DEFAULT_CLIENT_TIMEOUT 1
	int timeout;            /* just second granularity */
	pthread_t thr;          /* client service thread */
	fd_set rfds;            /* read select */
	struct {
		char        *pkt;   /* network packet */
		short trg;          /* target amount */
		short cur;          /* current amount */
		int ishd;           /* is header ?? */
	} n;
} __attribute__((aligned));

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_HEADER__ */
