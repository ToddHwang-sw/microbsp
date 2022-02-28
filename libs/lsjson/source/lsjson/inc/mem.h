#ifndef __MEM_API_DEFINITION__
#define __MEM_API_DEFINITION__

/* 4 MBytes */
#define MEMSZ   (4*1024*1024)
extern int sys_rmem;

extern void *   _os_malloc(char *,int, size_t );
extern void *   _os_realloc(char *,int, void *, size_t );
extern void *   _os_calloc(char *,int, size_t, size_t );
extern char *   _os_strdup(char *,int, const char *);
extern void     _os_free(char *, int, void *);

#define os_malloc(sz)       _os_malloc((char *)__FILE__,__LINE__,sz)
#define os_realloc(p,sz)    _os_realloc((char *)__FILE__,__LINE__,p,sz)
#define os_calloc(n,sz)     _os_calloc((char *)__FILE__,__LINE__,n,sz)
#define os_strdup(s)        _os_strdup((char *)__FILE__,__LINE__,s)
#define os_free(p)          _os_free((char *)__FILE__,__LINE__,p)

extern void 	__os_meminit(size_t);
extern void *   __os_malloc(size_t );
extern void *   __os_realloc(void *, size_t );
extern void *   __os_calloc(size_t, size_t );
extern char *   __os_strdup(const char *);
extern void     __os_free(void *);
extern int      __os_memspc(void);
extern int      __os_memblks(void (*func)(void *));

#endif /* __MEM_API_DEFINITION__ */
