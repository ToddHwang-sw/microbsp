/*******************************************************************************
 *
 * sel.c - select() function implementation
 *
 ******************************************************************************/
#include "oskit.h"

/*******************************************************************************
 *
 * net_port_proto_select_group
 *
 * DESCRIPTION
 *  Adding the set of port ids into a selector group.
 *  EX)
 *      net_sel_group grp;
 *      unsigned int a,b,c,d;  -> port id
 *      ...
 *      net_port_proto_select_group(&grp,a,b,c,d,0);
 *
 * PARAMETERS
 *  selg - selector group structure
 *  ...  - number of port list, to be ended with '0'.
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_proto_select_group(net_sel_group *selg,...)
{
	va_list ap;
	unsigned int fd;
	unsigned int include_ok,idx;
	unsigned int portid;
	net_port_st *portst;
	int counter;

	if(!selg) {
		os_debug_printf(OS_DBG_LVL_ERR,"null selector group::error\n");
		return ERROR;
	}

	counter = 0;
	va_start(ap,selg);
	while(1) {
		portid = va_arg(ap,unsigned int);
		if(portid == 0) { /* reaches to the end */
			break;
		}

		/* test where selg includes portid */
		include_ok = 0;
		for(idx=0; idx<selg->num; idx++) {
			if(portid == selg->member[idx]) {
				include_ok = 1;
			}
		}
		if(include_ok) {
			/* port is already included */
			os_debug_printf(OS_DBG_LVL_ERR,"port=%08x already included\n",portid);
			continue;
		}

		/* number of ports check */
		counter++;
		if(counter == NET_SEL_MAX_SIZE) {
			os_debug_printf(OS_DBG_LVL_ERR,"too many ports...\n");
			break;
		}

		/* look for a port */
		portst = net_port_search(portid);
		if(!portst) {
			os_debug_printf(OS_DBG_LVL_ERR,"invalid portid in (%d)th = %08x\n",counter,portid);
			va_end(ap);
			return ERROR;
		}
		/* fd */
		fd = NET_PORT_FD(portst);

		/* put this fd into fd_set */
		FD_SET(fd,&selg->rfds);
		FD_SET(fd,&selg->wfds);

