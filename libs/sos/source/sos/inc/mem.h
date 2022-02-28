/*******************************************************************************
 *
 * mem.h - Memory region controller
 *
 ******************************************************************************/
#ifndef MEM_REGION_CONTROLLER_HEADER
#define MEM_REGION_CONTROLLER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "oskit.h"

/* prototypes */
extern unsigned int os_mem_init(void);
extern void os_mem_delete(void);
extern void * os_mem_alloc(unsigned int);
extern void * os_mem_realloc(void *,unsigned int);
extern char * os_mem_strdup(const char *);
extern unsigned int os_mem_free(void *);
extern void os_mem_resinfo(region_info_t *);
extern void os_mem_verbose(void);

#ifdef __cplusplus
}
#endif

#endif  /* MEM_REGION_CONTROLLER_HEADER */
