#include "oskit.h"
#include "mesg.h"

typedef struct {
	unsigned int size;
	unsigned char pattern;
	unsigned char payload[300];
}__attribute__((packed)) MYPKT_STRUCT;

#define NET_PACKET_CLASS    0x10000004
#define SVC_NET_PORT        2440 /* physcial port */

#define CLI_CHANNEL_PORT    9900
#define MY_CHANNEL_PORT     4800

#define MAX_CLI 10
typedef struct {
	unsigned int used;
	unsigned int portid;
	unsigned int nportid;
	unsigned int channelid;
	unsigned int done;
	unsigned int thrs;
} __attribute__((packed)) cli_st;
static cli_st cli[MAX_CLI] = {0,};
static os_mutex_t cli_lock;

static __inline__ int cli_alloc_slot(void)
{
	int ix;

	for (ix = 0; ix < MAX_CLI; ix++) {
		if (!cli[ix].used) {
			cli[ix].used = 1;
			return ix;
		}
	}
	return -1;
}

static __inline__ int cli_find_slot(unsigned int nportid)
{
	int ix;

	for (ix = 0; ix < MAX_CLI; ix++) {
		if (!cli[ix].used)
	       		continue;

		if (cli[ix].nportid == nportid)
			return ix;
	}

	return -1;
}

static void tcp_recv_thread(void *arg);

/* upcall */
static void port_server_upcall(unsigned int id,unsigned int command, void *arg)
{
	unsigned int portid;
	unsigned int newportid;
	unsigned int serv_channelid;
	int index;

	switch( command ) {
	case NET_MESG_ACCEPT:
		portid = id;
		newportid = *(unsigned int *)arg;

		printf("ACCEPT %x %x\n", id, newportid);  

		serv_channelid = 
			net_channel_create(
				NET_PACKET_CLASS,
				CLI_CHANNEL_PORT);

		if (serv_channelid == ERROR) {
			printf("NEW_PORT_MESG::net_channel_create(..,%d)::error\n",
					CLI_CHANNEL_PORT);
			return ;
		}

		if (net_channel_register(
				newportid,
				serv_channelid) == ERROR) {
			printf("NEW_PORT_MESG::net_channel_register(%x::%x)::error\n",
					newportid, serv_channelid);
			return ;
		}

		_os_mutex_lock( &cli_lock );

		index = cli_alloc_slot();
		if (index < 0) {
			_os_mutex_unlock( &cli_lock );
			printf("NEW_PORT_MESG::slot allocation eror\n");
			return ;
		}

		cli[ index ].portid = portid;
		cli[ index ].nportid = newportid;
		cli[ index ].channelid = serv_channelid;
		cli[ index ].done = 0;
		cli[ index ].used = 1; /* overkill */

		_os_mutex_unlock( &cli_lock );
		break;

	case NET_MESG_NEW_PORT: 
		portid = id;
		newportid = *(unsigned int *)arg;

		printf("NEW_PORT %x %x\n", id, newportid); 

		_os_mutex_lock( &cli_lock );
		index = cli_find_slot( newportid );
		if (index < 0) {
			printf("NEW_PORT_MESG::slot find eror\n");
			_os_mutex_unlock( &cli_lock );
			return ;
		}

		cli[ index ].done = 0;
		cli[ index ].thrs = os_thread_create("thrrecv",
					(os_thrfunc_t)tcp_recv_thread,
					(void *)&(cli[ index ]), NULL, 0);
		_os_mutex_unlock( &cli_lock );
		break;

	case NET_MESG_DEL_PORT: 
		newportid = id;
		printf("DEL_PORT MESG %x\n", newportid); 

		_os_mutex_lock( &cli_lock );
		index = cli_find_slot( newportid );
		if (index < 0) {
			_os_mutex_unlock( &cli_lock );
			printf("DEL_PORT_MES::slot find eror\n");
			return ;
		}

		cli[ index ].done = 1;
		os_thread_delete( cli[ index ].thrs );
		net_channel_delete( cli[ index ].channelid );
		net_port_delete( newportid );
		cli[ index ].used = 0;

		_os_mutex_unlock( &cli_lock );

		printf("RESOURCE CLEANED ... \n");
		break;

	default: 
		printf("unknown MESG\n"); 
		break;
	}

}

