/*******************************************************************************
 *
 * thread.c - Thread
 *
 ******************************************************************************/
#include "oskit.h"

/*
 * List containers for thread resources
 */
static OS_LIST *thread_list;
static OS_THREAD thread_init; /* init thread */

/*******************************************************************************
 *
 * os_thread_init - Initializing THREAD
 *
 * DESCRIPTION
 *  Creating resource pools for thread
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
unsigned int os_thread_init(void)
{
	OS_THREAD *newthr;
	unsigned int res;

	/* thread list */
	if( (thread_list=os_list_create(THREAD_NAME)) == NULL ) {
		return ERROR;
	}
	os_debug_printf(OS_DBG_LVL_VER,"thread_list(%s) created\n",THREAD_NAME);

	/* init thread */
	newthr = (OS_THREAD *)&thread_init;
	strcpy ( (char *)newthr->name, (const char *)THREAD_NAME );          /* resource name */
	strncat( (char *)newthr->name, (const char *)"init", STR_LEN );  /* name */
	newthr->func        = NULL;
	newthr->arg         = NULL;     /* argument */
	newthr->sig         = 0;    /* signal */
	newthr->dfunc       = NULL;
	newthr->restartable = 0;
	newthr->restartcnt  = 0;
	newthr->runthr      = _os_thread_self();/* thread id */

	/* thread into resource pool */
	res = os_list_item_insert(thread_list,newthr);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(%s)::error\n",newthr->name);
		return ERROR;
	}
	newthr->id          = res;/* id */

	return OKAY;
}

/*******************************************************************************
*
*  T H R E A D    F U N C T I O N S
*
*******************************************************************************/

/*******************************************************************************
 *
 * os_thread_wrapper - Wrapper function...
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  (>0)  - Thread ID
 *  0     - Error
 *
 * ERRORS
 *   N/A
 *
 */
static void * os_thread_wrapper(void *arg)
{
	OS_THREAD *thrp = (OS_THREAD *)arg;

	if(!thrp) {
		os_debug_printf(OS_DBG_LVL_ERR,"null param\n");
		_os_thread_exit(0);
	}

	/* run practical logic */
	thrp->func(thrp->arg);

	/* done */
	os_debug_printf(OS_DBG_LVL_VER,"done\n");

	/* exit... */
	_os_thread_exit(0);
}

/*******************************************************************************
 *
 * os_monitor_thread - Just keep thread controller
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  func - practical business logic...
 *
 * RETURNS
 *  (>0)  - Thread ID
 *  0     - Error
 *
 * ERRORS
 *   N/A
 *
 */
static void * os_monitor_thread(void *arg)
{
	unsigned int res;
	OS_THREAD *thrp = (OS_THREAD *)arg;

	/* thread has been created... */
	while(1) {

		/* Processing this message is up to programmer's responsibility */
		/* signalling a thread to wake up */
		os_thread_signal_send(thrp->id,OS_THREAD_SIG_START);

		/* reach here when a thread dead */
		_os_thread_join(thrp->runthr,NULL);
		os_debug_printf(OS_DBG_LVL_VER,"joined with (run) %ld\n",thrp->runthr);

		/* invoke delete upcall function */
		if(thrp->dfunc) {
			os_debug_printf(OS_DBG_LVL_VER, "upcall (run) %ld\n",thrp->runthr);
			thrp->dfunc(thrp->arg);
		}

		/* restartable thread... */
		if(!thrp->restartable)
			break;

		os_debug_printf(OS_DBG_LVL_VER,"Restarting a thread \n");

		/* spawning a user thread function */
		if( _os_thread_create(&(thrp->runthr),os_thread_wrapper,(void *)thrp) )
			os_debug_printf(OS_DBG_LVL_ERR,"os_thread_create(%s)::failed\n",thrp->name);

		/* increasing restart count */
		++thrp->restartcnt;
	} /* while(1) */

	/* remve thread from a list */
	res = os_list_item_delete(thread_list,thrp->id);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_delete(%s)::error\n",thrp->name);
		_os_thread_exit(0);
		return ERROR;
	}

	/* freeing resources */
	os_mem_free(thrp);

	/* clean up message queue */
	os_debug_printf(OS_DBG_LVL_VER,"(runner)done\n");

	/* exiting thread */
	_os_thread_exit(0);
}

