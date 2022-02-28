/*******************************************************************************
 *
 * dbg.c - debugging module
 *
 ******************************************************************************/
#include "oskit.h"
#include "mesg.h"

/* not used... */
/*static*/ unsigned int os_debug_pool_size = DEBUG_MEM_CLASS_SIZE;

/* temporary debug buffer */
static unsigned int dbg_mutex;
static unsigned char dbg_temp_buff[DBG_BUF_LEN];
static unsigned int dbg_log_amount = 0;

#define ONE_SECOND 1000000
static unsigned int dbg_clock_tick = ONE_SECOND;

/* prototype */
static os_thrfunc_t dbg_thr(void *arg);

/*******************************************************************************
 *
 * dbg_msgq_delete_upcall
 *
 * DESCRIPTION
 */
static os_delfunc_t dbg_msgq_delete_upcall(void *arg)
{
	os_mutex_delete(dbg_mutex);
}

/*******************************************************************************
 *
 * debug_init
 *
 * DESCRIPTION
 *  Initializing debugging system, and then creating a log thread.
 *  It establishes a connection with log server to prepare for centralized
 *  remote logging.
 *
 * PARAMETERS
 *
 *
 * RETURNS
 *
 *
 *
 * ERRORS
 *
 * .
 */
unsigned int debug_init(void)
{
	unsigned int res;
	unsigned char tname[STR_LEN];

	/* debugger mutex name */
	os_res_make_name((unsigned char *)tname,(unsigned char *)MUTEX_NAME,0x99);

	/* mutex */
	dbg_mutex = os_mutex_create((const char *)tname); /* debugging mutex */
	if(dbg_mutex == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_create::error\n");
		return ERROR;
	}

	/* debugger thread name */
	os_res_make_name((unsigned char *)tname,(unsigned char *)THREAD_NAME,0x99);

	/* none of delete function & non-restartable */
	res = os_thread_create((const char *)tname,(os_thrfunc_t)&dbg_thr,NULL,NULL,0);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_thread_create(..)::error\n");
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"done\n");
	return OKAY;
}

/*******************************************************************************
 *
 * dbg_thr - Logging thread
 *
 * DESCRIPTION
 *   Receiving log message, and then prints it out to console/network
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 * .
 */
os_thrfunc_t dbg_thr(void *arg)
{
	/* initial message */
	/* It awaits the arrival of signal xxxx_START in blocking mode */
	if(os_thread_signal_recv(os_thread_self(),1) != OS_THREAD_SIG_START) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid thread message\n");
	}else{

		while(1) {
			os_thread_sleep(dbg_clock_tick); /* 1 second */

			/* is empty ?? */
			if(!dbg_log_amount) {
				if((dbg_clock_tick+ONE_SECOND) <
				   (60*ONE_SECOND)) { /* 60 seconds - maximum time bound */
					dbg_clock_tick += ONE_SECOND; /* increasing... */
				}
				continue;
			}
			os_mutex_lock(dbg_mutex,0);

			debug_flush(); /* flushing... */

			os_mutex_unlock(dbg_mutex);
		}

	}
}

/*******************************************************************************
 *
 * debug_print - Printing out debugging function
 *
 * DESCRIPTION
 *  Printing out debug message. The message may be printed on the console,
 *  otherwise deleivered through network for remote logging.
 *
 * PARAMETERS
 *
 *
 * RETURNS
 *  length of message
 *
 * ERRORS
 *  N/A
 * .
 */
unsigned int debug_print(const char *fmt, ...)
{
	va_list ap;
	unsigned int len;

	/* lock */
	os_mutex_lock(dbg_mutex,0);

	va_start(ap,fmt);
	len = sprintf((char *)dbg_temp_buff+dbg_log_amount,"%8ld - ",os_time_get());
	dbg_log_amount += len;
	len = vsprintf((char *)dbg_temp_buff+dbg_log_amount,fmt,ap);
	dbg_log_amount += len;
	va_end(ap);

	dbg_clock_tick = ONE_SECOND;

	if(dbg_log_amount>=(DBG_BUF_LEN-DBG_BUFF_MIN_AMOUNT)) {
		debug_flush(); /* flushing... */
	}

	/* unlock */
	os_mutex_unlock(dbg_mutex);

	return OKAY;
}

/*******************************************************************************
 *
 * debug_flush
 *
 * DESCRIPTION
 *  Flushing out all the debugging message stacked up.
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  OKAY
 *
 * ERRORS
 *  N/A
 * .
 */
unsigned int debug_flush(void)
{
	os_printf(dbg_temp_buff);
	dbg_log_amount = 0;
	return OKAY;
}

