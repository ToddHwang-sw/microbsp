/*******************************************************************************
 *
 * net.c - network socket & resource management module
 *
 ******************************************************************************/
#include "oskit.h"

/* size of resource pool for network resources */
/* By default, it takes NET_MEM_CLASS_SIZE in net.h */
/*static*/ unsigned int os_network_pool_size = NET_MEM_CLASS_SIZE;

static OS_LIST *net_channel_pool; /* channel pool */
static OS_LIST *net_port_pool;    /* port pool */

/* delete upcall function */
static os_delfunc_t net_port_delete_upcall(void *arg);

/*******************************************************************************
 *
 * net_init - Initializing network module
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  N/A
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_init(void)
{
	unsigned int res;

	/* initializing protocol blocks */
	os_net_proto_init();

	/* memory pool creation */
	res = buff_create_class(NET_MEM_CLASS,NULL,os_network_pool_size);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"buff_create_class(%08x)::error\n",os_network_pool_size);
		return ERROR;
	}

	/* channel pool creation */
	net_channel_pool = os_list_create(NET_CHAN_CLASS_NAME);
	if(net_channel_pool == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_create(%s)::error\n",NET_CHAN_CLASS_NAME);
		return ERROR;
	}

	/* port pool creation */
	net_port_pool = os_list_create(NET_PORT_CLASS_NAME);
	if(net_port_pool == ERROR) {
		os_list_delete(net_channel_pool);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_create(%s)::error\n",NET_PORT_CLASS_NAME);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(pool=%08x)done\n",os_network_pool_size);

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_addrinfo_make - Organizing address information
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  addr - address structure
 *  ip   - ip address
 *  port - port number
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_addrinfo_make(net_addrinfo_t *addr,unsigned int ip,unsigned int port)
{
	struct sockaddr_in *addrblk;

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null address block\n");
		return ERROR;
	}

	/* configure */
	addr->ip = ip;
	addr->port = port;

	/* filling up sockaddr_in structure */
	addrblk = (struct sockaddr_in *)(addr->addr);
	addrblk->sin_family      = AF_INET;
	if(ip==NET_ANY_IP) {
		addrblk->sin_addr.s_addr = INADDR_ANY;
	}else{
		addrblk->sin_addr.s_addr = htonl(ip);
	}
	addrblk->sin_port        = htons(port);

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_addrinfo_cleanup - Cleaning up addrinfo
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  addr - address structure
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_addrinfo_cleanup(net_addrinfo_t *addr)
{
	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null address block\n");
		return ERROR;
	}

	memset((char *)addr,0,sizeof(net_addrinfo_t));
	addr->addrsize = sizeof(struct sockaddr_in);

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_addrinfo_get - Taking address information
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  addr - address structure
 *  ip   - ip address
 *  port - port number
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_addrinfo_get(net_addrinfo_t *addr,unsigned int *ip,unsigned int *port)
{
	struct sockaddr_in *addrblk;

	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"null address block\n");
		return ERROR;
	}

	/* configure */
	addrblk = (struct sockaddr_in *)&(addr->addr);

	/* dump into address structure */
	addr->ip   = ntohl(addrblk->sin_addr.s_addr);
	addr->port = ntohs(addrblk->sin_port);

	/* dump into parameter */
	*ip = addr->ip;
	*port = addr->port;

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_packet_catchup - Take a packet from an interface
 *
 * DESCRIPTION
 *  Receiving a packet from an interface and allocates a memory as big as its
 *  size from a resource pool NET_MEM_CLASS.
 *
 * PARAMETERS
 *  pport - port structure
 *
 * RETURNS
 *  net_packet
 *
 * ERRORS
 *  N/A
 */
static net_packet * net_port_packet_catchup(net_port_st *pport)
{
	unsigned int res;
	unsigned int payload_size;
	net_packet *pkt;
	net_protocol_st *proto;

	/* select protocol */
	proto = (net_protocol_st *)pport->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"null protocol(id=%08x,type=%d)::error\n",pport->id,pport->type);
		return NULL;
	}

	/* pkt struture */
	pkt = pport->interface_packet;
	if(!pkt) {
		os_debug_printf(OS_DBG_LVL_ERR,"interface packet(pport=%08x)::null\n",pport->id);
		return NULL;
	}

	/* header receive */
	res = net_port_proto_recv(pport->id,(unsigned char *)&(pkt->h),sizeof(net_packet_hdr),NULL);
	if(res == ERROR) {
		/* RX error */
		os_debug_printf(OS_DBG_LVL_ERR,
		                "...proto_recv(pport=%08x,sizeof(packet header))::error\n",pport->id);
		return NULL;
	}

#if 1
	os_debug_printf(OS_DBG_LVL_ERR,"RECV::pkt->h.preamble=%c%c%c%c%c%c...\n",
	                pkt->h.preamble[0],
	                pkt->h.preamble[1],
	                pkt->h.preamble[2],
	                pkt->h.preamble[3],
	                pkt->h.preamble[4],
	                pkt->h.preamble[5]);
	os_debug_printf(OS_DBG_LVL_ERR,"RECV::pkt->h.type=%08x\n",ntohl(pkt->h.type));
	os_debug_printf(OS_DBG_LVL_ERR,"RECV::pkt->h.vsp =%d\n",ntohs(pkt->h.vsport));
	os_debug_printf(OS_DBG_LVL_ERR,"RECV::pkt->h.vdp =%d\n",ntohs(pkt->h.vdport));
#endif

	/* sanity check */
	payload_size = ntohl(pkt->h.size); /* payload size  - 2005/05/09 */
	if(pport->payload_size < payload_size) {
		os_debug_printf(OS_DBG_LVL_ERR,"payload size = %d/%d::error\n",
				payload_size,pport->payload_size);
		return NULL;
	}

	os_debug_printf(OS_DBG_LVL_VER,"payload_size = %d\n",payload_size);

	/* payload receive */
	res = net_port_proto_recv(pport->id,(unsigned char *)pkt->payload,payload_size,NULL); /* net_packet_arrive */
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"...proto_recv(pport=%08x,sizeof(packet header))::error\n",pport->id);
		return NULL;
	}

	/* data sturucture fill up */
	if( strncmp((const char *)pkt->h.preamble,
	            (const char *)NET_PACKET_PREAMBLE,
	            strlen(NET_PACKET_PREAMBLE)) != 0 ) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pkt->payload);
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pkt);
		os_debug_printf(OS_DBG_LVL_ERR,"pkt->h.preamble=%c%c%c%c%c%c...::error\n",
		                pkt->h.preamble[0],
		                pkt->h.preamble[1],
		                pkt->h.preamble[2],
		                pkt->h.preamble[3],
		                pkt->h.preamble[4],
		                pkt->h.preamble[5]);
		return NULL;
	}

	/* packet structure framing */
	pkt->h.type     = ntohl(pkt->h.type);
	pkt->h.size     = ntohl(pkt->h.size);/* 2005/05/09 : unsigned short -> unsigned int */
	pkt->h.vsport   = ntohs(pkt->h.vsport);
	pkt->h.vdport   = ntohs(pkt->h.vdport);
	pkt->h.option   = ntohl(pkt->h.option);
	pkt->h.checksum = ntohl(pkt->h.checksum);
	if( pport->pktfunc )
		pkt->priv = (unsigned char *)(pport->pktfunc(pkt)); /* private field */
	else
		pkt->priv = NULL;

	return pkt;
}

/*******************************************************************************
 *
 * net_channel_comparator - comparator function
 *
 */
static unsigned int net_channel_comparator(OS_LIST_DATA *d,void *arg)
{
	net_channel_st *channel;

	if(!d || !arg) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid argument\n");
		return ERROR;
	}

	channel = (net_channel_st *)d->data;
	if(!channel) {
		os_debug_printf(OS_DBG_LVL_ERR,"null channel \n");
		return ERROR;
	}

	return (channel->cnum == *(unsigned int *)arg) ? OKAY : ERROR;
}

/*******************************************************************************
 *
 * net_channel_find - finding a channel with port #
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port id
 *  chan - channel number
 *
 * RETURNS
 *  channel ID/ERROR
 *
 * ERRORS
 *   N/A
 */
static net_channel_st * net_channel_find(net_port_st *port,unsigned int chan)
{
	OS_LIST_DATA *d;

	if(!(port->chanlist)) {
		os_debug_printf(OS_DBG_LVL_ERR,"net_channel_find(port=%08x,chan=%d)::null list\n",
		                NET_PORT_ID(port),chan);
		return ERROR;
	}

	d = os_list_find(port->chanlist, net_channel_comparator, (void *)&chan);
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_find(chan=%d)::error\n",chan);
		return ERROR;
	}
	return (net_channel_st *)d->data;
}

/*******************************************************************************
 *
 * net_channel_packet_deliver - insert a packet into the queue in a channel
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  pchan  - channel structure
 *  pkt - packet structure
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *   N/A
 */
