#ifndef __MALLOC_INTERFACES__
#define __MALLOC_INTERFACES__

/* Memory debugging....
 */
struct memlist {
	void *next;
	void *prev;
}__attribute__((aligned));

/* root configuration */
struct memspace {
	#define __MEMBLOCK_MAGIC__  0xbab0d00d
	unsigned int magic;
	size_t sz;
	size_t rsz;
	size_t free;
	char * blk;
	void * spc; /* mspace */
	struct memlist list;
} __attribute__((aligned));
#define spc_base(mblk)  ((mblk) ? ((mblk)->blk) : NULL)
#define spc_size(mblk)  (int)((mblk) ? ((mblk)->free) : 0)

/* private block header */
struct memheader {
	#define __MEMHEADER_MAGIC__ 0xabcdd00d
	unsigned int magic;
	unsigned int sz;
#ifdef MEMORY_TRACK
	char fn[32];
	int  line;
#endif /* MEMORY_TRACK */
	struct memlist list;
} __attribute__((aligned));
#define mem_size(blk)   (((struct memheader *)((char *)(blk) - sizeof(struct memheader)))->sz)

/* container_of declaration */
#include <stddef.h>

/* This macro borrowed from the Linux kernel.  */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type, member) );	\
})

extern struct memspace *    mem_init(size_t sz);
extern int                  mem_deinit(struct memspace *mblk);
extern void *               mem_alloc(struct memspace *mblk, size_t sz);
extern void *               mem_calloc(struct memspace *mblk, size_t nmemb, size_t sz);
extern void *               mem_realloc(struct memspace *mblk, void * ptr, size_t sz);
extern char *               mem_strdup(struct memspace *mblk, const char *s);
extern int                  mem_free(struct memspace *mblk, void *mb);
extern int                  mem_navigate(struct memspace *mblk, void (*func)(void *) );

#endif  /* __MALLOC_INTERFACES__ */

