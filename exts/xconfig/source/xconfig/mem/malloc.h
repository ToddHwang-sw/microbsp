#ifndef __MALLOC_INTERFACES__
#define __MALLOC_INTERFACES__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* root configuration */
struct memspace {
	#define __MEMBLOCK_MAGIC__  0xbab0d00d
	unsigned int magic;
	size_t sz;
	size_t rsz;
	size_t free;
	char * blk;
	void * spc; /* mspace */
} __attribute__((aligned));
#define spc_base(mblk)  ((mblk) ? ((mblk)->blk) : NULL)
#define spc_size(mblk)  (int)((mblk) ? ((mblk)->free) : 0)

/* private block header */
struct memheader {
	#define __MEMHEADER_MAGIC__ 0xabcdd00d
	unsigned int magic;
	unsigned int sz;
} __attribute__((packed));
#define mem_size(blk)   (((struct memheader *)((char *)(blk) - sizeof(struct memheader)))->sz)

extern struct memspace *    mem_init(size_t sz);
extern int                  mem_deinit(struct memspace *mblk);
extern void *               mem_alloc(struct memspace *mblk, size_t sz);
extern void *               mem_calloc(struct memspace *mblk, size_t nmemb, size_t sz);
extern void *               mem_realloc(struct memspace *mblk, void * ptr, size_t sz);
extern char *               mem_strdup(struct memspace *mblk, const char *s);
extern int                  mem_free(struct memspace *mblk, void *mb);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __MALLOC_INTERFACES__ */

