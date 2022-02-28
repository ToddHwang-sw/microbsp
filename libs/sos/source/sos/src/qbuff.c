/*******************************************************************************
 *
 * qbuff.c - FIFO queueing buffer manipulation
 *
 ******************************************************************************/
#include "oskit.h"

/* QUEUE list */
static OS_LIST *queue_list;

/*******************************************************************************
 *
 * os_qbuff_init - initializing a queue system
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
unsigned int os_qbuff_init(void)
{
	queue_list = os_list_create(QUEUE_NAME);
	if(!queue_list) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_create(%s)::error\n",QUEUE_NAME);
		queue_list = NULL;
		return ERROR;
	}
	return OKAY;
}

/*******************************************************************************
 *
 * os_qbuff_create - making a queue
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
unsigned int os_qbuff_create(const char *name,unsigned int pool,os_delfunc_t delf)
{
	OS_QHEAD *newq;
	unsigned int res;

	newq = (OS_QHEAD *)os_mem_alloc(sizeof(OS_QHEAD));
	if(!newq) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%s)os_mem_alloc(sizeof(OS_QHEAD))::error\n",name);
		return ERROR;
	}

	/* initializing */
	newq->list         = NULL;
	newq->next         = NULL;
	newq->current_size = 0;
	newq->pool         = pool;/* 03/17 - pool */
	newq->delfunc      = delf;/* 03/17 - delete function */
	newq->lock         = os_mutex_create((const char *)name);
	if(newq->lock == ERROR) {
		os_mem_free(newq);
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_create(%s)\n",name);
		return ERROR;
	}

	res=os_list_item_insert(queue_list,(void *)newq);
	if(res == ERROR) {
		os_mem_free(newq);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(%s)\n",name);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(%s)::done\n",name);

	return res;
}

/*
 *  memory allocation/deallocation functions
 */
/*******************************************************************************
 *
 * memory allocation
 */
static void * os_qnode_mem_alloc(OS_QHEAD *qh,unsigned int sz)
{
	if( !qh ) {
		os_debug_printf(OS_DBG_LVL_ERR,"::qhead null\n");
		return NULL;
	}

	if( qh->pool )
		return (void *)buff_alloc_data(qh->pool,sz);
	else
		return (void *)os_mem_alloc(sz);
}

/*******************************************************************************
 *
 * memory deallocation
 */
static void os_qnode_mem_free(OS_QHEAD *qh,void *buf)
{
	if( !qh ) {
		os_debug_printf(OS_DBG_LVL_ERR,"::qhead null\n");
	}else{

		if( qh->pool )
			buff_free_data(qh->pool,(unsigned char *)buf);
		else
			os_mem_free(buf);

	}
}

/*******************************************************************************
 *
 * os_qbuff_delete_node - Deleting a node in a queue
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  qhead - head of queue
 *  qnode - node
 *  del_f - delete function
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
static void os_qbuff_delete_node(OS_QHEAD *qhead,OS_QNODE *qnode,os_delfunc_t del_f)
{
	unsigned char *qf;

	if(!qhead || !qnode) {
		os_debug_printf(OS_DBG_LVL_ERR,"null argument\n");
		return;
	}

	/* qf is a queue buffer */
	qf = qnode->data;
	if(!qf) {
		os_debug_printf(OS_DBG_LVL_ERR,"null data\n");
		return;
	}

	/* freeing up node & data */
	if( del_f ) del_f((void *)qf); /* user-defined memory free */

	os_qnode_mem_free(qhead,qf);   /* qbuff */
	os_qnode_mem_free(qhead,qnode); /* qnode */

}

/*******************************************************************************
 *
 * os_qbuff_flush - Flushing a queue
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
unsigned int os_qbuff_flush(unsigned int id)
{
	OS_QHEAD *qhead;
	OS_QNODE *d, *trace;
	os_delfunc_t del_f; /* delete function */

	qhead = (OS_QHEAD *)os_obj_find(queue_list,id);
	if(!qhead) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return ERROR;
	}

	/* empty list */
	if(qhead->list==NULL)
		return OKAY;

	os_mutex_lock( qhead->lock, 0 );

	trace = qhead->list;
	del_f = qhead->delfunc; /* delete function */
	do {
		d = trace;
		trace = trace->next;
		if(d) os_qbuff_delete_node(qhead,d,del_f);
	} while(trace);

	qhead->list = NULL; /* null list */
	qhead->current_size = 0; /* empty */

	os_mutex_unlock( qhead->lock );

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_qbuff_delete - initializing a queue system
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
unsigned int os_qbuff_delete(unsigned int id)
{
	unsigned int res;
	OS_QHEAD *qhead;

	qhead = (OS_QHEAD *)os_obj_find(queue_list,id);
	if(!qhead) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return ERROR;
	}

	/* qbuffer flushing */
	if( os_qbuff_flush(id) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_qbuff_flush(%08x)::error\n",id);
		return ERROR;
	}

	/* delete mutex */
	res = os_mutex_delete( qhead->lock );
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_delete(%08x)\n",qhead->lock);
		return ERROR;
	}

	/* delete qhead */
	res = os_mem_free( qhead );
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mem_free(queue_list,%08x)\n",id);
		return ERROR;
	}


	/* deleteing items */
	res = os_list_item_delete( queue_list, id );
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_delete(queue_list,%08x)\n",id);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_qbuff_put - inserting a block
 *
 * DESCRIPTION
 *  Allocating a memory and then making a node
 *
 * PARAMETERS
 *  buff - buffer pointer
 *  size - block size
 *
 * RETURNS
 *  NULL - Failed.
 *
 * ERRORS
 *  N/A
 */