/*******************************************************************************
 *
 * os_thread_create - Creating a thread
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  name - thread name
 *  func - thread handler function
 *  arg  - argument for the handler
 *  prio - priority
 *
 * RETURNS
 *  (>0)  - Thread ID
 *  0     - Error
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_thread_create(const char *name,
                              os_thrfunc_t func,
                              void *arg,
                              os_delfunc_t dfunc, /* delete function */
                              unsigned int restartable) /* restartable flag */
{
	unsigned char tname[STR_LEN];
	unsigned int res;
	OS_THREAD *newthr;

	newthr = (OS_THREAD *)os_mem_alloc(sizeof(OS_THREAD)); /* allocating a memory */
	if(!newthr) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mem_alloc(sizeof(OS_THREAD))::error\n");
		return ERROR;
	}
	strcpy ( (char *)newthr->name, (const char *)THREAD_NAME );         /* resource name */
	strncat( (char *)newthr->name, (const char *)name, STR_LEN );   /* name */
	newthr->func        = func;     /* handler */
	newthr->arg         = arg;      /* argument */
	newthr->sig         = 0;    /* signal */
	newthr->dfunc       = dfunc;    /* delete function */
	newthr->restartable = restartable;  /* restartable */

	/* thread into resource pool */
	res = os_list_item_insert(thread_list,newthr);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(%s)::error\n",newthr->name);
		/* deleting a data block */
		os_mem_free(newthr);
		return ERROR;
	}

	/* res -> thread ID */
	newthr->id = res;

	/* spawning a user thread function */
	if( _os_thread_create(&(newthr->runthr),os_thread_wrapper,(void *)newthr) ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_thread_create(%s)::failed\n",newthr->name);
		/* deleting a data block */
		os_mem_free(newthr);
		return ERROR;
	}
	os_debug_printf(OS_DBG_LVL_VER,"new thread (runthr) %ld\n",newthr->runthr);

	/* running monitor thread */
	if( _os_thread_create(&(newthr->monthr),os_monitor_thread,newthr) ) {
		os_debug_printf(OS_DBG_LVL_ERR,"_os_thread_create(%s)::failed\n",name);
		/* deleting a data block */
		os_mem_free(newthr);
		/* deleting an item */
		os_list_item_delete(thread_list,res);
		return ERROR;
	}
	os_debug_printf(OS_DBG_LVL_VER,"new thread (monthr) %ld\n",newthr->monthr);

	/*
	 * VERY IMPORTANT:
	 * Especially Linux environment keeps a important meaning of pthread_detach.
	 * Windows/Solaris does not incur any resource problem in thread handling, since they clearly
	 * free up all resources used by a thread to be killed. However, Linux handles threads at the
	 * same level to an application, user explicitly specify the moment of resource arrangement.
	 * pthread_detach() imposed a thread to run out of thread manager control, instead make it
	 * run as a separate task in the system level. We do not need to concern about resource
	 * collection.
	 * To the contrary, pthread_detach() makes a thread not to be synchronized through pthread_join(),
	 * os_thread_wrapper() should not be detached, but it will be joined in monitor thread.
	 * - 
	 */
	/* _os_thread_detach(newthr->monthr); */
	os_debug_printf(OS_DBG_LVL_VER,"(%s)(osid=%08x)done\n",name,newthr->id);
	/* newthr->id has been setup in parent task...  */
	return newthr->id;
}

/*******************************************************************************
 *
 * os_thread_find_with_resid - Comparator function for os_thread_find
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
static unsigned int os_thread_find_with_resid(
	OS_LIST_DATA *d,
	void *arg)
{
	if(!arg || !d) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid parameter\n");
		return ERROR; /* invalid parameter */
	}

	return (d->resid == *(unsigned int *)arg) ? OKAY : ERROR;
}

/*******************************************************************************
 *
 * os_thread_find_with_child_id - Comparator function for os_thread_find
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
static unsigned int os_thread_find_with_child_id(
	OS_LIST_DATA *d,
	void *arg)
{
	OS_THREAD *os_thr;

	if(!arg || !d) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid parameter\n");
		return ERROR; /* invalid parameter */
	}

	if(d->data) {
		os_thr = (OS_THREAD *)d->data;
	}else{
		os_debug_printf(OS_DBG_LVL_ERR,"inconsistent error\n");
		return ERROR;
	}

	return (((unsigned int)os_thr->runthr) == *(unsigned int *)arg) ? OKAY : ERROR;
}

/*******************************************************************************
 *
 * os_thread_signal_send - Sending a signal to a thread
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_thread_signal_send(unsigned int id,unsigned int sig)
{
	OS_LIST_DATA *d;
	OS_THREAD *thr;

	d = os_list_find(thread_list, os_thread_find_with_resid, (void *)&id );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_thread_signal_send::os_list_find::failed (%08x)\n",id);
		return ERROR;
	}
	thr = (OS_THREAD *)(d->data);

	/* signalling */
	thr->sig = sig;
	os_debug_printf(OS_DBG_LVL_VER,"signal thread (%08x)\n",sig );

	return OKAY;
}

/*******************************************************************************
 *
 * os_thread_signal_recv - Receiving a signal
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ERROR/msg
 *
 * ERRORS
 *  N/A
 */
unsigned int os_thread_signal_recv(unsigned int id,unsigned int mode)
{
	OS_LIST_DATA *d;
	OS_THREAD *thr;
	unsigned int sig;

	d = os_list_find(thread_list, os_thread_find_with_resid, (void *)&id );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_thread_signal_recv::os_list_find::failed (%08x)\n",id);
		return ERROR;
	}
	thr = (OS_THREAD *)(d->data);

	/*
	 * When mode == 0, this function works in non-blocking mode,
	 * therefore, it may return immediately without any new signal.
	 * However, mode != 0 may cause this API to work in blocking mode,
	 * It await the arrival of signal.
	 */
	if( mode != 0 ) {
		while( thr->sig  == 0 ) { os_thread_sleep(10); }
	}

	sig = thr->sig; /* get signal    */
	thr->sig = 0;  /* clear signal  */

	os_debug_printf(OS_DBG_LVL_VER,"recv signal (%08x)\n",sig );
	return sig; /* return signal */
}

