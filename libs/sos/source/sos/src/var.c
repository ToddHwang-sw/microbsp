/*******************************************************************************
 *
 * var.c - MUTEX & COND
 *
 ******************************************************************************/
#include "os.h"
#include "mem.h"
#include "dbg.h"

/*
 * List containers for OS resources
 */
static OS_LIST *cond_list;
static OS_LIST *mutex_list;

/*******************************************************************************
 *
 * os_var_init - Initializing COND & MUTEX
 *
 * DESCRIPTION
 *  Creating resource pools for cond/mutex.
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  ERROR - Error
 *  OKAY    - Success
 *
 * ERRORS
 *  N/A
 */
unsigned int os_var_init(void)
{
	/* mutex list */
	if( (mutex_list=os_list_create(MUTEX_NAME)) == NULL ) {
		return ERROR;
	}
	os_debug_printf(OS_DBG_LVL_VER,"mutex_list(%s) created\n",MUTEX_NAME);

	/* cond list */
	if( (cond_list=os_list_create(COND_NAME)) == NULL ) {
		os_list_delete(mutex_list);
		return ERROR;
	}
	os_debug_printf(OS_DBG_LVL_VER,"cond_list(%s) created\n",COND_NAME);

	return OKAY;
}

/*******************************************************************************
*
*  M U T E X     S E M A P H O R E     F U N C T I O N S
*
*******************************************************************************/

/*******************************************************************************
 *
 * os_mutex_create - Creating a mutex
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  name - mutex name
 *
 * RETURNS
 *  (>0)  - Mutex ID
 *  0     - Error
 *
 * ERRORS
 *  N/A
 *
 */
unsigned int os_mutex_create(const char *name)
{
	unsigned int res;
	OS_MUTEX *newmut;

	newmut = (OS_MUTEX *)os_mem_alloc(sizeof(OS_MUTEX)); /* allocating a memory */
	if(!newmut) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mem_alloc(sizeof(OS_MUTEX)::error\n");
		return ERROR;
	}
	strcpy ( (char *)newmut->name, (const char *)MUTEX_NAME );        /* resource name */
	strncat( (char *)newmut->name,(const char *)name, STR_LEN );  /* name */
  #ifdef MUTEX_OWNER_CHECK
	newmut->owner = 0;              /* owner */
  #endif
	newmut->status = OS_MUTEX_FREE;     /* FREE status */
	newmut->validity = 0;           /* counter */

	res = os_list_item_insert(mutex_list,newmut); /* mutex into resource pool */
	if(res == ERROR) {
		os_mem_free(newmut);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(%s)::error\n",name);
		return ERROR;
	}

	_os_mutex_init(&(newmut->osid));

	os_debug_printf(OS_DBG_LVL_VER,"(%s)(%08x)::done\n",name,res);

	return res;
}

/*******************************************************************************
 *
 * os_obj_find_with_id - Comparator function for os_mutex_find
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *   N/A
 *
 */
static unsigned int os_obj_find_with_id( OS_LIST_DATA *d, void *arg)
{
	if(!arg) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid parameter\n");
		return ERROR; /* invalid parameter */
	}
	if(!d->data) {
		os_debug_printf(OS_DBG_LVL_ERR,"inconsistent error\n");
		return ERROR;
	}
	return (d->resid == *(unsigned int *)arg) ? OKAY : ERROR;
}

/*******************************************************************************
 *
 * os_obj_find - Finding ID of an object
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  ID of object
 *
 * ERRORS
 *   N/A
 *
 */
void * os_obj_find(OS_LIST *h,unsigned int id)
{
	unsigned int mid = id;
	OS_LIST_DATA *d;

	d = os_list_find(h, os_obj_find_with_id, (void *)&mid );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_find(%08x)::error\n",id);
		return NULL;
	}
	return d->data;
}

/*******************************************************************************
 *
 * os_mutex_find - Finding ID of mutex
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  ID of MUTEX
 *
 * ERRORS
 *   N/A
 *
 */
OS_MUTEX * os_mutex_find(unsigned int id)
{
	unsigned int mid = id;
	OS_LIST_DATA *d;

	d = os_list_find(mutex_list, os_obj_find_with_id, (void *)&mid );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_find(%08x)::error\n",id);
		return NULL;
	}
	return (OS_MUTEX *)(d->data);
}

/*******************************************************************************
 *
 * os_mutex_delete - Deleting a mutex
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - mutex ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_mutex_delete(unsigned int id)
{
	OS_MUTEX *mut;
	unsigned int res;

	/* mutex object free */
	mut = os_mutex_find(id);
	if(!mut) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_find(%08x)::error\n",id);
		return ERROR;
	}

  #if 0
	/*
	 * If any other thread competing object will be
	 * released.
	 */
	if(mut->status == OS_MUTEX_LOCKED) {
		_os_mutex_unlock( &(mut->osid) ); /* unlock */
	}
  #endif

	os_mem_free(mut); /* deleting mutex */

	/* list free */
	res = os_list_item_delete(mutex_list,id);
	if(res != OKAY) {
		/* error state */
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_delete(%08x)::error\n",id);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);
	return OKAY;
}

