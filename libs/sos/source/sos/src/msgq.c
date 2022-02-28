/*******************************************************************************
 *
 * msgq.c - OS Porting layer
 *
 ******************************************************************************/
#include "oskit.h"

static OS_LIST * msgq_list;  /* the container of msgq */

/*******************************************************************************
 *
 * os_msgq_init - Initializing a message queue system
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *
 *
 * ERRORS
 *  N/A
 */
unsigned int os_msgq_init(void)
{
	msgq_list = os_list_create(MSGQ_NAME);
	if(!msgq_list) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_create(%s)\n",MSGQ_NAME);
		return ERROR;
	}
	return OKAY;
}

/*******************************************************************************
 *
 * os_msgq_create - Creating a message queue
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID of message queue/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_msgq_create(const char *name,unsigned int pool,os_delfunc_t f)
{
	OS_MESG_HEAD *msgh;
	unsigned char tname[STR_LEN];
	unsigned int res;

	msgh = (OS_MESG_HEAD *)os_mem_alloc(sizeof(OS_MESG_HEAD)); /* message header */
	if(!msgh) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%s)os_mem_alloc(sizeof(OS_MESG_HEAD))::error\n",name);
		return ERROR;
	}

	sprintf((char *)tname,(const char *)"%sc",name);
	msgh->cond = os_cond_create((const char *)tname); /* conditional variable */
	if(msgh->cond == ERROR) {
		os_mem_free(msgh);
		os_debug_printf(OS_DBG_LVL_ERR,"os_cond_create(%s):;error\n",tname);
		return ERROR;
	}

	sprintf((char *)tname,(const char *)"%sq",name);
	msgh->qbuff = os_qbuff_create((const char *)tname,pool,f); /* qbuffer */
	if(msgh->qbuff == ERROR) {
		os_cond_delete(msgh->cond);
		os_mem_free(msgh);
		os_debug_printf(OS_DBG_LVL_ERR,"os_qbuff_create(%s)::error\n",tname);
		return ERROR;
	}

	res=os_list_item_insert(msgq_list,(void *)msgh); /* insert into a list */
	if(res == ERROR) {
		os_cond_delete(msgh->cond);
		os_qbuff_delete(msgh->qbuff);
		os_mem_free(msgh);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(%s)::error\n",name);
		return ERROR;
	}

	/* signalling variables */
	msgh->counter    = 0;

	os_debug_printf(OS_DBG_LVL_VER,"(%s)(%08x)::done\n",name,res);

	return res;
}

/*******************************************************************************
 *
 * os_msgq_delete - Deleting a message queue
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID of message queue
 *
 * ERRORS
 *  N/A
 */
unsigned int os_msgq_delete(unsigned int id)
{
	unsigned int res;
	OS_MESG_HEAD *mh;

	mh = (OS_MESG_HEAD *)os_obj_find(msgq_list,id);
	if(!mh) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return ERROR;
	}

	res = os_qbuff_delete(mh->qbuff);
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)os_qbuff_delete(%08x)::error\n",id,mh->qbuff);
		return ERROR;
	}

	/* conditional variable */
	res = os_cond_delete(mh->cond);
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)os_cond_delete(%08x)::error\n",id,mh->cond);
		return ERROR;
	}

	res = os_mem_free(mh);
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)os_mem_free::error\n",id);
		return ERROR;
	}

	res = os_list_item_delete(msgq_list,id);
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)os_list_item_delete::error\n",id);
		return ERROR;
	}

	return OKAY;
}

/*******************************************************************************
 *
 * os_msgq_flush - Flushing a message queue
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  ID of message queue
 *
 * ERRORS
 *  N/A
 */
unsigned int os_msgq_flush(unsigned int id)
{
	unsigned int res;
	OS_MESG_HEAD *mh;

	mh = (OS_MESG_HEAD *)os_obj_find(msgq_list,id);
	if(!mh) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return ERROR;
	}

	res = os_qbuff_flush(mh->qbuff);
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)os_qbuff_flush(%08x)::error\n",id,mh->qbuff);
		return ERROR;
	}

	return OKAY;
}

/*******************************************************************************
 *
 * os_msgq_message - Making a message queue
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
unsigned int os_msgq_message(OS_MESG *msg,unsigned int command,unsigned char *buff,unsigned int sz)
{
	if(!msg) {
		os_debug_printf(OS_DBG_LVL_ERR,"NULL parameter::error\n");
		return ERROR;
	}
	msg->command = command; /* command */
	msg->data = buff;   /* buffer */
	msg->size = sz;     /* size */
	return OKAY;
}

/*******************************************************************************
 *
 * os_msgq_send - Sending a data to message queue
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
unsigned int os_msgq_send(unsigned int id,OS_MESG *msg)
{
	OS_MESG_HEAD *mh;
	unsigned long msgaddr; /* message address */
	unsigned int res;

	if(!msg) { /* buff check */
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)::null data\n",id);
		return ERROR;
	}

	mh = (OS_MESG_HEAD *)os_obj_find(msgq_list,id);
	if(!mh) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return ERROR;
	}

	/* Architecture... 64bit ? 32bit ? */
	/* sizeof(unsigned long) - 8 bytes? 4 bytes ? */
	msgaddr = (unsigned long)msg; /* address of message */

	res = os_qbuff_put(mh->qbuff,(char *)&msgaddr,sizeof(unsigned long)); /* pointer copy */
	if(res==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%08x)os_qbuff_put(%08x)::error\n",id,mh->qbuff);
		return ERROR;
	}

	/* signalling */
	os_cond_signal(mh->cond);

	/* increasing counter */
	++mh->counter;

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return OKAY;
}