static unsigned int net_channel_packet_deliver(net_channel_st *pchan,net_packet *rpkt)
{
	unsigned int buffid;
	unsigned int res;
	OS_MESG *msg;
	net_packet *pkt;

	/* packet check */
	if(!rpkt) {
		os_debug_printf(OS_DBG_LVL_ERR,"null pkt\n");
		return ERROR;
	}

	/* channel check */
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"null channel\n");
		return ERROR;
	}

	buffid = pchan->buff; /* buffer id */

	/* lock channel */
	os_mutex_lock( pchan->lock, 0 );

	/* copying a packet from 'rpkt' */

	/* allocating pkt structure */
	pkt = (net_packet *)buff_alloc_data(buffid,sizeof(net_packet));
	if(!pkt) {
		os_mutex_unlock( pchan->lock );
		os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(sizeof(net_packet))::error\n");
		return ERROR;
	}

	/* allocating payload */
	if(rpkt->h.size > 0) {
		pkt->payload = (unsigned char *)buff_alloc_data(buffid,rpkt->h.size);
		if(!(pkt->payload)) {
			buff_free_data(buffid,(unsigned char *)pkt);
			os_mutex_unlock(pchan->lock);
			os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(%d)::error\n",rpkt->h.size);
			return ERROR;
		}
	}

	/* header copy */
	memcpy((unsigned char *)&(pkt->h),(unsigned char *)&(rpkt->h),sizeof(net_packet_hdr));

	/* payload copy */
	if(rpkt->h.size>0)
		memcpy((unsigned char *)(pkt->payload),(unsigned char *)(rpkt->payload),rpkt->h.size);
	else
		pkt->payload = NULL;

	/* private information copy */
	pkt->priv = rpkt->priv;

	/* Now... creating a message structure for delivery... */

	/* building a message */
	msg = (OS_MESG *)buff_alloc_data(buffid,sizeof(OS_MESG));
	if(!msg) {
		buff_free_data(buffid,(unsigned char *)pkt->payload);
		buff_free_data(buffid,(unsigned char *)pkt);
		os_mutex_unlock(pchan->lock);
		os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(sizeof(OS_MESG))::error\n");
		return ERROR;
	}

	/* pointer message */
	res = os_msgq_message(msg,NET_PACKET_MESSAGE,(unsigned char *)pkt,0);
	if(!res) {
		buff_free_data(buffid,(unsigned char *)pkt->payload);
		buff_free_data(buffid,(unsigned char *)pkt);
		os_mutex_unlock(pchan->lock);
		os_debug_printf(OS_DBG_LVL_ERR,"os_msgq_message()::error\n");
		return ERROR;
	}

	/* packet insert */
	res = os_msgq_send(pchan->msgq,msg);
	if(res == ERROR) {
		buff_free_data(buffid,(unsigned char *)pkt->payload);
		buff_free_data(buffid,(unsigned char *)pkt);
		os_mutex_unlock( pchan->lock );
		os_debug_printf(OS_DBG_LVL_ERR,"os_msgq_send(channel=%08x)\n",pchan->cnum);
		return ERROR;
	}

	pchan->packets += 1; /* packets */
	pchan->total_bytes += pkt->h.size; /* payload size */

	/* unlock channel */
	os_mutex_unlock( pchan->lock );

	os_debug_printf(OS_DBG_LVL_VER,"(id=%08x)::done\n",pchan->cnum);

	return OKAY;
}

/*******************************************************************************
 *
 * net_null_packet_push - Pushing null packets to a channel
 *
 * DESCRIPTION
 *
 */
static void net_null_packet_push(OS_LIST_DATA *d)
{
	net_channel_st *pchan;
	unsigned int buffid;
	unsigned int res;
	net_packet *pkt;

	if(!d->data) {
		os_debug_printf(OS_DBG_LVL_ERR,"null list item\n");
		return;
	}

	/* channel structure */
	pchan = (net_channel_st *)d->data;

	/* buffer id */
	buffid = pchan->buff;

	/* allocating a packet */
	pkt = (net_packet *)buff_alloc_data(buffid,sizeof(net_packet));
	if(!pkt) {
		os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(sizeof(net_packet))::error\n");
		return;
	}

	/* packet structure */
	memcpy(pkt->h.preamble,NET_PACKET_PREAMBLE,strlen(NET_PACKET_PREAMBLE));
	pkt->h.type = htonl(NET_PACKET_NULL_TYPE);
	pkt->h.size = 0;
	pkt->h.vsport = 0;
	pkt->h.vdport = 0;
	pkt->h.option = 0;
	pkt->h.checksum = 0;
	pkt->payload = NULL;

	res = net_channel_packet_deliver(pchan,pkt);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"net_channel_packet_deliver\n");
	}

	/* freeing current packet */
	buff_free_data(buffid,(unsigned char *)pkt);

	return;

}

/*******************************************************************************
 *
 * net_port_bcast_channel - Broadcasting a packet to all channels
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  port - port structure
 *  pkt  - packet
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
/*static*/ void net_port_bcast_channel(net_port_st *port)
{
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid argument\n");
	}
	else {
		os_debug_printf(OS_DBG_LVL_VER,"net_port_bcast_channel(port=%08x)\n",port->id);
		os_list_navigate(port->chanlist,NULL,net_null_packet_push);
	}
}

/*******************************************************************************
 *
 * net_port_packet_handler - Just receiving packets
 *
 * DESCRIPTION
 *  Endless loop to listen up an interface and takes an incoming packets
 *
 * PARAMETERS
 *  pport - port structure
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
static os_thrfunc_t net_port_packet_handler(void *arg)
{
	unsigned int id;
	unsigned int res;
	unsigned short chan;
	net_packet *pkt;
	net_channel_st *pchan;
	net_port_st *pport = (net_port_st *)arg;

	if(!pport) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid argument\n");
		return ERROR;
	}

	/* clean up error flag */
	net_port_errno_set(pport->id,E_NOERROR);

	/* creating interface_packet */

	/* interface packet */
	if(!pport->interface_packet) {
		pport->interface_packet = (net_packet *)buff_alloc_data(NET_MEM_CLASS,sizeof(net_packet));
		if(!pport->interface_packet) {
			buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
			os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(sizeof(net_packet))::error\n");
			return ERROR;
		}

		/* interface packet - payload */
		pport->interface_packet->payload = (unsigned char *)buff_alloc_data(NET_MEM_CLASS,pport->payload_size);
		if(!pport->interface_packet->payload) {
			buff_free_data(NET_MEM_CLASS,(unsigned char *)(pport->interface_packet));
			buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
			os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(%d)::error\n",pport->payload_size);
			return ERROR;
		}
		os_debug_printf(OS_DBG_LVL_VER,"(%08x)interface packet buffer created \n",pport->id);
	} /* if(!pport->interface_packet */

	while(1) {
		/* pkt is allocated from NET_MEM_CLASS pool */
		pkt = net_port_packet_catchup(pport);
		if(!pkt) {
			os_debug_printf(OS_DBG_LVL_ERR,"net_port_packet_catchup(port=%08x)::error\n",pport->id);
			/*
			 * Checking if any error has been found...
			 */
			if( net_port_errno_get(pport->id) != E_NOERROR ) {
				os_debug_printf(OS_DBG_LVL_ERR,"port(port=%08x)::real error\n",pport->id);
				/* killing port thread structure */
				/* net_port_delete(pport->id); */
				return ERROR; /* return immediately */
			}
			continue;
		}
		chan = pkt->h.vdport; /* destination channel */

		/* channel disable is configured.. */
		if(pport->option & NET_PORT_DISABLE) {
			os_debug_printf(OS_DBG_LVL_VER,"(port=%d)=CHANNEL disabled\n",pport->id);
			/* wait until port will be released from disable masking */
			while( pport->option & NET_PORT_DISABLE ) {
				os_thread_sleep(1000);
			}
			continue;
		}

		/* looking for channel */
		pchan = net_channel_find(pport,chan);
		if(!pchan) {
			os_debug_printf(OS_DBG_LVL_ERR,"net_channel_find(port=%x,chan=%d)::error\n",pport->id,chan);
			os_thread_sleep(1000); 
			continue;
		}

		/* push a packet to channel structure */
		res = net_channel_packet_deliver(pchan,pkt);
		if(res == ERROR) {
			os_debug_printf(OS_DBG_LVL_ERR,"net_channel_packet_deliver(channel=%08x,port=%08x)::error\n",
			                pchan->cnum,pport->id);
			os_thread_sleep(1000); 
			continue;
		}

		/* requested by jspark */
		if(pport->catchup_delay) 
			os_thread_sleep(pport->catchup_delay);
	}   // end of while
}

