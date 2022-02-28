/*******************************************************************************
 *
 * buff.c - buffer management module
 *
 *
 ******************************************************************************/
#include "oskit.h"

static OS_LIST *buffer_class_list;

/*******************************************************************************
 *
 * buff_init - initialize buffer module
 *
 * DESCRIPTION
 *  The buff_init() function initialize buffer management module.
 *
 * PARAMETERS
 *  None.
 *
 * RETURNS
 *  If successful, it returns OK(0). Otherwise it returns ERROR(-1)
 *
 * ERRORS
 *
 * .
 */
unsigned int buff_init(void)
{
	buffer_class_list = os_list_create(BUFFER_CLASS_NAME);
	if(!buffer_class_list) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_create(%s)::error\n",BUFFER_CLASS_NAME);
		return ERROR;
	}
	return OKAY;
}

/* Align predicates */
#define MEM_ALIGN(s,ab) (((s)&~(ab-1))+(ab))

/*******************************************************************************
 *
 * buff_create_class - class create
 *
 * DESCRIPTION
 *  The buff_create_class() function create class for a buffer.
 *
 * PARAMETERS
 *  id - The id is used to identify among many of classes
 *      in the outstanding buffer classes.
 *  classsize - define for the total size of the class.
 *
 * RETURNS
 *  If successful, it returns OK(0). Otherwise it returns ERROR(-1)
 *
 * ERRORS
 *  OKAY/ERROR
 *
 */