/*******************************************************************************
 *
 * os_msgq_recv_internal - Fetching a message from a message queue
 *
 * DESCRIPTION
 */
static OS_MESG *os_msgq_recv_internal(OS_MESG_HEAD *mh)
{
	OS_MESG *m;
	unsigned long msgaddr; /* message address */
	unsigned int res;

	if(!mh) {
		os_debug_printf(OS_DBG_LVL_ERR,"null parameter\n");
		return NULL;
	}

	res = os_qbuff_get(mh->qbuff,(char *)&msgaddr,sizeof(unsigned long)); /* pointer copy */
	if(res!=sizeof(unsigned long)) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_qbuff_get(%08x)::error\n",mh->qbuff);
		return NULL;
	}

	m = (OS_MESG *)msgaddr;
	if(!m) {
		os_debug_printf(OS_DBG_LVL_ERR,"NULL data \n");
		return NULL;
	}

	return m;
}

/*******************************************************************************
 *
 * os_msgq_recv - Receiving a message from a message queue
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - queue id
 *  bmode - blocking/nonblocking
 *
 * RETURNS
 *  ID of message queue
 *
 * ERRORS
 *  N/A
 */
OS_MESG *os_msgq_recv(unsigned int id,unsigned int bmode)
{
	OS_MESG_HEAD *mh;
	OS_MESG *m;

	mh = (OS_MESG_HEAD *)os_obj_find(msgq_list,id);
	if(!mh) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return NULL;
	}

	if (bmode && (os_qbuff_current_size(mh->qbuff) == 0))
		return NULL;

	os_cond_acquire(mh->cond);

	/* something... */
	m = os_msgq_recv_internal(mh);
	if(!m) {
		os_cond_release(mh->cond); /* releasing conditional variable */
		os_debug_printf(OS_DBG_LVL_ERR,"null message returned from (%08x)\n",mh->qbuff);
		return NULL;
	}

	--(mh->counter);

	os_cond_release(mh->cond); /* releasing conditional variable */

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return m;
}

/*******************************************************************************
 *
 * os_msgq_recv_timeout - Receiving a message from a message queue with time bound
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  id - queue id
 *  bmode - blocking/nonblocking
 *
 * RETURNS
 *  ID of message queue
 *
 * ERRORS
 *  N/A
 */
OS_MESG *os_msgq_recv_timeout(unsigned int id,unsigned long tm)
{
	OS_MESG_HEAD *mh;
	OS_MESG *m;

	mh = (OS_MESG_HEAD *)os_obj_find(msgq_list,id);
	if(!mh) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
		return NULL;
	}

	/*
	 * Two way checking....
	 */
	/* None of messages were found... */
	if(os_qbuff_current_size(mh->qbuff)==0) {
		os_cond_acquire_timeout(mh->cond,tm); /* 1 second */
		if(os_qbuff_current_size(mh->qbuff)==0) {
			/* timeout */
			os_cond_release(mh->cond); /* releasing conditional variable */
			os_debug_printf(OS_DBG_LVL_VER,"os_qbuff_current_size(%08x)::still 0\n",mh->qbuff);
			return NULL;
		}
	}

	/* something... */
	m = os_msgq_recv_internal(mh);
	if(!m) {
		os_cond_release(mh->cond); /* releasing conditional variable */
		os_debug_printf(OS_DBG_LVL_ERR,"null message returned from (%08x)\n",mh->qbuff);
		return NULL;
	}

	--(mh->counter);

	os_cond_release(mh->cond); /* releasing conditional variable */

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)::done\n",id);

	return m;
}

/*******************************************************************************
 *
 *  msgq_printout_stats -
 *
 *
 */
static void msgq_printout_stats(OS_MESG_HEAD *mh)
{
	debug_print("<MSGQ>    ::counter(%08x)\n",mh->counter);
}

/*******************************************************************************
 *
 *  msgq_printout_stats2 -
 *
 *
 */
static void msgq_printout_stats2(OS_LIST_DATA *d)
{
	msgq_printout_stats((OS_MESG_HEAD *)(d->data));
}

/*******************************************************************************
 *
 *  msgq_verbose -
 *
 * DESCRIPTION
 *	Displaying the debug message of the message queue specified in
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
unsigned int msgq_verbose(int id)
{
	OS_MESG_HEAD *mh;

	if(id == (-1)) {
		os_list_navigate(msgq_list, NULL, msgq_printout_stats2);
	}else{
		mh = (OS_MESG_HEAD *)os_obj_find(msgq_list,id);
		if(!mh) {
			os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(%08x)::error\n",id);
			return ERROR;
		}
		msgq_printout_stats(mh);
	}

	return OKAY;
}