/*******************************************************************************
 *
 * net_port_proto_socket - socket()
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
unsigned int net_port_proto_socket(unsigned int portid)
{
	net_port_st *port;
	net_protocol_st *proto;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* protocol */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid protocol (%08x)\n",portid);
		return ERROR;
	}

	/* creating network instance (socket) */
	port->fd = proto->net_socket(port);
	if(port->fd==ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"proto(type=%d)->net_socket(%08x)::error\n",
		                port->type,port->id);
		return ERROR;
	}
	port->sig = 0; /* clean up signal */

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_proto_close- close()
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
unsigned int net_port_proto_close(unsigned int portid)
{
	net_port_st *port;
	net_protocol_st *proto;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* protocol */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid protocol (%08x)\n",portid);
		return ERROR;
	}

	/* close socket */
	return proto->net_close(port);
}

/*******************************************************************************
 *
 * net_port_proto_connect - connect()
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
unsigned int net_port_proto_connect(unsigned int portid,net_addrinfo_t *addr)
{
	net_port_st *port;
	net_addrinfo_t *addrbase;
	net_protocol_st *proto;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* protocol */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid protocol (%08x)\n",portid);
		return ERROR;
	}

	/* address container */
	addrbase = (addr) ? addr : &(port->addr); /* remote address */

	/* connect */
	return proto->net_connect(port,addrbase);
}

/*******************************************************************************
 *
 * net_port_proto_bind - bind()
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
unsigned int net_port_proto_bind(unsigned int portid,net_addrinfo_t *addr)
{
	net_port_st *port;
	net_addrinfo_t *addrbase;
	net_protocol_st *proto;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* protocol */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid protocol (%08x)\n",portid);
		return ERROR;
	}

	/* address container */
	addrbase = (addr) ? addr : &(port->addr); /* local address */

	/* bind */
	return proto->net_bind(port,addrbase);
}

/*******************************************************************************
 *
 * net_port_proto_listen - listen()
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
unsigned int net_port_proto_listen(unsigned int portid)
{
	net_port_st *port;
	net_protocol_st *proto;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* protocol */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid protocol (%08x)\n",portid);
		return ERROR;
	}

	/* listen */
	return proto->net_listen(port);
}

/*******************************************************************************
 *
 * net_port_proto_accept - accept()
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
unsigned int net_port_proto_accept(unsigned int portid,net_addrinfo_t *addr)
{
	net_port_st *port;
	net_addrinfo_t *addrbase;
	net_protocol_st *proto;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* protocol */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid protocol (%08x)\n",portid);
		return ERROR;
	}

	/* address container */
	addrbase = (addr) ? addr : &(port->addr); /* remote address */

	/* accept */
	return proto->net_accept(port,addrbase);
}

/*******************************************************************************
 *
 * net_port_proto_recv - Receiving a packet from directly port structure
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid  - port ID
 *  buffer  - buffer
 *  size    - size
 *  addr    - address information
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *   N/A
 */
unsigned int net_port_proto_recv(unsigned int portid,unsigned char *buff,unsigned int sz,
                                 net_addrinfo_t *addr)
{
	int res;
	net_port_st *port;
	net_protocol_st *proto;
	net_addrinfo_t *addrbase;

	if(!buff) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid buffer\n");
		return ERROR;
	}

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* protocol block */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"protocol block null\n");
		return ERROR;
	}

	/* address container */
	addrbase = (addr) ? addr : &(port->addr); /* remote address */

	/* header sending */
	res = proto->net_recv(port,(void *)buff,sz,addrbase);
	if(res != OKAY) {
		os_debug_printf(OS_DBG_LVL_ERR,"proto->net_recv(%08x,header)::error\n",port->id);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"portid=%08x,size=%d(done)\n",port->id,sz);

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_proto_send - Sending a packet to directly port structure
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid  - port ID
 *  buffer  - buffer
 *  size    - size
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *   N/A
 */
unsigned int net_port_proto_send(unsigned int portid,unsigned char *buff,unsigned int sz,
                                 net_addrinfo_t *addr)
{
	int res;
	net_port_st *port;
	net_protocol_st *proto;
	net_addrinfo_t *addrbase;

	if(!buff) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid buffer\n");
		return ERROR;
	}

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* protocol block */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"protocol block null\n");
		return ERROR;
	}

	/* address container */
	addrbase = (addr) ? addr : &(port->addr); /* remote address */

	/* header sending */
	res = proto->net_send(port,(void *)buff,sz,addrbase);
	if(res != OKAY) {
		os_debug_printf(OS_DBG_LVL_ERR,"proto->net_send(%08x,header)::error\n",port->id);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"portid=%08x,size=%d(done)\n",port->id,sz);

	return OKAY;
}

/* defined in proto.c */
extern unsigned int net_port_proto_iscommunicatable(net_port_st *);

/*******************************************************************************
 *
 * net_port_proto_isconnected - connected()
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
unsigned int net_port_proto_isconnected(unsigned int portid)
{
	net_port_st *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* availability */
	return net_port_proto_iscommunicatable(port);
}

/*******************************************************************************
 *
 * net_port_proto_isblockmode - tests if non-block/block mode
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  1 : block     mode
 *  0 : non-block mode
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_proto_isblockmode(unsigned int portid)
{
	net_port_st *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* block_mode -> 1 : non-block mode */
	/* block_mode -> 0 : block     mode */
	return !(port->block_mode);
}

/*******************************************************************************
 *
 * net_port_proto_packet_handler_start -
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
unsigned int net_port_proto_packet_handler_start(unsigned int portid)
{
	unsigned char tname[STR_LEN];
	unsigned int res;
	net_port_st *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* checking up any other thread */
	if(port->func) {
		os_debug_printf(OS_DBG_LVL_ERR,"already function is created(%08x)\n",portid);
		return ERROR;
	}

	res = 1; /* available */
	if( net_port_change(port->id,
	                    NET_PORT_AVAIL_SET,
	                    (void *)&res ) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"net_port_change(..AVAILABLE..,port=%08x)::error\n",port->id);
		return ERROR;
	}

	/* spawns thread */
	sprintf((char *)tname,(const char *)"%s%s%d",THREAD_NAME,"PORT",portid+1);
	res = os_thread_create((const char *)tname,(os_thrfunc_t)net_port_packet_handler,(void *)port,
	                       (os_delfunc_t)net_port_delete_upcall,0);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"net_port_change(..FD_SET..,new_port=%08x)::error\n",portid);
		return ERROR;
	}

	/* function */
	port->func = res;

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)done\n",portid);

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_listener - port listener
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
static os_thrfunc_t net_port_listener(void *arg)
{
	unsigned int res;
	unsigned int portid;
	net_port_st *pport;
	net_protocol_st *proto;

	pport = (net_port_st *)arg; /* port structure */
	if(!pport) {
		/* null port structure */
		os_debug_printf(OS_DBG_LVL_ERR,"null port structure\n");
		net_port_delete(pport->id);
		return ERROR;
	}

	/* port id */
	portid = pport->id;

	/* select protocol */
	proto = (net_protocol_st *)pport->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"null protocol(type=%d)::error\n",pport->type);
		net_port_delete(pport->id);
		return ERROR;
	}

	res = 1; /* available */
	if( net_port_change(portid,
	                    NET_PORT_AVAIL_SET,
	                    (void *)&res ) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"net_port_change(..AVAILABLE..,port=%08x)::error\n",pport->id);
		net_port_delete(portid);
		return ERROR;
	}

	/* upcall invocation at client side */
	if(pport->upcall) { pport->upcall(NET_PORT_ID(pport),NET_MESG_NEW_PORT,NULL); }

	/* takes a packet from an interface */
	net_port_packet_handler(pport);

	os_thread_delete(os_thread_self()); /* doesn't reach to here */
	return ERROR; /* doesn't reach to here */
}

/* declaration - defined below...*/
static unsigned int net_port_resource_cleanup(net_port_st *);

