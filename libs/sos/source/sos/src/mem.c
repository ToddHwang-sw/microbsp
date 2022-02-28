/*******************************************************************************
 *
 * mem.c - Basic Porting layer
 *
 ******************************************************************************/
#include "oskit.h"

/* default value is OS_SYSMEM_SIZE */
/* The following is a variable accessible from an outside */
/*static*/ unsigned int os_system_pool_size = OS_SYSMEM_SIZE;

static os_mutex_t os_mem_lock;
static unsigned char *os_sys_space;
static regionid_t os_sys_region;
static int os_mem_initialized = 0;

/*******************************************************************************
 *
 * os_mem_init - Initializing system memory area
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
unsigned int os_mem_init(void)
{
	os_sys_space = (unsigned char *)_os_malloc( os_system_pool_size );
	if(!os_sys_space) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_malloc(%08x)::error\n",os_system_pool_size);
		return ERROR;
	}

	if(os_region_init()==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_region_init()::error\n");
		_os_free(os_sys_space);
		return ERROR;
	}

	os_sys_region = os_region_create((char *)os_sys_space,os_system_pool_size);
	if(os_sys_region == NULL) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_region_init()::error\n");
		_os_free(os_sys_space);
		return ERROR;
	}

	os_mem_initialized = 1;

	_os_mutex_init(&os_mem_lock);

	os_debug_printf(OS_DBG_LVL_VER,"(pool=%08x)done\n",os_system_pool_size);

	return OKAY;
}

/*******************************************************************************
 *
 * os_mem_delete - Deleting a system memory area
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
void os_mem_delete(void)
{
	os_mem_initialized = 0;
	_os_free(os_sys_space);
}

/*******************************************************************************
 *
 * os_mem_alloc - Allocating a block from a region
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
void * os_mem_alloc(unsigned int size)
{
	void *p;

	if(!os_mem_initialized)
	       return NULL;

	_os_mutex_lock(&os_mem_lock);

	p = (void *)os_region_alloc(os_sys_region,size);
	if(p) 
		memset(p,0,size);

	_os_mutex_unlock(&os_mem_lock);

	return p;
}

/*******************************************************************************
 *
 * os_mem_realloc - Reallocating a block from a region
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
void * os_mem_realloc(void *mem,unsigned int nsize)
{
	void *p;

	if(!os_mem_initialized)
		return NULL;

	_os_mutex_lock(&os_mem_lock);

	p = (void *)os_region_realloc(os_sys_region,mem,nsize);

	if(p)
		memset(p,0,nsize);

	_os_mutex_unlock(&os_mem_lock);

	return p;
}

/*******************************************************************************
 *
 * os_mem_calloc - Allocating a set of blocks
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
void * os_mem_calloc(unsigned int esize,unsigned int ecount)
{
	void *p;

	if(!os_mem_initialized)
		return NULL;

	_os_mutex_lock(&os_mem_lock);

	p = (void *)os_region_calloc(os_sys_region,esize,ecount);

	if(p) 
		memset(p,0,esize*ecount);

	_os_mutex_unlock(&os_mem_lock);

	return p;
}

/*******************************************************************************
 *
 * os_mem_strdup - String duplication
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
char * os_mem_strdup(const char *s)
{
	char *p;

	if(!os_mem_initialized)
		return NULL;

	_os_mutex_lock(&os_mem_lock);

	p = (void *)os_region_alloc(os_sys_region,strlen(s)+1);

	if(p) {
		memset(p,0,strlen(s)+1);
	       	memcpy(p,s,strlen(s));
	}

	_os_mutex_unlock(&os_mem_lock);

	return p;
}

/*******************************************************************************
 *
 * os_mem_free - Freeing a block from a region
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
unsigned int os_mem_free(void *mem)
{
	if(!os_mem_initialized)
       		return ERROR;

	_os_mutex_lock(&os_mem_lock);

	os_region_free(os_sys_region,mem);

	_os_mutex_unlock(&os_mem_lock);

	return OKAY;
}

/*******************************************************************************
 *
 * os_mem_resinfo - Obtaining resource information
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
void os_mem_resinfo(region_info_t *t)
{
	_os_mutex_lock(&os_mem_lock);

	os_region_getinfo(os_sys_region,t);

	_os_mutex_unlock(&os_mem_lock);
}

/*******************************************************************************
 *
 * os_mem_verbose - Printing out region information
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID
 *
 * ERRORS
 *  N/A
 */
void os_mem_verbose(void)
{
	static region_info_t os_sys_region_info;

	os_region_getinfo(os_sys_region,&os_sys_region_info);

	debug_print("sblks    = %08x \n",os_sys_region_info.smblks   );
	debug_print("ordblks  = %08x \n",os_sys_region_info.ordblks  );
	debug_print("fordblks = %08x \n",os_sys_region_info.fordblks );
	debug_print("uordblks = %08x \n",os_sys_region_info.uordblks );
	debug_print("arena    = %08x \n",os_sys_region_info.arena    );
	debug_print("fsmblks  = %08x \n",os_sys_region_info.fsmblks  );
	debug_print("keepcost = %08x \n",os_sys_region_info.keepcost );
	debug_print("usmblks  = %08x \n",os_sys_region_info.usmblks  );
}