#define greater_than(a,b)   ((a)>(b) ? (a) : (b))

		/* set up maxfd values */
		selg->maxfd = greater_than(selg->maxfd,fd);

		/* inserting it into a member list */
		selg->member[selg->num] = portid;
		selg->num += 1;

		os_debug_printf(OS_DBG_LVL_VER,"Inserting a port (%08x)\n",portid);
	}
	va_end(ap);

	os_debug_printf(OS_DBG_LVL_VER,"done\n");

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_proto_select_delete
 *
 * DESCRIPTION
 *  Deleting a port from a selector group
 *
 * PARAMETERS
 *  selg - selector group structure
 *  portid - port to be deleted
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_proto_select_delete(net_sel_group *selg,unsigned int portid)
{
	unsigned int fd;
	int deleted_ok;
	unsigned int copy_maxfd;
	fd_set copy_rfds;
	fd_set copy_wfds;
	unsigned int *copy_member;
	unsigned int copy_num;
	unsigned int idx;
	unsigned int id;
	net_port_st *port;

	/* parameter check */
	if(!selg) {
		os_debug_printf(OS_DBG_LVL_ERR,"null selector group::error\n");
		return ERROR;
	}

	/* member class allocation */
	/* statical decalaration may cause stack depth to deeply grow up
	   and it may be able to incur serious problem. */
	copy_member = (unsigned int *)buff_alloc_data(NET_MEM_CLASS,
	                                              sizeof(unsigned int)*NET_SEL_MAX_SIZE);
	if(copy_member==NULL) {
		os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(..NET_SEL_MAX_SIZE)::error\n");
		return ERROR;
	}
	memset((char *)copy_member,0,sizeof(unsigned int)*NET_SEL_MAX_SIZE); /* zeroing... */

	/* initial value */
	copy_maxfd = 0;
	copy_num   = 0;
	FD_ZERO(&copy_rfds);
	FD_ZERO(&copy_wfds);

	deleted_ok = 0; /* flag */
	for(idx=0; idx<selg->num; idx++) {
		/* port id */
		id = selg->member[idx];

		/* port block */
		port = net_port_search(id);
		if(!port) {
			os_debug_printf(OS_DBG_LVL_ERR,"null port block(id=%08x)\n",id);
			continue;
		}

		/* port to be deleted */
		if(id == portid) {
			/* skip !! */
			if( deleted_ok ) {
				/* already deleted */
				os_debug_printf(OS_DBG_LVL_ERR,"more than 2 deleted ports(%08x)\n",id);
			}
			deleted_ok = 1;
			continue;
		}

		/* file descriptor - socket itself for a port */
		fd = NET_PORT_FD(port);

		/* put this fd into copy fd_set */
		FD_SET(fd,&copy_rfds);
		FD_SET(fd,&copy_wfds);

		copy_maxfd = greater_than(copy_maxfd,fd);

		/* inserting it into a copy member list */
		copy_member[copy_num] = id;
		copy_num += 1;
	}

	/* check up any ports to be deleted */
	if(!deleted_ok) {
		os_debug_printf(OS_DBG_LVL_ERR,"No ports deleted for id=%08x\n",id);
		buff_free_data(NET_MEM_CLASS,(unsigned char *)copy_member);
		return ERROR;
	}

	/* cleaning up port selector */
	net_port_proto_select_cleanup(selg);

	/* member list */
	memcpy((char *)selg->member,(char *)copy_member,sizeof(unsigned int)*NET_SEL_MAX_SIZE);

	/* number of items */
	selg->num   = copy_num;

	/* cleaning up resources for member copy */
	buff_free_data(NET_MEM_CLASS,(unsigned char*)copy_member);

	/* read/write fd_sets */
	memcpy((char *)&selg->rfds,(char *)&copy_rfds,sizeof(fd_set));
	memcpy((char *)&selg->wfds,(char *)&copy_wfds,sizeof(fd_set));

	/* maxfd */
	selg->maxfd = copy_maxfd;

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_proto_select
 *
 * DESCRIPTION
 *  doing select() operation with the set of port ids.
 *  '0' value for tm means by infinite waiting for an event of packet send/receive.
 *
 * PARAMETERS
 *  selg - selector structure
 *  mode - read/write
 *  tm   - microsecond time
 *
 * RETURNS
 *  (-1)  - error
 *  > 0   - number of ports with oustanding network transaction
 *
 * ERRORS
 *  N/A
 */
int net_port_proto_select(net_sel_group *selg,unsigned int mode,unsigned long tm)
{
	int res;
	struct timeval tv;
	fd_set local_rfds;
	fd_set local_wfds;

	/* parameter check */
	if(!selg) {
		os_debug_printf(OS_DBG_LVL_ERR,"null selector group::error\n");
		return (-1);
	}

	/* empty check */
	if(!selg->num) {
		os_debug_printf(OS_DBG_LVL_ERR,"empty members\n");
		return (-1);
	}

	/* time processing */
	if(tm) {
		tv.tv_sec = tm / 1000000; /* seconds */
		tv.tv_usec = tm % 1000000; /* microseconds */
	}

	/* depending on mode... */
	switch(mode) {
	case NET_READ_SELECT:
		local_rfds = selg->rfds;
		res = select(selg->maxfd+1,&local_rfds,NULL,NULL,(tm) ? &tv : NULL); /* read */
		selg->res_rfds = local_rfds;
		break;

	case NET_WRITE_SELECT:
		local_wfds = selg->wfds;
		res = select(selg->maxfd+1,NULL,&local_wfds,NULL,(tm) ? &tv : NULL); /* write */
		selg->res_wfds = local_wfds;
		break;

	case NET_RW_SELECT:
		local_rfds = selg->rfds;
		local_wfds = selg->wfds;
		res = select(selg->maxfd+1,&local_rfds,&local_wfds,NULL,(tm) ? &tv : NULL); /* read+write */
		selg->res_rfds = local_rfds;
		selg->res_wfds = local_wfds;
		break;

	default:
		os_debug_printf(OS_DBG_LVL_ERR,"invalid mode = %08x\n",mode);
		return (-1);
	}

	if(res<0) {
		/* select() returns error */
		os_debug_printf(OS_DBG_LVL_ERR,"select(%s)::returns %d:;error\n",
		                (mode==NET_READ_SELECT) ? "read" : "write",res);
		return res;
	}

	return res;
}

/*******************************************************************************
 *
 * net_port_proto_select_test
 *
 * DESCRIPTION
 *  Examining whether a specific port is now having a network transaction.
 *
 * PARAMETERS
 *  selg - selector structure
 *  mode - read/write
 *  portid  - portid
 *
 * RETURNS
 *  (-1) - error
 *  0    - no transaction
 *  1    - transaction
 *
 * ERRORS
 *  N/A
 */
int net_port_proto_select_test(net_sel_group *selg,unsigned int mode,unsigned int portid)
{
	unsigned int idx,include_ok;
	net_port_st *port;
	int res;

	if(!selg) {
		os_debug_printf(OS_DBG_LVL_ERR,"null selector group::error\n");
		return (-1);
	}

	/* valid port check */
	port = net_port_search(portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port = %08x\n",portid);
		return (-1);
	}

	include_ok = 0;

	/* test where selg includes portid */
	for(idx=0; idx<selg->num; idx++) {
		if(portid == selg->member[idx]) {
			include_ok = 1;
		}
	}

	if(!include_ok) {
		/* portid has not been found !! */
		os_debug_printf(OS_DBG_LVL_ERR,"selector does not have %08x port\n", portid);
		return (-1);
	}

	/* mode check */
	switch(mode) {
	case NET_READ_SELECT:
		res = FD_ISSET(NET_PORT_FD(port),&selg->res_rfds);
		break;
	case NET_WRITE_SELECT:
		res = FD_ISSET(NET_PORT_FD(port),&selg->res_wfds);
		break;
	default:
		os_debug_printf(OS_DBG_LVL_ERR,"invalid mode = %08x\n",mode);
		return (-1);
	}

	return res;
}

/*******************************************************************************
 *
 * net_port_proto_select_cleanup
 *
 * DESCRIPTION
 *  Cleaning up selector structure..
 *
 * PARAMETERS
 *  selg - selector structure
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_proto_select_cleanup(net_sel_group *selg)
{
	if(!selg) {
		os_debug_printf(OS_DBG_LVL_ERR,"null selector group::error\n");
		//return (-1);
		return ERROR;
	}

	/* initializing ... */
	FD_ZERO(&selg->rfds);
	FD_ZERO(&selg->wfds);
	FD_ZERO(&selg->res_rfds);
	FD_ZERO(&selg->res_wfds);
	selg->num = 0;
	selg->maxfd = 0;

	return OKAY;
}

/*******************************************************************************
 *
 * EXAMPLE)
 *
 *   net_sel_group selg;
 *   ...
 *   net_port_proto_select_cleanup(&selg);
 *   ...
 *   net_port_proto_select_group(&selg,porta,0);
 *   ...
 *   net_port_proto_select_group(&selg,portb,portc,0);
 *   ...
 *   while( 1 ){
 *     net_port_proto_select(&selg);
 *     ...
 *     if( net_port_proto_select_test(&selg,porta) ){
 *          do something...
 *     }else
 *     if( net_port_proto_select_test(&selg,portb) ){
 *          do something...
 *     }else
 *     if( net_port_proto_select_test(&selg,portc) ){
 *          do something...
 *     }
 *
 *   }
 *
 *
 */
