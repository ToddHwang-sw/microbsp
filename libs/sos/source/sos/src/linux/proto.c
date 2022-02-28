/*******************************************************************************
 *
 * proto.c - protocol block
 *
 ******************************************************************************/
#include "oskit.h"

/* UDP protocol block */
static net_protocol_st net_proto_udp_block;
/* TCP protocol block */
static net_protocol_st net_proto_tcp_block;

/*******************************************************************************
 *
 * os_net_proto_select - Selecting protocol block
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  type - protocol type
 *
 * RETURNS
 *  NULL - Failed.
 *
 * ERRORS
 *  N/A
 */
net_protocol_st *os_net_proto_select(unsigned int type)
{
	switch(type) {
	case NET_PORT_PROTO_UDP: return &net_proto_udp_block;
	case NET_PORT_PROTO_TCP: return &net_proto_tcp_block;
	default: return NULL;
	}
}

/*******************************************************************************
 *
 * os_net_proto_identify - Identifying protocol block
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port structure
 *
 * RETURNS
 *  ERROR/protocol id
 *
 * ERRORS
 *  N/A
 */
unsigned int os_net_proto_identify(net_port_st *port)
{
	/* sanity check */
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid parameter\n");
		return ERROR;
	}

	/* non protocol */
	if(!port->proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"NULL protocol (%08x)\n",port->id);
		return ERROR;
	}

	if(port->proto == &net_proto_udp_block) return NET_PORT_PROTO_UDP;
	if(port->proto == &net_proto_tcp_block) return NET_PORT_PROTO_TCP;

	os_debug_printf(OS_DBG_LVL_ERR,"unidentified protocol for (%08x)\n",port->id);
	return ERROR;
}

/*******************************************************************************
 *
 * net_port_proto_iscommunicatable - Examing whether we can send/receive
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port structure
 *
 * RETURNS
 *  ERROR/protocol id
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_proto_iscommunicatable(net_port_st *port)
{
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	switch(port->type) {
	case NET_PORT_TYPE_CLIENT:
		/* client... */
		return (NET_PORT_STATUS(port,NET_PORT_STATUS_CONN)) ? OKAY : ERROR;

	case NET_PORT_TYPE_SERVER:
		/* server... */
		return (NET_PORT_STATUS(port,NET_PORT_STATUS_BIND)) ? OKAY : ERROR;

	default:
		os_debug_printf(OS_DBG_LVL_ERR,"Undefined type (%08x)\n",port->id);
		return ERROR;
	}

	return ERROR;
}

/*******************************************************************************
*
*  U  D  P
*
*******************************************************************************/

/*******************************************************************************
 *
 * net_udp_socket - socket
 *
 * DESCRIPTION
 *  Creating and returning a new socket descriptor. Just replacement of
 *  socket() BSD API for OS separation.
 *
 * PARAMETERS
 *  port - physical port structure
 *
 * RETURNS
 *  ERROR - failure
 *  socket descriptor - success
 *
 * ERRORS
 *  N/A
 */
int net_udp_socket(net_port_st *port)
{
	int fd;
	int size;

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	/* socket() already called... */
	if(NET_PORT_STATUS(port,NET_PORT_STATUS_SOCKET)) {
		os_debug_printf(OS_DBG_LVL_ERR,"already established\n");
		return ERROR;
	}

	/* socket creation */
	fd  = socket(AF_INET,SOCK_DGRAM,0);
	if(fd<0) {
		os_errno_printf();
		os_debug_printf(OS_DBG_LVL_ERR,"socket(AF_INET,SOCK_DGRAM,0)::error\n");
		return ERROR;
	}

	size = port->payload_size; 
	if ( setsockopt( fd,
			SOL_SOCKET,
			SO_RCVBUF,
			&size,
			sizeof(size) ) < 0 ) {
		os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(..SO_RCVBUF.., size=%d)::error\n",size);
		return ERROR;
	}

	size = port->payload_size; 
	if ( setsockopt( fd,
			SOL_SOCKET,
			SO_SNDBUF,
			&size,
			sizeof(size) ) < 0 ) {
		os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(..SO_SNDBUF.., size=%d)::error\n",size);
		return ERROR;
	}

	/* socket called */
	port->status |= NET_PORT_STATUS_SOCKET;

	os_debug_printf(OS_DBG_LVL_VER,"(fd=%d)::done\n",fd);

	return fd; /* new fd */
}

