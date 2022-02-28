/*******************************************************************************
 *
 * os.h - OS Porting layer
 *
 ******************************************************************************/
#ifndef UTV_SW_OS_H
#define UTV_SW_OS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * constants
 */
#include "const.h"

/*
 * OS dependent header file $(TOPDIR)/inc/$(OS)
 */
#include "os_specific.h"

/*
 * delete function....
 * Originally, it is used in qbuff.h.
 * Depending on the type of members in a queue,
 * user may free up all resources allocated for the queue
 * through just simple function invocation 'os_qbuff_...._(flush/delete).
 *
 *****************************/
typedef unsigned int (*os_delfunc_t)(void *);

/****************************
 *
 * R E S O U R C E    M A N A G E M E N T
 *
 *****************************/

/* list data structure */
typedef struct __list_data {
	void *data;             /* data */
#define OS_LIST_NO_KEY  (long)(0xffffffff)
	long key;               /* key */
	unsigned int resid;         /* object ID */
	struct __list_data *next;
	struct __list_data *prev;
}OS_LIST_DATA;

/* list container  -- pool */
typedef struct __list {
	unsigned char name[STR_LEN];
	OS_LIST_DATA *data;
	os_mutex_t mut;
	unsigned int counter;
}OS_LIST;

/* list operation */
extern OS_LIST * os_list_create(const char *name);
extern unsigned int os_list_delete(OS_LIST *h);
extern unsigned int os_list_flush(OS_LIST *h,os_delfunc_t f);
extern OS_LIST_DATA * os_list_find(OS_LIST *h, unsigned int (*comp)(OS_LIST_DATA *,void *),void *arg);
extern void os_list_navigate(OS_LIST *h, void (*haction)(OS_LIST *),void (*action)(OS_LIST_DATA *));
extern unsigned int os_list_sanity_check(OS_LIST *h);
extern unsigned int os_list_item_insert(OS_LIST *h,void *item);
extern unsigned int os_list_item_delete(OS_LIST *h,unsigned int id);
extern void * os_list_item_pop(OS_LIST *h);

/* list operation based on key */
extern void * os_list_findbykey(OS_LIST *h,long key);
extern unsigned int os_list_item_insert_order(OS_LIST *h,long key, void *item);
extern unsigned int os_list_item_insert_hash(OS_LIST *h,long key, void *item);
extern unsigned int os_list_item_deletebykey(OS_LIST *h,long key);
extern long os_list_firstkey(OS_LIST *h);

/* generic search function */
extern void * os_obj_find(OS_LIST *h,unsigned int id);

/****************************
 *
 * D  A  T  A      S  T  R  U  C  T  U  R  E
 *
 *****************************/

/* thread container */
typedef struct _os_thread_t
{
	unsigned char name[STR_LEN]; /* name */
	os_thrfunc_t func;      /* handler */
	os_delfunc_t dfunc;     /* deletion handler */
	os_thread_t runthr;     /* run thread ID (returned by OS) */
	os_thread_t monthr;     /* mon thread ID (returned by OS) */
	void * arg;         /* thread argument */

#define OS_THREAD_SIG_NONE  0x00000000  /* START   message */
#define OS_THREAD_SIG_START 0x00000001  /* START   message */
#define OS_THREAD_SIG_STOP  0x00000002  /* STOP   message */
	unsigned int sig;   /* signal */
	unsigned int restartable; /* restartable thread */
	unsigned int restartcnt; /* restart count */
	unsigned int id;        /* ID manageable within base component system */
} OS_THREAD;

/* mutex container */
typedef struct {
	unsigned char name[STR_LEN]; /* name */
	unsigned int owner;     /* owner */
	unsigned char validity; /* 1/0 */
#define OS_MUTEX_FREE   0x01
#define OS_MUTEX_LOCKED 0x02
	unsigned char status;   /* state */
	os_mutex_t osid;        /* MUTEX itself */
	unsigned int counter;   /* retry counter - trylock */
}OS_MUTEX;

/* cond container */
typedef struct {
	unsigned char name[STR_LEN]; /* name */
	unsigned int owner;     /* owner */
	os_cond_t osid;     /* COND itself */
	os_mutex_t mutex;       /* MUTEX */
	unsigned long sigtime;  /* time signalled */
	unsigned long waittime; /* time awating */
	os_mutex_t lock;    /* MUTEX */
}OS_COND;

/****************************
 *
 * I  N  I  T  I  A  L  I  Z  A  T  I  O  N
 *
 *****************************/

