/*******************************************************************************
 *
 * qbuff.h - Queue buffer header
 *
 ******************************************************************************/
#ifndef QUEUE_BUFFER_HEADER
#define QUEUE_BUFFER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/* OS dependent header */
#include "os.h"

/* qeo buffer node structure */
typedef struct _qeo_buffer_node {
	unsigned char *data;
	unsigned int size;          /* size */
	unsigned int tail;          /* starting location of
	                               available data in buffer */
	unsigned int unused;        /* for future use */
	struct _qeo_buffer_node *next;  /* next node */
}__attribute__((aligned)) OS_QNODE;
#define OS_QNODE_INITIALIZER {NULL,0,0,0,NULL} /* initial value */

/* qeo buffer head structure */
typedef struct _qeo_buffer_head {
	int channel;            /* channel */
	unsigned int current_size;      /* total sum of list */
	OS_QNODE *list;         /* list of node */
	unsigned int lock;          /* lock */
	unsigned int pool;          /* memory pool for data allocation */
	os_delfunc_t delfunc;   /* delete function */
	struct _qeo_buffer_head *next;  /* next head */
}__attribute__((aligned)) OS_QHEAD;

/* prototypes */
extern unsigned int os_qbuff_init(void);
extern unsigned int os_qbuff_create(const char *,unsigned int pool,os_delfunc_t delf);
extern unsigned int os_qbuff_delete(unsigned int);
extern unsigned int os_qbuff_flush(unsigned int);
extern unsigned int os_qbuff_put(unsigned int,char *,int);
extern unsigned int os_qbuff_get(unsigned int,char *,int);
extern unsigned int os_qbuff_current_size(unsigned int);
extern unsigned int os_qbuff_sanity_check(unsigned int);

#ifdef __cplusplus
}
#endif

#endif  /* QUEUE_BUFFER_HEADER */
