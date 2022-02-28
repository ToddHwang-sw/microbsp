#include "oskit.h"
#include "mesg.h"

#define NET_TEST_MEM_CLASS  0x10000002
#define NET_PACKET_CLASS    0x10000004

#define SVC_NET_PORT        2440 /* physcial port */
#define SERVER_CHANNEL_PORT 3000 /* logical channel */
#define CLIENT_CHANNEL_PORT 2000 /* +               */

typedef struct {
	unsigned int channel;
	unsigned int id;
}cli_channel_st;

#define CLIENT_CHANNEL_NUM  3
#define INSTANTIATE_NUM CLIENT_CHANNEL_NUM

static cli_channel_st cli_channelid[CLIENT_CHANNEL_NUM] =
{ {CLIENT_CHANNEL_PORT,0},
  {CLIENT_CHANNEL_PORT+100,0},
  {CLIENT_CHANNEL_PORT+200,0}}; /* client channels */

static unsigned int cli_net_port;  /* port information */
static unsigned int port_man_flag; /* networking flag */
static unsigned int port_man_restart; /* restarting flag */
static unsigned int thrs[CLIENT_CHANNEL_NUM] = {0,0,0};
static unsigned int port_man_thr;
static os_thrfunc_t client_port_man(void *arg);
static os_delfunc_t client_port_man_delfunc(void *arg);

/* upcall */
static void udp_client_upcall(unsigned int fd,unsigned int command, void *arg)
{
	switch( command ) {
	case NET_MESG_NEW_PORT: printf("NEW_PORT MESG\n");  break;
	case NET_MESG_DEL_PORT:
		printf("DEL_PORT MESG\n");
		/* port closed... */

		/* port manager stopped */
		port_man_flag = 0;
		break;
	case NET_MESG_ACCEPT: printf("ACCEPT_PORT MESG\n");  break;
	default: printf("unknown MESG\n"); break;
	}

}

/* sender thread */
static void os_txrx_thread(void *arg)
{
	unsigned int res;
	unsigned int pkt_size;
	unsigned char * pkt_payload;
	net_packet *pkt;
	unsigned char pattern;
	unsigned char *msg;
	cli_channel_st *ch = (cli_channel_st *)arg;

	if(!ch) {
		printf("null channel\n");
		return;
	}

	while(1) {

		if( !net_port_proto_isconnected(cli_net_port) ) {
			printf("management is not yet activated\n");
			os_thread_sleep(1000000);
			continue;
		}

		pkt_size = abs((rand())%NET_MAX_PAYLOAD) & ~0x3;
		/* 4 bytes aligned size */

		pkt_payload = buff_alloc_data(NET_PACKET_CLASS,pkt_size);
		if(!pkt_payload) {
			printf("buff_alloc_data(pkt_size)::error\n");
			continue;
		}

		pattern = abs(rand())%0x7f; /* pattern generate */
		*(unsigned char *)pkt_payload = (unsigned char)pattern;
		memset( (unsigned char *)(pkt_payload + sizeof(unsigned char )),
		        pattern,
		        pkt_size-sizeof(unsigned char));

		/* building packet structure */
		pkt = net_packet_make(
			ch->id,
			SERVER_CHANNEL_PORT,
			COMM_DEBUG_MESSAGE,
			pkt_size,
			0,
			pkt_payload );

		/* network packet send */
		if((res=net_packet_send(ch->id,pkt))==ERROR) {
			printf("net_packet_send(%08x)::error\n",ch->id);
			net_packet_free(ch->id,pkt);
			continue;
		}

		printf("sending packet to channel = %d(%d)\n",ntohs(pkt->h.vdport),pkt_size);

		/* network packet free */
		net_packet_free(ch->id,pkt);

		/* receive a packet */
		pkt = net_packet_recv(ch->id,1000000); /* 1 second */
		if(!pkt) {
			printf("null packet\n");
			continue;
		}
		msg = (unsigned char *)NET_PACKET_PAYLOAD(pkt);

		printf("message from server-> %s from channel=%d \n",msg,pkt->h.vdport);

		net_packet_free(ch->id,pkt);

		//os_thread_sleep(1000);
	}

	net_port_delete( cli_net_port );
}

/*
 * port manager delete function
 */
static os_delfunc_t client_port_man_delfunc(void *arg)
{
	int ii;

	printf("%s\n",__func__);

	/* channel delete */
	for(ii=0; ii<INSTANTIATE_NUM; ii++) {
		if(net_channel_status(NET_CHANNEL_DISABLE,cli_channelid[ii].id)
		   != ERROR) {
			printf("channel deleting... %08x\n",cli_channelid[ii].id);
			net_channel_flush(cli_channelid[ii].id);
			net_channel_delete(cli_channelid[ii].id);
		}
	}
}

/*
 * port manager...
 */
static os_thrfunc_t client_port_man(void *arg)
{
	int ii;
	int payload_sz;
	net_addrinfo_t addr;

	printf("client port manager started\n");

	/* client */
	for(ii=0; ii<INSTANTIATE_NUM; ii++) {
		cli_channelid[ii].id = net_channel_create(
			NET_PACKET_CLASS, cli_channelid[ii].channel);
		if(cli_channelid[ii].id == ERROR) {
			printf("error in net_channel_create(%d)\n",cli_channelid[ii].channel);
			return 0;
		}
		printf("channel creating... %08x\n",cli_channelid[ii].id);
	}

	/* address create */
	net_addrinfo_make(&addr,NET_IP(127,0,0,1),SVC_NET_PORT);
	//net_addrinfo_make(&addr,NET_IP(192,168,0,101),SVC_NET_PORT);

	/* port create */

	/* client network port create */
	cli_net_port = net_port_create(
		NET_PORT_TYPE_CLIENT,
		NET_PORT_PROTO_UDP,
		&addr,
		NULL,
		udp_client_upcall );
	if(cli_net_port == ERROR) {
		printf("error in net_port_create()\n");
		return 0;
	}
	printf("cli_net_port = %x created!!!\n",cli_net_port);

	/* activating port */
	if(net_port_activate(cli_net_port) == ERROR) {
		printf("error in net_port_activate()\n");
		return 0;
	}

	/* port structure assignment */
	for(ii=0; ii<INSTANTIATE_NUM; ii++)
		net_channel_register(cli_net_port,cli_channelid[ii].id);

	while(1) {
		if(!port_man_flag) {
			printf("Normal return...\n");
			port_man_restart = 1; /* restart again */
			return 0;
		}
		os_thread_sleep(100000);
	}
}

int main()
{
	int ii;

	/* OS initializing */
	os_init();

	/* flag */
	port_man_flag = 1; /* start ... */
	port_man_restart = 0; /* restart ... */

	/* packet buffer creation */
	ii = buff_create_class(NET_PACKET_CLASS,NULL,1024*1024); /* 1 mbytes */
	if(ii ==ERROR) {
		printf("malloc error\n");
		return 0;
	}

	/* port manager */
	port_man_thr = os_thread_create("portmanager",(os_thrfunc_t)client_port_man, NULL,
	                                    (os_delfunc_t)client_port_man_delfunc, 0 );
	if( port_man_thr == ERROR ) {
		printf("os_create(port_manager) error\n");
		return 0;
	}

	/* running... */
	for(ii=0; ii<INSTANTIATE_NUM; ii++)
		thrs[ii] = os_thread_create("thrs",
			(os_thrfunc_t)os_txrx_thread,(void *)&(cli_channelid[ii]),NULL,0);

	/* inifinite loop */
	while(1) 
		os_thread_sleep(50000);

	return 0;
}