/* resource manager */
extern unsigned int os_res_init( void );

/* thread system initialization */
extern unsigned int os_thread_init( void );

/* mutex & conditional variable system initialization */
extern unsigned int os_var_init( void );

/* time system initialization */
extern unsigned int os_time_init( void );

/****************************
 *
 * R E S O U R C E     I D
 *
 *****************************/

/* sub-structure */
typedef struct {
	unsigned int count;
}os_res_info_sub_t;

/* resource information */
typedef struct {
	os_res_info_sub_t thread;
	os_res_info_sub_t mutex;
	os_res_info_sub_t cond;
	os_res_info_sub_t buffer;
	os_res_info_sub_t queue;
	os_res_info_sub_t msgq;
	os_res_info_sub_t bclass;
	os_res_info_sub_t nport;
	os_res_info_sub_t nchannel;
}os_res_info_t;

/* allocation */
extern unsigned int os_res_id_alloc(OS_LIST *);
/* deallocation */
extern unsigned int os_res_id_free(OS_LIST *,unsigned int);
/* applying a new ID */
extern unsigned int os_res_id_make_inuse(OS_LIST *,unsigned int);
/* making a name */
extern void os_res_make_name(unsigned char *,unsigned char *,unsigned int);
/* resource information */
extern void os_res_info(os_res_info_t *);

/****************************
 *
 * O  S    S  P  E  C  I  F  I  C    I  N  I  T
 *
 *****************************/

/* initialization */
extern unsigned int os_specific_init(void);
extern unsigned int os_post_init(void);

/****************************
 *
 * T  H  R  E  A  D
 *
 *****************************/

/* thread creation */
extern unsigned int os_thread_create(const char *name,
                                     os_thrfunc_t func,
                                     void *arg,
                                     os_delfunc_t dfunc,
                                     unsigned int restartable);

/* thread find */
extern unsigned int os_thread_find(const char *name);

/* thread find */
extern unsigned char * os_thread_name(unsigned int id);

/* thread self */
extern unsigned int os_thread_self(void);

/* thread delete */
extern void os_thread_delete(unsigned int id);

/* thread restart */
extern unsigned int os_thread_restart(unsigned int id);

/* thread restartable flag setup */
extern unsigned int os_thread_restartable(unsigned int id,unsigned int flag);

/* thread signal send */
extern unsigned int os_thread_signal_send(unsigned int id,unsigned int msg);

/* thread signal receive */
extern unsigned int os_thread_signal_recv(unsigned int id,unsigned int mode);

/* thread sleep */
extern void os_thread_sleep(unsigned int msec);

/* thread navigation */
extern void os_thread_verbose(void);

/* thread join */
extern unsigned int os_thread_isalive(unsigned int *c,unsigned int size);

extern void os_thread_log(void (*logfunc)(char*, int));

/****************************
 *
 *  M  U  T  E  X
 *
 *****************************/

/* mutex creation */
extern unsigned int os_mutex_create(const char *name);

/* mutex lock */
extern unsigned int os_mutex_lock(unsigned int id,unsigned int msec);

/* mutex unlock */
extern unsigned int os_mutex_unlock(unsigned int id);

/* mutex delete */
extern unsigned int os_mutex_delete(unsigned int id);

/* mutex navigation */
extern void os_mutex_verbose(void);

/****************************
 *
 *  C  O  N  D
 *
 *****************************/

/* condition creation */
extern unsigned int os_cond_create(const char *name);

/* cond signalling */
extern unsigned int os_cond_signal(unsigned int id);

/* cond wait */
extern unsigned int os_cond_acquire(unsigned int id);

/* cond wait with timeout */
extern unsigned int os_cond_acquire_timeout(unsigned int id,unsigned long tm);

/* cond wait */
extern unsigned int os_cond_release(unsigned int id);

/* cond delete */
extern unsigned int os_cond_delete(unsigned int id);

/* waiting time */
extern unsigned long os_cond_wait_time_get(unsigned int id);

/* cond navigation */
extern void os_cond_verbose(void);

/****************************
 *
 * T  I  M  E
 *
 *****************************/

/* time get */
extern unsigned long os_time_get(void);

/****************************
 *
 * E  R  R  N  O
 *
 *****************************/
#include "error.h"

/* errno */
extern void os_errno_set(unsigned int num);
extern unsigned int os_errno_get(void);

#ifdef __cplusplus
}
#endif

#endif /* UTV_SW_OS_H */
