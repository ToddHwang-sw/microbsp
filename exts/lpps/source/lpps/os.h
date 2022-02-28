#ifndef __OS_HEADER__
#define __OS_HEADER__

#ifdef __cplusplus
extern "C"{
#endif

#include "malloc.h"
#include "errno.h"
#include <syslog.h>
#include <errno.h>

/* return values */
#define RET_EPERM		-EPERM
#define RET_ENOENT		-ENOENT
#define RET_EEXIST		-EEXIST
#define RET_EMEM		-ENOMEM
#define RET_EIO			-EIO
#define RET_OKAY		0

//
// defined in lpps.cpp 
//
#define DEBUG_NONE    0
#define DEBUG_LPPS    1
#define DEBUG_FUSE    2
#define DEBUG_MEMORY  3
extern int sys_debug;
extern int sys_daemon;
extern int use_syslog;

#define ERR(fmt,args...)	{ \
				if (sys_daemon) \
					syslog(LOG_INFO,"LPPS:#:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
				else    \
					if (use_syslog)  \
						syslog(LOG_INFO,"LPPS:@:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
					else \
						fprintf(stderr,"LPPS:#:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
				}
#define DBG(fmt,args...)	{ \
				if (sys_debug >= DEBUG_LPPS){ \
					if (sys_daemon)       \
						syslog(LOG_INFO,"LPPS:@:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
					else { \
						if (use_syslog)  \
							syslog(LOG_INFO,"LPPS:@:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
						else \
							fprintf(stderr,"LPPS:@:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
					} \
				} \
				}
#define PRT(fmt,args...) 	printf("LPPS:+:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args)

/* bit operations */
#define bit_includes(val,bit)	(((val) & (bit)) == (bit))
#define bit_excludes(val,bit)	{ val &= ~(bit); }
#define bit_adds(val,bit)	{ val |= (bit);  }
#define bit_clear(val)		bit_excludes((val),(val))

#ifdef __cplusplus
};
#endif

#endif /* __OS_HEADER__ */