/*******************************************************************************
 *
 * net_port_server - port server
 *
 * DESCRIPTION
 *  For TCP connection, it spawns handler function for each channel connection,
 *  but one handler continues to listening the port as long as UDP connection.
 *
 * PARAMETERS
 *  arg - port structure
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
static os_thrfunc_t net_port_server(void *arg)
{
	unsigned int res;
	unsigned int ptype;
	unsigned int portid;
	net_port_st *pport;
	net_port_st *nport;
	net_protocol_st *proto;
	net_addrinfo_t addrinfo;
	unsigned int nportid; /* new port */
	unsigned char tname[STR_LEN];

	pport = (net_port_st *)arg; /* port structure */
	if(!pport) {
		/* null port structure */
		os_debug_printf(OS_DBG_LVL_ERR,"null port structure\n");
		net_port_delete(pport->id);
		return ERROR;
	}

	/* port id */
	portid = pport->id;

	/* select protocol */
	proto = (net_protocol_st *)pport->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"null protocol(type=%d)::error\n",pport->type);
		net_port_delete(pport->id);
		return ERROR;
	}

	/* protocol testing */
	ptype = os_net_proto_identify(pport);
	if(ptype == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"protocol block is invalid\n");
		net_port_delete(pport->id);
		return ERROR;
	}

	/* UDP communication basically shares with one network socket descriptor
	 * with many other logical channels unlikely with TCP protocol. Just
	 * listening the socket is the simplest way.
	 */
	/* UDP server */
	if(ptype ==NET_PORT_PROTO_UDP) {

		res = 1; /* available */
		if( net_port_change(portid,
		                    NET_PORT_AVAIL_SET,
		                    (void *)&res ) == ERROR ) {
			os_debug_printf(OS_DBG_LVL_ERR,"net_port_change(..AVAILABLE..,port=%08x)::error\n",pport->id);
			net_port_delete(pport->id);
			return ERROR;
		}

		/* upcall invocation to inform of new port creation */
		if(pport->upcall) {
			pport->upcall(NET_PORT_ID(pport),NET_MESG_NEW_PORT,NULL);
		}

		/* creating a new thread for packet receiver */
		net_port_packet_handler((void*)pport);

		return ERROR;
		/* unreachable to here !! */
	}

	/* TCP communication allocates every different socket descriptor for
	 * each logical channel, thus it is needed to handle each socket
	 * descriptor.
	 */

	/* TCP server */
	if(ptype == NET_PORT_PROTO_TCP) {

TCP_RETRY_FOR_SERVER:
		os_debug_printf(OS_DBG_LVL_VER,"..Now start accept(%08x)!!\n",pport->id);
		/* accept */
		net_addrinfo_cleanup(&addrinfo);
		res = net_port_proto_accept(portid,&addrinfo);

		/* res must be communication channel descriptor (socket) */
		if(res == ERROR) {
			os_debug_printf(OS_DBG_LVL_ERR,"...proto_accept(%08x)::error\n",pport->id);
			net_port_delete(portid);
			return ERROR;
		}

		os_debug_printf(OS_DBG_LVL_VER,"..Accept done !! (%08x)!!\n",pport->id);

		switch( pport->type ) {

		/* multi server */
		case NET_PORT_TYPE_SERVER:

			/* Copying a port with a new socket fd = res */
			if( (nportid=net_port_duplicate(NET_PORT_ID(pport),
			                                res, /* new socket returned from accept */
			                                NET_PORT_STATUS_FULL_SERVER) /* new status */
			     ) == ERROR) {
				os_debug_printf(OS_DBG_LVL_ERR,"net_port_duplicate(pport=%08x)::error\n",pport->id);
				net_port_delete(NET_PORT_ID(pport));
				goto TCP_RETRY_FOR_SERVER; /* retrying... */
			}
			os_debug_printf(OS_DBG_LVL_VER,"port copy from %08x to %08x\n",pport->id,nportid);

			/* new port structure */
			nport = net_port_search(nportid);
			if(!nport) {
				os_debug_printf(OS_DBG_LVL_ERR,"net_port_search(nport=%08x)::error\n",nportid);
				net_port_resource_cleanup(nport);
				net_port_delete(pport->id);
				goto TCP_RETRY_FOR_SERVER; /* retrying... */
			}

			/* upcall invocation to inform of new port creation */
			if(pport->upcall) { 
				pport->upcall(NET_PORT_ID(pport),NET_MESG_ACCEPT,(void *)&(nportid)); 
			}

			/* TODO - ??? */
			if(nportid == 0xffffffff) {
				os_debug_printf(OS_DBG_LVL_ERR,"net_port_search(nport=%08x)::error\n",nportid);
				nport->upcall = NULL; /* silent cleanup */
				net_port_resource_cleanup(nport);
				net_port_proto_close(nport->fd);
				goto TCP_RETRY_FOR_SERVER; /* retrying... */
			}

			/* upcall invocation to inform of new port service */
			if(pport->upcall)
				pport->upcall(NET_PORT_ID(pport),NET_MESG_NEW_PORT,(void *)&(nportid)); 

			/* attaching handler */
			if( net_port_proto_packet_handler_start(nportid) == ERROR ) {
				os_debug_printf(OS_DBG_LVL_ERR,"net_port_proto_packet_handler_start(%08x)::error\n",nportid);
				net_port_resource_cleanup(nport);
				net_port_delete(NET_PORT_ID(pport));
				return ERROR;
			}

			goto TCP_RETRY_FOR_SERVER; /* retrying... */

		default:
			os_debug_printf(OS_DBG_LVL_ERR,"..unknown TCP type!!!! (%08x)!!\n",pport->id);
			return ERROR;
		} /* switch (.. MULTI_SERVER ) */

	} /* if( ... PROTO_TCP) */

	os_debug_printf(OS_DBG_LVL_VER,"..Net_server_.. done !!!! (%08x)!!\n",pport->id);
	return ERROR;
}

/*******************************************************************************
 *
 * net_port_search - Finding a port structure
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid - port id
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
net_port_st * net_port_search(unsigned int portid)
{
	net_port_st *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return NULL;
	}

	return port;
}


/*******************************************************************************
 *
 * net_port_change - Chainging a port structure
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
unsigned int net_port_change(unsigned int portid,unsigned int flag,void *value)
{
	net_port_st   *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

/* macro for status update */
#define UPDATE_STATUS_FLAG_MACRO(v,flg)          \
	if((v) == 0)                     \
		port->status &= ~(flg);  \
	else                             \
		port->status |= (flg);

	switch( flag ) {
	case NET_PORT_FD_SET:
		port->fd = *(unsigned int *)value;  /* fd change */
		return OKAY;
	case NET_PORT_AVAIL_SET:
	{
		unsigned int val = *(unsigned int *)value;

		UPDATE_STATUS_FLAG_MACRO(val,NET_PORT_STATUS_ACTIVE)

		switch(port->type) {
		case NET_PORT_TYPE_CLIENT:
			/* client... */
			UPDATE_STATUS_FLAG_MACRO(val,NET_PORT_STATUS_CONN)
			break;

		case NET_PORT_TYPE_SERVER:
			/* server... */
			UPDATE_STATUS_FLAG_MACRO(val,NET_PORT_STATUS_BIND)
			break;

		default:
			os_debug_printf(OS_DBG_LVL_ERR,"Undefined type (%08x)\n",port->id);
			return ERROR;
		}

		return OKAY;
	}
	case NET_PORT_BUFF_SET: /* N/A */
		return OKAY;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)done\n",portid);

	return ERROR;
}

/*******************************************************************************
 *
 * net_port_create - Creating port structure
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  type    - type
 *  proto   - protocol
 *  addr    - address information
 *  pktfunc - function invocated for every incoming packet
 *  upcall  - upcall function invocated in specific reason
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_create(
	unsigned int type,
	unsigned int proto,
	net_addrinfo_t *addr,
	void * (*pktfunc)(void *),
	void (*upcall)(unsigned int,unsigned int,void *))
{
	unsigned char tname[STR_LEN];
	unsigned int portid;
	net_port_st *pport;

	/* sanity check */
	if(!addr) {
		os_debug_printf(OS_DBG_LVL_ERR,"empty address information\n");
		return ERROR;
	}

	/* creating a port structure */
	pport = (net_port_st *)buff_alloc_data(NET_MEM_CLASS,
	                                       sizeof(net_port_st));
	if(!pport) {
		os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(sizeof(net_port_st))::error\n");
		return ERROR;
	}

	/* initializing */
	pport->type   = type;
	pport->status = 0; /* not yet available - status clearing */
	pport->fd     = 0;
	pport->sig    = 0;
	pport->payload_size = NET_MAX_PAYLOAD;
	pport->pktfunc= pktfunc; /* packet function */
	pport->upcall = upcall; /* upcall function */
	memcpy( (char *)&(pport->addr), (char *)addr, sizeof(net_addrinfo_t) ); /* address information */

	/*
	 * Initially, this packet buffer had been allocated within this function,
	 * but we changed to have it done at the ahred of packet_catchup since
	 * the maximum size of packet for this function might be decided later.
	 * ToddHWang 
	 */
	pport->interface_packet = NULL;

	/* protocol */
	pport->proto  = (void *)os_net_proto_select(proto);
	if(pport->proto == NULL) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
		os_debug_printf(OS_DBG_LVL_ERR,"os_net_proto_select(%08x)::error\n",proto);
		return ERROR;
	}

	/* inserting it into a port pool */
	portid = os_list_item_insert( net_port_pool, (void *)pport );
	if(portid == ERROR) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(port)::error\n");
		return ERROR;
	}
	/* portid = ID of port */
	pport->id        = portid;

	/* empty option */
	pport->option    = 0;

	/* null function */
	pport->func      = 0;

	/* blocking mode */
	pport->block_mode= 0;

	/* copy count */
	pport->copycount = 0;

	/* delay */
	pport->catchup_delay = 0;

	/* port name */
	sprintf((char *)tname,(const char *)"nPORT%08x",portid);
	strncpy((char *)pport->name,(const char *)tname,STR_LEN);

	/* mutex for channel */
	sprintf((char *)tname,(const char *)"%s%08x","PORT",portid);
	pport->lock = os_mutex_create((const char *)tname);
	if(pport->lock == ERROR) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
		os_list_item_delete(net_port_pool,portid);
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_create(port=%08x)::error\n",portid);
		return ERROR;
	}

	/* channel list */
	sprintf((char *)tname,(const char *)"%s%08x",NET_CHAN_NAME,portid);
	pport->chanlist = os_list_create((const char *)tname);
	if(!pport->chanlist) {
		os_mutex_delete(pport->lock);
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
		os_list_item_delete(net_port_pool,portid);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_create(chanels at %08x)::error\n",portid);
		return ERROR;
	}

	/* errno */
	net_port_errno_set(pport->id,E_NOERROR);

	/* success */
	os_debug_printf(OS_DBG_LVL_VER,"(ip=%08x,port=%d)(%08x)done\n",
	                pport->addr.ip,pport->addr.port, portid);

	return portid; /* port id */
}