/*******************************************************************************
 *
 * os_mutex_lock - Locking a mutex
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - mutex ID
 *  t  - time (milliseconds)
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_mutex_lock(unsigned int id,unsigned int t)
{
	OS_MUTEX *m;
	unsigned int sleep_base;
	unsigned long given_time;
	unsigned long init_time, last_time, wait_time;
	unsigned long tot_wait_time;

	m = os_mutex_find(id);
	if(!m) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_lock(%08x)\n",id);
		return ERROR;
	}

	if(t) { /* time driven try to obtain a lock */
		/* determing sleep base interval */
		sleep_base = t/10; /* divide by 10 */
		given_time = (sleep_base<100) ? 100 : sleep_base; /* 100 microseconds is minimum */
		while(1) {
			init_time = os_time_get();
			if( _os_mutex_trylock(&(m->osid)) == 0 )
				break; /* yes! lock obtained */
			os_thread_sleep(given_time); /* sleep */
			last_time = os_time_get();
			wait_time = (unsigned long)(last_time - init_time); /* wait time */
			tot_wait_time += wait_time;                     /* total wait time */
			if( tot_wait_time > t ) {
				return ERROR; /* time out */
			}
		}
	}
	else /* inifinite waiting for a lock */
		_os_mutex_lock( &(m->osid) );

	if( ++m->validity != 1 ) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::validaity error\n",id);
		_os_mutex_unlock( &(m->osid) );
		return ERROR;
	}

	if(m->status == OS_MUTEX_LOCKED) { /* locked ?? */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::status error\n",id);
		_os_mutex_unlock( &(m->osid) );
		return ERROR;
	}
	m->status = OS_MUTEX_LOCKED;

  #ifdef MUTEX_OWNER_CHECK
	if(m->owner != 0) { /* who owns this ... */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::ownership error\n",id);
		_os_mutex_unlock( &(m->osid) );
		return ERROR;
	}

	m->owner = os_thread_self();
  #endif

	/* os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id); */

	return OKAY;
}

/*******************************************************************************
 *
 * os_mutex_unlock - Unlocking a mutex
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - mutex ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_mutex_unlock(unsigned int id)
{
	OS_MUTEX *m;

	m = os_mutex_find(id);
	if(!m) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_find(%08x)\n",id);
		_os_mutex_unlock( &(m->osid) );
		return ERROR;
	}

	if( --m->validity != 0 || m->status != OS_MUTEX_LOCKED ) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::validity && status error\n",id);
		_os_mutex_unlock( &(m->osid) );
		return ERROR;
	}

	m->status = OS_MUTEX_FREE;
  #ifdef MUTEX_OWNER_CHECK
	m->owner = 0;
  #endif

	_os_mutex_unlock( &(m->osid) );

	/* os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id); */

	return OKAY;
}

static void os_mutex_walkup_action(OS_LIST_DATA *h)
{
	os_debug_printf(OS_DBG_LVL_VER,"<MUTEX> ::name(%32s)::addr(%p)::data(%p)::counter(%d)\n",
	                ((OS_MUTEX *)(h->data))->name, h, h->data, ((OS_MUTEX *)(h->data))->counter );
}

/*******************************************************************************
 *
 * os_mutex_verbose - MUTEX list verbose message
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *   N/A
 *
 */
void os_mutex_verbose(void)
{
	os_list_navigate(mutex_list, NULL, os_mutex_walkup_action);
}

/*******************************************************************************
*
*  C O N D I T I O N A L    V A R I A B L E S    F U N C T I O N S
*
*******************************************************************************/

/*******************************************************************************
 *
 * os_cond_create - Creating a conditional variables
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  name - cond name
 *
 * RETURNS
 *  (>0)  - Mutex ID
 *  0     - Error
 *
 * ERRORS
 *  N/A
 *
 */
unsigned int os_cond_create(const char *name)
{
	unsigned int res;
	OS_COND *newcond;

	newcond = (OS_COND *)os_mem_alloc(sizeof(OS_COND)); /* allocating a memory */
	if(!newcond) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mem_alloc(sizeof(OS_COND))::error\n");
		return ERROR;
	}
	strcpy ( (char *)newcond->name, (const char *)COND_NAME );          /* reosurce name */
	strncat( (char *)newcond->name, (const char *)name, STR_LEN );  /* name */
  #ifdef MUTEX_OWNER_CHECK
	newcond->owner = 0;                 /* owner */
  #endif

	res = os_list_item_insert(cond_list,newcond); /* cond into resource pool */
	if(res == ERROR) {
		os_mem_free(newcond);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(%s)::error\n",name);
		return ERROR;
	}

	_os_mutex_init(&(newcond->lock)); /* own lock */
	_os_mutex_init(&(newcond->mutex)); /* MUTEX */
	_os_cond_init(&(newcond->osid)); /* COND ID from OS function */

	os_debug_printf(OS_DBG_LVL_VER,"(%s)(%08x)::done\n",name,res);

	return res;
}