unsigned int os_qbuff_put(unsigned int id,char *buff,int size)
{
	OS_QHEAD *qhead;
	OS_QNODE *qnode,*trace;
	unsigned char *qbuff;

	if(id==0) { /* resource id */
		os_debug_printf(OS_DBG_LVL_ERR,"id->0\n");
		return ERROR;
	}

	if(!buff) { /* buffer check */
		os_debug_printf(OS_DBG_LVL_ERR,"buffer->NULL\n");
		return ERROR;
	}

	if(size<=0) { /* size check */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)size=%d\n",id,size);
		return ERROR;
	}

	/* qhead lookup */
	qhead = (OS_QHEAD *)os_obj_find(queue_list,id);
	if(!qhead) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return ERROR;
	}

	os_mutex_lock( qhead->lock, 0 ); /* critical region */

	/* allocating node */
	qnode = (OS_QNODE *)os_qnode_mem_alloc(qhead,sizeof(OS_QNODE));
	if(!qnode) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)os_qnode_mem_alloc(sizeof(OS_QNODE))->NULL\n",id);
		os_mutex_unlock( qhead->lock );
		return ERROR;
	}

	/* allocating data */
	qbuff = (unsigned char *)os_qnode_mem_alloc(qhead,size);
	if(!qbuff) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)os_qnode_mem_alloc(size=%d)->NULL\n",id,size);
		os_mutex_unlock( qhead->lock );
		os_qnode_mem_free(qhead,qnode);
		return ERROR;
	}

	memcpy(qbuff,buff,size); /* data copying */

	qnode->next = NULL;
	qnode->data = qbuff;
	qnode->size = size;
	qnode->tail = 0; /* initial starting position */
	qnode->unused = 0;  /* meaningless value */

	/*
	 * inserting a node to a buffer node
	 */
	if( !(qhead->list) ) {
		/* first item */
		qhead->list = qnode;
	}else{
		trace = qhead->list;
		while(trace->next) { trace = trace->next; }
		/* trace to the end of node */
		trace->next = qnode;
	}

	/* measurement */
	qhead->current_size += size;

	os_mutex_unlock( qhead->lock ); /* end of region */

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_qbuff_get - returning a queue node
 *
 * DESCRIPTION
 *  Allocating a memory and then making a node
 *
 * PARAMETERS
 *  buff - buffer pointer
 *  size - block size
 *
 * RETURNS
 *  NULL - Failed.
 *
 * ERRORS
 *  N/A
 */