/*******************************************************************************
 *
 * net_port_duplicate - Copying a port structure
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  oportid   - original port id
 *  newsock   - new socket
 *  newstatus - new status of the new socket
 *
 * RETURNS
 *  NULL/net_port_st
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_duplicate(unsigned int oportid,int newsock,unsigned int newstatus)
{
	unsigned char tname[STR_LEN];
	unsigned int portid;
	net_port_st *pport;
	net_port_st *oport;

	/* port information */
	oport = (net_port_st *)os_obj_find(net_port_pool,oportid);
	if(!oport) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",oportid);
		return ERROR;
	}

	/* creating a port structure */
	pport = (net_port_st *)buff_alloc_data(NET_MEM_CLASS,sizeof(net_port_st));
	if(!pport) {
		os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(sizeof(net_port_st))::error\n");
		return ERROR;
	}

	/* initializing */
	pport->type   = oport->type;  /* type */
	memcpy( (char *)&(pport->addr), (char *)&(oport->addr), sizeof(net_addrinfo_t) ); /* address */
	if(newsock != 0) {
		/* new socket is assigned */
		pport->status = newstatus;  /* new status */
		pport->fd     = newsock;    /* new socket */
	}else{
		/* may be dangerous */
		pport->status = 0;          /* new status */
		pport->fd     = 0;          /* new socket */
	}
	pport->sig    = 0;            /* signal */
	pport->pktfunc= oport->pktfunc; /* packet function */
	pport->upcall = oport->upcall; /* upcall function */
	pport->proto  = oport->proto; /* protocol */
	pport->func   = 0;            /* service function */
	pport->option = 0;            /* option clear */
	pport->block_mode = 0;        /* blocking mode */
	pport->catchup_delay = 0;     /* delay time */
	pport->payload_size = oport->payload_size; /* payload */

	/* interface packet comes to be allocated later by the
	 * same reason in port_create. */
	pport->interface_packet = NULL;

	/* copycount */
	oport->copycount += 1;
	pport->copycount = oport->copycount;

	/* inserting it into a port pool */
	portid = os_list_item_insert( net_port_pool, (void *)pport );
	if(portid == ERROR) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(port)::error\n");
		return ERROR;
	}
	/* portid = ID of port */
	pport->id    = portid;

	/* port name */
	sprintf((char *)tname,(const char *)"nPORT%08x",portid+(256+pport->copycount)); /* new port number */
	strncpy((char *)pport->name,(const char *)tname,STR_LEN);

	/* mutex for channel */
	sprintf((char *)tname,(const char *)"%s%08x","PORT",portid);
	pport->lock = os_mutex_create((const char *)tname);
	if(pport->lock == ERROR) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
		os_list_item_delete(net_port_pool,portid);
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_create(port=%08x)::error\n",portid);
		return ERROR;
	}

	/* channel list */
	sprintf((char *)tname,(const char *)"%s%08x",NET_CHAN_NAME,portid);
	pport->chanlist = os_list_create((const char *)tname);
	if(!pport->chanlist) {
		os_mutex_delete(pport->lock);
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pport);
		os_list_item_delete(net_port_pool,portid);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_create(chanels at %08x)::error\n",portid);
		return ERROR;
	}

	/* clean up error code */
	net_port_errno_set(pport->id,E_NOERROR);

	/* success */
	os_debug_printf(OS_DBG_LVL_VER,"(ip=%08x,port=%d)(%08x)done\n",
	                pport->addr.ip,pport->addr.port,portid);

	return portid; /* port structure*/
}

/*******************************************************************************
 *
 * net_port_errno_set - Setting error flag
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid  - port id
 *  errcode - error code
 *
 * RETURNS
 *  N/A
 *
 * ERRORS
 *  N/A
 */
void net_port_errno_set(unsigned int portid,unsigned int errcode)
{
	net_port_st *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return;
	}

	/* error code */
	port->port_errno |= errcode;

	return;
}

/*******************************************************************************
 *
 * net_port_errno_get - Getting error flag
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid  - port id
 *
 * RETURNS
 *  (-1)         - invalid portid
 *  E_xxxx   - error code
 *
 * ERRORS
 *  N/A
 */
int net_port_errno_get(unsigned int portid)
{
	net_port_st *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return (-1);
	}

	return port->port_errno;
}

/*******************************************************************************
 *
 * net_port_delete_upcall - Port delete function
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  arg - port delete function
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
static os_delfunc_t net_port_delete_upcall(void *arg)
{
	net_port_st *port = (net_port_st *)arg;
	net_protocol_st *proto;

	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null port\n");
		return ERROR;
	}

	/* protocol block */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"null protocol in %08x\n",port->id);
		return ERROR;
	}

	/* resource clean up */
	if(net_port_resource_cleanup(port) == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"net_port_resource_cleanup(%08x)::error\n",port->id);
		return ERROR;
	}

	net_port_proto_close(NET_PORT_ID(port)); /* 08062019 */

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)done\n",port->id);

	return (os_delfunc_t)OKAY;
}

