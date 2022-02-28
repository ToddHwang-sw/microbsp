/*******************************************************************************
 *
 * net.h - Network module
 *
 ******************************************************************************/
#ifndef _NETWORK_MODULE_HEADER
#define _NETWORK_MODULE_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/* memory class information */
#define NET_MEM_CLASS   0x20300001
#define NET_MEM_CLASS_SIZE  2048*1024   /* 2Mbytes */

/* payload length range */
#define NET_MIN_PAYLOAD 4       /* minimum payload length */
/* #define NET_MAX_PAYLOAD 1400 */       /* maximum payload length */
#define NET_MAX_PAYLOAD (512*1024)        /* maximum payload length */
#define NET_PAYLOAD_LEN_CHECK(s)        (((s)>NET_MAX_PAYLOAD)||((s)<NET_MIN_PAYLOAD))

#define NET_PACKET_PREAMBLE "uNETPKT"    /* preamble string */

/* network protocol header structure */
typedef struct
{
	unsigned char preamble[12];     /* preamble */
	unsigned int type;          /* type */
#define NET_PACKET_NULL_TYPE    0xFFFFFFFF

	unsigned int size;          /* size */
	unsigned short vsport;      /* source port # */
	unsigned short vdport;      /* destination port # */
	unsigned int option;        /* option */
	unsigned int checksum;      /* checksum */
}__attribute__((packed)) net_packet_hdr;

/* net packet structure */
typedef struct
{
	net_packet_hdr h;
	void *payload;
	void *priv;             /* It may be used for multiple purpose.  */
	/* User-level packet tagging is possible */
	/* through this field or any kind of     */
	/* packet-level diagnostics can be done  */
	/* through this field                    */
}__attribute__((packed)) net_packet;
#define NET_PACKET_HEADER(x)  ((x)->h)
#define NET_PACKET_PAYLOAD(x) ((x)->payload)
#define NET_PACKET_PRIVATE(x) ((x)->priv)

/* Internal message only for network use */
#define NET_PACKET_MESSAGE      0x5a3a2a1a

/*
 * It may happen that TX/RX APIs (sendto/recvfrom/send/recv) return zero
 * under any arbitrary reason. Otherwise, such a thing is very usual in terms
 * of non-blocking I/O mode. Therefore, we provide TX/RX retry counter and
 * it a thread attempts to TX/RX as many as NET_PORT_MAX_RETRY_COUNTER.
 *
 */
#define NET_PORT_MAX_RETRY_COUNTER  5
#define NET_PORT_MAX_RETRY_COUNTER_EX   5000        

/* address information */
#define NET_ADDR_LEN  32
typedef struct {
	unsigned int ip;        /* ip address */
	unsigned int port;          /* port # */
	unsigned char addr[NET_ADDR_LEN]; /* struct sockaddr_in */
	unsigned int addrsize;      /* address size */
}__attribute__((packed)) net_addrinfo_t;

/* channel block */
typedef struct {
	unsigned char name[STR_LEN]; /* name */
	unsigned int buff;          /* buffer */
	unsigned int cnum;          /* channel number */
	unsigned int total_bytes;   /* total bytes */
	unsigned int packets;       /* packet # */
	unsigned int pktq;          /* packet queue */
	unsigned int msgq;          /* message queue */
	unsigned int lock;          /* lock */
	unsigned char disabled;     /* activate/deactivate flag */
	void *       port;          /* port id  - only for TX use */
}__attribute__((aligned)) net_channel_st;