unsigned int os_qbuff_get(unsigned int id,char *buff,int size)
{
	int total_copy_amount,
	    copy_amount,
	    current_available_amount,
	    still_remaining_amount;
	int continue_flag;
	unsigned char *copy_loc;
	OS_QNODE *node;
	OS_QHEAD *qhead;
	os_delfunc_t del_f; /* delete function */

	if(!buff) { /* buff check */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::buff->NULL\n",id);
		return ERROR;
	}

	if(size<=0) { /* size check */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::SIZE->%d\n",id,size);
		return ERROR;
	}

	qhead = (OS_QHEAD *)os_obj_find(queue_list,id);
	if(!qhead) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)\n",id);
		return ERROR;
	}

	os_mutex_lock( qhead->lock, 0 ); /* infinite trying */

	del_f = qhead->delfunc; /* delete function */

	total_copy_amount = 0;
	continue_flag = 1;
	while(continue_flag) { /* copying to a buffer */

		/* looking up node */
		node = qhead->list;
		if(!node) {
			//os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::null node\n",id);
			break;
		}

		/* checks up outstanding data */
		if(!node->data) {
			os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::null data\n",id);
			break;
		}

		/* starting location of data */
		copy_loc = node->data + node->tail;

		/* calculating amount data to be able to copy */
		current_available_amount = node->size - node->tail;

		/* total amount to take in the future */
		still_remaining_amount = size - total_copy_amount;

		/* real size of data to catch from the current block */
		copy_amount = (current_available_amount>still_remaining_amount) ?
		              still_remaining_amount : current_available_amount;

		os_debug_printf(OS_DBG_LVL_VER,
		                "QB:: Copying node=%p amount %d from %d, size=%d,current=%d,still=%d\n",
		                node, copy_amount, current_available_amount, size, total_copy_amount, still_remaining_amount);

		/* data copying */
		memcpy( buff + total_copy_amount,
		        copy_loc,
		        copy_amount );

		node->tail += copy_amount; /* advance tail pointer */

		if(node->tail > node->size)
			os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::fatal error - tail=%d/size=%d\n\n\n",
					id,node->tail,node->size);

		/* deleting node if this runs out of all of data */
		if(node->tail == node->size ) {

			/* checking up again */
			if( qhead->list != node ) {
				os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::fatal error\n",id);
				break;
			}

			/* advance head pointer */
			qhead->list = node->next;

			/* freeing up node & data */
			os_qbuff_delete_node(qhead,node,NULL);

			os_debug_printf(OS_DBG_LVL_VER, "QB:: Deleting node (%p) \n", node);

		}/* if(node->tail == .... */

		/* calculate copyable amount */
		total_copy_amount += copy_amount;

		/* check up whether we need to stop or not */
		if( (qhead->list == NULL) || (total_copy_amount >= size) ) {
			continue_flag = 0;
		}

		/* measurement */
		qhead->current_size -= copy_amount;

		os_debug_printf(OS_DBG_LVL_VER,"(%08x)copying(%d,%d,%d)\n",id,copy_amount,total_copy_amount,size);
	}

	os_mutex_unlock( qhead->lock );

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return total_copy_amount;
}

/*******************************************************************************
 *
 * os_qbuff_current_size - returning the current size of queue
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - queue id
 *
 * RETURNS
 *  current_size/0
 *
 * ERRORS
 *  N/A
 */
unsigned int os_qbuff_current_size(unsigned int id)
{
	OS_QHEAD *qhead;
	unsigned int csize;

	qhead = (OS_QHEAD *)os_obj_find(queue_list,id);
	if(!qhead) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)\n",id);
		return 0;
	}

	os_mutex_lock( qhead->lock, 0 ); /* infinite trying */
	csize = qhead->current_size;
	os_mutex_unlock( qhead->lock ); /* infinite trying */

	return csize;
}

/*******************************************************************************
 *
 *  qbuff_printout_stats -
 *
 *
 */
static void qbuff_printout_stats(OS_QHEAD *qh)
{
	debug_print("<QHEAD>  ::pool(%08x)::size(%08x)\n",qh->pool,qh->current_size);
}

/*******************************************************************************
 *
 *  qbuff_printout_stats2 -
 *
 *
 */
static void qbuff_printout_stats2(OS_LIST_DATA *d)
{
	qbuff_printout_stats((OS_QHEAD *)(d->data));
}

/*******************************************************************************
 *
 *  qbuff_verbose -
 *
 * DESCRIPTION
 *	Displaying the debug message of the queue buffer specified in
 *  a parameter. When the parameter is (-1), it displays the debugging informa-
 *  tion of all message queues.
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
unsigned int qbuff_verbose(int id)
{
	OS_QHEAD *qh;

	if(id == (-1)) {
		os_list_navigate(queue_list, NULL, qbuff_printout_stats2);
	}else{
		qh= (OS_QHEAD *)os_obj_find(queue_list,id);
		if(!qh) {
			os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
			return ERROR;
		}
		qbuff_printout_stats(qh);
	}

	return OKAY;
}

/*******************************************************************************
 *
 * os_qbuff_sanity_check - checking sanity
 *
 * DESCRIPTION
 *  counting method
 *
 * PARAMETERS
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
unsigned int os_qbuff_sanity_check(unsigned int id)
{
	unsigned int cnt;
	OS_QHEAD *queueh, *qh;
	OS_QNODE *qn;
	int crashed = 0;

	queueh = (OS_QHEAD *)os_obj_find(queue_list,id);
	if(!queueh) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return ERROR;
	}

	qh = queueh;
	os_mutex_lock(qh->lock,0);
	while(qh) {
		cnt = 0;
		qn = qh->list; /* node */
		while(qn) {
			cnt += (qn->size-qn->tail);
			qn = qn->next;
		}
		if(cnt!=qh->current_size) {
			os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::size error\n",id);
			crashed = 1;
		}
		qh = qh->next;
	}
	os_mutex_unlock(qh->lock);

	return (!crashed) ? OKAY : ERROR;
}