unsigned int buff_create_class(unsigned int id, unsigned char *baseaddr, unsigned int size)
{
	unsigned int res;
	unsigned int init_size;
	unsigned int real_size;
	buff_class_node * cls;
	unsigned char tname[STR_LEN];

	/* class structure */
	cls = (buff_class_node *)os_mem_alloc(sizeof(buff_class_node));
	if(!cls) {
		os_debug_printf(OS_DBG_LVL_ERR,"buff_create_class(%08x)::error\n",id);
		return ERROR;
	}

	/* mutex lock name */
	os_res_make_name((unsigned char *)tname,(unsigned char *)MUTEX_NAME,id);

	/* mutex lock creation */
	cls->lock = os_mutex_create((const char *)tname);
	if(cls->lock == ERROR) {
		os_mem_free(cls);
		os_debug_printf(OS_DBG_LVL_ERR,"(%d)os_mutex_create(%s)::error\n",id,tname);
		return ERROR;
	}

	/* history mutex */
	os_res_make_name((unsigned char *)tname,(unsigned char *)MUTEX_NAME,(id+1));
	cls->h_lock = os_mutex_create((const char *)tname);
	if(!cls->h_lock) {
		os_mem_free(cls);
		os_debug_printf(OS_DBG_LVL_ERR,"mutex_create(%s)::error\n",tname);
		return ERROR;
	}

	/* buffer itself */
	/* If baseaddr is specified, it is used as a base address of an memory pool
	   , otherwise, it is needed to call os_malloc() */
	/* os_malloc() makes buffer be separated from the basic component memory area */
	if(baseaddr) {
		if(size<(2*4096)) {
			/* smallest size */
			os_mutex_delete(cls->lock);
			os_mem_free(cls);
			os_debug_printf(OS_DBG_LVL_ERR,"(%d)too small size(%s)::error\n",id,tname);
			return ERROR;
		}
		init_size = size;
		cls->system_base = baseaddr; /* malloced space */
		cls->fixedbase = 1;
	}else{
		if(size<4096) {
			/* smallest size */
			os_mutex_delete(cls->lock);
			os_mem_free(cls);
			os_debug_printf(OS_DBG_LVL_ERR,"(%d)too small size(%s)::error\n",id,tname);
			return ERROR;
		}
		init_size = size+4096; /* 4 Kbytes more for safety */
		cls->system_base = (unsigned char *)_os_malloc(init_size); /* newly created space */
		if(!cls->system_base) {
			/* smallest size */
			os_mutex_delete(cls->lock);
			os_mem_free(cls);
			os_debug_printf(OS_DBG_LVL_ERR,"(%d)malloc failure(%s,size=%08x,original=%08x)::error\n",
			                id,tname,init_size,size);
			return ERROR;
		}
		cls->fixedbase = 0;
	}
	memset(cls->system_base,0,init_size); /* It will contaminate system memory... */
	cls->base = (unsigned char *)MEM_ALIGN((unsigned long)cls->system_base,0x20);
	/* 32 bytes alignment */
	if(!cls->base) {
		if(!cls->fixedbase) _os_free(cls->system_base);
		os_mutex_delete(cls->lock);
		os_mem_free(cls);
		os_debug_printf(OS_DBG_LVL_ERR,"(%d)os_malloc(%08x)::error\n",id,real_size);
		return ERROR;
	}

	/* address order comparison */
	if((unsigned long)cls->base<(unsigned long)cls->system_base) {
		/* decreasing order memory management */
		if(!cls->fixedbase) _os_free(cls->system_base);
		os_mutex_delete(cls->lock);
		os_mem_free(cls);
		os_debug_printf(OS_DBG_LVL_ERR,"(%d)(%s)SYSTEM MEMORY IS DECREASING ORDER::error\n",
		                id,tname);
		return ERROR;
	}
	real_size = (unsigned int)(init_size-(
			   (unsigned long)cls->base-(unsigned long)cls->system_base
			   ));

	/* region create */
	cls->data = os_region_create((char *)cls->base,real_size);
	if(!cls->data) {
		if(!cls->fixedbase) _os_free(cls->system_base);
		os_mutex_delete(cls->lock);
		os_mem_free(cls);
		os_debug_printf(OS_DBG_LVL_ERR,"(%d)os_malloc(%08x)::error\n",id,real_size);
		return ERROR;
	}

	/* initial */
	cls->id        = id;
	cls->totsize   = real_size;
	cls->availsize = real_size;
	cls->numblocks = 0;
	cls->osize     = size;/* original size */

	/* buffer name */
	os_res_make_name((unsigned char *)tname,(unsigned char *)BUFFER_NAME,id);
	strncpy((char *)cls->name,(const char *)tname,STR_LEN);

	/* inserting into class pool */
	res = os_list_item_insert(buffer_class_list,(void *)cls);
	if(res == ERROR) {
		if(cls->system_base) _os_free(cls->system_base);
		os_mutex_delete(cls->lock);
		os_mem_free(cls);
		os_debug_printf(OS_DBG_LVL_ERR,"(%d)os_list_item_insert::error\n",id);
		return ERROR;
	}

	cls->objectid  = res;/* object ID */
	cls->h_start = 0; /* no debugging by default */
	cls->h  = NULL; /* null list */

	/* 'res' value is no longer used. Indexing is done with
	 * user-defined ID for user-friendly interface.
	 */

	buff_history_class(id,1); /* debugging */

	// buff_verbose(-1);  /* verbose */

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::size=%08x,base=%p,sysbase=%p::done\n",
	                id,cls->totsize,cls->base,cls->system_base);


	return OKAY;
}

/*******************************************************************************
 *
 * buff_class_comparator - Comparator function for buff_class_find
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
static unsigned int buff_class_comparator(OS_LIST_DATA *d, void *arg)
{
	buff_class_node *cls;

	if(!arg) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid parameter\n");
		return ERROR; /* invalid parameter */
	}

	if(d->data) {
		cls = (buff_class_node *)d->data;
	}else{
		os_debug_printf(OS_DBG_LVL_ERR,"inconsistent error\n");
		return ERROR;
	}
	return (cls->id == *(unsigned int *)arg) ? OKAY : ERROR;
}

/*******************************************************************************
 *
 * buff_class_find - Finding a specific buffer class
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  buff_class_node object
 *
 * ERRORS
 *   N/A
 *
 */
static buff_class_node * buff_class_find(unsigned int id)
{
	OS_LIST_DATA *d;
	buff_class_node *cls;

	d = os_list_find(buffer_class_list, buff_class_comparator, (void *)&id );
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"failed\n");
		return NULL;
	}
	cls = (buff_class_node *)(d->data);

	return cls; /* buffer class */
}

