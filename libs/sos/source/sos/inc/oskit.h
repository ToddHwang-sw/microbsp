/*******************************************************************************
 *
 * oskit.h - Master header for OS Porting layer
 *
 ******************************************************************************/
#ifndef _OS_MASTER_HEADER_FILE
#define _OS_MASTER_HEADER_FILE

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"
#include "os.h"
#include "qbuff.h"
#include "msgq.h"
#include "malloc.h"
#include "error.h"
#include "mem.h"
#include "buff.h"
#include "dbg.h"
#include "net.h"
#include "sel.h"

typedef struct {
	unsigned int system_pool_size; /* bytes */
	unsigned int debug_pool_size;  /* bytes */
	unsigned int network_pool_size; /* bytes */
}OS_SYSPARAM;

/* prototypes */
extern void os_reinit_sysparam(OS_SYSPARAM *);  /* chainging default values... */
extern void os_init(void);                          /* initialization */
extern void os_set_debug_level(unsigned int);       /* debugging level function */

#ifdef __cplusplus
}
#endif

#endif /* _OS_MASTER_HEADER_FILE */