/*******************************************************************************
 *
 * net_udp_connect - connect
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port structure
 *  addr - address information
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_udp_connect(net_port_st *port,net_addrinfo_t *addr)
{
	int fd;
	int res;
	struct sockaddr_in *addrblk;
	struct sockaddr_in localaddr;

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	/* - Both bind() and connect() get to be invoked
	 *  together, invcation check should not be done. (KA)
	 * - avail operation has been changed to just
	 *  examinatio of duplicated socket API call.
	 */
	if(NET_PORT_STATUS(port,NET_PORT_STATUS_CONN)) {
		os_debug_printf(OS_DBG_LVL_ERR,"already connected\n");
		return ERROR;
	}

	/* socket has been done? */
	fd  = port->fd;
	if(fd<=0) {
		os_debug_printf(OS_DBG_LVL_ERR,"connect(fd<0)::error\n");
		return ERROR;
	}

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"address block is NULL\n");
		return ERROR;
	}

	/* remote information */
	addrblk = (struct sockaddr_in *)&(addr->addr);

	/* UDP client HAS TO use a local-defined address variable to connect (who know why?? !-_-;; ) */
	/* It has been on the expectablable themes. addr information in a port structure may be
	   raced by upstanding threads. Hence, it may be better to take it apart which contained
	   within a connect routine.  
	 */
	memset(&localaddr, 0, sizeof(struct sockaddr_in));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = addrblk->sin_port;
	localaddr.sin_addr.s_addr = addrblk->sin_addr.s_addr;

	/* connect */
	res = connect(fd,(struct sockaddr *)&localaddr,sizeof(struct sockaddr_in));
	if(res < 0 ) {
		/* connection failed */
		os_errno_printf();
		os_debug_printf(OS_DBG_LVL_ERR,"connect(fd=%d)::error\n",fd);
		return ERROR;
	}

	/* connected */
	port->status |= NET_PORT_STATUS_CONN;

	os_debug_printf(OS_DBG_LVL_VER,"connect(fd=%d,port=%08x)::done\n",fd,port->id);

	return OKAY;
}

/*******************************************************************************
 *
 * net_udp_bind - bind
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port structure
 *  addr - address information
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_udp_bind(net_port_st *port,net_addrinfo_t *addr)
{
	int fd;
	int res;
	int optval = 1;
	struct sockaddr_in *addrblk;

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	/* already bind() */
	if(NET_PORT_STATUS(port,NET_PORT_STATUS_BIND)) {
		os_debug_printf(OS_DBG_LVL_ERR,"already established\n");
		return ERROR;
	}

	/* socket has been done? */
	fd  = port->fd;
	if(fd<=0) {
		os_debug_printf(OS_DBG_LVL_ERR,"socket(AF_INET,SOCK_DGRAM,0)::error\n");
		return ERROR;
	}

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"address block is NULL\n");
		return ERROR;
	}

	/* remote information */
	addrblk = (struct sockaddr_in *)&(addr->addr);

	/* REUSEADDR option */
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(optval))<0) {
		os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(fd=%d,SO_REUSEADDR...)::error\n",fd);
		return ERROR;
	}

	/* bind */
	res = bind(fd,(struct sockaddr *)addrblk,sizeof(struct sockaddr));
	if(res < 0 ) {
		/* connection failed */
		os_errno_printf();
		os_debug_printf(OS_DBG_LVL_ERR,"bind(fd=%d)::error\n",fd);
		return ERROR;
	}

	/* availability */
	port->status |= NET_PORT_STATUS_BIND;

	os_debug_printf(OS_DBG_LVL_VER,"bind(fd=%d,port=%d)::done\n",fd,port->id);

	return OKAY;
}