/*******************************************************************************
 *
 * buff_delete_class - delete class
 *
 * DESCRIPTION
 *  The buff_delete_class() function deletes class.
 *
 * PARAMETERS
 *  id - The id that was used to cleate classes
 *
 *
 * RETURNS
 *  If successful, it returns OK(0). Otherwise it returns ERROR(-1)
 *
 * ERRORS
 *
 * .
 */
unsigned int buff_delete_class(unsigned int id)
{
	unsigned int res;
	unsigned int counter;
	buff_class_node *cls;
	buff_history_st *p,*trace;

	cls = buff_class_find(id);
	if(!cls) { /* class check */
		os_debug_printf(OS_DBG_LVL_ERR,"buff_class_find(%08x)::error\n",id);
		return ERROR;
	}

	/* history stopping */
	if(cls->h_start)
		buff_history_class(id,0); /* stop */

	/* history deleting... */
	os_mutex_lock(cls->h_lock,0);
	trace = cls->h;
	counter = 0;
	while(trace) { p = trace;
		           trace = trace->next;
		           _os_free(p);
		           ++counter;} /* deleting history class */
	os_mutex_unlock(cls->h_lock);
	os_mutex_delete(cls->h_lock); /* deleting mutex for historying */
	if(counter) {
		os_debug_printf(OS_DBG_LVL_ERR,"Still %d buffer blocks are not yet deleted\n",counter);
	}

	os_mutex_delete(cls->lock); /* mutex */
	if(!cls->fixedbase) _os_free(cls->system_base); /* buffer */

	res = os_list_item_delete(buffer_class_list,cls->objectid);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_delete(%08x,res=%08x)\n",id,cls->objectid);
		return ERROR;
	}

	os_mem_free(cls);

	// buff_verbose(-1);  /* verbose */

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)done\n",id);

	return OKAY;
}

/* MAGIC NUM + id + size + history buffer pointer */
#define BUFF_OFFSET_SIZE  (sizeof(unsigned long)+sizeof(unsigned int)+sizeof(unsigned int)+sizeof(unsigned long))
/*#define HALF_BUFF_SIZE(z) ((z)>>1) */
#define HALF_BUFF_SIZE(z) ((z))

/* size + (BUFF_OFFSET_SIZE) + 32 bytes surplus and 8 bytes aligning */
#define CALC_ACTUAL_BUFF_SIZE(sz)   MEM_ALIGN((sz)+BUFF_OFFSET_SIZE+32,8)

/*******************************************************************************
 *
 * buff_alloc_data - buffer allocation
 *
 * DESCRIPTION
 *  The buff_alloc_data() function is for buffer allocation.
 *
 * PARAMETERS
 *  id - The id that was used to cleate classes
 *  size - the memory size to allocate
 *
 * RETURNS
 *  If successful, it returns buffer pointer. Otherwise it returns NULL.
 *
 * ERRORS
 *
 *
 */

