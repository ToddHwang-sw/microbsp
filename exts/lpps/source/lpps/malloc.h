#ifndef __MALLOC_INTERFACES__
#define __MALLOC_INTERFACES__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MEMORY_TRACK
struct memlist;

struct memlist {
	struct memlist * next;
	struct memlist * prev;
}__attribute__((packed));
#endif

/* root configuration */
struct memspace {
	#define __MEMBLOCK_MAGIC__  0xbab0deed
	unsigned int magic;
	size_t sz;
	size_t rsz;
	size_t free;
	char * blk;
	void * spc; /* mspace */
#ifdef MEMORY_TRACK
	/* memory trace... */
	struct memlist list;
#endif
} __attribute__((aligned));
#define spc_base(mblk)  ((mblk) ? ((mblk)->blk) : NULL)
#define spc_size(mblk)  (int)((mblk) ? ((mblk)->free) : 0)

/* private block header */
struct memheader {
	#define __MEMHEADER_MAGIC__ 0xabcdbeef
	unsigned int magic;
	unsigned int sz;
#ifdef MEMORY_TRACK
	#define FNLEN 32
	char fn[FNLEN];
	int line;
	struct memlist list;
#endif
} __attribute__((packed));
#define mem_size(blk)   (((struct memheader *)((char *)(blk) - sizeof(struct memheader)))->sz)

#include <syslog.h>

/* 
 * P O R T I N G   L A Y E R 
 */
extern int sys_rmem;
extern struct memspace *os_memspc;
extern pthread_mutex_t os_memlock;
extern int sys_daemon;
#define MEMDBG(fmt,args...)	
#define MEMERR(fmt,args...)	{ \
				if (sys_daemon) \
					syslog(LOG_INFO,"MEM :#:%-20s:%-4d:%08x" fmt,(char *)__func__,(int)__LINE__,(int)pthread_self(),##args); \
				else    \
					fprintf(stderr,"MEM :#:%-20s:%-4d:%08x" fmt,(char *)__func__,(int)__LINE__,(int)pthread_self(),##args); \
				}

extern void *   _os_malloc(char *,int, size_t );
extern void *   _os_realloc(char *,int, void *, size_t );
extern void *   _os_calloc(char *,int, size_t, size_t );
extern char *   _os_strdup(char *,int, const char *);
extern void     _os_free(char *, int, void *);
extern int	_os_meminfo(char *, int, int *, int);

#define os_malloc(sz)       	_os_malloc((char *)__func__,__LINE__,sz)
#define os_realloc(p,sz)    	_os_realloc((char *)__func__,__LINE__,p,sz)
#define os_calloc(n,sz)     	_os_calloc((char *)__func__,__LINE__,n,sz)
#define os_strdup(s)        	_os_strdup((char *)__func__,__LINE__,s)
#define os_free(p)      	_os_free((char *)__func__,__LINE__,p)
#define os_meminfo(p,sz,rsz,typ)	_os_meminfo(p,sz,rsz,typ)

/* memory information */
extern char *	os_memblock(int);
extern int	os_memsize(int);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __MALLOC_INTERFACES__ */