/*******************************************************************************
 *
 * net_udp_listen - listen
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port sturucture
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_udp_listen(net_port_st *port)
{
	return OKAY;
}

/*******************************************************************************
 *
 * net_udp_accept - accept
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port sturucture
 *  addr - address information
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_udp_accept(net_port_st *port,net_addrinfo_t *addr)
{
	return ERROR;
}

/*******************************************************************************
 *
 * net_udp_send - send
 *
 * DESCRIPTION
 *  Sending a data. The data is sent out in the way of best-effort traffic.
 *  None of minimum data block is used.
 *
 * PARAMETERS
 *  port - port structure
 *  blk  - data
 *  size - data size
 *  addr - address information
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_udp_send(net_port_st *port,void *blk,int size,net_addrinfo_t *addr)
{
	int fd;
	int amount;
	int count = 0;
	int tx_retry_count;
	int send_unit;
	struct sockaddr_in *addrblk;

	/* parameter check */
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null parameter\n");
		return ERROR;
	}

	/* aviailabiity check */
	/* not yet activated...?? */
	if(!net_port_proto_iscommunicatable(port)) {
		net_port_errno_set(port->id,E_NET_PORT_UNESTAB);
		os_debug_printf(OS_DBG_LVL_ERR,"not yet established\n");
		return ERROR;
	}

	fd = port->fd; /* socket descriptor */

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null address block\n");
		return ERROR;
	}

	/* peer information */
	addrblk = (struct sockaddr_in *)&(addr->addr);

	/* reset counter */
	tx_retry_count = 0;

	while(count<size) {
		/* send_unit = size-count; */
		/*
		 * WinCE operating system strictly discern the type of
		 * recvfrom() and recv(), thus we also identify them.
		 */
		switch( port->type ) {
		case NET_PORT_TYPE_CLIENT:
			/* connected socket */
			amount = send(fd,(char *)blk+count,size-count,0);
			break;
		case NET_PORT_TYPE_SERVER:
			/* unconnected socket */
			amount = sendto(fd,(char *)blk+count,
			                size-count,
			                0,
			                (struct sockaddr*)addrblk, sizeof(struct sockaddr));
			break;
		default:
			os_debug_printf(OS_DBG_LVL_ERR,"Unknown port type=%d\n",port->type);
			return ERROR;
		}
		if(amount < 0 ) {
			os_errno_printf();
			net_port_errno_set(port->id,E_NET_PORT_TX);
			os_debug_printf(OS_DBG_LVL_ERR,"send(%d,%08lx,%d)::error(%d)\n",
			                fd,(unsigned long)blk,size,amount);
			return ERROR;
		}
		/* It may happen in blocking mode */
		if(amount == 0) {
			os_debug_printf(OS_DBG_LVL_VER,"send(%d,%d/%d)-zero send\n",fd,count,size);
			os_thread_sleep(100000); /* 100 milliseconds */
			if( ++(tx_retry_count) == NET_PORT_MAX_RETRY_COUNTER ) {
				net_port_errno_set(port->id,E_NET_PORT_TX);
				os_debug_printf(OS_DBG_LVL_ERR,"(%d,%08lx,%d)::continued zero(%d)\n",
				                fd,(unsigned long)blk,size,amount);
				return ERROR;
			}else 
				continue;
		}
		tx_retry_count = 0; /* reset counter */
		count += amount;
	}/* while(count... */

	return OKAY;
}

