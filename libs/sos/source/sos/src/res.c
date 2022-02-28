/*******************************************************************************
 *
 * res.c - OS Porting layer
 *
 ******************************************************************************/
#include "oskit.h"

/*
 * System resources to be managed
 */
static struct _os_res_pool {
	char name[STR_LEN];
	unsigned int base;
	unsigned int counter;
	unsigned char usevec[OS_RES_BIT_SIZE];
	unsigned int used;
	os_mutex_t lock;
} os_res_pool[OS_RES_NUM] = {
	/* used block */
	{THREAD_NAME,       0x10000000, 0, "", 1, },
	{MUTEX_NAME,        0X20000000, 0, "", 1, },
	{COND_NAME,         0X30000000, 0, "", 1, },
	{BUFFER_NAME,       0X40000000, 0, "", 1, },
	{QUEUE_NAME,        0X50000000, 0, "", 1, },
	{MSGQ_NAME,         0x60000000, 0, "", 1, },
	{BUFFER_CLASS_NAME,     0x70000000, 0, "", 1, },
	{NET_PORT_CLASS_NAME,   0x80000000, 0, "", 1, },
	{NET_CHAN_CLASS_NAME,   0x90000000, 0, "", 1, },
	{NET_CHAN_NAME,     0xa0000000, 0, "", 1, },
	{NET_SEL_NAME,      0xb0000000, 0, "", 1, },
	{ORDER_LIST_NAME,   0xc0000000, 0, "", 1, },
	{WIN32_THREAD_NAME, 0xd0000000, 0, "", 1, },
	/* unused block */
	{NO_RES_NAME,       0xe0000000, 0, "", 0, },
	{NO_RES_NAME,       0xf0000000, 0, "", 0, },
	{NO_RES_NAME,       0x11000000, 0, "", 0, },
	{NO_RES_NAME,       0x12000000, 0, "", 0, },
	{NO_RES_NAME,       0x14000000, 0, "", 0, },
	{NO_RES_NAME,       0x15000000, 0, "", 0, },
	{NO_RES_NAME,       0x16000000, 0, "", 0, }
};

/*******************************************************************************
 *
 * os_res_init - Initializing resource management
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID / ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_res_init(void)
{
	int ii;

	/* initializing use vector */
	for(ii=0; ii<OS_RES_NUM; ii++) {
		memset( os_res_pool[ii].usevec, 0, OS_RES_BIT_SIZE );
		_os_mutex_init( &(os_res_pool[ii].lock) );
	}

	return OKAY;
}

/*******************************************************************************
 *
 * os_res_newid_gen - Generating resource ID
 *
 * DESCRIPTION
 */
static unsigned int os_res_newid_gen(unsigned int vector)
{
	int indx;
	int nth;
	int bits;
	unsigned char val;
	struct _os_res_pool *res_pool;

	/* parameter check up */
	if(!(vector>=0 && vector<OS_MAX_RES)) {
		return ERROR;
	}

	/* res_pool */
	res_pool = (struct _os_res_pool *)&os_res_pool[vector];

	_os_mutex_lock( &(res_pool->lock) );

	for(indx=0; indx<OS_MAX_RES; indx++) {
		nth  = indx / sizeof(unsigned char);
		bits = indx % sizeof(unsigned char);
		val = (0x80)>>bits;
		if((res_pool->usevec[nth] & val)==0) {
			res_pool->usevec[nth] |= val;
			_os_mutex_unlock( &(res_pool->lock) );
			return indx;
		}
	}
	_os_mutex_unlock( &(res_pool->lock) );

	return OS_MAX_RES;
}

/*******************************************************************************
 *
 * os_res_newid_ret - Returning resource ID
 *
 * DESCRIPTION
 */