/*******************************************************************************
 *
 * net_port_activate - Spawning a RX threads
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid - port id
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_activate(unsigned int portid)
{
	net_addrinfo_t addr_local;
	unsigned char tname[STR_LEN];
	net_port_st *port;
	os_thrfunc_t proc;
	unsigned int res;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* select proper RX function */
	switch(port->type) {
	case NET_PORT_TYPE_CLIENT:
		if (port->fd <= 0) {
			/*
			 * channel setup
			 */
			/* creating network instance (socket) */
			if((res=net_port_proto_socket(portid))==ERROR) {
				os_debug_printf(OS_DBG_LVL_ERR,"...proto_socket(type=%d,%08x)::error\n",port->type,portid);
				net_port_resource_cleanup(port);
				return ERROR;
			}

			/* bind */
			net_addrinfo_make(&addr_local,NET_ANY_IP,0);
			if( net_port_proto_bind(portid,&addr_local) == ERROR ) {
				os_debug_printf(OS_DBG_LVL_ERR,"bind(type=%d,%08x) error\n",port->type,portid);
				net_port_resource_cleanup(port);
				return ERROR;
			}

			/* connect / etc... */
			if((res=net_port_proto_connect(portid,&(port->addr)))==ERROR) {
				os_debug_printf(OS_DBG_LVL_ERR,"...proto_connect(type=%d,%08x)::error\n",port->type,port->id);
				net_port_resource_cleanup(port);
				return ERROR;
			}
		}
		/* handler */
		proc = (os_thrfunc_t)&net_port_listener;
		break;
	case NET_PORT_TYPE_SERVER:
		if (port->fd <= 0) {
			/*
			 * channel setup
			 */
			/* creating network instance (socket) */
			if((res=net_port_proto_socket(portid))==ERROR) {
				os_debug_printf(OS_DBG_LVL_ERR,"...proto_socket(type=%d,%08x)::error\n",port->type,portid);
				net_port_resource_cleanup(port);
				return ERROR;
			}

			/* bind ... */
			if((res=net_port_proto_bind(portid,&(port->addr)))==ERROR) {
				os_debug_printf(OS_DBG_LVL_ERR,"...proto_bind(type=%d,%08x)::error\n",port->type,portid);
				net_port_resource_cleanup(port);
				return ERROR;
			}

			/* listen / etc... */
			if((res=net_port_proto_listen(portid))==ERROR) {
				os_debug_printf(OS_DBG_LVL_ERR,"...proto_listen(type=%d,%08x)::error\n",port->type,portid);
				net_port_resource_cleanup(port);
				return ERROR;
			}
		}
		/* handler */
		proc = (os_thrfunc_t)&net_port_server;
		break;
	default:
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port type(%08x)\n",portid);
		return ERROR;
	}

	/* controller creation */
	sprintf((char *)tname,(const char *)"portT%08x",portid);

	/*
	 * restartable function...
	 */
	res = os_thread_create((const char *)tname,proc,(void *)port,(os_delfunc_t)net_port_delete_upcall,0);
	if(res == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_thread_create(%s,%08x)::error\n",port->name,portid);
		net_port_resource_cleanup(port);
		return ERROR;
	}

	/* new port function */
	port->func = res;

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_resource_cleanup - deleting a network port structure
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid - port number
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
static unsigned int net_port_resource_cleanup(net_port_st *port)
{
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null port\n");
		return ERROR;
	}

	/* upcall invocation to inform of port deletion */
	if(port->upcall) {
		port->upcall(NET_PORT_ID(port),NET_MESG_DEL_PORT,(void *)&(port->sig));
	}

	/* ToddHwang */
	if(os_mutex_lock(port->lock,0) == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"trying to lock failed for (port=%08x)::error\n",port->id);
	}

	/* closing socket */
	/*
	 * Socket fd may be cleaned by two cases; occasional socket closedown and
	 * intentional port deletion. When a socket fd has been cleared by the system itself,
	 * and SIGPIPE signalled, socket has been already cleared in net_port_delete.
	 *
	 */
	net_port_proto_close(port->id);

	/* clean up list */
	if( os_list_item_delete(net_port_pool,port->id) == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_delete(port=%08x)::error\n",port->id);
		return ERROR;
	}

	/* deleting channel list  - It may occur warning message */
	/* Actual channels are not deleted, it should be up to programmer's will. */
	if( os_list_delete(port->chanlist) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_delete(chanlist=%p)::error\n",port->chanlist);
		return ERROR;
	}

	/* deleting TX mutex */
	if( os_mutex_delete(port->lock) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_delete(port=%08x)::error\n",port->id);
		return ERROR;
	}

	/* interface_packet might be only NULL */
	if(port->interface_packet) {
		/* interface packet container deleting... - payload */
		buff_free_data(NET_MEM_CLASS,(unsigned char *)port->interface_packet->payload);

		/* interface packet container deleting... */
		buff_free_data(NET_MEM_CLASS,(unsigned char *)port->interface_packet);
	}

	/* clean up port structure */
	buff_free_data(NET_MEM_CLASS,(unsigned char *)port);

	os_debug_printf(OS_DBG_LVL_VER,"done\n");

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_delete - deleting a network port
 *
 * DESCRIPTION
 *  - Port deleting mechanism is changed to derive thread-self deletion methodology.
 *  Mainly, it kills up system-provided socket fd for deleting a port instance.
 *  Actual resource cleanup has been moved to an upcall function of a packet handler
 *  assigned to each port. When user is free of the port packet handler system by
 *  only using APIs begging with "net_port_proto_xxxx", this function directly
 *  clean up the resource by calling net_xxx_resource_cleanup().
 *
 * PARAMETERS
 *  portid - port number
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_delete(unsigned int portid)
{
	unsigned int deleted_func_id;
	net_port_st *port;
	unsigned char resource_tobecleaned = 0; /* flag */

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* TODO - Required??? */
	/* broadcasting a message */
	// net_port_bcast_channel(port);

	/* flag setting */
	if(!port->func) {
		/*
		 * In case of any handler was associated with the port, we don't
		 * need to call net_port_resource_cleanup, since it may be
		 * incurred from the deletion upcall function of the thread for the handler.
		 *
		 */
		resource_tobecleaned = 1;
	}

	/* directly deleting a socket fd  */
	/* closing socket */
	/*
	 * After closing a port, SIGPIPE has been caused from a system in Windows/Linux.
	 * If any other packet handler thread was in alive, it will be deleted by itself.
	 * thread delete function will be invoked and resource_cleanup was invoked automatically.
	 *
	 */
	net_port_proto_close(portid);

	/*
	 *
	 * When a control reaches to here, the resource of a port may or may not be
	 * cleaned depending on the existence of handler function. Suppose that a user
	 * uses only APIs beginning with "net_port_proto_xxxx" then, handler function
	 * was not created and handled, we have to intentionally clean up the resouces
	 * for the port, since there is no way to call resource_cleanup function in delete
	 * upcall routine.
	 */
	if(resource_tobecleaned) {
		os_debug_printf(OS_DBG_LVL_ERR,"resource not to be cleaned up\n");
		/* direct resource cleanup */
		if(net_port_resource_cleanup(port) == ERROR) {
			os_debug_printf(OS_DBG_LVL_ERR,"resource_cleanup(%08x)::error\n",portid);
			return ERROR;
		}
	}

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)done\n",portid);

	return OKAY;
}

/*******************************************************************************
 *
 * net_port_cleanup - Cleaning up port resource
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid - port number
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_port_cleanup(unsigned int portid)
{
	net_port_st *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	if(net_port_resource_cleanup(port) == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"resource_cleanup(%08x)::error\n",portid);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)done\n",portid);

	return OKAY;
}

/*******************************************************************************
 *
 * net_channel_q_delete_upcall - upcall for message queue deletion
 *
 * DESCRIPTION
 */
static os_delfunc_t net_channel_q_delete_upcall(void *arg)
{
	unsigned long maddr;
	OS_MESG *m;
	unsigned int mbuffid;
	net_packet *pkt;
	net_packet_hdr *hdr;
	unsigned int buffid;

	if(arg==NULL) {
		os_debug_printf(OS_DBG_LVL_ERR,"null data found\n");
		return ERROR;
	}

	/* address */
	maddr = *(unsigned long *)arg;

	/* message */
	m = (OS_MESG *)maddr;

	/* message buffer id */
	mbuffid = buff_get_info(BUFF_GET_ID,(unsigned char *)m);

	/* packet */
	pkt = (net_packet *)OS_MESG_DATA(m);
	if(!pkt) {
		os_debug_printf(OS_DBG_LVL_ERR,"null packet\n");
		return ERROR;
	}

	/* payload test */
	if(!pkt->payload) {
		os_debug_printf(OS_DBG_LVL_ERR,"null payload\n");
	}

	/* packet buffer id */
	buffid = buff_get_info(BUFF_GET_ID,(unsigned char *)pkt);

	/* header */
	hdr = (net_packet_hdr *)&(pkt->h);
	if(hdr==NULL) {
		os_debug_printf(OS_DBG_LVL_ERR,"null data found\n");
		return ERROR;
	}

	/* header validity */
	if(!(hdr->preamble[0] == 'u' &&
	     hdr->preamble[1] == 'N' &&
	     hdr->preamble[2] == 'E' &&
	     hdr->preamble[3] == 'T')) {
		os_debug_printf(OS_DBG_LVL_ERR,"Broken packet \n");
		return ERROR;
	}

	/* verbose */
	os_debug_printf(OS_DBG_LVL_ERR,"flushing packets = (buff=%08x)(type=%08x)(vsp=%d)(vdp=%d)(size=%d)\n",
	                buffid, ntohl(hdr->type), ntohs(hdr->vsport), ntohs(hdr->vdport), ntohs(hdr->size));

	/* freeing up packet */
	buff_free_data(buffid,(unsigned char *)(pkt->payload));
	buff_free_data(buffid,(unsigned char *)(pkt));
	buff_free_data(mbuffid,(unsigned char *)m);

	return (os_delfunc_t)OKAY;
}

/*******************************************************************************
 *
 * net_channel_create - making a channel instance & inserting it into port list
 *
 * DESCRIPTION
 *   Creating a channel structure, however, port field of the net_channel_st is
 *   not yet specified. Instead, it will be specified by net_channel_register().
 *
 * PARAMETERS
 *  buffid - buffer id
 *  port - channel number
 *
 * RETURNS
 *  channel ID/ERROR
 *
 * ERRORS
 *   N/A
 */
unsigned int net_channel_create(unsigned int buff, unsigned int port)
{
	unsigned int channelid;
	net_channel_st *pchan;
	unsigned char tname[STR_LEN];

	/* creating a channel structure */
	pchan = (net_channel_st *)buff_alloc_data(NET_MEM_CLASS,
	                                          sizeof(net_channel_st));
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(sizeof(net_channel_st))::error\n");
		return ERROR;
	}

	/* filling up data sturucture */
	sprintf((char *)tname,(const char *)"CHAN%08x",port);
	strncpy((char *)pchan->name,(const char *)tname,STR_LEN);
	pchan->buff       = buff;
	pchan->cnum       = port;
	pchan->total_bytes    = 0;
	pchan->packets    = 0;
	pchan->port       = 0;/* invalid port */

	/* message queue with default delete function */
	sprintf((char *)tname,(const char *)"%s%s%08x",MSGQ_NAME,"chan",port);
	pchan->msgq = os_msgq_create((const char *)tname,NET_MEM_CLASS,(os_delfunc_t)net_channel_q_delete_upcall);
	if(pchan->msgq == ERROR) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pchan);
		os_debug_printf(OS_DBG_LVL_ERR,"os_msgq_create(%s,channel=%d)::error\n",tname,port);
		return ERROR;
	}

	/* mutex for channel */
	sprintf((char *)tname,(const char *)"%s%s%08x",MUTEX_NAME,"chan",port);
	pchan->lock = os_mutex_create((const char *)tname);
	if(pchan->lock == ERROR) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pchan);
		os_msgq_delete(pchan->msgq);
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_create(channel=%d)::error\n",port);
		return ERROR;
	}

	/* inserting it into a port pool */
	channelid = os_list_item_insert( net_channel_pool, (void *)pchan );
	if(channelid == ERROR) {
		buff_free_data(NET_MEM_CLASS,(unsigned char *)pchan);
		os_msgq_delete(pchan->msgq);
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(channel=%d)::error\n",port);
		return ERROR;
	}

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)\n",channelid);

	return channelid;
}

