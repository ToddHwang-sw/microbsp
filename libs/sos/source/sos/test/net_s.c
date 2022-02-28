#include "oskit.h"
#include "mesg.h"

#define NET_PACKET_CLASS    0x10000002

#define SVC_NET_PORT        2440 /* physical port */

#define SERVER_CHANNEL_PORT 3000 /* logical channel */
#define CLIENT_CHANNEL_PORT 2000 /* +               */

static unsigned int serv_channelid;   /* server channel */
static unsigned int serv_net_port;    /* port information */

/* upcall */
static void udp_server_upcall(unsigned int fd,unsigned int command, void *arg)
{
	switch( command ) {
	case NET_MESG_NEW_PORT: 
		printf("NEW_PORT MESG\n");  
		break;
	case NET_MESG_DEL_PORT: 
		printf("DEL_PORT MESG\n");  
		break;
	case NET_MESG_ACCEPT: 
		printf("ACCEPT_PORT MESG\n");  
		break;
	default: 
		printf("unknown MESG\n"); 
		break;
	}
}

static void packet_recv_thread(void *arg)
{
	net_packet *pkt;
	unsigned char *mesg;
	unsigned int schannel;

	while(1) {

		/* RECEIVE */
		pkt = net_packet_recv(serv_channelid,2000000);
		if(!pkt) {
			printf("null packet\n");
			continue;
		}

		printf("server::recv::preamble=%c%c%c%c..\n",
		       pkt->h.preamble[0],
		       pkt->h.preamble[1],
		       pkt->h.preamble[2],
		       pkt->h.preamble[3]);
		printf("server::recv::size=%d from channel=%d to channel=%d\n",
		       pkt->h.size,pkt->h.vsport,pkt->h.vdport);

		/* source channel */
		schannel = pkt->h.vsport;

		net_packet_free(serv_channelid,pkt);

		mesg = buff_alloc_data(NET_PACKET_CLASS,56);
		memset(mesg,0,56);
		strncpy(mesg,"I am sam",56);

		/* SEND */
		pkt = net_packet_make(
			serv_channelid,
			schannel, /* destination ping - pong */
			COMM_DEBUG_MESSAGE,
			56,
			0,
			mesg);
		if(!pkt) {
			printf("error in net_...make()\n");
			exit(0);
		}
		printf("server::send::size=%d from channel=%d to channel=%d\n",
		       ntohs(pkt->h.size),ntohs(pkt->h.vsport),ntohs(pkt->h.vdport));

		net_packet_send(serv_channelid,pkt);

		net_packet_free(serv_channelid,pkt);
	}
}

static int counter = 0;

int main()
{
	int ii;
	unsigned int thrs[4];
	net_addrinfo_t addr;

	/* OS initializing */
	os_init();

	/* buffer class create */
	ii = buff_create_class(NET_PACKET_CLASS,NULL,1*1024*1024); /* 32 Kbytes */
	if(ii==ERROR) {
		printf("buffer_create_class::error\n");
		return -1;
	}

	/* channel create */

	/* server */
	serv_channelid = net_channel_create(
		NET_PACKET_CLASS,   /* buffer class */
		SERVER_CHANNEL_PORT /* channel number */
		);
	if(serv_channelid == ERROR) {
		printf("error in net_channel_create(%d)\n",SERVER_CHANNEL_PORT);
		exit(0);
	}

	printf("server channeid = %x\n",serv_channelid);

	/* port create */
	net_addrinfo_make(&addr,NET_ANY_IP,SVC_NET_PORT);

	/* server network port create */
	serv_net_port = net_port_create(
		NET_PORT_TYPE_SERVER,
		NET_PORT_PROTO_UDP,
		&addr,
		NULL, /* pktfunction */
		udp_server_upcall /* upcall */
		);

	if(serv_net_port == ERROR) {
		printf("error in net_port_create()\n");
		exit(0);
	}

	/* registering */
	if(net_channel_register(
		   serv_net_port,
		   serv_channelid)==ERROR) {
		printf("error in net_channel_port_set()::error\n");
		exit(0);
	}

	/* port activate */
	if(net_port_activate(
		   serv_net_port) == ERROR) {
		printf("error in net_port_activate()::error\n");
		exit(0);
	}

	thrs[0] = os_thread_create("thrrecv",(os_thrfunc_t)packet_recv_thread,NULL,NULL,0);

	buff_verbose(NET_PACKET_CLASS);

	while(1) {
		os_thread_sleep(1000000);
		++counter;
		if(counter == 10) {
			/* net_port_delete(serv_net_port); */
		}
	}

	return 0;
}