/*******************************************************************************
 *
 * net_udp_recv - recv
 *
 * DESCRIPTION
 *  Receiving a data. The data is elevated in the way of best-effort traffic.
 *  None of minimum data block is used.
 *
 * PARAMETERS
 *  port - port structure
 *  blk  - data
 *  size - data size
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_udp_recv(net_port_st *port,void *blk,int size,net_addrinfo_t *addr)
{
	int fd;
	int amount;
	int count = 0;
	int recv_unit;
	int rx_retry_count;
	struct sockaddr_in addrblk;
	unsigned int addrsize;

	/* parameter check */
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null parameter\n");
		return ERROR;
	}

	if(!net_port_proto_iscommunicatable(port)) {
		net_port_errno_set(port->id,E_NET_PORT_UNESTAB);
		os_debug_printf(OS_DBG_LVL_ERR,"not yet established\n");
		return ERROR;
	}

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null address block\n");
		return ERROR;
	}

	fd = port->fd; /* socket descriptor */

	/* remote address - copy */
	memcpy((char *)&addrblk,(char *)(addr->addr),sizeof(struct sockaddr_in));
	/* addr size */
	addrsize = sizeof(struct sockaddr_in);

	/* reset retry count */
	rx_retry_count = 0;
	while(count<size) {
		recv_unit = size-count;
		/*
		 * Sendto()/send() are strictly distinguished in udp_send() routine.
		 * however, we don't do it in recv(). We just use recvfrom().
		 */
		amount = recvfrom(fd,(char *)blk+count,
		                  size-count,
		                  0,
		                  (struct sockaddr *)&addrblk, &addrsize);
		if(amount < 0) {
			os_errno_printf();
			net_port_errno_set(port->id,E_NET_PORT_RX);
			os_debug_printf(OS_DBG_LVL_ERR,"recv(%d,%08lx,%d)::error(%d)\n",
			                fd,(unsigned long)blk,size,amount);
			return ERROR;
		}
		/* It may happen in blocking mode */
		if(amount == 0) {
			os_debug_printf(OS_DBG_LVL_VER,"recv(%d,%d/%d)-zero recv\n",fd,count,size);
			os_thread_sleep(100000); /* 100 milliseconds */
			if( ++(rx_retry_count) == NET_PORT_MAX_RETRY_COUNTER ) {
				net_port_errno_set(port->id,E_NET_PORT_RX);
				os_debug_printf(OS_DBG_LVL_ERR,"recv(%d,%08lx,%d)::continued zero(%d)\n",
				                fd,(unsigned long)blk,size,amount);
				return ERROR;
			}else continue;
		}
		rx_retry_count = 0; /* reset */
		count += amount;
	}/* while(count... */

	/* restore address information */
	memcpy((char *)(addr->addr),(char *)&addrblk,sizeof(struct sockaddr_in));
	addr->addrsize = addrsize; /* size */

	return OKAY;
}

/*******************************************************************************
 *
 * net_udp_close - close
 *
 * DESCRIPTION
 *  Closing a socket descriptor
 *
 * PARAMETERS
 *  port - port structure
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_udp_close(net_port_st *port)
{
	/* parameter check */
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null parameter\n");
		return ERROR;
	}

	/* If port comes to an any situation that is operable.
	 */
	if(port->status == 0) {
		os_debug_printf(OS_DBG_LVL_ERR,"not yet established\n");
		return ERROR;
	}

	os_close(port->fd); /* close */

	port->status = 0; /* clear status */

	return OKAY;
}

/*******************************************************************************
*
*  T  C  P
*
*******************************************************************************/

/*******************************************************************************
 *
 * net_tcp_socket - socket
 *
 * DESCRIPTION
 *  Creating TCP socket
 *
 * PARAMETERS
 *  port - port structure
 *
 * RETURNS
 *  Socket descriptor...
 *
 * ERRORS
 *  N/A
 */
int net_tcp_socket(net_port_st *port)
{
	int fd;
	int size;

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	if(NET_PORT_STATUS(port,NET_PORT_STATUS_SOCKET)) {
		os_debug_printf(OS_DBG_LVL_ERR,"already established\n");
		return ERROR;
	}

	/* socket creation */
	fd  = socket(AF_INET,SOCK_STREAM,0);
	if(fd<0) {
		os_errno_printf();
		os_debug_printf(OS_DBG_LVL_ERR,"socket(AF_INET,SOCK_STREAM,0)::error\n");
		return ERROR;
	}

	size = port->payload_size; 
	if ( setsockopt( fd,
			SOL_SOCKET,
			SO_RCVBUF,
			&size,
			sizeof(size) ) < 0 ) {
		os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(..SO_RCVBUF.., size=%d)::error\n",size);
		return ERROR;
	}

	size = port->payload_size; 
	if ( setsockopt( fd,
			SOL_SOCKET,
			SO_SNDBUF,
			&size,
			sizeof(size) ) < 0 ) {
		os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(..SO_SNDBUF.., size=%d)::error\n",size);
		return ERROR;
	}

	/* socket called */
	port->status |= NET_PORT_STATUS_SOCKET;

	os_debug_printf(OS_DBG_LVL_VER,"(fd=%d)::done\n",fd);

	return fd; /* new fd */
}