unsigned char * buff_alloc_data(unsigned int id,unsigned int size)
{
	unsigned char* p_buf = NULL; /* used in common */
	buff_class_node *cls;
	unsigned long magic_num;
	buff_history_st *h_temp;
	buff_history_st *pivot;
	unsigned int real_len;

	cls = buff_class_find(id);
	if(!cls) { /* class check */
		os_debug_printf(OS_DBG_LVL_ERR,"buff_class_find(%08x)::error\n",id);
		return NULL;
	}

	if(size<=0) { /* size check */
		os_debug_printf(OS_DBG_LVL_ERR,"size=%d::error\n",size);
		return NULL;
	}

	os_mutex_lock(cls->lock,0); /* lock */

	/* greater than originally defined size */
	if(size>cls->osize) {
		os_debug_printf(OS_DBG_LVL_ERR,"size=%d/%d is too big::error\n",size, cls->osize);
		os_mutex_unlock(cls->lock); /* unlock */
		return NULL;
	}

	/* 8 bytes aligned size */
	/* 64 bytes created more.... */
	real_len = (unsigned int)CALC_ACTUAL_BUFF_SIZE(size); /* 8 bytes aligned */

	/* comparison to total memory size */
	if(real_len >= HALF_BUFF_SIZE(cls->totsize)) {
		/*
		 * Memory management is basically implemented under the rule of
		 * binary exponential tree architecture. The tree traversal is an actual
		 * heap with only two child in any level of the tree. Henceforth,
		 * the largest block to be allocated in the pool is a half of the
		 * pool size.
		 */
		os_debug_printf(OS_DBG_LVL_ERR,
		                "(%08x)Memory to be requested is too big(size=%d/%d/%d)\n",
		                id,real_len,cls->availsize,cls->totsize);
		os_mutex_unlock(cls->lock); /* unlock */
		return NULL;
	}

	/* comparison to available memory size */
	if(real_len >= cls->availsize) {
		os_debug_printf(OS_DBG_LVL_ERR,
		                "(%08x)Not enough memory(size=%d/%d)\n",
		                id,real_len,cls->availsize);
		os_mutex_unlock(cls->lock); /* unlock */
		return NULL;
	}

	/* region allocating */
	p_buf = (unsigned char *)os_region_alloc(cls->data,real_len);

	if(!p_buf) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_region_alloc(%p,%d/%d)::error\n",
		                cls->data,real_len,cls->availsize);
		os_mutex_unlock(cls->lock); /* unlock */
		return NULL;
	}

	/* magic number setting to make it sure not to be overwritten */
	*(unsigned long *)p_buf = BOUNDARY_MAGIC_NUM;                             /* beginning marker */
	/*
	 * -
	 * buff_get_info() may return the length of buffer, and in case that a user
	 * tries to clear up the content of the buffer based on the size returned from this
	 * API, it causes memory fault or memory crash if "real_len" would be recorded as
	 * a length of the buffer......
	 */
	*(unsigned int  *)(p_buf+sizeof(unsigned long)) = (unsigned int)size;     /* requested size */
	*(unsigned int  *)(p_buf+sizeof(unsigned long)
	                   +sizeof(unsigned int))    = (unsigned int)id;/* id */
	*(unsigned int *)(p_buf+real_len-sizeof(unsigned long)) = BOUNDARY_MAGIC_NUM; /* ending marker */

	//os_debug_printf(OS_DBG_LVL_VER,"magic num setting::done\n");

	/* statistics information */
	cls->availsize -= real_len;
	cls->numblocks += 1;

	/* history start... */
	if(cls->h_start) {
		/* new block */
		h_temp = (buff_history_st *)_os_malloc(sizeof(buff_history_st));
		if(!h_temp) {
			os_debug_printf(OS_DBG_LVL_ERR,"os_malloc(%08x,history buffer)::error\n",id);
		}else{
			/* buff_history_st node */
			memset(h_temp->oname,0,STR_LEN);
		#if 0
			/* thread name ??? */
			strncpy( (char *)h_temp->oname,
			         (const char *)os_thread_name(os_thread_self()),
			         STR_LEN); /* owner thread name */
		#endif
			h_temp->size  = size;/* requested size */
			h_temp->data  = p_buf;/* buffer location */
			h_temp->next  = NULL;

			os_mutex_lock(cls->h_lock,0);
			if(cls->h) {
				pivot = cls->h;
				while(pivot->next) pivot = pivot->next; /* to the end */
				pivot->next = h_temp;
			}else{
				cls->h = h_temp;
			}

			/* history buffer pointer*/
			*(unsigned long *)(p_buf
			                   +sizeof(unsigned long)
			                   +sizeof(unsigned int)
			                   +sizeof(unsigned int))  = (unsigned long)h_temp;

			os_debug_printf(OS_DBG_LVL_VER,"history_add(history buffer=%08lx)::done\n",
			                (unsigned long)h_temp);

			os_mutex_unlock(cls->h_lock);
			goto EXIT_BUFF_ALLOC;
		}
	}

	/* nullifying history buffer pointer */
	*(unsigned long *)(p_buf
	                   +sizeof(unsigned long)
	                   +sizeof(unsigned int)
	                   +sizeof(unsigned int))  = (unsigned long)0;/* NULL */

