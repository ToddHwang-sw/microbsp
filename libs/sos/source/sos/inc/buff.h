/*******************************************************************************
 *
 * buff.h - Buffer management module
 *
 ******************************************************************************/
#ifndef __U_BUFFER_H__
#define __U_BUFFER_H__

#include "const.h"

#ifdef __cplusplus
extern "C" {
#endif

/* buffer malloc/free history structure */
typedef struct _buff_hist {
	unsigned char oname[STR_LEN];
	unsigned int size;
	unsigned char *data;
	struct _buff_hist *next;
}buff_history_st;

/* class structure */
typedef struct {
	unsigned int id;
	unsigned int objectid;
	unsigned char name[STR_LEN]; /* name */
	unsigned int osize;             /* original size to be specified */
	unsigned int totsize;           /* total size of class */
	unsigned int availsize;         /* available size      */
	unsigned int numblocks;         /* number of block data block */
	unsigned int lock;              /* class lock */
	regionid_t data;                /* data block */
	unsigned char *base;            /* base address of data */
	unsigned char *system_base;     /* malloced' address */
	unsigned char fixedbase;        /* fixed base? */
	unsigned char h_start;      /* history flag */
	unsigned int h_lock;            /* history lock */
	buff_history_st *h;     /* history vector - for debugging */
}buff_class_node;

#define BOUNDARY_MAGIC_NUM 0x5a5aefe5

/* prototypes */
extern unsigned int buff_init(void);
extern unsigned int buff_create_class(unsigned int id, unsigned char *baseaddr, unsigned int classsize);
extern unsigned int buff_delete_class(unsigned int id);
extern unsigned char * buff_alloc_data(unsigned int id,unsigned int size);
extern unsigned int buff_free_data(unsigned int id, unsigned char* buff);

/* turning on/off history function */
extern unsigned int buff_history_class(unsigned int id, unsigned int flag);

#define BUFF_GET_ID    0x00000011
#define BUFF_GET_SIZE  0x00000022
extern unsigned int buff_get_info(unsigned int command, unsigned char* buff);
extern unsigned int buff_current_availsize(unsigned int id);

extern unsigned int buff_verbose(int id);

#ifdef __cplusplus
}
#endif

#endif  /* __U_BUFFER_H__ */