/* port block */
typedef struct {
	unsigned int id;
#define NET_PORT_ID(x)  ((x)->id)

	unsigned char name[STR_LEN];        /* name */
#define NET_PORT_NAME(x)    ((x)->name)

#define NET_PORT_TYPE_CLIENT    0x01        /* client */
	/* only 1 server type - multiple server */
#define NET_PORT_TYPE_SERVER    0x02        
	unsigned int type;                  /* TCP/UDP */
#define NET_PORT_TYPE(x)    ((x)->type)

	net_addrinfo_t addr;            /* address information */

#define NET_PORT_STATUS_SOCKET  0x01        /* socket() */
#define NET_PORT_STATUS_CONN    0x02        /* connect() */
#define NET_PORT_STATUS_BIND    0x04        /* bind() */
#define NET_PORT_STATUS_ACCEPT  0x08        /* accept() */
#define NET_PORT_STATUS_ACTIVE  0x10        /* activated */
#define NET_PORT_STATUS_FULL_SERVER \
	(NET_PORT_STATUS_SOCKET| \
	 NET_PORT_STATUS_BIND| \
	 NET_PORT_STATUS_ACCEPT| \
	 NET_PORT_STATUS_ACTIVE)
	unsigned char status;               /* port status */
#define NET_PORT_STATUS(x,y)    (((x)->status)&(y))
	/* macro */

	unsigned int fd;                /* socket id */
#define NET_PORT_FD(x)  ((x)->fd)

	OS_LIST *chanlist;                  /* list of channel with an association */

	/*
	 * just for multi-server support */
	unsigned int copycount;             /* number of copy*/

#define NET_PORT_SIG_KILL       0x00000001
#define NET_PORT_SIG_DELETE     0x00000002
	unsigned int sig;               /* signal */

	unsigned int lock;                  /* lock for TX */

	void *proto;                    /* protocol block */

	unsigned int func;                  /* Rx function */

	void * (*pktfunc)(void *);              /* packet function */

#define NET_MESG_NEW_PORT       0x00110001
#define NET_MESG_DEL_PORT       0x00110002
#define NET_MESG_ACCEPT     0x00110003
	void (*upcall)(unsigned int,unsigned int,void *); /* upcall function */

	/*
	 * Incoming packet will not be elevated up to a queue
	 * within an associated channel. User may catch up the
	 * packet only by using net_port_packet_recv().
	 * However, they may be able to use net_packet_send()
	 * if any of channel are associated with. Otherwise,
	 * the only way to send a packet is to use net_port_packet_send()
	 * which directly buries the packet into the port.
	 */
#define NET_PORT_DISABLE        0x00000001
#define NET_PORT_NONBLOCK_DISABLE   0x00000002
#define NET_PORT_KEEPALIVE_DISABLE  0x00000004
	unsigned int option;                /* option */

	/* temp packet container */
	net_packet *interface_packet;       /* packets directly fetched from interface */

	/* errno for port */
	unsigned int port_errno;            /* errno for port */

	/* non-blocking mode / blocking mode
	 */
	unsigned char block_mode;

	unsigned int payload_size;              /* payload size */

	unsigned int catchup_delay;             /* packet handler repetition delay */

}__attribute__((aligned)) net_port_st;

/* protocol block */
typedef struct {
	int (*net_socket)(net_port_st *);               /* socket -> BSD */
	int (*net_connect)(net_port_st *,net_addrinfo_t *);     /* connect */
	int (*net_bind)(net_port_st *,net_addrinfo_t *);    /* bind */
	int (*net_listen)(net_port_st *);               /* listen */
	int (*net_accept)(net_port_st *,net_addrinfo_t *);      /* accept */
	int (*net_send)(net_port_st *,void *,int,net_addrinfo_t *); /* sendto/write */
	int (*net_recv)(net_port_st *,void *,int,net_addrinfo_t *); /* recvfrom/read */
	int (*net_close)(net_port_st *);                /* close */
	int (*net_ioctl)(net_port_st *,unsigned int,void *);        /* ioctl */
}__attribute__((aligned)) net_protocol_st;

/* initialization protocol block */
extern void os_net_proto_init(void);

/* prototypes */

/* init */
extern unsigned int net_init(void);

/* address information */
#define NET_ANY_IP          0xFFFFFFFF
extern unsigned int net_addrinfo_make(net_addrinfo_t *info,
                                      unsigned int ip,
                                      unsigned int port);
extern unsigned int net_addrinfo_get(net_addrinfo_t *info,
                                     unsigned int *ip,
                                     unsigned int *port);

/* port */
#define NET_PORT_PROTO_UDP      0x0101
#define NET_PORT_PROTO_TCP      0x0202
extern unsigned int net_port_create(unsigned int type,
                                    unsigned int proto,
                                    net_addrinfo_t *addr,
                                    void * (*pktfunc)(void *),
                                    void (*upcall)(unsigned int,unsigned int,void *));
extern unsigned int net_port_duplicate(unsigned int portid,int newsock,unsigned int newstatus);
extern unsigned int net_port_activate(unsigned int portid);
extern unsigned int net_port_delete(unsigned int portid);
extern unsigned int net_port_cleanup(unsigned int portid);
extern void         net_port_bcast_channel(net_port_st * portid);
extern net_port_st * net_port_search(unsigned int portid);
extern void         net_port_errno_set(unsigned int portid,unsigned int errcode);
extern int          net_port_errno_get(unsigned int portid);