EXIT_BUFF_ALLOC:

	os_mutex_unlock(cls->lock); /* unlock */

	// buff_verbose(-1);  /* verbose */

	os_debug_printf(OS_DBG_LVL_VER,"buff_alloc_data(%08x)(pos=%p)(size=%d/%d)::done\n",id,p_buf,size,cls->availsize);

	return p_buf+BUFF_OFFSET_SIZE;
}

/*******************************************************************************
 *
 * buff_free_data - buffer free
 *
 * DESCRIPTION
 *  Freeing up a block and returns it to the system memory. In the
 *  advance of the physical memory arrangement, any bit of sanity check may be
 *  performed to verify safe operation.
 *
 *
 * PARAMETERS
 *  id - The id that was used to cleate classes
 *  buff - buffer pointer that will be free.
 *
 * RETURNS
 *  If successful, it returns OK(0). Otherwise it returns ERROR(-1)
 *
 * ERRORS
 *
 * .
 */
unsigned int buff_free_data(unsigned int id, unsigned char* buff)
{
	unsigned char * p_buf;
	unsigned int size;
	unsigned int real_len;
	unsigned int buffid;
	unsigned long magic_num;
	unsigned long hloc;
	unsigned int block_found;
	buff_class_node *cls;
	buff_history_st *hst;
	buff_history_st *trace, *prev;

	/* buff null check */
	if(!buff) {
		os_debug_printf(OS_DBG_LVL_ERR,"null data block\n");
		return ERROR;
	}

	/* real address */
	p_buf = buff - BUFF_OFFSET_SIZE;

	cls = buff_class_find(id);
	if(!cls) { /* class check */
		os_debug_printf(OS_DBG_LVL_ERR,"buff_class_find(%08x)::error\n",id);
		return ERROR;
	}

	//os_debug_printf(OS_DBG_LVL_VER,"buff_class_find(%08x)::done\n",id);
	os_mutex_lock(cls->lock,0); /* lock */

	/* Checking current block is still available */
	if(cls->h_start) {
		os_mutex_lock(cls->h_lock,0);
		hst = cls->h; /* root node */
		block_found = 0; /* flag */
		while(hst) {
			if((unsigned long)hst->data == (unsigned long)p_buf) {
				/* same block */
				block_found = 1;
				break;
			}
			hst = hst->next; /* next node */
		}
		if(!block_found) {
			os_debug_printf(OS_DBG_LVL_ERR,"block(%08lx) is not found - Error\n",
			                (unsigned long)buff);
			os_mutex_unlock(cls->h_lock);
			os_mutex_unlock(cls->lock);
			return ERROR;
		}
		os_mutex_unlock(cls->h_lock);
	}

	/* magic number setting to make it sure not to be overwritten */
	magic_num = *(unsigned long *)p_buf; /* beginning marker */
	size      = *(unsigned int *)(p_buf+sizeof(unsigned long));/* size */
	buffid    = *(unsigned int *)(p_buf+sizeof(unsigned long)+sizeof(unsigned int));/* buffid */
	hloc      = *(unsigned long *)(p_buf+sizeof(unsigned long)+sizeof(unsigned int)+sizeof(unsigned int));/* history buff loc */

	//os_debug_printf(OS_DBG_LVL_VER,"magic number setting (%08x)::done\n",p_buf);

	if( magic_num != BOUNDARY_MAGIC_NUM ) { /* beginning marker */
		/* overwritten */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x %ld %d)overwritten - beginning magic crash \n",id, magic_num,BOUNDARY_MAGIC_NUM);
	}

	/* size check */
	if( size <= 0 || size >= HALF_BUFF_SIZE(cls->totsize) ) {
		/* overwritten */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)overwritten - size crash \n",id);
	}

	/* buffer id check */
	if( id != buffid ) {
		/* none of buffer id */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)buffid(%08x) is not matched \n",id,buffid);
	}

	/* history buffer check */
	if(hloc) {
		os_mutex_lock(cls->h_lock,0);
		hst = (buff_history_st *)hloc;
		trace = cls->h;
		if(trace == hst) {
			cls->h = trace->next; /* move pointer */
		}else{
			while(trace!=hst && trace) {
				prev = trace;
				trace = trace->next;
			}
			/* now deleting... */
			if(!trace) {
				os_debug_printf(OS_DBG_LVL_ERR,"history link chain is deleted(%08x)\n",buffid);
				os_debug_printf(OS_DBG_LVL_ERR,"Memory leak might happen(%08x)\n",buffid);
			}else{
				prev->next = trace->next;
			}
		}
		/* validity test */
		if( trace->size != size ||
		    ((unsigned long)trace->data != (unsigned long)p_buf) ||
		    (!block_found) ) {
			os_debug_printf(OS_DBG_LVL_ERR,"(%p) might be crashed \n",buff);
		}
		_os_free(trace); /* deleting a node from a chain */
		os_debug_printf(OS_DBG_LVL_VER,"(%p) block successfully deleted \n",trace);
		os_mutex_unlock(cls->h_lock);
	}/* if(hloc */

	/* obtaining real buffer size */
	real_len = (unsigned int)CALC_ACTUAL_BUFF_SIZE(size);

	//os_debug_printf(OS_DBG_LVL_VER,"buffer id check (%08x)::done\n",p_buf);

	/* changed this to avoid misalignment */
	magic_num = *(unsigned int *)(p_buf+real_len-sizeof(unsigned long)); /* ending marker */
	//memcpy(&magic_num, p_buf+size+BUFF_OFFSET_SIZE, sizeof(unsigned long));
	if( magic_num != BOUNDARY_MAGIC_NUM ) {
		/* overwritten */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)overwritten - ending magic crash \n",id);
	}
	//os_debug_printf(OS_DBG_LVL_VER,"ending magic checking (%08x)::done\n",p_buf);

	/* free */
	os_region_free(cls->data,p_buf);

	os_debug_printf(OS_DBG_LVL_VER,"os_region_free (%p)::done\n",p_buf);

	/* statistics information */
	cls->availsize += real_len;
	cls->numblocks -= 1;

	os_mutex_unlock(cls->lock); /* unlock */

	// buff_verbose(-1);  /* verbose */

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)(pos=%p)(size=%d)::done\n",id,p_buf,size);

	return OKAY;
}