/*******************************************************************************
 *
 * os_thread_self - Finding ID of current thread
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  ID of current thread
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_thread_self(void)
{
	unsigned int id = _os_thread_self(); /* system id */
	OS_LIST_DATA *d;
	OS_THREAD *thr;

	d = os_list_find(thread_list, os_thread_find_with_child_id, (void *)&id );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"failed (%08x)\n",id);
		return ERROR;
	}
	thr = (OS_THREAD *)(d->data);

	return d->resid; /* resource id */
}

/*******************************************************************************
 *
 * os_thread_delete - Deleting a thread
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - thread ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
void os_thread_delete(unsigned int id)
{
	OS_LIST_DATA *d;
	OS_THREAD *thr;

	/* join process */
	d = os_list_find(thread_list, os_thread_find_with_resid, (void *)&id );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"failed (%08x)\n",id);
		return;
	}
	thr = (OS_THREAD *)(d->data);
	if(!thr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null thread(%08x)\n",id);
		return;
	}

	/* OS dependent system call */
	_os_thread_stop(thr->runthr);
	os_debug_printf(OS_DBG_LVL_VER,"kill (run) %ld\n",thr->runthr);

	/* Restartable thread doesn't return.
	 */
	/* reach here when a thread dead */
	if(!thr->restartable) {
		_os_thread_join(thr->monthr,NULL);
		os_debug_printf(OS_DBG_LVL_VER,"joined with (mon) %ld\n",thr->monthr);
	}

	/* done */
	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);
}

/*******************************************************************************
 *
 * os_thread_restart - Restarting a thread
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - thread ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_thread_restart(unsigned int id)
{
	OS_LIST_DATA *d;
	OS_THREAD *thr;

	/* chainging attributes */
	d = os_list_find(thread_list, os_thread_find_with_resid, (void *)&id );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"failed (%08x)\n",id);
		return ERROR;
	}
	thr = (OS_THREAD *)(d->data);
	if(!thr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null thread(%08x)\n",id);
		return ERROR;
	}
	thr->restartable = 1; /* restartable thread... */
	thr->restartcnt  = 0;/* reset counter */

	/* Deleting a thread */
	os_thread_delete(id);

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_thread_restartable - Changing restartable option of a thread
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - thread ID
 *
 * RETURNS
 *   OKAY/ERROR
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_thread_restartable(unsigned int id,unsigned int flag)
{
	OS_LIST_DATA *d;
	OS_THREAD *thr;

	/* chainging attributes */
	d = os_list_find(thread_list, os_thread_find_with_resid, (void *)&id );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"failed (%08x)\n",id);
		return ERROR;
	}
	thr = (OS_THREAD *)(d->data);
	if(!thr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null thread(%08x)\n",id);
		return ERROR;
	}
	thr->restartable = flag; /* restartable thread... */

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_thread_name - Returning the name of a thread
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  name
 *
 * ERRORS
 *   N/A
 *
 */
unsigned char * os_thread_name(unsigned int id)
{
	OS_LIST_DATA *d;
	OS_THREAD *thr;

	d = os_list_find(thread_list, os_thread_find_with_resid, (void *)&id );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"failed\n");
		return ERROR;
	}
	thr = (OS_THREAD *)(d->data);

	return thr->name; /* name */
}

/*******************************************************************************
 *
 * os_thread_isalive - Testing set of threads are alive
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *   OKAY - Any of alive threads are found.
 *   ERROR - None of alive threads
 *
 * ERRORS
 *   N/A
 *
 */
unsigned int os_thread_isalive(unsigned int *c,unsigned int size)
{
	unsigned int ix;
	OS_LIST_DATA *d;
	for(ix=0; ix<size; ix++) {
		d = os_list_find(thread_list,os_thread_find_with_resid,(void *)&(c[ix]));
		if(d) return OKAY;
	}
	return ERROR;
}

/*******************************************************************************
 *
 * os_thread_sleep - Sleeping a thread
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  microseconds time
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *   N/A
 *
 */
void os_thread_sleep(unsigned int msec)
{
	usleep(msec);
}

/*******************************************************************************
 *
 * os_thread_walkup_haction/os_thread_walkup_action - Action function in
 *      os_list_navigate
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 *
 */
static void os_thread_walkup_haction(OS_LIST *h)
{
	printf("<INFO> Resource (%s)\n",h->name);
	printf("<INFO>   Number (%d)\n",h->counter);
}

static void os_thread_walkup_action(OS_LIST_DATA *h)
{
	printf("<INFO>  Thread(%s) :: Addr(%p) AddrD(%p))\n",((OS_THREAD *)(h->data))->name, h, h->data);
}

/*******************************************************************************
 *
 * os_thread_verbose - Thread list verbose message
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
void os_thread_verbose(void)
{
	os_list_navigate(thread_list, NULL /*os_thread_walkup_haction*/, os_thread_walkup_action);
}