/* flags */
#define NET_PORT_FD_SET     0x10000001
#define NET_PORT_AVAIL_SET  0x10000002
#define NET_PORT_BUFF_SET   0x10000003
extern unsigned int net_port_change(unsigned int portid,unsigned int flag,void *arg);

/* channels */
extern unsigned int net_channel_create(unsigned int buffid, unsigned int port);
extern unsigned int net_channel_register(unsigned int port,unsigned int num);
extern unsigned int net_channel_flush(unsigned int channelid);
extern unsigned int net_channel_delete(unsigned int channelid);
extern net_packet * net_packet_make(
	unsigned int schannelid,
	unsigned short dchannel,
	unsigned int type,
	unsigned short size,
	unsigned int option,
	void           *payload);
extern unsigned int net_packet_free(unsigned int channelid,net_packet *pkt);
extern unsigned int net_packet_send(unsigned int channelid,net_packet *pkt);
extern net_packet * net_packet_recv(unsigned int channelid,unsigned int bmode);

#define NET_CHANNEL_BUFFER      0x00000001
#define NET_CHANNEL_ENABLE      0x00000002
#define NET_CHANNEL_DISABLE     0x00000003
extern unsigned int net_channel_status(unsigned int command,unsigned int channelid);

/* protocol block */
extern net_protocol_st * os_net_proto_select(unsigned int type);
extern unsigned int os_net_proto_identify(net_port_st *port);

/* protocol access function for BSD compliant */
extern unsigned int net_port_proto_socket(unsigned int portid);
extern unsigned int net_port_proto_close(unsigned int portid);
extern unsigned int net_port_proto_connect(unsigned int portid,net_addrinfo_t *addr);
extern unsigned int net_port_proto_bind(unsigned int portid,net_addrinfo_t *addr);
extern unsigned int net_port_proto_listen(unsigned int portid);
extern unsigned int net_port_proto_accept(unsigned int portid,net_addrinfo_t *addr);
extern unsigned int net_port_proto_send(
	unsigned int portid,
	unsigned char *buff,
	unsigned int sz,
	net_addrinfo_t *addr);
extern unsigned int net_port_proto_recv(
	unsigned int portid,
	unsigned char *buff,
	unsigned int sz,
	net_addrinfo_t *addr);
extern unsigned int net_port_proto_isconnected(unsigned int portid);
extern unsigned int net_port_proto_isblockmode(unsigned int portid);
extern unsigned int net_port_proto_packet_handler_start(unsigned int portid);

/*
 * command field
 */
/* channeling */
#define NET_PORT_ENABLE_ACTION      0x00000001
#define NET_PORT_DISABLE_ACTION         0x00000002
/* blocking mode */
#define NET_PORT_ENABLE_NONBLOCK        0x00000003
#define NET_PORT_DISABLE_NONBLOCK       0x00000004
/* keepalive mode */
#define NET_PORT_ENABLE_KEEPALIVE           0x00000005
#define NET_PORT_DISABLE_KEEPALIVE          0x00000006
/* address information get */
#define NET_PORT_GET_ADDR_INFO      0x00000007
#define NET_PORT_SET_ADDR_INFO      0x00000008
/* socket address information get */
#define NET_PORT_GET_SOCK_ADDR_INFO     0x00000009
/* payload size setting */
#define NET_PORT_GET_PAYLOAD_SIZE       0x0000000A
#define NET_PORT_SET_PAYLOAD_SIZE       0x0000000B
/* delay setting */
#define NET_PORT_GET_DELAY_TIME     0x0000000c
#define NET_PORT_SET_DELAY_TIME     0x0000000d
/* LINGER option */
#define NET_PORT_SO_LINGER                  0x0000000e

/* option processing */
extern unsigned int net_port_option_configure(unsigned int portid,unsigned int command,void *arg);

/* useful definitions */
#define NET_IP(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

/*
 * To adapt Windows application Return values...
 */
#define SOCKET_ERROR                (-1)
//#define MSG_DONTWAIT    0x40            /* Nonblocking i/o for this operation only */

#ifdef __cplusplus
}
#endif

#endif /* UTVSW_NETWORK_MODULE_HEADER */