/*******************************************************************************
 *
 * net_tcp_connect - connect
 *
 * DESCRIPTION
 *  As long as client side calls this, just connect() call will be invocated,
 *  if he just use an option type NET_CLIENT_..... In terms of server node,
 *  this function sets up ports with both listen() and bind().
 *
 * PARAMETERS
 *  port - port structure
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
int net_tcp_connect(net_port_st *port,net_addrinfo_t *addr)
{
	int fd;
	int res;
	int blocked;
	int optval = 1;
	struct sockaddr_in *addrblk;
	static int backlog = 5; /* statically defined */

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	/* already connected */
	if(NET_PORT_STATUS(port,NET_PORT_STATUS_CONN)) {
		os_debug_printf(OS_DBG_LVL_ERR,"already established\n");
		return ERROR;
	}

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null address block\n");
		return ERROR;
	}

	/* socket has been done? */
	fd  = port->fd;
	if(fd<=0) {
		os_debug_printf(OS_DBG_LVL_ERR,"socket(AF_INET,SOCK_STREAM,0)::error\n");
		return ERROR;
	}

	addrblk = (struct sockaddr_in *)&(addr->addr);

	/* connect */
	res = connect(fd,(struct sockaddr *)addrblk,sizeof(struct sockaddr));

	/* error processing */
	if (res < 0) {
		os_debug_printf(OS_DBG_LVL_ERR,"connect(fd=%d)::error::cannot connect\n",fd);
		net_port_errno_set(port->id,E_NET_CANNOTCONN);
		return ERROR;
	}

	/* availability */
	port->status |= NET_PORT_STATUS_CONN;

	/* success */
	net_port_errno_set(port->id,E_NOERROR);

	os_debug_printf(OS_DBG_LVL_VER,"(fd=%d,port=%08x)::done\n",fd,port->id);

	return OKAY;
}

/*******************************************************************************
 *
 * net_tcp_bind - bind
 *
 * DESCRIPTION
 *  As long as client side calls this, just connect() call will be invocated,
 *  if he just use an option type NET_CLIENT_..... In terms of server node,
 *  this function sets up ports with both listen() and bind().
 *
 * PARAMETERS
 *  port - port structure
 *  addr - address information
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
int net_tcp_bind(net_port_st *port,net_addrinfo_t *addr)
{
	int fd;
	int res;
	int optval = 1;
	struct sockaddr_in *addrblk;

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	/* already bind ?? */
	if(NET_PORT_STATUS(port,NET_PORT_STATUS_BIND)) {
		os_debug_printf(OS_DBG_LVL_ERR,"already BIND()\n");
		return ERROR;
	}

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null address block\n");
		return ERROR;
	}

	/* socket has been done? */
	fd  = port->fd;
	if(fd<=0) {
		os_debug_printf(OS_DBG_LVL_ERR,"socket(AF_INET,SOCK_STREAM,0)::error\n");
		return ERROR;
	}

	addrblk = (struct sockaddr_in *)&(addr->addr);

	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,sizeof(optval))<0) {
		os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(fd=%d,SO_REUSEADDR...)::error\n",fd);
		return ERROR;
	}

	/* bind */
	res = bind(fd,(struct sockaddr *)addrblk,sizeof(struct sockaddr));
	if(res < 0 ) {
		/* connection failed */
		os_errno_printf();
		os_debug_printf(OS_DBG_LVL_ERR,"bind(fd=%d)::error\n",fd);
		return ERROR;
	}

	/* availability */
	port->status |= NET_PORT_STATUS_BIND;

	os_debug_printf(OS_DBG_LVL_VER,"(fd=%d,port=%08x)::done\n",fd,port->id);

	return OKAY;
}

