/*******************************************************************************
 *
 * os_specific.h - Linux OS Porting layer header
 *
 ******************************************************************************/
#ifndef _LINUXSW_OS_SPECIFIC_H
#define _LINUXSW_OS_SPECIFIC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Linux specific header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/signal.h>
//#include <sys/siginfo.h>
#ifdef OS_DEBUG_SYSLOG
#include <syslog.h>
#endif
#ifndef LWIP_NETWORK
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <pthread.h>

/* Debugging function */
#ifdef OS_DEBUG
extern unsigned int dbg_printf_flag;
#ifdef OS_DEBUG_SYSLOG
#define os_debug_printf(lvl,fmt,args ...)    \
{ \
		if( dbg_printf_flag >= lvl) {\
			syslog(LOG_INFO,"%-20s|%-5d|"fmt,__FUNCTION__,__LINE__, ##args); }\
}
#else
#define os_debug_printf(lvl,fmt,args ...)    \
{ \
		if( dbg_printf_flag >= lvl) { \
			printf("%-20s|%-5d|"fmt,__FUNCTION__,__LINE__,##args); } \
}
#endif /* OS_DEBUG_SYSLOG */
#else
#define os_debug_printf(lvl,fmt,args ...)
#endif

/* malloc */
#define _os_malloc(s)       malloc(s)
#define _os_calloc(s,c)     calloc(s,c)
#define _os_realloc(s,n)    realloc(s,n)
#define _os_free(s)         free((s))

/* thread */
typedef void (*os_thrfunc_t)(void *); /* thread function type */
#define os_thread_t     pthread_t
/* wrapper for pthread_create */
#define _os_thread_create(i,f,a) \
	pthread_create(i,NULL,f,a)
#define _os_thread_exit(t)  pthread_exit(t)
#define _os_thread_join(t,r)    pthread_join(t,r)
#define _os_thread_stop(t)  pthread_cancel(t)
#define _os_thread_detach(t)    pthread_detach(t)
#define _os_thread_self()   pthread_self()

/* mutex */
#define os_mutex_t      pthread_mutex_t
#define _os_mutex_init(l)   pthread_mutex_init((l),NULL)
#define _os_mutex_lock(l)   pthread_mutex_lock((l))
#define _os_mutex_trylock(l)    pthread_mutex_trylock((l))
#define _os_mutex_unlock(l) pthread_mutex_unlock((l))
#define _os_mutex_destroy(l)    pthread_mutex_destroy((l))


/* conditional */
#define os_cond_t       pthread_cond_t
#define _os_cond_init(c)    pthread_cond_init((c),NULL)
#define _os_cond_signal(c)  pthread_cond_signal(c)
#define _os_cond_wait(c,m)  pthread_cond_wait((c),(m))
#define _os_cond_twait(c,m,ts)  pthread_cond_timedwait((c),(m),ts)

/* OS time */
#define os_time_t       struct timeval
#define os_gettime(s)       gettimeofday(s,NULL)
#define os_make_longtime(s) (unsigned long)((s.tv_sec*1000000)+(s.tv_usec))

/* ioctl/fcntl */
#define os_ioctl        ioctl
#define os_fcntl        fcntl
#define os_close        close

/* network error processing */
#define NET_ERR_RWBLOCK     (errno == EAGAIN    || errno == EWOULDBLOCK)
#define NET_ERR_CONNBLOCK   (errno == EINPROGRESS   || errno == EALREADY)
#define NET_ERR_CONNDONE    (errno == EISCONN)

extern int os_errno_printf(void);
extern int os_errno(void);

extern void os_printf(unsigned char *);
extern unsigned int os_net_block_mode_set(int,int);

#ifdef __cplusplus
}
#endif

#endif
