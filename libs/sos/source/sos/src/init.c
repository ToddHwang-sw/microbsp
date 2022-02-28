/*******************************************************************************
 *
 * init.c - Initialization
 *
 ******************************************************************************/
#include "oskit.h"

/* os_debug_printf leveling */
//unsigned int dbg_printf_flag = OS_DBG_LVL_ERR;
unsigned int dbg_printf_flag = OS_DBG_LVL_ERR | OS_DBG_LVL_VER;

#define SYSTEM_PANIC exit(0)
static void OS_INIT_MACRO(unsigned int (*f)(void),const char *mesg)
{
	if( f() == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"%s",mesg);
		SYSTEM_PANIC;
	}
}

/*******************************************************************************
 *
 * os_init - Invocating all initialization functions
 *
 * DESCRIPTION
 *  Initializing base components.
 *  os_specific_init() and os_post_init() ?????
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
void os_init(void)
{
	OS_INIT_MACRO(os_specific_init,"OS dependent init\n");  /* OS dependent module */
	OS_INIT_MACRO(os_res_init,"Resource manager init\n");       /* resource manager */
	OS_INIT_MACRO(os_mem_init,"Memory system error\n"); /* memory buffer */
	OS_INIT_MACRO(os_post_init,"OS post init\n");       /* OS dependent module */
	OS_INIT_MACRO(os_time_init,"Time system error\n");      /* time */
	OS_INIT_MACRO(os_var_init,"Cond/mutex system error\n"); /* cond, mutex */
	OS_INIT_MACRO(os_qbuff_init,"Queue system error\n");    /* qbuffer */
	OS_INIT_MACRO(os_msgq_init,"Message system error\n");   /* message queue */
	OS_INIT_MACRO(buff_init,"Buffer system error\n");   /* buffer handling */
	OS_INIT_MACRO(os_thread_init,"Thread system error\n");  /* thread */
	OS_INIT_MACRO(debug_init,"Debugging system error\n");   /* logging */
	OS_INIT_MACRO(net_init,"Network system error\n");   /* network */
}

/* external variables to contain system-wide parameters */
extern unsigned int os_system_pool_size;   /* declared in mem.c */
extern unsigned int os_debug_pool_size;    /* declared in dbg.c */
extern unsigned int os_network_pool_size;  /* declared in net.c */

/*******************************************************************************
 *
 * os_reinit_sysparam - Chainging system parameters
 *
 * DESCRIPTION
 *  Chainging default values in system parameters.
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
void os_reinit_sysparam(OS_SYSPARAM *sysp)
{
#define OS_REINIT_SETUP(x,y,z)  \
	if((sysp->x)>(y)) {   \
		z = sysp->x; \
	}

	if(sysp) {
		/* system pool */
		OS_REINIT_SETUP(system_pool_size,OS_SYSMEM_SIZE,os_system_pool_size)
		/* debugger pool */
		OS_REINIT_SETUP(debug_pool_size,DEBUG_MEM_CLASS_SIZE,os_debug_pool_size)
		/* network memory */
		OS_REINIT_SETUP(network_pool_size,NET_MEM_CLASS_SIZE,os_network_pool_size)
	}
}

/*******************************************************************************
 *
 * os_set_debug_level - Chainging debug printf flags
 *
 * DESCRIPTION
 *  Chainging dbg_printf_flag variable..
 *
 * PARAMETERS
 *  mode - 1 : Just Error
 *  mode - 2 : Verbose mode
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
void os_set_debug_level(unsigned int mode)
{
	if( mode == 1 ) {
		dbg_printf_flag = OS_DBG_LVL_ERR;
	}else{
		dbg_printf_flag = OS_DBG_LVL_ERR | OS_DBG_LVL_VER;
	}
}

/*******************************************************************************
 *
 * os_verbose - verbose message for all system info
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
void os_verbose(void)
{
	printf("************** MEM VERBOSE ***************** \n");
	os_mem_verbose();
	printf("************** THREAD VERBOSE*************** \n");
	os_thread_verbose();
	printf("************** MUTEX VERBOSE**************** \n");
	os_mutex_verbose();
	printf("************** COND VERBOSE **************** \n");
	os_cond_verbose();
	printf("******************************************** \n");
}
