/*******************************************************************************
 *
 * const.h - Constants header
 *
 ******************************************************************************/
#ifndef OS_CONST_HEADER
#define OS_CONST_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/* name of OS resources */
#define THREAD_NAME     "sysTHRD"
#define COND_NAME       "sysCOND"
#define MUTEX_NAME      "sysMUTX"
#define QUEUE_NAME      "sysQUEU"
#define BUFFER_NAME     "sysBUFF"
#define MSGQ_NAME       "sysMSGQ"
#define BUFFER_CLASS_NAME   "sysBCLS"
#define NET_PORT_CLASS_NAME "sysNETP"
#define NET_CHAN_CLASS_NAME "sysNETC"
#define NET_CHAN_NAME       "sysCHAN"
#define NET_SEL_NAME        "sysSELT"
#define ORDER_LIST_NAME     "sysOLST"
#define WIN32_THREAD_NAME   "sysWTHR"
#define NO_RES_NAME     "UNKNOWN"

#define OS_RES_NUM  20     /* thread, cond, mutex, queue, buffer & etc. */
#define OS_MAX_RES  4000   /* changed from 256 -> 4000 */
#define OS_RES_BIT_SIZE (OS_MAX_RES/sizeof(unsigned char))

/* string length */
#define STR_LEN     32

/* return values */
#define ERROR       0
#define OKAY        1

/* system memory space */
#define OS_SYSMEM_SIZE  (9*4096) /* 36 Kbytes */

/* debugging message level parameters */
#define OS_DBG_LVL_ERR  0x00000001
#define OS_DBG_LVL_VER  0x00000002
#define OS_DBG_LVL_MSG  0x00000003

#ifdef __cplusplus
}
#endif

#endif /* OS_CONST_HEADER */