/*******************************************************************************
 *
 * buff_current_availsize - current size of buffer
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *
 * .
 */
unsigned int buff_current_availsize(unsigned int id)
{
	buff_class_node *cls;

	cls = buff_class_find(id);
	if(!cls) { /* class check */
		os_debug_printf(OS_DBG_LVL_ERR,"buff_class_find(%08x)::error\n",id);
		return ERROR;
	}

	return cls->availsize;
}

/*******************************************************************************
 *
 * buff_history_class - tracking buffer malloc/free history
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - The id is used to identify among many of classes
 *      in the outstanding buffer classes.
 *  flag - 1/0
 *
 * RETURNS
 *  If successful, it returns OK(0). Otherwise it returns ERROR(-1)
 *
 * ERRORS
 *
 * .
 */
unsigned int buff_history_class(unsigned int id, unsigned int flag)
{
	buff_class_node *cls;

	cls = buff_class_find(id);
	if(!cls) { /* class check */
		os_debug_printf(OS_DBG_LVL_ERR,"buff_class_find(%08x)::error\n",id);
		return ERROR;
	}

	switch(flag) {
	case 1: /* start */
		if(cls->h_start) {
			os_debug_printf(OS_DBG_LVL_ERR,"already started\n");
			return ERROR;
		}
		cls->h_start = 1;
		break;
	case 0: /* stop */
		if(!cls->h_start) {
			os_debug_printf(OS_DBG_LVL_ERR,"already stopped\n");
			return ERROR;
		}
		cls->h_start = 0;
		break;
	default:
		os_debug_printf(OS_DBG_LVL_ERR,"buff_history_class(%08x)::unknown flag\n",id);
		return ERROR;
	}

	return OKAY;
}