/*******************************************************************************
 *
 * net_tcp_listen - listen
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port structure
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
int net_tcp_listen(net_port_st *port)
{
	int fd;
	int res;
	static int backlog = 5; /* statically defined */

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	/* socket has been done? */
	fd  = port->fd;
	if(fd<=0) {
		os_debug_printf(OS_DBG_LVL_ERR,"socket(AF_INET,SOCK_STREAM,0)::error\n");
		return ERROR;
	}

	res = listen(fd,backlog);
	if(res < 0 ) {
		/* connection failed */
		os_errno_printf();
		os_debug_printf(OS_DBG_LVL_ERR,"listen(fd=%d)::error\n",fd);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(fd=%d)done\n",fd);

	return OKAY;
}

/*******************************************************************************
 *
 * net_tcp_accept - accept
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
int net_tcp_accept(net_port_st *port,net_addrinfo_t *addr)
{
	int fd;
	int res;
	struct sockaddr_in *addrblk;
	unsigned int *addrsize;

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid paramer\n");
		return ERROR;
	}

	/* normal socket ??? */
	if(!NET_PORT_STATUS(port,NET_PORT_STATUS_SOCKET)) {
		os_debug_printf(OS_DBG_LVL_ERR,"not yet established\n");
		return ERROR;
	}

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null address block\n");
		return ERROR;
	}

	/* socket has been done? */
	fd  = port->fd;
	if(fd<=0) {
		os_debug_printf(OS_DBG_LVL_ERR,"socket(AF_INET,SOCK_STREAM,0)::error\n");
		return ERROR;
	}

	addrblk = (struct sockaddr_in *)&(addr->addr);

	/* addr size */
	addrsize = (unsigned int *)&(addr->addrsize);
	*addrsize = sizeof(struct sockaddr_in);

	/* do accept */
	res = accept(fd,(struct sockaddr *)addrblk,addrsize);
	if(res < 0) {
		os_debug_printf(OS_DBG_LVL_ERR,"accept(%d,port=%08x)::error\n",fd,port->id);
		os_errno_printf();
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(fd=%d,newfd=%d)done\n",fd,res);

	return res; /* newfd */
}

/*******************************************************************************
 *
 * net_tcp_send - send
 *
 * DESCRIPTION
 *  Sending a data. The data is sent out in the way of best-effort traffic.
 *  None of minimum data block is used.
 *
 * PARAMETERS
 *  port - port structure
 *  blk  - data
 *  size - data size
 *  addr - address information
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_tcp_send(net_port_st *port,void *blk,int size,net_addrinfo_t *addr)
{
	int fd;
	int amount;
	int count = 0;
	int tx_retry_count;
	int tx_retry_count_ex;
	int send_unit;
	int error;

	/* parameter check */
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null parameter\n");
		return ERROR;
	}

	/* physical channel is not yet established... */
	if(!net_port_proto_iscommunicatable(port)) {
		os_debug_printf(OS_DBG_LVL_ERR,"not yet established\n");
		net_port_errno_set(port->id,E_NET_PORT_UNESTAB);
		return ERROR;
	}

	/* reset counter */
	tx_retry_count = 0;
	tx_retry_count_ex = 0;

	fd = port->fd; /* socket descriptor */
	while(count<size) {
		amount = send(fd,(char *)blk+count,size-count, MSG_DONTWAIT);

		if(amount < 0 ) {
			/* Retry */
			error = os_errno();
			if (error != EAGAIN && error != EINTR) {
				os_debug_printf(OS_DBG_LVL_ERR,"send error (%d)\n", error);
				return ERROR;
			}

			if( ++(tx_retry_count_ex) == NET_PORT_MAX_RETRY_COUNTER_EX ) {
				os_debug_printf(OS_DBG_LVL_ERR,"(%d,%08lx,%d)::exceed max retry count(%d)\n",
				                fd,(unsigned long)blk,size,amount);
				return ERROR;
			} else {
				os_thread_sleep(1);
				continue;
			}
		}

		if(amount == 0) {
			os_thread_sleep(100000); /* 100 milliseconds */
			if( ++(tx_retry_count) == NET_PORT_MAX_RETRY_COUNTER ) {
				net_port_errno_set(port->id,E_NET_PORT_TX);
				os_debug_printf(OS_DBG_LVL_ERR,"(%d,%08lx,%d)::continued zero(%d)\n",
				                fd,(unsigned long)blk,size,amount);
				return ERROR;
			}else{
				continue;
			}
		}

		tx_retry_count = 0; /* reset counter */
		count += amount;
	}/* while(count... */

	return OKAY;
}