static void tcp_recv_thread(void *arg)
{
	cli_st *_cli = (cli_st *)arg;
	net_packet *pkt;
	net_packet *dpkt = NULL;
	MYPKT_STRUCT *mypkt;
	int round = 0;

	while(!_cli->done) {

		/* RECEIVE */
		pkt = net_packet_recv(_cli->channelid,0);
		if(!pkt) {
			printf("null packet\n");
			exit(0);
		}
		printf("server::recv::preamble=%c%c%c%c..\n",
		       pkt->h.preamble[0],
		       pkt->h.preamble[1],
		       pkt->h.preamble[2],
		       pkt->h.preamble[3]);
		printf("server::recv::size=%d from channel=%d to channel=%d\n",
		       pkt->h.size,pkt->h.vsport,pkt->h.vdport);

		/* freeing up receive packet */
		net_packet_free(_cli->channelid,pkt);
		/* freeing up packet */
		if (dpkt)
			net_packet_free(_cli->channelid,dpkt);

		/* allocating packet */
		mypkt = (MYPKT_STRUCT *)buff_alloc_data(NET_PACKET_CLASS,sizeof(MYPKT_STRUCT));
		if(!mypkt) {
			printf("error\n");
			exit(0);
		}

		/* filling them up */
		mypkt->size = abs(rand())%100 & ~0x3;
		mypkt->pattern = abs(rand())%0xff;

		if (mypkt->size > 100) {
			printf("Wow~~~ error ...\n");
			mypkt->size = 100;
		}
		memset(mypkt->payload,mypkt->pattern,mypkt->size);

		/* SEND */
		dpkt = net_packet_make(
			_cli->channelid,
			MY_CHANNEL_PORT, /* destination */
			0x12345678,
			sizeof(MYPKT_STRUCT),
			0,
			(void *)mypkt);

		if(!dpkt) {
			printf("error in net_...make()\n");
			exit(0);
		}

		printf("server::send::size=%d from channel=%d to channel=%d\n",
		       ntohl(dpkt->h.size),ntohs(dpkt->h.vsport),ntohs(dpkt->h.vdport));

		net_packet_send(_cli->channelid,dpkt);

		printf("server::ROUND %d\n", round++);
	}

	printf("\n\nTHREAD DONE...\n\n");
}

static int counter = 0;

int main()
{
	int ii;
	unsigned int serv_net_port;
	net_addrinfo_t addr;

	/* OS initializing */
	os_init();

	/* client lock */
	_os_mutex_init( &cli_lock );

	/* buffer class create */
	ii = buff_create_class(NET_PACKET_CLASS,NULL,1*1024*1024); /* 32 Kbytes */
	if(ii==ERROR) {
		printf("buffer_create_class::error\n");
		return 1;
	}

	/* port create */
	net_addrinfo_make(&addr,NET_ANY_IP,SVC_NET_PORT);

	/* server network port create */
	serv_net_port = net_port_create(
		NET_PORT_TYPE_SERVER,
		NET_PORT_PROTO_TCP,
		&addr,
		NULL, /* pktfunction */
		port_server_upcall/* upcall */
		);

	if(serv_net_port == ERROR) {
		printf("error in net_port_create()\n");
		exit(0);
	}

	/* port activate */
	if(net_port_activate(
		   serv_net_port) == ERROR) {
		printf("error in net_port_activate()::error\n");
		exit(0);
	}

	while(1) {
		os_thread_sleep(1000000);
		buff_verbose(NET_PACKET_CLASS);
	}

	return 0;
}