/*******************************************************************************
 *
 * buff_get_info - buffer information
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *
 * .
 */
unsigned int buff_get_info(unsigned int command, unsigned char* buff)
{
	unsigned int id,size;
	unsigned char* p_buf;

	if(!buff) {
		os_debug_printf(OS_DBG_LVL_ERR,"null buffer\n");
		return ERROR;
	}

	/* information block */
	p_buf = buff - BUFF_OFFSET_SIZE;

	switch( command ) {
	case BUFF_GET_ID:
		id   = *(unsigned int *)(p_buf+sizeof(unsigned long)+sizeof(unsigned int));  /* buffid */
		return id;
	case BUFF_GET_SIZE:
		size = *(unsigned int *)(p_buf+sizeof(unsigned long));  /* size */
		return size;
	default:
		os_debug_printf(OS_DBG_LVL_ERR,"unknown command=%08x\n",command);
		return ERROR;
	}

	return OKAY;
}

static void buff_printout_stats(buff_class_node *cls)
{
	buff_history_st *trace;
	unsigned char *start;
	unsigned long magicnum;
	unsigned int sz;
	unsigned int id;
	region_info_t info;

	if(!cls) {
		os_debug_printf(OS_DBG_LVL_ERR,"No class \n");
		return;
	}

	os_mutex_lock(cls->lock,0);
	printf("<BUFFER>::region(%p)::id(%08x)::total size(%08x)::empty size(%08x)::blocks(%d)\n",
	       cls->data,
	       cls->id,
	       cls->totsize,
	       cls->availsize,
	       cls->numblocks);

	/* history... */
	if(cls->h_start) {
		os_mutex_lock(cls->h_lock,0);
		trace = cls->h;
		while(trace) {
			printf(
				"     + history::owner(%s)::size(%08x)::loc(%p)\n",trace->oname, trace->size, trace->data);
			start = trace->data;
			if((magicnum=*(unsigned long *)start) != BOUNDARY_MAGIC_NUM)
				printf("     + history::boundary magic crashed(o=%08x,r=%08lx)\n",
				       BOUNDARY_MAGIC_NUM,magicnum);
			if((sz=*(unsigned int *)(start+sizeof(unsigned long))) != trace->size )
				printf("     + history::size crashed(o=%08x,r=%08x)\n",
				       trace->size,sz);
			if((id=*(unsigned int *)(start+sizeof(unsigned long)+sizeof(unsigned int))) != cls->id )
				printf("     + history::id crashed(o=%08x,r=%08x)\n",
				       cls->id,id);
			trace = trace->next;
		}
		os_mutex_unlock(cls->h_lock);
	}

	os_mutex_unlock(cls->lock);
}

static void buff_printout_stats2(OS_LIST_DATA *d)
{
	buff_printout_stats((buff_class_node *)(d->data));
}

/*******************************************************************************
 *
 *  buff_verbose -
 *
 * DESCRIPTION
 *	Displaying the debug message of the memory buffer class specified in
 *  a parameter. When the parameter is (-1), it displays the debugging informa-
 *  tion of all classes.
 *
 * PARAMETERS
 *  int id - class id. When the parameter is (-1), it displays the debugging informa-
 *  tion of all classes.
 *
 * RETURNS
 *  If successful, it returns OK(0). Otherwise it returns ERROR(-1)
 *
 * ERRORS
 *
 * .
 */
unsigned int buff_verbose(int id)
{
	buff_class_node *cls;

	if(id == (-1)) {
		os_list_navigate(buffer_class_list, NULL, buff_printout_stats2);
	}else{
		cls = buff_class_find(id);
		if(!cls) { /* class check */
			os_debug_printf(OS_DBG_LVL_ERR,"buff_class_find(%08x)::error\n",id);
			return ERROR;
		}
		buff_printout_stats(cls);
	}

	return OKAY;
}

