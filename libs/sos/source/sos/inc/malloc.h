#ifndef _MALLOC_HEADER_
#define _MALLOC_HEADER_

#ifdef __cplusplus
extern "C" {
#endif

typedef void * regionid_t;

/* information block */
typedef struct mallinfo {
	size_t arena;  /* non-mmapped space allocated from system */
	size_t ordblks; /* number of free chunks */
	size_t smblks; /* always 0 */
	size_t hblks;  /* always 0 */
	size_t hblkhd; /* space in mmapped regions */
	size_t usmblks; /* maximum total allocated space */
	size_t fsmblks; /* always 0 */
	size_t uordblks; /* total allocated space */
	size_t fordblks; /* total free space */
	size_t keepcost; /* releasable (via malloc_trim) space */
}region_info_t;

/*
 * Function declarations
 */
extern int      os_region_init(void);
extern regionid_t os_region_create(char *, size_t);
extern void    *os_region_alloc(regionid_t, size_t);
extern void    *os_region_calloc(regionid_t, size_t, size_t);
extern void os_region_free(regionid_t, void*);
extern void    *os_region_realloc(regionid_t, void*, size_t);
extern void    *os_region_aligned_alloc(regionid_t, size_t, size_t);
extern int os_region_getinfo(regionid_t, void *);

#ifdef __cplusplus
}
#endif

#endif
