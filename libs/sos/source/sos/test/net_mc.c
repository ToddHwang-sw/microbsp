#include "oskit.h"
#include "mesg.h"

typedef struct {
	unsigned int size;
	unsigned char pattern;
	unsigned char payload[300];
} __attribute__((packed)) MYPKT_STRUCT;

#define SVC_NET_PORT        2440 /* physcial port */

#define SVC_CHANNEL_PORT    4800
#define MY_CHANNEL_PORT	    9900
#define CHANNEL_BUFF_ID     0xabcd0001

static unsigned int cli_net_port;  /* port information */
static unsigned int cli_channel_id; /* channel id */

/* upcall */
static void port_client_upcall(unsigned int fd,unsigned int command, void *arg)
{
	switch( command ) {
	case NET_MESG_NEW_PORT: printf("NEW_PORT MESG\n");  break;
	case NET_MESG_DEL_PORT: printf("DEL_PORT MESG\n");  break;
	case NET_MESG_ACCEPT: printf("ACCEPT_PORT MESG\n");  break;
	default: printf("unknown MESG\n"); break;
	}

}

/* sender thread */
static void os_txrx_thread(void *arg)
{
	int ii;
	unsigned int channelid;
	net_packet *tpkt, *rpkt;
	MYPKT_STRUCT *mypkt;

	if(!arg) {
		printf("null channel\n");
		return;
	}

	channelid = *(unsigned int *)arg;

	while(1) {

		/* packet allocation */
		mypkt = (MYPKT_STRUCT *)buff_alloc_data(CHANNEL_BUFF_ID,sizeof(MYPKT_STRUCT));
		if(!mypkt) {
			printf("error \n");
			continue;
		}

		/* filling them up */
		mypkt->size = abs(rand())%100 & ~0x3;
		mypkt->pattern = abs(rand())%0xff;
		memset(mypkt->payload,mypkt->pattern,mypkt->size);

		/* packet make */
		tpkt = net_packet_make(channelid,
					  MY_CHANNEL_PORT,
		                          SVC_CHANNEL_PORT,
		                          sizeof(MYPKT_STRUCT),
		                          0,
		                          (void *)mypkt);

		/* send ... */
		if(net_packet_send(channelid,tpkt) == ERROR ) {
			printf("packet send error at %08x\n",channelid);
			exit(0);
		}
		printf("Packet send\n");

		/* freeing packet */

		/* recv */
		rpkt = net_packet_recv(channelid,0);
		if(!rpkt) {
			printf("null packet\n");
			continue;
		}
		printf("Packet recv type = %08x\n",rpkt->h.type);

		mypkt = (MYPKT_STRUCT *)NET_PACKET_PAYLOAD(rpkt);
		if(!mypkt) {
			printf("null payload\n");
			continue;
		}

		/* validity */
		for(ii=0; ii<mypkt->size; ii++) {
			if(mypkt->payload[ii] != mypkt->pattern) {
				printf("paket broken\n");
				exit(0);
			}
		}

		/* freeing packet */
		net_packet_free(channelid,(void *)rpkt);
		net_packet_free(channelid,(void *)tpkt);

	       	/* os_thread_sleep(100000); */
	}
}

int main()
{
	int res;
	unsigned int thrs;
	net_addrinfo_t addr;

	/* OS initializing */
	os_init();

	/* address create */
	//net_addrinfo_make(&addr,NET_IP(192,168,2,251),SVC_NET_PORT);
	net_addrinfo_make(&addr,NET_IP(127,0,0,1),SVC_NET_PORT);

	/* port create */

	/* client network port create */
	cli_net_port = net_port_create(
		NET_PORT_TYPE_CLIENT,
		NET_PORT_PROTO_TCP,
		&addr,
		NULL,
		port_client_upcall );

	if(cli_net_port == ERROR) {
		printf("error in net_port_create()\n");
		exit(0);
	}

	/* socket */
	if( net_port_proto_socket(cli_net_port) == ERROR ) {
		printf("socket error\n");
		exit(0);
	}

	/* connect */
	if( net_port_proto_connect(cli_net_port,NULL) == ERROR ) {
		printf("socket error\n");
		exit(0);
	}

	/* channel buffer creation */
	if( (res=buff_create_class(CHANNEL_BUFF_ID,NULL,64*1024)) == ERROR) {
		printf("buff_class error\n");
		exit(0);
	}

	/* channel creation */
	if( (cli_channel_id = net_channel_create(CHANNEL_BUFF_ID,SVC_CHANNEL_PORT)) == ERROR) {
		printf("channel creation error\n");
		exit(0);
	}

	/* channel association */
	if( net_channel_register(cli_net_port,cli_channel_id) == ERROR ) {
		printf("channel register error\n");
		exit(0);
	}

	/* packet handler */
	if( net_port_proto_packet_handler_start(cli_net_port) == ERROR ) {
		printf("packet handler start error \n");
		exit(0);
	}

	/* do something */
	thrs = os_thread_create("thrclient",(os_thrfunc_t)os_txrx_thread,(void *)&cli_channel_id,(os_delfunc_t)NULL,0);
	if(thrs == ERROR) {
		printf("thread error\n");
		exit(0);
	}

	/* wait */
	while(1){
	       	os_thread_sleep(1000000);
		buff_verbose(CHANNEL_BUFF_ID);
	}

	return 0;
}