static unsigned int os_res_newid_ret(unsigned int vector,unsigned int indx)
{
	int nth;
	int bits;
	unsigned char val;
	struct _os_res_pool *res_pool;

	/* parameter check up */
	if(!(vector>=0 && vector<OS_MAX_RES)) {
		return ERROR;
	}

	/* res_pool */
	res_pool = (struct _os_res_pool *)&os_res_pool[vector];

	_os_mutex_lock( &(res_pool->lock) );

	nth  = indx / sizeof(unsigned char);
	bits = indx % sizeof(unsigned char);
	val = (0x80)>>bits;
	if((res_pool->usevec[nth] & val)==0) {
		/* error */
		printf("mismatched (not allocated block)... %d\n",indx);
		_os_mutex_unlock( &(res_pool->lock) );
		return ERROR;
	}

	/* clean up bit */
	os_res_pool[vector].usevec[nth] &= ~val;

	_os_mutex_unlock( &(res_pool->lock) );

	return OKAY;
}

/*******************************************************************************
 *
 * os_res_register - Register resource information
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID / ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_res_register(unsigned char *name)
{
	int ii;

	/* existing block check */
	for(ii=0; ii<OS_RES_NUM; ii++) {
		if(strncmp((char *)os_res_pool[ii].name,
		           (const char *)name,7)==0) {
			return ERROR;
		}
	}

	/* searching for unoccupied block */
	for(ii=0; ii<OS_RES_NUM; ii++) {
		if(!os_res_pool[ii].used) {
			strncpy( (char *)os_res_pool[ii].name, (const char *)name, STR_LEN );
			os_res_pool[ii].used = 1; /* used */
			return OKAY;
		}
	}

	return ERROR;
}

/*******************************************************************************
 *
 * os_res_id_alloc - Allocating an object ID
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID / ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_res_id_alloc(OS_LIST *h)
{
	int ii;
	int newval;

	for(ii=0; ii<OS_RES_NUM; ii++) {
		if(strncmp((char *)os_res_pool[ii].name,(const char *)h->name,7)==0) {
			/*
			 * Please refer to inc/const.h about '7'.
			 */
			/* generate resource ID */
			newval = os_res_newid_gen(ii);
			if(newval == OS_MAX_RES) {
				/* resource run out of */
				os_debug_printf(OS_DBG_LVL_ERR,"failed(%s)::newval == OS_MAX_RES\n",h->name);
				return ERROR;
			}
			return (os_res_pool[ii].base + newval);
		}
	}
	return ERROR;
}

/*******************************************************************************
 *
 * os_res_id_free - Freeing an object ID
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID / ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_res_id_free(OS_LIST *h,unsigned int id)
{
	int ii;
	int newval;

	for(ii=0; ii<OS_RES_NUM; ii++) {
		if(strncmp((char *)os_res_pool[ii].name,(const char *)h->name,7)==0) {
			/*
			 * Please refer to inc/const.h about '7'.
			 */
			/* generate resource ID */
			newval = os_res_newid_ret(ii,(id-os_res_pool[ii].base));
			if(newval == ERROR) {
				/* resource run out of */
				os_debug_printf(OS_DBG_LVL_ERR,"failed(%s)::newval == ERROR\n",h->name);
				return ERROR;
			}
			return OKAY;
		}
	}
	return ERROR;
}

/*******************************************************************************
 *
 * os_res_id_make_inuse - Applying a new ID
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID / ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_res_id_make_inuse(OS_LIST *h,unsigned int newid)
{
	return OKAY;
}

/*******************************************************************************
 *
 * os_res_make_name - Making a resource name
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
 */
void os_res_make_name(unsigned char *buff,unsigned char *name,unsigned int id)
{
	sprintf((char *)buff,(const char *)"%s%08x",name,id);
}

/*******************************************************************************
 *
 * os_res_info - Resource information
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  os_res_info_t
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
void os_res_info(os_res_info_t *t)
{
	unsigned int index;

	index = 0;
	t->thread.count   = os_res_pool[index].counter;

	++index;
	t->mutex.count    = os_res_pool[index].counter;

	++index;
	t->cond.count     = os_res_pool[index].counter;

	++index;
	t->buffer.count   = os_res_pool[index].counter;

	++index;
	t->queue.count    = os_res_pool[index].counter;

	++index;
	t->msgq.count     = os_res_pool[index].counter;

	++index;
	t->bclass.count   = os_res_pool[index].counter;

	++index;
	t->nport.count    = os_res_pool[index].counter;

	++index;
	t->nchannel.count = os_res_pool[index].counter;
}