/*******************************************************************************
 *
 * net_tcp_recv - recv
 *
 * DESCRIPTION
 *  Receiving a data. The data is elevated in the way of best-effort traffic.
 *  None of minimum data block is used.
 *
 * PARAMETERS
 *  port - port structure
 *  blk  - data
 *  size - data size
 *  addr - address information
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
int net_tcp_recv(net_port_st *port,void *blk,int size,net_addrinfo_t *addr)
{
	int fd;
	int amount;
	int count = 0;
	int rx_retry_count;
	int recv_unit;

	/* parameter check */
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null parameter\n");
		return ERROR;
	}

	/* not yet activated?? */
	if(!net_port_proto_iscommunicatable(port)) {
		net_port_errno_set(port->id,E_NET_PORT_UNESTAB);
		os_debug_printf(OS_DBG_LVL_ERR,"not yet established\n");
		return ERROR;
	}

	/* reset counter */
	rx_retry_count = 0;

	fd = port->fd; /* socket descriptor */
	while(count<size) {
		amount = recv(fd,(char *)blk+count,size-count,0);
		if(amount < 0) {
			os_errno_printf();
			net_port_errno_set(port->id,E_NET_PORT_RX);
			os_debug_printf(OS_DBG_LVL_ERR,"recv(%d,%08lx,%d)::error(%d)\n",
			                fd,(unsigned long)blk,size,amount);
			return ERROR;
		}
		/* It may happen in blocking mode */
		if(amount == 0) {
			os_thread_sleep(100000); /* 100 milliseconds */
			if( ++(rx_retry_count) == NET_PORT_MAX_RETRY_COUNTER ) {
				net_port_errno_set(port->id,E_NET_PORT_RX);
				os_debug_printf(OS_DBG_LVL_ERR,"recv(%d,%08lx,%d)::continued zero(%d)\n",
				                fd,(unsigned long)blk,size,amount);
				return ERROR;
			}else continue;
		}
		rx_retry_count = 0; /* reset */
		count += amount;
	}/* while(count... */

	return OKAY;
}

/*******************************************************************************
 *
 * net_tcp_close - close
 *
 * DESCRIPTION
 *  Closing a socket descriptor
 *
 * PARAMETERS
 *  port - port structure
 *
 * RETURNS
 *
 * ERRORS
 *  N/A
 */
int net_tcp_close(net_port_st *port)
{
	/* parameter check */
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null parameter\n");
		return ERROR;
	}

	/* If port comes to an any situation that is operable.
	 */
	if( port->status == 0 ) {
		os_debug_printf(OS_DBG_LVL_ERR,"not yet established\n");
		return ERROR;
	}

	os_close(port->fd); /* close */

	/* turn off availability */
	port->status = 0;

	return OKAY;
}

/*******************************************************************************
*
*  I  O  C  T  L
*
*******************************************************************************/

/*
 * block mode
 ******************************************************************************/
static unsigned int net_proto_ioctl_blockmode(int fd,int mode)
{
	return os_net_block_mode_set(fd,mode);
}

