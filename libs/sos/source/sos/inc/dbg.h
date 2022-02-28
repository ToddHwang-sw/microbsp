/*******************************************************************************
 *
 * dbg.h - debug module header
 *
 ******************************************************************************/
#ifndef _DEBUGGER_MODULE_HEADER
#define _DEBUGGER_MODULE_HEADER

#define DEBUG_MEM_CLASS_SIZE  (8192)

#define DBG_BUF_LEN     (DEBUG_MEM_CLASS_SIZE)  /* 8 kbytes */
#define DBG_BUFF_MIN_AMOUNT (1024)          /* 1 Kbytes */

/* prototypes */
extern unsigned int debug_init(void);
extern unsigned int debug_print(const char *fmt, ...);
extern unsigned int debug_flush(void);


#endif