/*******************************************************************************
 *
 * os_cond_find - Finding ID of COND
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  ID of COND
 *
 * ERRORS
 *   N/A
 *
 */
OS_COND * os_cond_find(unsigned int id)
{
	unsigned int mid = id;
	OS_LIST_DATA *d;

	d = os_list_find(cond_list, os_obj_find_with_id, (void *)&mid );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_find(%08x)\n",id);
		return NULL;
	}
	return (OS_COND *)(d->data);
}

/*******************************************************************************
 *
 * os_cond_delete - Deleting a conditional variable
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - cond ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_cond_delete(unsigned int id)
{
	unsigned int res;
	OS_COND *d;

  #if 0
	/*
	 * If any other thread/object is racing to acquire current
	 * conditional variable, they better to get out of the waiting loop,
	 * and make them have a chance to check up the existence of
	 * current conditional variable.
	 */
	if( os_cond_signal(id) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_cond_find(%08x)::error\n",id);
		return ERROR;
	}
  #endif

	d = os_cond_find(id);
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_cond_find(%08x)::error\n",id);
		return ERROR;
	}

	os_mem_free(d); /* deleting conditional variable */

	res = os_list_item_delete(cond_list,id);
	if(res != OKAY) {
		/* error state */
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_delete(%08x)::error\n",id);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_cond_signal - Signalling to a cond
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - COND ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_cond_signal(unsigned int id)
{
	OS_COND *d;

	d = os_cond_find(id);
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_cond_find(%08x)::error\n",id);
		return ERROR;
	}

	_os_mutex_lock( &(d->mutex) );
	d->sigtime = os_time_get();
	_os_cond_signal( &(d->osid) );
	_os_mutex_unlock( &(d->mutex) );

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_cond_acquire - Waiting a signal
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - COND ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_cond_acquire(unsigned int id)
{
	OS_COND *d;

	d = os_cond_find(id);
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_cond_find(%08x)::error\n",id);
		return ERROR;
	}

	_os_mutex_lock(&(d->mutex));
	d->waittime = os_time_get();
	_os_cond_wait(&(d->osid),&(d->mutex));

	/* calculating how long... */
	d->waittime = os_time_get() - d->waittime;

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_cond_acquire_timeout - Waiting a signal with timeout
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - COND ID
 *  tm - time (microseconds)
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_cond_acquire_timeout(unsigned int id,unsigned long tm)
{
	OS_COND *d;
	struct timespec ts;
	os_time_t tv;

	d = os_cond_find(id);
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_cond_find(%08x)::error\n",id);
		return ERROR;
	}

	_os_mutex_lock(&(d->mutex));

	/* calculate time to wait for */
	os_gettime(&tv);
	ts.tv_sec = tv.tv_sec + (tm/1000000);
	ts.tv_nsec = (tv.tv_usec + (tm%1000000))*1000; /* nanosecond */

	/* current time */
	d->waittime = os_time_get();

	/* timed wait */
	_os_cond_twait(&(d->osid),&(d->mutex),&ts);

	/* calculating how long... */
	d->waittime = os_time_get() - d->waittime;

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_cond_wait_time_get - Calculating wait time.
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - COND ID
 *
 * RETURNS
 *   time/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned long os_cond_wait_time_get(unsigned int id)
{
	OS_COND *d;
	struct timespec ts;

	d = os_cond_find(id);
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_cond_find(%08x)::error\n",id);
		return ERROR;
	}

	return d->waittime;
}

/*******************************************************************************
 *
 * os_cond_release - Releasing from a signal lock
 *
 * DESCRIPTION
 *  This API is paired with os_cond_acquire().
 *  os_cond_acquire();
 *   ...
 *  os_cond_release();
 *
 * PARAMETERS
 *  id - COND ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_cond_release(unsigned int id)
{
	OS_COND *d;

	d = os_cond_find(id);
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_cond_find(%08x)::error\n",id);
		return ERROR;
	}

	_os_mutex_unlock(&(d->mutex));

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);
	return OKAY;
}

/*
 * toolkit function
 *****************************************************************************/
static void os_cond_walkup_action(OS_LIST_DATA *h)
{
	os_debug_printf(OS_DBG_LVL_VER,"<COND>  ::name(%32s)::addr(%p)data(%p) \n",
	                ((OS_COND *)(h->data))->name, h, h->data );
}

/*******************************************************************************
 *
 * os_cond_verbose - COND list verbose message
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *   N/A
 *
 */
void os_cond_verbose(void)
{
	os_list_navigate(cond_list, NULL, os_cond_walkup_action);
}