/*******************************************************************************
 *
 * net_channel_register - Setting a port structure for a channel
 *
 * DESCRIPTION
 *  Case 1) In the middle of packet delivery from the port func(), what happens?
 *
 * PARAMETERS
 *  portid - port structure
 *  chanid - channel id
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *   N/A
 */
unsigned int net_channel_register(unsigned int portid,unsigned int chanid)
{
	net_channel_st *pchan;
	net_port_st *port;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	/* channel information */
	pchan = (net_channel_st *)os_obj_find(net_channel_pool,chanid);
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid channel id (%08x)\n",chanid);
		return ERROR;
	}

	/* port inserting ... */
	/* inserting a channel */
	if(os_list_item_insert(port->chanlist,(void *)pchan) == ERROR) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_insert(%08x,%08lx)\n",
		                NET_PORT_ID(port),
		                (unsigned long)pchan);
		return ERROR;
	}

	/* channel mapping */
	/* lock channel */
	os_mutex_lock( pchan->lock, 0 );
	/* set up port structure */
	pchan->port = (void *)port;
	/* unlock channel */
	os_mutex_unlock( pchan->lock );

	os_debug_printf(OS_DBG_LVL_VER,"(%08x::%x)done\n",portid,chanid);
	return OKAY;
}

/*******************************************************************************
 *
 * net_channel_flush - Flushing a channel
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  channelid - channel
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_channel_flush(unsigned int channelid)
{
	net_channel_st *pchan;

	/* channel information */
	pchan = (net_channel_st *)os_obj_find(net_channel_pool,channelid);
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid channel id (%08x)\n",channelid);
		return ERROR;
	}

	/* lock */
	os_mutex_lock(pchan->lock,0);

	/* cleanup channel id */
	if( os_msgq_flush(pchan->msgq) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_msgq_flush(%08x)::error\n",channelid);
		os_mutex_unlock(pchan->lock);
		return ERROR;
	}

	/* unlock */
	os_mutex_unlock(pchan->lock);

	os_debug_printf(OS_DBG_LVL_VER,"done(%08x)\n",channelid);

	return OKAY;
}

/*******************************************************************************
 *
 * net_channel_delete - deleting a channel
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  channelid - channel
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_channel_delete(unsigned int channelid)
{
	net_channel_st *pchan;

	/* channel information */
	pchan = (net_channel_st *)os_obj_find(net_channel_pool,channelid);
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid channel id (%08x)\n",channelid);
		return ERROR;
	}

	/* flushing a message queue */
	if( os_msgq_flush(pchan->msgq) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_msgq_flush(%08x)::error\n",pchan->msgq);
		return ERROR;
	}

	/* deleting a message queue */
	if( os_msgq_delete(pchan->msgq) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_msgq_delete(%08x)::error\n",pchan->msgq);
		return ERROR;
	}

	/* deleting a mutex */
	if( os_mutex_delete(pchan->lock) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mutex_delete(%08x)::error\n",pchan->msgq);
		return ERROR;
	}

	/* cleanup channel id */
	if( os_list_item_delete(net_channel_pool,channelid) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_list_item_delete(%08x)::error\n",channelid);
		return ERROR;
	}

	/* cleaning up memory resource */
	buff_free_data(NET_MEM_CLASS,(unsigned char *)pchan);

	os_debug_printf(OS_DBG_LVL_VER,"done(%08x)\n",channelid);

	return OKAY;
}

/*******************************************************************************
 *
 * net_channel_status - status of a channel
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  channelid - channel
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int net_channel_status(unsigned int command,unsigned int channelid)
{
	net_channel_st *pchan;

	/* channel information */
	pchan = (net_channel_st *)os_obj_find(net_channel_pool,channelid);
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid channel id (%08x)\n",channelid);
		return ERROR;
	}

	/* command processing */
	switch(command) {
	case NET_CHANNEL_BUFFER:
		return pchan->total_bytes;
	case NET_CHANNEL_ENABLE:
		pchan->disabled = 0;
		return OKAY;
	case NET_CHANNEL_DISABLE:
		pchan->disabled = 1;
		return OKAY;
	default:
		os_debug_printf(OS_DBG_LVL_ERR,"Unknown command=%08x\n",command);
		break;
	}

	return ERROR;
}

/*******************************************************************************
 *
 * net_packet_calc_checksum - Calculating a checksum
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  p - payload field
 *  size - payload length
 *
 * RETURNS
 *  checksum value
 *
 * ERRORS
 *   N/A
 */
static unsigned int net_packet_calc_checksum(unsigned char *p,unsigned short size)
{
	/*  net.h defines the length of checksum field
	 *  to be "unsigned int".
	 */
#define CHECKSUM_UNIT   sizeof(unsigned int)

	unsigned int checksum;
	unsigned int ix;
	unsigned int adj_size = size/CHECKSUM_UNIT;

	checksum = 0x0; /* initial value */
	for(ix=0; ix<adj_size; ix+=CHECKSUM_UNIT) {
		checksum ^= ~(*(p+ix));
	}

	os_debug_printf(OS_DBG_LVL_VER,"checksum = %08x\n",checksum);
	return checksum;
}

/*******************************************************************************
 *
 * net_packet_make - Building a packet structure
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  schannelid - source channel id
 *  dchannel - destination channel
 *  type - packet type
 *  size - payload size
 *  option - not used
 *  payload - real packet payload
 *
 * RETURNS
 *  NULL/packet
 *
 * ERRORS
 *   N/A
 */
net_packet * net_packet_make(
	unsigned int schannelid,
	unsigned short dchannel,
	unsigned int type,
	unsigned short size,
	unsigned int option,
	void *payload)
{
	unsigned int buffid;
	unsigned short schannel;
	net_channel_st *pchan;
	net_packet *tpkt;

	/* No way to figure out the exact value of payload length */
  #if 0
	if(NET_PAYLOAD_LEN_CHECK(size)) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid size = %d\n",size);
		return NULL;
	}
  #endif

	/* source channel is found */
	pchan = (net_channel_st *)os_obj_find(net_channel_pool,schannelid);
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(chan=%08x)::error\n",schannelid);
		return NULL;
	}

	/* buffer id */
	buffid = pchan->buff;
	/* channel number */
	schannel = pchan->cnum;

	tpkt = (net_packet *)buff_alloc_data(buffid,sizeof(net_packet));
	if(!tpkt) {
		os_debug_printf(OS_DBG_LVL_ERR,"buff_alloc_data(sizeof(net_packet))::error\n");
		return NULL;
	}

	/* packet structure */
	memcpy(tpkt->h.preamble,NET_PACKET_PREAMBLE,strlen(NET_PACKET_PREAMBLE));
	tpkt->h.type = htonl(type);
	tpkt->h.size = htonl(size); 
	tpkt->h.vsport = htons(schannel);
	tpkt->h.vdport = htons(dchannel);
	tpkt->h.option = htonl(option);
	tpkt->h.checksum = htonl(net_packet_calc_checksum((unsigned char *)payload,size));
	tpkt->payload = payload;

	os_debug_printf(OS_DBG_LVL_VER,"(d=%d,s=%d,type=%08x,size=%d)\n",dchannel,schannel,type,size);

	return tpkt;
}

/*******************************************************************************
 *
 * net_packet_free - Freeing up a packet structure
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  schannelid - source channel id
 *  pkt - packet structure
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *   N/A
 */
unsigned int net_packet_free(unsigned int schannelid,net_packet *pkt)
{
	unsigned int buffid;
	net_channel_st *pchan;

	if(!pkt) {
		os_debug_printf(OS_DBG_LVL_ERR,"null packet\n");
		return ERROR;
	}

	if(!pkt->payload) {
		os_debug_printf(OS_DBG_LVL_ERR,"null payload\n");
		return ERROR;
	}

	/* source channel is found */
	pchan = (net_channel_st *)os_obj_find(net_channel_pool,schannelid);
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_obj_find(chan=%08x)::error\n",schannelid);
		return ERROR;
	}

	/* buffer id */
	buffid = pchan->buff;

	/* freeing up data */
	buff_free_data(buffid,(unsigned char *)pkt->payload);
	buff_free_data(buffid,(unsigned char *)pkt);

	os_debug_printf(OS_DBG_LVL_VER,"(%08x)done\n",schannelid);

	return OKAY;
}

/*******************************************************************************
 *
 * net_packet_send - Sending a packet
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid  - port ID
 *  pkt - packet structure
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *   N/A
 */