/*******************************************************************************
 *
 * net_proto_ioctl - ioctl
 *
 * DESCRIPTION
 *  Creating TCP socket
 *
 * PARAMETERS
 *  port - port structure
 *  command - command
 *  arg - argument
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
int net_proto_ioctl(net_port_st *port,unsigned int command,void *arg)
{
	int optval;
	net_addrinfo_t *addrinfo;

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null port\n");
		return ERROR;
	}

	switch( command ) {

	/* get remote address information */
	case NET_PORT_GET_ADDR_INFO:
		addrinfo = (net_addrinfo_t *)arg;
		memcpy( (char *)addrinfo, (char *)&(port->addr), sizeof(net_addrinfo_t));
		return OKAY;

	/* set remote address information */
	case NET_PORT_SET_ADDR_INFO:
		addrinfo = (net_addrinfo_t *)arg;
		memcpy( (char *)&(port->addr), (char *)addrinfo, sizeof(net_addrinfo_t));
		return OKAY;

	/* get socket address information */
	case NET_PORT_GET_SOCK_ADDR_INFO:
		addrinfo = (net_addrinfo_t *)arg;
		getsockname(port->fd,(struct sockaddr *)addrinfo->addr,(socklen_t *)&(addrinfo->addrsize));
		return OKAY;

	/* block mode */
	case NET_PORT_ENABLE_NONBLOCK:
		port->block_mode = 1;
		return net_proto_ioctl_blockmode(port->fd,1);

	case NET_PORT_DISABLE_NONBLOCK:
		port->block_mode = 0;
		return net_proto_ioctl_blockmode(port->fd,0);

	/* keepalive options*/
	case NET_PORT_ENABLE_KEEPALIVE:
		optval = 1;
		if(setsockopt(port->fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&optval,sizeof(optval))<0) {
			os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(fd=%d,SO_KEEPALIVE,(ENABLE)...)::error\n",port->fd);
			return ERROR;
		}
		return OKAY;

	case NET_PORT_DISABLE_KEEPALIVE:
		optval = 0;
		if(setsockopt(port->fd,SOL_SOCKET,SO_KEEPALIVE,(char *)&optval,sizeof(optval))<0) {
			os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(fd=%d,SO_KEEPALIVE,(DISABLE)...)::error\n",port->fd);
			return ERROR;
		}
		return OKAY;

	case NET_PORT_SO_LINGER:
	{
		struct linger linger;
		linger.l_onoff   = 1;
		linger.l_linger  = 0;
		if (setsockopt(port->fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) != 0) {
			os_debug_printf(OS_DBG_LVL_ERR,"setsockopt(fd=%d,SO_LINGER,(ENABLE)...)::error\n",port->fd);
			return ERROR;
		}
		return OKAY;
	}

	default:
		os_debug_printf(OS_DBG_LVL_ERR,"unknown command=%08x\n",command);
		return ERROR;
	}
}

#if 0
/* UDP protocol block */
static net_protocol_st net_proto_udp_block =
{
	net_udp_socket,
	net_udp_connect,
	net_udp_bind,
	net_udp_listen,
	net_udp_accept,
	net_udp_send,
	net_udp_recv,
	net_udp_close,
	net_proto_ioctl
};

/* TCP protocol block */
static net_protocol_st net_proto_tcp_block =
{
	net_tcp_socket,
	net_tcp_connect,
	net_tcp_bind,
	net_tcp_listen,
	net_tcp_accept,
	net_tcp_send,
	net_tcp_recv,
	net_tcp_close,
	net_proto_ioctl
};
#endif

/*******************************************************************************
 *
 * os_net_proto_init -
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
void os_net_proto_init(void)
{
	/* UDP */
	net_proto_udp_block.net_socket  = net_udp_socket;
	net_proto_udp_block.net_connect = net_udp_connect;
	net_proto_udp_block.net_bind    = net_udp_bind;
	net_proto_udp_block.net_listen  = net_udp_listen;
	net_proto_udp_block.net_accept  = net_udp_accept;
	net_proto_udp_block.net_send    = net_udp_send;
	net_proto_udp_block.net_recv    = net_udp_recv;
	net_proto_udp_block.net_close   = net_udp_close;
	net_proto_udp_block.net_ioctl   = net_proto_ioctl;

	/* TCP */
	net_proto_tcp_block.net_socket  = net_tcp_socket;
	net_proto_tcp_block.net_connect = net_tcp_connect;
	net_proto_tcp_block.net_bind    = net_tcp_bind;
	net_proto_tcp_block.net_listen  = net_tcp_listen;
	net_proto_tcp_block.net_accept  = net_tcp_accept;
	net_proto_tcp_block.net_send    = net_tcp_send;
	net_proto_tcp_block.net_recv    = net_tcp_recv;
	net_proto_tcp_block.net_close   = net_tcp_close;
	net_proto_tcp_block.net_ioctl   = net_proto_ioctl;

	signal( SIGPIPE , SIG_IGN );

	os_debug_printf(OS_DBG_LVL_VER, "done\n");
}