unsigned int net_packet_send(unsigned int channelid,net_packet *pkt)
{
	unsigned int payload_len;
	unsigned int res;
	unsigned short actual_size;
	net_channel_st *pchan;
	net_port_st *port;
	net_protocol_st *proto;
	OS_MUTEX *m; /* xxx */

	/* argument test */
	if(!pkt) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid pkt\n");
		return ERROR;
	}

	/* payload test */
	if(!pkt->payload) {
		os_debug_printf(OS_DBG_LVL_ERR,"null payload\n");
		return ERROR;
	}

	/* channel information */
	pchan = (net_channel_st *)os_obj_find(net_channel_pool,channelid);
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid channel id (%08x)\n",channelid);
		return ERROR;
	}

	/* port information */
	port = (net_port_st *)pchan->port;
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"null port structure\n");
		return ERROR;
	}

	/* lock */
	os_mutex_lock(port->lock,0);

	/* protocol block */
	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_mutex_unlock(port->lock);
		os_debug_printf(OS_DBG_LVL_ERR,"protocol block null\n");
		return ERROR;
	}

	/* payload size test */
	payload_len = ntohl(pkt->h.size); 

	/* calculate total packet size */
	actual_size = sizeof(net_packet_hdr)+payload_len;

	if(port->payload_size < actual_size) {
		os_mutex_unlock(port->lock);
		os_debug_printf(OS_DBG_LVL_ERR,"error in payload length(=%d,max size=%d)\n",
		                payload_len,port->payload_size);
		return ERROR;
	}

	/* clean up error flag */
	net_port_errno_set(port->id,E_NOERROR);

	res = net_port_proto_send(port->id,(unsigned char *)&(pkt->h),sizeof(net_packet_hdr),NULL);
	if(res != OKAY) {
		os_mutex_unlock(port->lock);
		os_debug_printf(OS_DBG_LVL_ERR,"port(port=%08x)::real error\n",port->id);
		return ERROR; /* return immediately */
	}

	/* clean up error flag */
	net_port_errno_set(port->id,E_NOERROR);

	/* packet sending */
	res = net_port_proto_send(port->id,(unsigned char *)(pkt->payload),payload_len,NULL);
	if(res != OKAY) {
		os_mutex_unlock(port->lock);
		os_debug_printf(OS_DBG_LVL_ERR,"port(port=%08x)::real error\n",port->id);
		return ERROR; /* return immediately */
	}

	/* unlock */
	os_mutex_unlock(port->lock);

	os_debug_printf(OS_DBG_LVL_VER,"dchannel=%d,schannel=%d,size=%d,(%08x)done\n",
	                ntohs(pkt->h.vdport),ntohs(pkt->h.vsport),ntohl(pkt->h.size),port->id); /* pkt->h.size : short -> int */

	return OKAY;
}

/*******************************************************************************
 *
 * net_packet_recv - Receiving a packet
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  channelid  - channel ID
 *  tm - expiration time (microseconds)
 *       = 0 : infinite waiting
 *       > 0 : time driven
 *
 * RETURNS
 *  NULL/packet
 *
 * ERRORS
 *   N/A
 */
net_packet * net_packet_recv(unsigned int channelid,unsigned int tm)
{
	unsigned int buff;
	net_channel_st *pchan;
	net_packet *pkt;
	OS_MESG *mesg;

	/* port information */
	pchan = (net_channel_st *)os_obj_find(net_channel_pool,channelid);
	if(!pchan) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid channel id (%08x)\n",channelid);
		return NULL;
	}

	/* buffer id */
	buff = pchan->buff;

	/* timeout processing */
	if(tm == 0) {
		/* infinite loop */
		mesg = os_msgq_recv(pchan->msgq,0);
	}else{
		/* timed recv */
		mesg = os_msgq_recv_timeout(pchan->msgq,(unsigned long)(tm));
	}
	if(!mesg) {
		/* In waiting mode, it might happen.... */
		if(tm==0) os_debug_printf(OS_DBG_LVL_ERR,"os_msgq_recv(%08x)::error\n",channelid);
		return NULL;
	}

	if(OS_MESG_COMMAND(mesg)!=NET_PACKET_MESSAGE) {
		os_debug_printf(OS_DBG_LVL_ERR,"message corrupted(%08x)::error\n",channelid);
		return NULL;
	}

	/* packet delivery structure */
	pkt = (net_packet *)OS_MESG_DATA(mesg);
	if(!pkt) {
		buff_free_data(buff,(unsigned char *)mesg);
		os_debug_printf(OS_DBG_LVL_ERR,"os_msgq_recv(%08x)::null data\n",channelid);
		return NULL;
	}

	/* lock */
	os_mutex_lock( pchan->lock, 0 );

	pchan->total_bytes -= pkt->h.size; /* payload size */
	pchan->packets -= 1; /* total packets */

	/* unlock */
	os_mutex_unlock(pchan->lock);

	/* NULL Packet */
	if(pkt->payload == NULL) {
		if(pkt->h.type == NET_PACKET_NULL_TYPE) {
		}
		/* free up */
		buff_free_data(buff,(unsigned char *)pkt);
		pkt = NULL;
	}

	/* cleaning up resources for mesg and pds */
	buff_free_data(buff,(unsigned char *)mesg);

	if(pkt) { 
		os_debug_printf(OS_DBG_LVL_VER,"dchannel=%d,schannel=%d,size=%d,(%p)done\n",
		                      pkt->h.vdport,pkt->h.vsport,pkt->h.size,pchan->port);
	}else{
		os_debug_printf(OS_DBG_LVL_VER,"null packet returned\n");
	}

	return pkt;
}

/*******************************************************************************
 *
 * net_port_option_configure - Setting up an option
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  portid  - port ID
 *  command - how
 *  arg     - argument
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *   N/A
 */
unsigned int net_port_option_configure(unsigned int portid,unsigned int command,void *arg)
{
	net_port_st *port;
	net_protocol_st *proto;

	/* port information */
	port = (net_port_st *)os_obj_find(net_port_pool,portid);
	if(!port) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid port id (%08x)\n",portid);
		return ERROR;
	}

	proto = (net_protocol_st *)port->proto;
	if(!proto) {
		os_debug_printf(OS_DBG_LVL_ERR,"invalid protocol (%08x)\n",portid);
		return ERROR;
	}

	/* command processing */
	switch(command) {
	case NET_PORT_ENABLE_ACTION: /* enable channel muliplexing */
		port->option &= ~NET_PORT_DISABLE;
		break;

	case NET_PORT_DISABLE_ACTION: /* disable channel multiplexing */
		port->option |= NET_PORT_DISABLE;
		break;

	case NET_PORT_ENABLE_NONBLOCK:
		port->option &= ~NET_PORT_NONBLOCK_DISABLE;
		proto->net_ioctl(port,NET_PORT_ENABLE_NONBLOCK,NULL);
		break;

	case NET_PORT_DISABLE_NONBLOCK:
		port->option |= NET_PORT_NONBLOCK_DISABLE;
		proto->net_ioctl(port,NET_PORT_DISABLE_NONBLOCK,NULL);
		break;

	case NET_PORT_ENABLE_KEEPALIVE:
		port->option &= ~NET_PORT_KEEPALIVE_DISABLE;
		proto->net_ioctl(port,NET_PORT_ENABLE_KEEPALIVE,NULL);
		break;

	case NET_PORT_DISABLE_KEEPALIVE:
		port->option |= NET_PORT_KEEPALIVE_DISABLE;
		proto->net_ioctl(port,NET_PORT_DISABLE_KEEPALIVE,NULL);
		break;

	case NET_PORT_GET_ADDR_INFO:
		return proto->net_ioctl(port,NET_PORT_GET_ADDR_INFO,arg);

	case NET_PORT_SET_ADDR_INFO:
		return proto->net_ioctl(port,NET_PORT_SET_ADDR_INFO,arg);

	case NET_PORT_SO_LINGER:
		proto->net_ioctl(port,NET_PORT_SO_LINGER,NULL);
		break;

	case NET_PORT_SET_PAYLOAD_SIZE:
		if(!arg) {
			os_debug_printf(OS_DBG_LVL_ERR,"null payload size\n");
			return port->payload_size;
		}
		port->payload_size = *(unsigned int *)arg;
		return port->payload_size;

	case NET_PORT_GET_PAYLOAD_SIZE:
		return port->payload_size;

	case NET_PORT_SET_DELAY_TIME:
		if(!arg) {
			os_debug_printf(OS_DBG_LVL_ERR,"useless delay time \n");
			return port->catchup_delay;
		}
		port->catchup_delay = *(unsigned int *)arg;
		return port->catchup_delay;

	case NET_PORT_GET_DELAY_TIME:
		return port->catchup_delay;

	case NET_PORT_GET_SOCK_ADDR_INFO:
		return proto->net_ioctl(port,NET_PORT_GET_SOCK_ADDR_INFO,arg);
	}

	os_debug_printf(OS_DBG_LVL_VER,"portid=%08x,command=%08x(done)\n",port->id,command);

	return OKAY;
}
