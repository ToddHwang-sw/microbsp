diff -uNr linux-rpi-5.15.y-orig/drivers/net/usb/lan78xx.c linux-rpi-5.15.y/drivers/net/usb/lan78xx.c
--- linux-rpi-5.15.y-orig/drivers/net/usb/lan78xx.c	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/drivers/net/usb/lan78xx.c	2023-05-14 14:17:07.297109859 -0700
@@ -29,6 +29,9 @@
 #include <linux/of_mdio.h>
 #include <linux/of_net.h>
 #include "lan78xx.h"
+#ifdef CONFIG_NET_NATBYP
+#include <net/natbyp.h>
+#endif
 
 #define DRIVER_AUTHOR	"WOOJUNG HUH <woojung.huh@microchip.com>"
 #define DRIVER_DESC	"LAN78XX USB 3.0 Gigabit Ethernet Devices"
@@ -3245,6 +3248,10 @@
 	struct lan78xx_net *dev = netdev_priv(net);
 	struct sk_buff *skb2 = NULL;
 
+#ifdef CONFIG_NET_NATBYP
+	natbyp_egress(skb);
+#endif
+
 	if (test_bit(EVENT_DEV_ASLEEP, &dev->flags))
 		schedule_delayed_work(&dev->wq, 0);
 
@@ -3425,6 +3432,14 @@
 	if (skb_defer_rx_timestamp(skb))
 		return;
 
+#ifdef CONFIG_NET_NATBYP
+	{
+		int rc = natbyp_ingress(skb);
+		if (rc == NATBYP_BYPASSED)
+			return;
+	}
+#endif
+
 	status = netif_rx(skb);
 	if (status != NET_RX_SUCCESS)
 		netif_dbg(dev, rx_err, dev->net,
diff -uNr linux-rpi-5.15.y-orig/include/linux/skbuff.h linux-rpi-5.15.y/include/linux/skbuff.h
--- linux-rpi-5.15.y-orig/include/linux/skbuff.h	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/include/linux/skbuff.h	2023-05-14 14:17:07.297109859 -0700
@@ -958,6 +958,16 @@
 	__u16			network_header;
 	__u16			mac_header;
 
+#ifdef CONFIG_NET_NATBYP
+	/*
+	 * NATBYP - net/core/natbyp.c
+	 */
+	void *			natbyp_wlist; /* wait list */
+    void *			natbyp_flow;
+    __u32			natbyp_bypassed;
+    __u32			natbyp_etime;
+#endif /* CONFIG_NET_NATBYP */
+
 #ifdef CONFIG_KCOV
 	u64			kcov_handle;
 #endif
diff -uNr linux-rpi-5.15.y-orig/include/net/natbyp.h linux-rpi-5.15.y/include/net/natbyp.h
--- linux-rpi-5.15.y-orig/include/net/natbyp.h	1969-12-31 16:00:00.000000000 -0800
+++ linux-rpi-5.15.y/include/net/natbyp.h	2023-05-14 14:17:07.297109859 -0700
@@ -0,0 +1,35 @@
+#ifndef __NATBYP_HEADERS__
+
+#define NATBYP_BYPASSED		0x01
+#define NATBYP_INSERTED		0x02
+#define NATBYP_REMOVED		0x03
+
+/*
+ * NAT direction
+ */
+#define NATBYP_UL		0x00
+#define NATBYP_DL		0x01
+extern int  natbyp_direction(struct net_device *dev);
+
+/*
+ * Entrance to NAT queue
+ */
+extern int  natbyp_ingress(struct sk_buff *skb);
+
+/*
+ * Exit from NAT queue
+ */
+extern int  natbyp_egress(struct sk_buff *skb);
+
+/*
+ * Freeing up resources
+ */
+extern void natbyp_destructor(struct sk_buff *skb);
+
+/*
+ * Eventing
+ */
+#define NATBYP_EVT_DEVDEL 0x0001
+extern void natbyp_fastev( int evt , void * param);
+
+#endif /* __NATBYP_HEADERS__ */
diff -uNr linux-rpi-5.15.y-orig/net/core/Makefile linux-rpi-5.15.y/net/core/Makefile
--- linux-rpi-5.15.y-orig/net/core/Makefile	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/net/core/Makefile	2023-05-14 14:17:07.297109859 -0700
@@ -37,3 +37,4 @@
 obj-$(CONFIG_BPF_SYSCALL) += sock_map.o
 obj-$(CONFIG_BPF_SYSCALL) += bpf_sk_storage.o
 obj-$(CONFIG_OF)	+= of_net.o
+obj-$(CONFIG_NET_NATBYP)	+= natbyp.o
diff -uNr linux-rpi-5.15.y-orig/net/core/natbyp.c linux-rpi-5.15.y/net/core/natbyp.c
--- linux-rpi-5.15.y-orig/net/core/natbyp.c	1969-12-31 16:00:00.000000000 -0800
+++ linux-rpi-5.15.y/net/core/natbyp.c	2023-05-14 14:19:56.481976178 -0700
@@ -0,0 +1,2345 @@
+#if defined(CONFIG_NETFILTER)
+#include <linux/module.h>
+#include <linux/types.h>
+#include <linux/kernel.h>
+#include <linux/version.h>
+#include <linux/slab.h>
+#include <linux/sched.h>
+#include <linux/timer.h>
+#include <linux/kthread.h>
+#include <linux/list.h>
+#include <linux/completion.h>
+#include <linux/proc_fs.h>
+#include <linux/seq_file.h>
+#include <linux/skbuff.h>
+#include <linux/if_ether.h>
+#include <linux/etherdevice.h>
+#include <linux/interrupt.h>
+#include <linux/netdevice.h>
+#include <linux/ip.h>
+#include <linux/tcp.h>
+#include <net/natbyp.h>
+
+/* Debugging Flag */
+//#define DEBUG_TIME_ESTIMATE
+
+/* Monitor Frequency */
+#define NATBYP_TIME_SCALE 2
+
+/* macros to probe TCP ACK packet */
+#define NATBYP_IS_TCP_ACK(iph,tcph) \
+	(((tcph)->ack) && (ntohs((iph)->tot_len) == (((iph)->ihl << 2) + ((tcph)->doff << 2))))
+
+/* 16 devices total */
+#define MAX_NATBYP_DEVS  16
+
+/* How many LAN devices */
+#define MAX_LAN_DEV 8
+
+/*
+ * For stability reason, we replace list processing by simple iteration. 
+ */
+#define NATBYP_VERSION	"ver 1.5.1 (2023.05)"
+
+/*
+ * NAT socket flag 
+ */
+#define NATBYP_FLAG_BYPASSED 	0x80000000
+#define NATBYP_FLAG_MARKER		0x40000000
+
+/*
+ * Log mode 
+ */
+#define NATBYP_VERBOSE_NONE		0x0
+#define NATBYP_VERBOSE_FLOW		0x1
+#define NATBYP_VERBOSE_STATE	0x2
+#define NATBYP_VERBOSE_PACKET	0x3
+
+/* verbose flag */
+static u16 natbyp_verbose = NATBYP_VERBOSE_STATE;
+
+/* printk */
+#define natbyp_print(level, fmt,args...)	\
+		{ \
+		if( natbyp_verbose >= NATBYP_VERBOSE_ ##level ) \
+			printk(KERN_INFO "[NATBYP : %-12.12s] I "fmt,__FUNCTION__,##args); \
+		}
+#define natbyp_errmsg(fmt,args...)	printk(KERN_INFO "[NATBYP : %-12.12s] E "fmt,__FUNCTION__,##args)
+
+/* device table structure */
+typedef struct {
+	u8 used;
+	struct net_device *dev;
+}__attribute__((aligned)) natbyp_dev_t;
+
+/* size of socket queue */
+#define MAX_SKB_QUEUE 			256
+
+/* queue structure to store socket buffer */
+typedef struct {
+
+	/*
+ 	* Socket buffers temporarily saved in this list to keep packet order. 
+ 	*/
+	u32 head;
+	u32 tail;
+	u32 occu;   /* occupancy */
+
+	struct sk_buff *skbs[ MAX_SKB_QUEUE ];
+}__attribute__((aligned)) skbq_t;
+
+/* NAT TCP flow container */
+typedef struct {
+#define NATBYP_MAGIC_4BYTES		0xbabeface
+	u32 magic;       /* magic data */
+	u8  used;        /* used bit */
+	u16 index;       /* index */
+
+	/* NAT information */
+	u32 ip;          /* destination IP */
+	u16 port;        /* destination Port */
+	u32 sip;         /* source IP */
+	u16 sport;       /* source Port */
+
+	u8  dir;	       /* flow direction - NATBYP_UL/NATBYP_DL */
+
+	u32 nated_ip;		            /* NAT destination IP */
+	u16 nated_port;	            /* NAT destination Port */
+	u16 nated_sport;	            /* NAT source Port - only available in UL traffic */
+	u8  nated_ethhdr[ETH_HLEN];	/* NAT ethernet header */
+
+	natbyp_dev_t *nated_dev; /* NAT device */
+#define NATDEV(x)	(struct net_device *)(((x)->nated_dev)->dev)
+	u8  mapped; 		/* NAT bypass mapped !! */
+
+	/* operation statistics */
+	struct {
+#define NATBYP_SLOT_TEST			0x01
+#define NATBYP_SLOT_REMOVE			0x02
+#define NATBYP_SLOT_BYPASS_READY	0x03
+#define NATBYP_SLOT_BYPASS_ACT		0x04
+		u8 mode;	/* mode */
+
+		/* marking state */
+#define NATBYP_BYPASS_MARK_INIT   	0x00
+#define NATBYP_BYPASS_MARK_READY  	0x01
+#define NATBYP_BYPASS_MARK_WAIT   	0x02
+#define NATBYP_BYPASS_MARK_DONE   	0x03
+		u8 mark_state;
+
+		/*
+		 * At least more subsequent transactions should happen at each stage. 
+		 * With no extra transactions, the counter keeps descreasing and it 
+		 * will free up corresponding flow resource. 
+		 */
+		s32 expiry_count;
+	}op;
+
+	u32 count;	    /* number of skbs */
+	u32 dropnum;   /* packets dropped */
+	u32 bytes;     /* packet bytes */
+	u32 bw;        /* latest bandwidth */
+	u8  ack;       /* Is this flow for ACK ? */
+
+	skbq_t skbq;  /* socket buffer queue */
+
+}__attribute__((aligned)) natbyp_flow_t;
+
+/*
+ * NATBYP_UL/NATBYP_DL  - include/net/natbyp.h 
+ */
+#define NUM_FLOW_TYPE 2
+#define dirname(n)	(((n) == NATBYP_UL)?"UL":(((n) == NATBYP_DL)?"DL":"??"))
+
+/* Device name length */
+#define NAMELEN	32
+
+enum{
+	LAN_TYPE = 0,
+	WAN_TYPE,
+	NUM_INT_TYPE,
+};
+
+static struct net_device *wandev_db = NULL;
+static struct net_device *landev_db[ MAX_LAN_DEV ]  = {0,};
+
+static u8   wandev_name[ NAMELEN ] = {0,};
+static u8   landev_name[ MAX_LAN_DEV ][ NAMELEN ] = {{0,},};
+
+/* NAT TCP flow information */
+typedef struct {
+	struct timer_list timer;
+
+	/* 
+	 * Expiry at each stage...
+	 */
+
+	/* 2 seconds */
+	#define DEFAULT_NATBYP_SLOT_MAX_EXPIRY 4
+	s32 expiry_max_count;
+
+	/* system drop number */
+	u32 pkt_drop[ NUM_FLOW_TYPE ];
+
+	/* Intentional ACK window update */
+	u8  window_update;
+
+#define MAX_WINDOW_SIZE	32
+#define MIN_WINDOW_SIZE	2
+#define MAX_WINDOW_LIMIT	(0xfff0)
+#define DEFAULT_WINDOW_SIZE	4
+	u32 window_scale;
+
+	/*
+ 	* NATBYP works for ACK packet ??
+ 	*/
+	u8  ack_handle;
+
+	/* 
+	 * flow forwarder
+	 *
+	 *
+	 * FLOW[0]   QUEUE  ----+
+	 * FLOW[1]   QUEUE  ---+|
+	 * FLOW[2]   QUEUE  -+ ||  +-->  FLOW TASK[ UL ] --> DEV QUEUE[ UL ]
+	 * FLOW[3]   QUEUE  -+-++--+ 
+	 * ...               | |   +-->  FLOW TASK[ DL ] --> DEV QUEUE[ DL ]
+	 * FLOW[N-1] QUEUE  -+ |
+	 * FLOW[N]   QUEUE  ---+
+	 *
+	 *
+	 */
+	struct task_struct *flow_task[ NUM_FLOW_TYPE ];
+	wait_queue_head_t flow_waitq[ NUM_FLOW_TYPE ];
+	u32 flow_task_id[ NUM_FLOW_TYPE ]; /* thread parameter */
+
+	/* Counter variables how many packets are processed */
+	u32 pkt_count[ NUM_FLOW_TYPE ];
+
+	/* Device array */
+	natbyp_dev_t ndevs[ MAX_NATBYP_DEVS ];
+
+}__attribute__((aligned)) natbyp_db_t;
+
+static natbyp_db_t * natbyp_db;
+
+// String operation .. 
+static __inline__ int _strcmp(const char *a, const char *b) 
+{
+	if (b)
+		return strncmp(a,b,strlen(b));
+	else
+		return (a) ? 1 : 0 /* !a && !b */;
+}
+
+/* Device operation function */
+
+/* Scanning Device List */
+static __inline__ void * natbyp_dev_search( struct net_device *netdev )
+{
+	int index;
+
+	if (!netdev)
+		return NULL;
+
+	for( index = 0; index < MAX_NATBYP_DEVS; index++ ) {	
+		if ( natbyp_db->ndevs[ index ].used &&
+				natbyp_db->ndevs[ index ].dev == netdev ) {
+			return &natbyp_db->ndevs[ index ];
+		}
+	}
+	return NULL;
+}
+
+/* insert */
+static int natbyp_dev_insert( struct net_device *netdev )
+{
+	int index;
+
+	for( index = 0; index < MAX_NATBYP_DEVS; index++ ) {	
+		if ( !natbyp_db->ndevs[ index ].used ) {
+			natbyp_db->ndevs[ index ].dev = netdev;
+			natbyp_db->ndevs[ index ].used = 1;
+			return index;
+		}
+	}
+	natbyp_errmsg("NATBYP[ERR]-INS/Too many devices\n");
+	return (-1);
+}
+
+/* delete */
+static int natbyp_dev_delete( struct net_device *netdev )
+{
+	int index;
+
+	for( index = 0; index < MAX_NATBYP_DEVS; index++ ) {	
+		if ( natbyp_db->ndevs[ index ].used &&
+				(natbyp_db->ndevs[ index ].dev == netdev) ) {
+			natbyp_db->ndevs[ index ].dev = NULL;
+			natbyp_db->ndevs[ index ].used = 0;
+			return index;
+		}
+	}
+	return (-1);
+}
+
+/* RESET COUNTER */
+static __inline__ void natbyp_reset_counter( natbyp_flow_t * flow )
+{
+	flow->op.expiry_count = natbyp_db->expiry_max_count;
+}
+
+/* CREATE */
+static __inline__ int natbyp_skbq_init( skbq_t *queue )
+{
+	queue->head = 0;
+	queue->tail = 0;
+	queue->occu = 0;
+
+	return 0;
+}
+
+/* EMPTY */
+static __inline__ int natbyp_skbq_empty( skbq_t *queue )
+{
+	return (queue->tail == queue->head)? 1 : 0;
+}
+
+/* PUT */
+static __inline__ int natbyp_skbq_put( skbq_t *queue, struct sk_buff *skb )
+{
+	int ntail;
+
+	if ( !queue || !skb ) 
+		return -1;
+
+	/* next tail */
+	ntail = queue->tail + 1;
+	if (ntail >= MAX_SKB_QUEUE) 
+		ntail = 0;
+
+	if (ntail == queue->head)
+		return -1; /* queue full */
+
+	queue->skbs[ queue->tail ] = skb;
+
+	/* update tail */
+	queue->tail = ntail;
+
+	++ (queue->occu);
+
+	return 0; /* success !! */
+}
+
+/* GET */
+static __inline__ struct sk_buff * natbyp_skbq_get( skbq_t *queue )
+{
+	struct sk_buff * pskb;
+	int nhead;
+
+	if ( !queue ) 
+		return NULL;
+
+	/* empty queue ? */
+	if (queue->head == queue->tail)
+		return NULL;
+
+	/* next head */
+	nhead = queue->head + 1;
+	if (nhead >= MAX_SKB_QUEUE) 
+		nhead = 0;
+
+	pskb = queue->skbs[ queue->head ];
+
+	/* update head */
+	queue->head = nhead;
+
+	-- (queue->occu);
+
+	return pskb; /* success !! */
+}
+
+/* test function  - check NAT information is changed !! */
+static __inline__ int tcp_flow_changed( natbyp_flow_t * flow , struct iphdr *iph , struct tcphdr *tcph )
+{
+	int result = 1; /* changed by default */
+
+	if( !flow || !iph || !tcph )
+		return 1; /* changed */
+
+	switch( flow->dir ) {
+	case NATBYP_DL:
+		result =  ((flow->nated_ip   != iph->daddr) ||
+				   (flow->nated_port != tcph->dest)) ? 1 : 0;
+		break;
+	case NATBYP_UL:
+		result =  ((flow->nated_ip   != iph->saddr) ||
+				   (flow->nated_port != tcph->dest)) ? 1 : 0;
+		break;
+	default:
+		natbyp_errmsg("FLOW[%-2d] Unknown direction (%08x) \n", flow->index , flow->dir);
+		break;
+	}
+
+	return result;
+}
+
+/* printout function */
+static void printout_natbyp_packet(const char *title, natbyp_flow_t * flow, struct sk_buff *skb, int off, int ack)
+{
+	u32 saddr;
+	u32 daddr;
+  	struct iphdr *iph;
+   	struct tcphdr *tcph;
+	char ipinfo[128];
+	char tag;
+
+	/* header indexing */
+	iph = (struct iphdr *)((u8 *)skb->data + off);
+	tcph = (struct tcphdr *)((u8 *)iph + sizeof(struct iphdr));
+
+	/* IP addresses */
+	saddr = ntohl(iph->saddr);
+	daddr = ntohl(iph->daddr);
+
+	memset(ipinfo, 0, sizeof(ipinfo));
+	snprintf(ipinfo, sizeof(ipinfo)-1, "%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d",
+		(saddr & 0xff000000) >> 24,
+		(saddr & 0x00ff0000) >> 16,
+		(saddr & 0x0000ff00) >> 8,
+		(saddr & 0x000000ff) >> 0,
+		ntohs(tcph->source),
+		(daddr & 0xff000000) >> 24,
+		(daddr & 0x00ff0000) >> 16,
+		(daddr & 0x0000ff00) >> 8,
+		(daddr & 0x000000ff) >> 0,
+		ntohs(tcph->dest));
+
+	/* symbol */
+	if (flow->op.mark_state == NATBYP_BYPASS_MARK_READY) {
+		tag = '@';
+	} else if (flow->op.mark_state == NATBYP_BYPASS_MARK_WAIT) {
+		tag = '#';
+	} else if (flow->op.mark_state == NATBYP_BYPASS_MARK_DONE) {
+		if (skb->natbyp_bypassed & NATBYP_FLAG_BYPASSED) {
+			tag = '>';
+		} else if (ack) {
+			tag = '<';
+		} else {
+			tag = ' ';
+		}
+	} else {
+		tag = '?';
+	}
+
+	natbyp_print(PACKET, "FLOW[%-2d] %3s %-3d %-8s [%-42s] %-4d %04x %04x %08x %08x %c\n",
+		flow->index,
+		title,
+		flow->count,
+		skb->dev->name,
+		ipinfo,
+		ntohs(iph->tot_len) - (iph->ihl*4),
+		ntohs(iph->id),
+		ntohs(iph->check),
+		ntohl(tcph->seq),
+		ntohl(tcph->ack_seq),
+		tag);
+}
+
+/*
+ *
+ * Generic operations 
+ *
+ */
+
+/* nat flow status */
+static char * natbyp_flow_status[5] = 
+			{ 
+			"UNKNOWN" ,
+			"TEST"    ,
+			"REMOVE"  ,
+			"READY"   ,
+			"BYPASS" 
+			};
+
+/* flow sanity check */
+#define natbyp_flow_ok(f)	(((f)->magic) == NATBYP_MAGIC_4BYTES)
+
+/* 16 -- maximum NAT flow */
+#define MAX_NATBYP_FLOW 	16
+
+/* NAT TCP flow container */
+static natbyp_flow_t * natbyp_flow = NULL ;
+
+/* types for flow table - payload/ack */
+/*  for better readability */
+typedef struct {
+	u8 name[12];
+	natbyp_flow_t *q[2]; /* 0 -> HIGH / 1 -> LOW */
+}natbyp_flow_table_t;
+
+/*
+ * Global flags 
+ */
+static __u16 natbyp_activated = 0;
+
+/***
+ *
+ * P R I V A T E    F U N C T I O N S
+ *
+ */
+
+/*
+ *
+ * PLEASE DO NOT DELETE THIS COMMENTED SECTION !!
+ * THE APS iphdr_csum and tcphdr_csum HAVE BEEN COMPLETELY PROBED. 
+ *
+ */
+
+/* IP header */
+typedef struct{
+	__u16 	ihl_v_tos;
+	__u16	tot_len;
+	__u16	id;
+	__u16	frag_off;
+	__u16	ttl_proto;
+	__u16	unused; /* checksum */
+	__u16	saddr_l;
+	__u16	saddr_h;
+	__u16	daddr_l;
+	__u16	daddr_h;
+}iphdr_16bit_cls;
+
+/* TCP header */
+typedef struct{
+	__u16	sport;
+	__u16	dport;
+	__u16	seqno_l;
+	__u16	seqno_h;
+	__u16	ackno_l;
+	__u16	ackno_h;
+	__u16	hlen_flags;
+	__u16	wnd;
+	__u16	unused; /* checksum */
+	__u16	urgp;
+	__u16	payload[0]; 
+}tcphdr_16bit_cls;
+
+#if 0
+/* IP header checksum */
+static u16 iphdr_csum(struct iphdr *iph)
+{
+	u32 chksum = 0;
+	iphdr_16bit_cls *p = (iphdr_16bit_cls *)iph;
+
+	/* checksum calculation */
+	chksum += p->ihl_v_tos;
+	chksum += p->tot_len;
+	chksum += p->id;
+	chksum += p->frag_off;
+	chksum += p->ttl_proto;
+	chksum += p->saddr_l;
+	chksum += p->saddr_h;
+	chksum += p->daddr_l;
+	chksum += p->daddr_h;
+	chksum = (chksum >> 16) + (chksum & 0xFFFF);
+	chksum = (chksum >> 16) + chksum;
+	chksum = ~chksum;
+
+	return (u16)chksum;
+}
+#endif
+
+/* TCP header checksum */
+static u16 tcphdr_csum(struct tcphdr *tcph, struct iphdr *iph )
+{
+	u32 i;
+	u32 chksum = 0;
+	int payload_len;
+	u16 *px;
+	tcphdr_16bit_cls *p = (tcphdr_16bit_cls *)tcph;
+	iphdr_16bit_cls *pi = (iphdr_16bit_cls *)iph;
+
+	/* TCP payload length */
+	payload_len = 
+		iph->tot_len - ((iph->ihl*4) + (tcph->doff*4));
+
+	/* header checksum */
+	chksum += p->sport;
+	chksum += p->dport;
+	chksum += p->seqno_l;
+	chksum += p->seqno_h;
+	chksum += p->ackno_l;
+	chksum += p->ackno_h;
+	chksum += p->hlen_flags;
+	chksum += p->wnd;
+	chksum += p->urgp; 
+
+	/* pseudo header */
+	chksum += pi->daddr_l;
+	chksum += pi->daddr_h;
+	chksum += pi->saddr_l;
+	chksum += pi->saddr_h;
+	chksum += (u16)IPPROTO_TCP;
+	chksum += (u16)(tcph->doff << 2);
+
+	/* option fields */
+	for(i = 0;i < ((tcph->doff << 2) - sizeof(struct tcphdr))/sizeof(__u16); i++)
+		chksum += (u16)(p->payload[ i ]);
+
+	/* payload */
+	px = (u16 *)((char *)tcph + (tcph->doff << 2));
+
+	/* payload checksum */
+	for(i = 0;i < payload_len/sizeof(__u16); i++) {
+		chksum += (u16)(*px);
+		++ px;
+	}
+
+	chksum = (chksum >> 16) + (chksum & 0xFFFF);
+	chksum = (chksum >> 16) + chksum;
+	chksum = ~chksum;
+
+	return (u16)chksum;
+}
+
+#ifdef DEBUG_TIME_ESTIMATE
+/* obtaining current time */
+static u32 natbyp_current_time(void)
+{
+	struct timeval tv;
+
+	do_gettimeofday(&tv);
+	return (u32)( ((u32)tv.tv_sec * 1000000) + (u32)tv.tv_usec );
+}
+#endif /* ..._TIME_ESTIMATE */
+
+/* zapping */
+static void natbyp_squeeze( void )
+{
+	natbyp_flow_t * flow;
+	int index;
+	
+	for( index = 0; index < MAX_NATBYP_FLOW; index++ ) {
+		flow = (natbyp_flow_t *)&(natbyp_flow[ index ]);	
+
+		/* check sanity !! */
+		if ( !flow->used )
+			continue;
+
+		if ( (flow->op.mode == NATBYP_SLOT_REMOVE) ) {
+
+			/* check */
+			if ( flow->count < 0 )
+				natbyp_errmsg("UNDER FLOW (%d:%d)\n",flow->index,flow->count);
+
+			flow->used = 0; /* free up */
+		}
+	}
+
+}
+
+/* defined below.. */
+static int handle_single_packet( natbyp_flow_t * flow , struct sk_buff *skb );
+
+/* this is the same function to natbyp_get for ACK packet */
+static natbyp_flow_t * natbyp_ack_get( struct iphdr * iph , struct tcphdr *tcph )
+{
+	natbyp_flow_t * flow;
+	int index;
+	int matched;
+
+	for( index = 0; index < MAX_NATBYP_FLOW; index++ ) {
+		flow = (natbyp_flow_t *)&(natbyp_flow[ index ]);	
+	
+		if ( !flow->used )
+			continue;
+
+		if ( !(flow->op.mode == NATBYP_SLOT_BYPASS_ACT) &&
+					(flow->op.mark_state == NATBYP_BYPASS_MARK_DONE) )
+			continue;
+
+		matched = ( (iph->saddr   == flow->nated_ip) && 
+					(tcph->source == flow->port) )? 1 : 0;
+
+		if ( matched )
+			return flow;
+	}
+
+	return NULL;
+}
+
+/* return corresponding natbyp_flow container */
+static natbyp_flow_t * natbyp_get( struct iphdr * iph , struct tcphdr *tcph , int dir )
+{
+	natbyp_flow_t * flow;
+	int index;
+	int matched;
+
+	for( index = 0; index < MAX_NATBYP_FLOW; index++ ) {
+		flow = (natbyp_flow_t *)&(natbyp_flow[ index ]);	
+	
+		if ( !flow->used )
+			continue;
+
+		matched = 0; /* clear */
+
+		switch( dir ) {
+		case NATBYP_DL: /* DL */
+			matched = ( (iph->daddr  == flow->ip) && (tcph->dest  == flow->port))? 1 : 0;
+			break;
+		case NATBYP_UL: /* UL */
+			matched = ( (iph->saddr  == flow->ip) && (tcph->source == flow->port))? 1 : 0;
+			break;
+		default:
+			return NULL;
+		}
+
+		if ( matched )
+			return flow;
+	}
+
+	return NULL;
+}
+
+/* allocate an empty queue */
+static natbyp_flow_t * natbyp_allocate( struct iphdr * iph , struct tcphdr *tcph , int dir )
+{
+	int index;
+	natbyp_flow_t *flow;
+	u32 saddr, daddr;
+
+	/* Search empty slot */
+	for( index = 0; index < MAX_NATBYP_FLOW; index++ ) {
+		if ( !natbyp_flow[ index ].used ) 
+			break;
+	}
+
+	/* No empty flow */
+	if ( index == MAX_NATBYP_FLOW ) {
+
+		u32 saddr = ntohl(iph->saddr);
+		u32 daddr = ntohl(iph->daddr);
+
+		/* Overflow may happen !! */
+		natbyp_errmsg("ALLOC FAIL::OVERFLOW (%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d)\n",
+			(saddr & 0xff000000) >> 24,
+			(saddr & 0x00ff0000) >> 16,
+			(saddr & 0x0000ff00) >> 8,
+			(saddr & 0x000000ff) ,
+			ntohs(tcph->source),
+			(daddr & 0xff000000) >> 24,
+			(daddr & 0x00ff0000) >> 16,
+			(daddr & 0x0000ff00) >> 8,
+			(daddr & 0x000000ff) ,
+			ntohs(tcph->dest)); 
+		return NULL;
+	}
+
+	/* find & fetch ... */
+	flow = (natbyp_flow_t *)&(natbyp_flow[ index ]);
+	if ( !flow ) {
+		natbyp_errmsg("ALLOC FAIL::FREE NODE BROKEN!\n");
+		return NULL;
+	}
+
+	/* found !! */
+	flow->magic = NATBYP_MAGIC_4BYTES;
+
+	/* flow information */
+	switch( dir ) {
+	case NATBYP_DL: /* DL */
+		/* destination address information */
+		flow->ip    = iph->daddr;
+		flow->port  = tcph->dest;
+		/* source address information */
+		flow->sip   = iph->saddr;
+		flow->sport = tcph->source;
+		break;
+	case NATBYP_UL: /* UL */
+		/* destination address information */
+		flow->ip    = iph->saddr;
+		flow->port  = tcph->source;
+		/* source address information */
+		flow->sip   = iph->daddr;
+		flow->sport = tcph->dest;
+		break;
+	default:
+		natbyp_errmsg("ALLOC FAIL::UNKNOWN DIRECTION (%s) !\n",dirname( dir ));
+		return NULL;
+	}
+
+	/* flow direction */
+	flow->dir  			= dir;
+	flow->dropnum       = 0;
+	flow->count         = 0;
+	flow->bytes         = 0;
+	flow->bw            = 0;
+	flow->nated_ip 	    = 0;
+	flow->nated_port 	= 0;
+	flow->mapped		= 0;
+
+	/* flow type */
+	flow->ack           = 0; /* undetermined */
+
+	/* flow detection counter */
+	natbyp_reset_counter( flow );
+	flow->op.mode       = NATBYP_SLOT_TEST; /* Initial state */
+	flow->op.mark_state = NATBYP_BYPASS_MARK_INIT;
+
+	/* queue init */
+	natbyp_skbq_init( &(flow->skbq) );
+
+	/* Now we use this */
+	flow->used = 1;
+
+	saddr = ntohl(iph->saddr);
+	daddr = ntohl(iph->daddr);
+
+	/* Overflow may happen !! */
+	natbyp_print(STATE, "FLOW[%-2d] ALLOCED (%d.%d.%d.%d:%d -> %d.%d.%d.%d:%d) \n",
+		flow->index,
+		(saddr & 0xff000000) >> 24,
+		(saddr & 0x00ff0000) >> 16,
+		(saddr & 0x0000ff00) >> 8,
+		(saddr & 0x000000ff) ,
+		ntohs(tcph->source),
+		(daddr & 0xff000000) >> 24,
+		(daddr & 0x00ff0000) >> 16,
+		(daddr & 0x0000ff00) >> 8,
+		(daddr & 0x000000ff) ,
+		ntohs(tcph->dest)); 
+
+	/* start timer */
+	if ( !timer_pending(&(natbyp_db->timer)) ) {
+		mod_timer( &(natbyp_db->timer) , jiffies + (HZ/(NATBYP_TIME_SCALE)) );
+		natbyp_print(STATE, "FLOW TIMER START \n");
+	}
+
+	return flow;
+}
+
+/* insert a skb into queue */
+static void natbyp_flow_map( natbyp_flow_t * flow, struct sk_buff * skb )
+{
+	if (!flow)
+		return;
+
+	/* natbyp parent */
+	skb->natbyp_flow = (void *)flow;
+
+	/* count increase */
+	++ flow->count;
+
+	/* traffic measurement */
+	flow->bytes += skb->len;
+}
+
+/* delete a skb from queue */
+static int natbyp_flow_unmap( struct sk_buff * skb )
+{
+	natbyp_flow_t * flow;
+
+	if ( !skb->natbyp_flow ) {
+		natbyp_errmsg("SKB ALREADY UNMAPPED\n");
+		return 1;
+	}
+
+	/* flow */
+	flow = (natbyp_flow_t *)(skb->natbyp_flow);
+
+	/* sanity check */
+	if ( !natbyp_flow_ok( flow ) ) {
+		natbyp_errmsg("NATBYP BROKEN\n");
+		return 1;
+	}
+
+	skb->natbyp_flow = NULL;
+
+	/* count decrease */
+	-- flow->count ;
+	if ( flow->count < 0 )
+		/*
+ 		* 2010/08/12 
+ 		*	- We cannot find any case that a control falls into here. 
+ 		*/
+		natbyp_errmsg("FLOW(%d) UNDERFLOW\n",flow->index);
+
+	return 0;
+}
+
+/* timer function */
+static void natbyp_flow_mon( struct timer_list *unused )
+{
+	int counter = 0;
+	natbyp_flow_t *flow;
+	int index;
+
+	for(index = 0;index < MAX_NATBYP_FLOW;index++ ) {
+
+		flow = (natbyp_flow_t *)&(natbyp_flow[ index ]);
+
+		if( !flow ) {
+			natbyp_errmsg("BROKEN LIST\n");
+			return ;
+		}
+
+		if( !flow->used )
+			continue;
+	
+		/* sanity check */
+		if ( !natbyp_flow_ok(flow) ){
+			natbyp_errmsg("NATBYP[%d] BROKEN\n", flow->index );
+			continue;
+		}
+
+		/* update traffic information */
+		flow->bw = flow->bytes;
+		flow->bytes = 0;
+
+		/* depending on working mode */
+		switch( flow->op.mode ) {
+		case NATBYP_SLOT_TEST:
+		case NATBYP_SLOT_BYPASS_READY:
+		case NATBYP_SLOT_BYPASS_ACT:
+
+			/* local counter for activated flow */
+			++ counter;
+
+			/* Current slot is allocated and 
+ 				it is placed on a state monitoring any more next packets come in */
+			if ( --flow->op.expiry_count <= 0 ) {
+				/* This state is prevented by "natbyp_reset_counter" call.  */
+				flow->op.mode = NATBYP_SLOT_REMOVE;
+				flow->mapped = 0;  /* unmapped !! */
+				natbyp_print(STATE, "FLOW[%-2d] REMOVE *SILENT FLOW* \n", flow->index);
+			} else {
+				/* state transition */
+				switch( flow->op.mode ) {
+				case NATBYP_SLOT_TEST:
+					/* Next State */
+					flow->op.mode = NATBYP_SLOT_BYPASS_READY;
+					break;
+
+				case NATBYP_SLOT_BYPASS_READY:
+				case NATBYP_SLOT_BYPASS_ACT:
+
+					/* Update state */
+					if ( flow->op.mode == NATBYP_SLOT_BYPASS_READY ) {
+						if( flow->mapped )  {
+							/*
+ 							* 1st step. 
+ 							*
+ 							*  -> MARK_READY
+ 							*  	Socket buffer found in this flow will be mared. 
+ 							*
+ 							*/
+							flow->op.mark_state = NATBYP_BYPASS_MARK_READY;
+							flow->op.mode = NATBYP_SLOT_BYPASS_ACT;
+				
+							natbyp_print(STATE, "FLOW[%-2d] BYPASS_READY_ACT\n", flow->index );
+						} else {
+							/* Not mapped !! */
+							flow->op.mode = NATBYP_SLOT_REMOVE;
+							flow->mapped = 0;
+							natbyp_print(STATE, "FLOW[%-2d] CANCELLED *NO EGRESS* \n", flow->index);
+						}
+					} else {
+						/* NATBYP_SLOT_BYPASS_ACT */
+
+						/* status of 3 queues - flow/dev[UL]/dev[DL] */
+						skbq_t *skbq  = &(flow->skbq);
+						natbyp_print(FLOW, "FLOW[%-2d] BYPASS_ACT %6u %6d %6d\n", 
+							flow->index, 
+						   	skbq->occu,
+							natbyp_db->pkt_count[ NATBYP_DL ],
+							natbyp_db->pkt_count[ NATBYP_UL ]);
+
+						/* clear counters */
+						natbyp_db->pkt_count[ NATBYP_UL ] = 0;
+						natbyp_db->pkt_count[ NATBYP_DL ] = 0;
+
+					}
+					break;
+				}
+			}
+			break;
+
+		case NATBYP_SLOT_REMOVE:
+			flow->used = 0; /* free up */
+			natbyp_print(STATE, "FLOW[%-2d] FLUSHED \n", flow->index );
+			break;
+
+		default:
+			natbyp_errmsg("Unknown working mode [%d/%d] \n", flow->index , flow->op.mode );
+		}
+	}
+
+	/* next timer setup if any flow is pending */
+	if( counter ) {
+		/* next timer wake up - 1 second later */
+		mod_timer( &(natbyp_db->timer) , jiffies + (HZ/(NATBYP_TIME_SCALE)) );
+	}else{
+		natbyp_squeeze(); /* 2012/02/13 */
+		natbyp_print(STATE, "FLOW TIMER STOP\n");
+	}
+}
+
+/* shoot skb to driver transmit function */
+static int natbyp_skb_send( struct sk_buff * skb , u32 dir )
+{
+	natbyp_flow_t *flow;
+	int rc;
+	natbyp_dev_t *ndev;
+
+	/* flow */
+	flow = (natbyp_flow_t *)(skb->natbyp_flow);
+	if( !flow ) {
+		natbyp_errmsg("SKB BROKEN\n");
+		skb->dev = NULL; /* unlink device ~ */
+		dev_kfree_skb( skb );
+		return -1;
+	}
+
+	/* not used flow */
+	if ( !flow->used ) {
+		natbyp_errmsg("FLOW[%-2d] UNUSED\n", flow->index);
+		skb->dev = NULL; /* unlink device ~ */
+		dev_kfree_skb( skb );
+		return -1;
+	}
+
+	/* make it single node */
+	/* very important !! - It prevents chained resource free at kfree_skb() */
+	skb_orphan(skb);
+
+	/* check device */
+	ndev = (natbyp_dev_t *)flow->nated_dev;
+	if ( !ndev->used ) {
+		natbyp_errmsg("FLOW[%-2d] DEVICE CRASH (len=%d)\n",flow->index,skb->len);
+		skb->dev = NULL; /* unlink device ~ */
+		dev_kfree_skb( skb );
+		return -1;
+	}
+
+	natbyp_reset_counter( flow ); /* update counter */
+
+	/* device queue transmit */
+	rc = dev_queue_xmit( skb );
+	switch (rc) {
+	case NETDEV_TX_OK:
+		break;
+	case NETDEV_TX_BUSY:
+		natbyp_errmsg("FLOW[%-2d] DEV_TX_BUSY[%s] \n", flow->index, dirname(dir));
+		++ natbyp_db->pkt_drop[ dir ];
+		break;
+	default:
+		natbyp_errmsg("FLOW[%-2d] DEV_TX_DROP[%s] %08x \n", flow->index, dirname(dir), rc);
+		++ natbyp_db->pkt_drop[ dir ];
+		break;
+	}
+
+	return 0;
+}
+
+
+/* handle one packet */
+static int handle_single_packet( natbyp_flow_t * flow , struct sk_buff *skb )
+{
+	struct net_device * dev;
+
+	if( !flow || !skb )
+		return 0;
+
+	/* output device */
+ 	dev = skb->dev;
+	if ( !dev ) {
+		natbyp_errmsg("FLOW[%-2d] NULL DEV, SKB DELETED \n",flow->index);
+		dev_kfree_skb( skb );
+		return 0;
+	}
+
+	if (!((flow->dir == NATBYP_UL) || (flow->dir == NATBYP_DL))) {
+		natbyp_errmsg("FLOW[%-2d] INVALID DIRECTION\n",flow->index);
+		dev_kfree_skb( skb );
+		return 0;
+	}
+
+ 	/* No need to checksum */ 
+	skb->ip_summed = CHECKSUM_UNNECESSARY;
+
+	/* update window size */
+	/* only ACK frame is applied */
+	if( natbyp_db->window_update ) {
+		struct iphdr *iph;
+		struct tcphdr *tcph;
+	
+		/* IP header */
+		iph = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
+		/* TCP header */
+		tcph = (struct tcphdr *)((char*)iph + sizeof(struct iphdr));
+
+		if( NATBYP_IS_TCP_ACK(iph,tcph) ) {
+			u16 newwin;
+
+			/* IP & TCP header */
+			newwin = (u16)( tcph->window * natbyp_db->window_scale ) ;
+
+			if( (newwin < MAX_WINDOW_LIMIT) && 
+					(newwin > tcph->window )) {
+				/* boom up window size */
+				tcph->window = newwin;
+				/* Update checksum */
+				tcph->check = tcphdr_csum( tcph , iph );
+			}
+		}
+	} /* if( natbyp_db->window_update ) ... */
+
+	natbyp_reset_counter( flow ); /* update counter */
+
+	if (natbyp_skb_send( skb, flow->dir )) {
+		natbyp_errmsg("FLOW[%-2d] OVERFLOW\n", flow->index);
+		/* definitely this will cause TCP traffic shutdown */
+		dev_kfree_skb( skb );
+		return 0;
+	}
+
+	return 1; /* 1 packet */
+}
+
+/* actual packet handler ... */
+static int handle_packets( int dir )
+{
+	int index;
+	int count;
+	natbyp_flow_t *flow;
+	struct sk_buff *skb;
+	
+	count = 0;
+
+	/* navigates outstanding flow in round-robin manner */		
+	for(index = 0; index < MAX_NATBYP_FLOW; index++) {
+
+		flow = (natbyp_flow_t *)&(natbyp_flow[ index ]);
+
+		/* Check 1 */
+		if ( !flow->used )
+			continue;
+
+		/* filtering matching flow type.  */
+		if ( flow->dir != dir )
+			continue; 
+
+		/* Check 2 */
+		if ( !((flow->op.mode == NATBYP_SLOT_BYPASS_ACT) &&
+				(flow->op.mark_state == NATBYP_BYPASS_MARK_DONE)) )
+			continue;
+
+		/* processing packets */
+		skb = natbyp_skbq_get( &(flow->skbq) );
+		if ( skb )
+			count += handle_single_packet( flow , skb );
+
+	} /* for( index ... */
+
+	return count;
+}
+
+static bool natbyp_any_packets( int dir )
+{
+	int index;
+	natbyp_flow_t *flow;
+	
+	/* navigates outstanding flow in round-robin manner */		
+	for(index = 0; index < MAX_NATBYP_FLOW; index++) {
+
+		flow = (natbyp_flow_t *)&(natbyp_flow[ index ]);
+
+		/* Check 1 */
+		if ( !flow->used )
+			continue;
+
+		/* filtering matching flow type.  */
+		if ( flow->dir != dir )
+			continue; 
+
+		/* if any of packets */
+		if ( !natbyp_skbq_empty(&(flow->skbq)) )
+			return true;
+
+	} /* for( index ... */
+
+	return false;
+}
+
+/*
+ * Daemon process to directly push packet to destined device. 
+ *
+ */
+static int natbyp_forwarder( void * arg )
+{
+	int count, tot_count;
+	int dir = *(u32 *)arg;
+
+	tot_count = count = 0;
+
+	while(!kthread_should_stop()) { /* endless loop */
+	//while(1) { /* endless loop */
+
+		/* total sum initialize */
+		if (!count) {
+	
+			set_current_state(TASK_INTERRUPTIBLE);
+
+			/* statistics */	
+			natbyp_db->pkt_count[ dir ] += tot_count;
+
+			wait_event_interruptible(
+				(natbyp_db->flow_waitq[ dir ]), (natbyp_any_packets( dir ) == true));
+
+			tot_count = 0;
+
+			__set_current_state(TASK_RUNNING);
+		}
+
+		/* handle packets */	
+		count = handle_packets( dir );
+
+		tot_count += count;
+
+	} /* while( 1 ) */
+
+	natbyp_print(STATE, "[%s] TERMINATED \n", dirname(dir));
+	return 0;
+}
+
+/***
+ *
+ * P U B L I C     F U N C T I O N S
+ *
+ */
+
+/* delete from q queue */
+int natbyp_ingress(struct sk_buff *skb)
+{
+	int tolandir; /* direction */
+	struct iphdr *iph;
+	struct tcphdr *tcph;
+	natbyp_flow_t *flow;
+	int ret = NATBYP_INSERTED; /* default */
+
+	if ( !skb )
+		goto normal_traffic;
+
+	/* clear bypass flag */
+	skb->natbyp_bypassed = 0;
+
+	/* skb field initial */
+	skb->natbyp_flow = NULL;
+
+	/* check activated or not !! */
+	if ( !natbyp_activated )
+		goto normal_traffic;
+
+	iph = (struct iphdr *)skb->data;
+
+	/* check IPv4 */
+	if( iph->version != IPVERSION )
+		goto normal_traffic;
+
+	/* check TCP packet */
+	if( iph->protocol != IPPROTO_TCP ) {
+		if ( (iph->protocol == IPPROTO_ICMP)  && 
+					(natbyp_verbose == NATBYP_VERBOSE_PACKET) ) {
+
+			u32 saddr = ntohl(iph->saddr);
+			u32 daddr = ntohl(iph->daddr);
+
+			natbyp_print(STATE, "ICMP [%d.%d.%d.%d -> %d.%d.%d.%d] %d \n",
+					(saddr & 0xff000000) >> 24,
+					(saddr & 0x00ff0000) >> 16,
+					(saddr & 0x0000ff00) >> 8,
+					(saddr & 0x000000ff) >> 0,
+					(daddr & 0xff000000) >> 24,
+					(daddr & 0x00ff0000) >> 16,
+					(daddr & 0x0000ff00) >> 8,
+					(daddr & 0x000000ff) >> 0,
+					ntohs(iph->tot_len) );
+		}
+		goto normal_traffic;
+	}
+
+	/*
+	 * TCP packet ... 
+	 */
+
+	/* TCP packet header */
+	tcph = (struct tcphdr *)((char *)iph + sizeof(struct iphdr));
+
+	/* TCP control packet check */
+	if ( tcph->syn | tcph->rst | tcph->fin | tcph->ece | tcph->cwr )
+		goto normal_traffic;
+
+	/*
+	 * ACK printout
+	 */
+	tolandir = natbyp_direction(skb->dev);
+	switch (tolandir) {
+	case -1:
+		goto normal_traffic;
+	case NATBYP_DL:
+		break;
+	case NATBYP_UL:
+		if ( NATBYP_IS_TCP_ACK( iph, tcph ) ) {
+			natbyp_flow_t *flow = natbyp_ack_get( iph, tcph );
+			if (flow) {
+	
+				/* ACK packet is also another type of transaction */	
+				natbyp_reset_counter( flow ); 
+
+				if (natbyp_verbose == NATBYP_VERBOSE_PACKET) {
+					/*
+				 	* Only the case of BYPASS_ACT ..
+				 	*/
+					printout_natbyp_packet("ACK", flow, skb, 0, 1);
+				}
+			
+				/* TIP: ACK traffic --> push traffic */	
+				wake_up_interruptible( &(natbyp_db->flow_waitq[ flow->dir ]) );
+			}
+		} else {
+			goto normal_traffic;
+		}
+		break;
+	}
+
+	/* TCP ACK packet - no TCP data payload */
+	if ( !natbyp_db->ack_handle ) {
+		if ( NATBYP_IS_TCP_ACK( iph, tcph ) )
+			goto normal_traffic;
+	}
+
+	/* check allocated container */
+	flow = natbyp_get( iph , tcph , tolandir );
+	if(!flow){
+		if ((flow = natbyp_allocate( iph , tcph , tolandir )) == NULL) {
+			goto normal_traffic;
+		}
+	}
+
+#ifdef DEBUG_TIME_ESTIMATE
+	/* check time */
+	skb->natbyp_etime = natbyp_current_time();
+#else
+	skb->natbyp_etime = 0;
+#endif
+
+	if (flow->op.mode == NATBYP_SLOT_REMOVE)
+		/*
+ 		* NATBYP_SLOT_REMOVE 
+ 		* 	- Slot to be deleted ( no more packets can be queued in )
+ 		*/
+		goto normal_traffic;
+
+	/* mapping skb into flow */
+	natbyp_flow_map(flow, skb);
+
+	/* resetting counter */
+	natbyp_reset_counter(flow);
+
+	if ( flow->op.mode == NATBYP_SLOT_BYPASS_ACT ) {
+
+		u32 addr, nataddr;
+		u16 natport, oldport;
+		u32 * paddr;
+
+		char *ethh;
+
+		/*
+ 		* marking a skb 
+ 		*/
+		if ( flow->op.mark_state == NATBYP_BYPASS_MARK_READY ) {
+
+			/* update flow type */
+			{
+			struct iphdr *iph = (struct iphdr *)skb->data;
+			struct tcphdr *tcph = (struct tcphdr *)((char *)iph + sizeof(struct iphdr));
+
+			flow->ack = NATBYP_IS_TCP_ACK( iph, tcph ) ? 1: 0;
+			}
+
+			/* initialize wait queue */
+			natbyp_skbq_init( &(flow->skbq) );
+
+			/* Marking an socket buffer */
+			skb->natbyp_bypassed |= NATBYP_FLAG_MARKER;
+			/*
+ 			* 2nd step. 
+ 			*
+ 			*  -> MARK_WAIT
+ 			*  	Socket buffer is marked and following socket buffer found at egress point
+ 			*  	will be buffered but isn't transferred to output device to keep packet order. 
+ 			*
+ 			*/
+			flow->op.mark_state = NATBYP_BYPASS_MARK_WAIT;
+
+			if ( natbyp_verbose == NATBYP_VERBOSE_PACKET )
+				printout_natbyp_packet("MRK", flow, skb, 0, 0);
+
+			/*
+			 * This packet will be detected in natbyp_egress() side...
+			 */
+			return NATBYP_INSERTED; /* normal return */	
+		}
+
+		/* configure bypass flag */
+		skb->natbyp_bypassed |= NATBYP_FLAG_BYPASSED;
+		
+		/* Todd 2013/02/14 -> PPP connection bug */
+		/* prediction ... check validity */
+		if (unlikely((skb->data-ETH_HLEN)<skb->head)) {
+			/* simply skip this !! */
+			/* I do not know what this is ... */
+			natbyp_print(STATE, "FLOW[%-2d] UNDER SKB SKIP\n",flow->index);
+			skb->natbyp_bypassed &= ~NATBYP_FLAG_BYPASSED; /* clear flag */
+			return ret; /* !BYPASSED */
+		}
+
+		/* Speedy partial checksum */
+		switch( tolandir ) {
+		case NATBYP_DL:
+			addr       = iph->daddr;
+			nataddr    = flow->nated_ip;
+			/* desintation address/port hack */
+			natport    = flow->nated_port;
+			oldport    = tcph->dest;
+			tcph->dest = flow->nated_port;
+			/* to be changed */
+			paddr      = (u32 *)&(iph->daddr);
+			break;
+		case NATBYP_UL:
+			addr        = iph->saddr;
+			nataddr     = flow->nated_ip;
+			/* source address/port hack */
+			natport      = flow->nated_sport;
+			oldport      = tcph->source;
+			tcph->source = flow->nated_sport;
+			/* to be changed */
+			paddr      = (u32 *)&(iph->saddr);
+			break;
+		}
+
+		/* Update TCP checksum */
+		inet_proto_csum_replace4(&tcph->check, skb, addr, nataddr, 1);
+		inet_proto_csum_replace2(&tcph->check, skb, oldport, natport, 0);
+
+		/* IP checksum */
+		csum_replace4(&iph->check, addr, nataddr);
+
+		/* Update IP address */
+		*paddr = flow->nated_ip;
+
+		/* ethernet header replacement */
+		ethh = (char *)iph - ETH_HLEN;
+		memcpy( (char *)ethh , flow->nated_ethhdr , ETH_HLEN );	
+
+		/* back to data pointer */
+		skb_push( skb , ETH_HLEN );
+
+		/* reset header */
+		skb_set_network_header( skb , ETH_HLEN );
+		skb_set_transport_header( skb , ETH_HLEN );
+		skb->mac_len = ETH_HLEN;
+
+		/* update device */
+		skb->dev = NATDEV(flow);
+
+		/*
+ 		* natbyp_activated : 2 (emulation mode)
+ 		*/
+		ret = NATBYP_BYPASSED;
+	} 
+
+	/* 
+	 * skb_push() affects offset of the following above.. 
+	 */
+	if (natbyp_verbose == NATBYP_VERBOSE_PACKET)
+		printout_natbyp_packet("ING", flow, skb, 
+					(ret == NATBYP_BYPASSED)? ETH_HLEN : 0, 0);
+
+	if ( ret == NATBYP_BYPASSED ) {
+
+		switch( flow->op.mark_state ) {
+		case NATBYP_BYPASS_MARK_INIT:
+		case NATBYP_BYPASS_MARK_READY:
+			/* shouldn't be here !! */
+			break;
+
+		case NATBYP_BYPASS_MARK_WAIT:
+		case NATBYP_BYPASS_MARK_DONE:
+			/*
+ 			* Simply buffering packets at this stage...
+			*
+			* THIS IS INTERRUPT CONTEXT !!
+ 			*/
+			if (natbyp_skbq_put( &(flow->skbq), skb ) < 0) {
+				natbyp_errmsg("FLOW[%-2d] ING FLOW OVERFLOW\n", flow->index);
+
+				/* definitely this will cause TCP traffic shutdown */
+				dev_kfree_skb( skb );
+			}
+
+			/* Wake up !! */
+			if (flow->op.mark_state == NATBYP_BYPASS_MARK_DONE)
+				wake_up_interruptible( &(natbyp_db->flow_waitq[ flow->dir ]) );
+			break;
+		}
+	} /* if (ret == NATBYP_BYPASSED) */
+
+normal_traffic:
+	return ret;
+
+}
+EXPORT_SYMBOL(natbyp_ingress);
+
+/* insert into a queue */
+int natbyp_egress(struct sk_buff *skb)
+{
+	struct ethhdr *ethh;
+	struct iphdr *iph;
+	struct tcphdr *tcph;
+	natbyp_flow_t *flow;
+
+	/* ethernet header */
+	ethh = (struct ethhdr *)skb->data;
+	if( ntohs(ethh->h_proto) != ETH_P_IP ) {
+		return 0;
+	}
+
+	/* check TCP packet */
+	iph = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
+	if( iph->protocol != IPPROTO_TCP ) {
+		if ( (iph->protocol == IPPROTO_ICMP)  && 
+					(natbyp_verbose == NATBYP_VERBOSE_PACKET) ) {
+
+			u32 saddr = ntohl(iph->saddr);
+			u32 daddr = ntohl(iph->daddr);
+
+			natbyp_print(STATE, "ICMP [%d.%d.%d.%d -> %d.%d.%d.%d] %d \n",
+					(saddr & 0xff000000) >> 24,
+					(saddr & 0x00ff0000) >> 16,
+					(saddr & 0x0000ff00) >> 8,
+					(saddr & 0x000000ff) >> 0,
+					(daddr & 0xff000000) >> 24,
+					(daddr & 0x00ff0000) >> 16,
+					(daddr & 0x0000ff00) >> 8,
+					(daddr & 0x000000ff) >> 0,
+					ntohs(iph->tot_len) );
+		}
+		return 0;
+	}
+
+	/* flow is not associated !! */
+	if ( !skb->natbyp_flow ) 
+		return 0;
+
+	/* parent flow structure */
+	flow = (natbyp_flow_t *)skb->natbyp_flow;
+
+	/* sanity check */
+	if ( !natbyp_flow_ok(flow) ){
+		natbyp_errmsg("NATBYP BROKEN\n");
+		return 0;
+	}
+
+	/* check current flow again */
+	if ( (flow->op.mode == NATBYP_SLOT_REMOVE) && (flow->count <= 0) ) {
+		flow->used = 0;
+		natbyp_errmsg("FLOW[%-2d] >> FLOW REMOVED \n",flow->index);
+		return 0;
+	}
+
+#ifdef DEBUG_TIME_ESTIMATE
+	/* update time */
+	skb->natbyp_etime = 
+		natbyp_current_time() - skb->natbyp_etime;
+#endif
+
+	/* TCP header */
+	tcph = (struct tcphdr *)((char *)iph + sizeof(struct iphdr));
+
+	/* Check we met marker socket buffer */	
+	if ( skb->natbyp_bypassed & NATBYP_FLAG_MARKER ) {
+
+		/* printout */
+		if( natbyp_verbose == NATBYP_VERBOSE_PACKET )
+			printout_natbyp_packet("DCT", flow, skb, ETH_HLEN, 0);
+
+		/* MARK complete !! */
+		flow->op.mark_state = NATBYP_BYPASS_MARK_DONE;
+
+		/* reset counter */
+		natbyp_reset_counter( flow );
+
+		wake_up_interruptible( &(natbyp_db->flow_waitq[ flow->dir ]) );
+	}
+
+	/* NAT information fills in */
+	if ( !flow->mapped ) {
+		if ( flow->op.mode == NATBYP_SLOT_BYPASS_READY ) {
+			u32 nated_ip;
+
+			/* Currently test stage */
+			memcpy( flow->nated_ethhdr , (char *)ethh , ETH_HLEN );
+			
+			switch( flow->dir ) {
+			case NATBYP_DL:
+				flow->nated_ip 		= iph->daddr; 
+				flow->nated_port 	= tcph->dest;
+				flow->nated_sport	= 0; /* No use .. */
+				flow->nated_dev		= (natbyp_dev_t *)natbyp_dev_search( skb->dev ); /* NATED device */
+				break;
+			case NATBYP_UL:
+				flow->nated_ip 		= iph->saddr; 
+				/* 2011/10/19 */
+				flow->nated_port 	= tcph->dest; 
+				flow->nated_sport	= tcph->source;
+				flow->nated_dev		= (natbyp_dev_t *)natbyp_dev_search( skb->dev ); /* NATED device */
+				break;
+			default:
+				natbyp_errmsg("FLOW[%-2d] UNKNOWN DIR (%08x) \n", flow->index, flow->dir );
+				break;
+			}
+
+			/* check validity */
+			if (!flow->nated_dev) {
+				natbyp_errmsg("FLOW[%-2d] NULL DEVICE\n",flow->index);
+				/* clean up flow */
+				flow->used = 0;
+				flow->op.mode = NATBYP_SLOT_REMOVE;
+				return 0;
+			}
+
+			nated_ip = ntohl(flow->nated_ip);
+
+			natbyp_print(STATE, "FLOW[%-2d] %d.%d.%d.%d:%d:%s MAPPED %s \n",
+				flow->index , 
+				(nated_ip & 0xff000000) >> 24 , 
+				(nated_ip & 0x00ff0000) >> 16 , 
+				(nated_ip & 0x0000ff00) >> 8 , 
+				(nated_ip & 0x000000ff) >> 0 , 
+				ntohs(flow->nated_port) , 
+				(char *)(NATDEV(flow)->name), 
+			    dirname(flow->dir));
+
+			flow->mapped 		= 1;
+		}
+	} else {
+		/* check NAT information */
+		/*
+ 		* Sockets bypassed in ingress function will keep changed IP header.
+ 		*/
+		if ( tcp_flow_changed( flow , iph , tcph ) && 
+				!(skb->natbyp_bypassed & NATBYP_FLAG_BYPASSED) ) {
+
+			u32 nated_ip, _ip;
+			u32 ip = 0;
+			u16 port = 0;
+			char title = 'X';
+
+			/* check */
+			switch( flow->dir ){
+			case NATBYP_DL:
+				ip = iph->daddr;
+				port = tcph->dest;
+				title = 'D';
+				break;
+			case NATBYP_UL:
+				ip = iph->saddr;
+				port = tcph->source;
+				title = 'S';
+				break;
+			default:
+				natbyp_errmsg("FLOW[%-2d] UNKNOWN DIRECTION(%08x) \n", flow->index, flow->dir );
+				break;
+			}
+
+			nated_ip = ntohl(flow->nated_ip);
+			_ip = ntohl(ip);
+
+			/* NAT information differs */
+			natbyp_print(STATE, "FLOW[%-2d] -- NAT CHANGE(%d.%d.%d.%d:%d -> [%c]%d.%d.%d.%d:%d)[%s -> %s]\n",
+				flow->index , 
+				(nated_ip & 0xff000000) >> 24,
+				(nated_ip & 0x00ff0000) >> 16,
+				(nated_ip & 0x0000ff00) >> 8,
+				(nated_ip & 0x000000ff) ,
+				ntohs(flow->nated_port), 
+				title,
+				(_ip & 0xff000000) >> 24,
+				(_ip & 0x00ff0000) >> 16,
+				(_ip & 0x0000ff00) >> 8,
+				(_ip & 0x000000ff) ,
+				ntohs(port),
+				(char *)(NATDEV(flow)->name),
+				skb->dev->name);
+
+			/* Flow stops!! */
+			/* Slot remove !! */
+			flow->op.mode = NATBYP_SLOT_REMOVE;
+			flow->mapped = 0; /* waste NAT information */
+		} /* check NAT */
+	} /* if( flow->mapped ) */
+
+	/* Decide whether we reached to a point that NAT bypass could be activated ... */
+	if ( flow->count == 0 ) {
+		switch( flow->op.mode ){
+		case NATBYP_SLOT_BYPASS_READY:
+			flow->op.mode = NATBYP_SLOT_BYPASS_ACT;	
+			natbyp_print(STATE, "FLOW[%-2d] (%s) BYPASS_ACT \n",flow->index,(char *)(NATDEV(flow)->name));
+	
+			/*
+ 			* TODO.
+ 			*
+ 			* 	- Wake up interface 
+ 			*/
+			break;
+		default:
+			break;
+		}
+	}
+
+	/* deleting from flow */
+	natbyp_flow_unmap(skb);
+
+	if( natbyp_verbose == NATBYP_VERBOSE_PACKET )
+		printout_natbyp_packet("EGR", flow, skb, ETH_HLEN, 0);
+
+	return 0;
+}
+EXPORT_SYMBOL(natbyp_egress);
+
+void natbyp_destructor(struct sk_buff *skb)
+{
+	/* sanity check 1 */
+	if (!skb)
+		return ;
+
+	/* delete a skb */	
+	natbyp_flow_unmap( skb );
+}
+EXPORT_SYMBOL(natbyp_destructor);
+
+void natbyp_fastev( int evt , void * param )
+{
+	struct net_device *ndev;
+	int index;
+
+	switch( evt ) {
+	case NATBYP_EVT_DEVDEL:
+		if (!param) {
+			natbyp_print(STATE, "DEVDEL : No Parameter\n");
+			break;
+		}
+		ndev = (struct net_device *)param;
+		natbyp_print(STATE, "DEVICE(%s) - DELETED\n",ndev->name);
+		natbyp_dev_delete( ndev );
+		/*
+ 		* 2013/04/18 - 
+ 		* Delaying to clean up pending skbs.
+ 		*/
+		/* wake up device flow */
+		for (index = 0; index < NUM_FLOW_TYPE; index++) 
+			#if 0
+			wake_up_interruptible( &(natbyp_db->dev_waitq[ index ]) );
+			#else
+			wake_up_interruptible( &(natbyp_db->flow_waitq[ index ]) );
+			#endif
+		msleep(	1000 ); /* long time sleep */
+		break;
+	default:
+		break;
+	}
+}
+EXPORT_SYMBOL(natbyp_fastev);
+
+/* NAT Q ENABLED / OR NOT */
+int natbyp_enabled( void )
+{
+	return  natbyp_activated;
+}
+EXPORT_SYMBOL(natbyp_enabled);
+
+int natbyp_direction( struct net_device *dev )
+{
+	if (dev == wandev_db) {
+		/* WAN device check */
+		return NATBYP_DL; /* WAN -> LAN */
+	} else {
+		int index;
+
+		/* LAN device check */
+		for (index = 0; index < MAX_LAN_DEV; index++) {
+			if (dev == landev_db[ index ]) 
+				return NATBYP_UL;  /* LAN -> WAN */
+		}
+
+		/* No direction */
+		return -1;
+	}
+}
+EXPORT_SYMBOL(natbyp_direction);
+
+/* bandwidth display granularity */
+#define MEGA_UNIT	((1000*1000) / NATBYP_TIME_SCALE)
+#define KILO_UNIT	((1000) / NATBYP_TIME_SCALE)
+
+/*
+ *
+ * P R O C   F U N C T I O N S 
+ *
+ *
+ */
+static int proc_natbyp_show(struct seq_file *s, void *dummy)
+{
+	int index;
+	natbyp_flow_t *flow;
+	u32 bw_n=0 ,bw_d;
+	u32 t_bw;
+	char bw_u;
+	char speed[24];
+	struct net_device *dev;
+
+	/* check activated or not */
+	if ( !natbyp_activated ) {
+		seq_printf(s,"%s not activated.\n", NATBYP_VERSION);
+		return 0;
+	}
+
+	seq_printf(s,"%s \n",NATBYP_VERSION);
+
+	/* Device list... */
+	seq_printf(s,"Devs : ");
+	for( index = 0; index < MAX_NATBYP_DEVS; index++ ) {
+		if ( natbyp_db->ndevs[ index ].used ) {
+			dev = natbyp_db->ndevs[ index ].dev;
+			seq_printf(s,"%s", dev? dev->name : "NULL" );
+
+			/* LAN ? WAN ? */
+			if (!_strcmp(dev->name,wandev_name)) {
+				seq_printf(s,"* ");
+			} else {
+				int index;
+
+				for (index = 0; index < MAX_LAN_DEV; index++) {
+					if (!_strcmp(dev->name,landev_name[ index ])) {
+						seq_printf(s,"^ ");
+						break;
+					}
+				}
+			
+				if (index == MAX_LAN_DEV) {
+					seq_printf(s,"  ");
+				}
+			} 
+		}
+	}
+	seq_printf(s,"\n\n");
+
+	seq_printf(s,"Flows: \n");
+	seq_printf(s,"NUM  TYPE     STATUS    PKT   DROP  TRAFFIC          NAT\n");
+	seq_printf(s,"-----------------------------------------------------------------------------\n");
+
+	t_bw = 0; /* total bandwidth */
+
+	for(index = 0; index < MAX_NATBYP_FLOW; index++) {
+		if ( !natbyp_flow[ index ].used ) 
+			continue;
+
+		flow = (natbyp_flow_t *)&(natbyp_flow[ index ]);
+
+		/* bandwidth calculation */
+
+		bw_n = (flow->bw*8) ;  /* Kbits */
+
+		t_bw += bw_n; /* total bandwidth */
+
+		if ( bw_n > MEGA_UNIT ) {
+			bw_d = (bw_n) - (bw_n & MEGA_UNIT);
+			bw_n /= MEGA_UNIT;
+			bw_d /= MEGA_UNIT;
+			bw_u = 'M';
+		}else
+		if ( bw_n > KILO_UNIT ) {
+			bw_d = (bw_n) - (bw_n & KILO_UNIT);
+			bw_n /= KILO_UNIT;
+			bw_d /= KILO_UNIT;
+			bw_u = 'K';
+		}else{
+			bw_d = bw_n;
+			bw_n = 0;
+			bw_u = 'K';
+		}
+
+		memset(speed, 0, sizeof(speed));
+		snprintf(speed,sizeof(speed)-1,"%d.%d%cbps", bw_n, bw_d, bw_u);
+
+		seq_printf(s,"%-3d  %-4s[%c]  %-8s  %-3d   %-3d   %-13.13s    ",
+			index ,
+			dirname(flow->dir),
+			flow->ack ? 'A':'D',
+			((flow->op.mode >= NATBYP_SLOT_TEST) && 
+					(flow->op.mode <= NATBYP_SLOT_BYPASS_ACT))? 
+				natbyp_flow_status[ flow->op.mode ] : natbyp_flow_status[ 0 ],
+			flow->count , flow->dropnum , speed);
+
+		if( flow->mapped && 
+				(flow->op.mode != NATBYP_SLOT_REMOVE) ) {
+
+			u32 ip    = ntohl(flow->ip);
+			u16 port  = ntohs(flow->port);
+			u32 sip   = ntohl(flow->sip);
+			u16 sport = ntohs(flow->sport);
+			char *d   = (flow->dir == NATBYP_DL) ? "<-" : "->" ;
+
+			seq_printf(s,"[%d.%d.%d.%d:%d  %s  %d.%d.%d.%d:%d]\n",
+				(ip & 0xff000000) >> 24,
+				(ip & 0x00ff0000) >> 16,
+				(ip & 0x0000ff00) >> 8,
+				(ip & 0x000000ff) ,
+				port ,
+				d,
+				(sip & 0xff000000) >> 24,
+				(sip & 0x00ff0000) >> 16,
+				(sip & 0x0000ff00) >> 8,
+				(sip & 0x000000ff),
+				sport 
+			);
+		} else 
+			seq_printf(s,"\n");
+
+	}
+
+	/* total bandwidth printout */
+	if ( t_bw ) {
+		int  t_bw_d;
+		char t_bw_u;
+
+		if ( t_bw > MEGA_UNIT ) {
+			t_bw_d = (t_bw) - (t_bw & MEGA_UNIT);
+			t_bw /= MEGA_UNIT;
+			t_bw_d /= MEGA_UNIT;
+			t_bw_u = 'M';
+		}else
+		if ( t_bw > KILO_UNIT ) {
+			t_bw_d = (t_bw) - (t_bw & KILO_UNIT);
+			t_bw /= KILO_UNIT;
+			t_bw_d /= KILO_UNIT;
+			t_bw_u = 'K';
+		}else{
+			t_bw_d = bw_n;
+			t_bw = 0;
+			t_bw_u = 'K';
+		}
+
+		seq_printf(s,"-----------------------------------------------------------------------------\n");
+		seq_printf(s,"                                    %d.%d%cbps\n",t_bw , t_bw_d , t_bw_u );
+	}
+
+	if ( natbyp_db->pkt_drop[ NATBYP_UL ] || natbyp_db->pkt_drop[ NATBYP_DL ] ) {
+		seq_printf(s,"                               [UL:%-4d DL:%-4d] Packet drops\n",
+				natbyp_db->pkt_drop[ NATBYP_UL ] , natbyp_db->pkt_drop[ NATBYP_UL ] );
+	}
+
+	return 0;
+}
+
+static int proc_natbyp_open(struct inode *inode, struct file *file)
+{
+	return single_open(file, proc_natbyp_show, NULL);
+}
+
+/*
+ *
+ * # echo en > /proc/natbyp                      - enabling NATBYP
+ * # echo dis > /proc/natbyp                     - disabling NATBYP
+ * # echo log state/packet/dis > /proc/natbyp    - Logging  (state/packet/disabling)
+ * # echo dev <name> lan/wan   > /proc/natbyp    - Device related 
+ *
+ */
+static ssize_t proc_natbyp_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
+{
+	static __u8 cmds[64];
+#define MAX_ARGS	16
+	__u8 *args[MAX_ARGS];
+	int index,nth;
+
+	if (copy_from_user(cmds, buffer, (count>64)?64:count ))
+		return -EFAULT;
+
+	/* tokenizing */
+	index = 0;
+	nth = 0;
+	
+	while (index < count)
+	{
+		/* skipping head blank... */
+		while( (cmds[index] == ' ') && (index < count) ) ++index;
+		
+		if ( index >= count ) break;
+
+		args[nth++] = (__u8 *)&(cmds[index]);
+		
+		while(( (cmds[index] != ' ') &&
+				(cmds[index] != '\n')) && (index < count) ) ++index;
+
+		cmds[index++] = 0x0;
+
+		if (nth >= MAX_ARGS) 
+			break;
+	}
+
+	if (!nth)
+		return count;
+
+	if( !_strcmp(args[0],"en") ) {
+		natbyp_activated = 1;  /* enable */
+	} else
+	if( !_strcmp(args[0],"dis") ){
+		natbyp_activated = 0;  /* disable */	
+	} else
+	if( !_strcmp(args[0],"log") ){
+		if( nth < 2 )
+			return count;
+		if( !_strcmp(args[1],"none") ){
+			natbyp_verbose = NATBYP_VERBOSE_NONE;
+		}else
+		if( !_strcmp(args[1],"flow") ){
+			natbyp_verbose = NATBYP_VERBOSE_FLOW;
+		}else
+		if( !_strcmp(args[1],"state") ){
+			natbyp_verbose = NATBYP_VERBOSE_STATE;
+		}else
+		if( !_strcmp(args[1],"packet") ){
+			natbyp_verbose = NATBYP_VERBOSE_PACKET;
+		}
+	} else
+	if( !_strcmp(args[0],"window") ){
+		if( nth < 2 )
+			return count;
+		if( !_strcmp(args[1],"dis") ){
+			natbyp_db->window_update = 0;
+		}else
+		if( !_strcmp(args[1],"en") ){
+			natbyp_db->window_update = 1;
+		}else{
+			int newval = 
+				simple_strtoul( args[1] , (char **)&(args[1]) , 10 );
+			/*
+ 			* MIN : 1536
+ 			* MAX : 8192
+ 			*/
+			if( !( (newval > MIN_WINDOW_SIZE) && (newval <= MAX_WINDOW_SIZE) ) )
+				return count;
+
+			/* new window size */
+			natbyp_db->window_scale = newval;
+		}
+	} else
+	if( !_strcmp(args[0],"ack") ){
+		if( nth < 2 )
+			return count;
+		if( !_strcmp(args[1],"dis") ){
+			natbyp_db->ack_handle = 0;
+		}else
+		if( !_strcmp(args[1],"en") ){
+			natbyp_db->ack_handle = 1;
+		}
+	} else
+	if( !_strcmp(args[0],"dev") ){
+		char * devname;
+
+		/* 2013/02/28 */
+		/* device selective control */
+		/* this menu comes out with ppp0 interface */
+		/* PPP0 cannot go with NAT bypass traffic boosting */
+		/*  
+ 		*  # echo dev ppp0 ack dis > /proc/net/natbyp 
+ 		* 
+ 		*/
+		if( nth < 3 )
+			return count;
+
+		devname = (char *)args[1]; /* device name */
+
+		if( !_strcmp(args[2],"lan") ){
+			/* UL device */
+			int index;
+
+			for (index = 0; index < MAX_LAN_DEV; index++) {
+				if (*landev_name[ index ] == 0x0) {
+					memset( landev_name[ index ], 0, NAMELEN );
+					strncpy( landev_name[ index ], devname, NAMELEN-1 );
+					break;
+				}
+			}
+		}else
+		if( !_strcmp(args[2],"wan") ){
+			/* DL device */
+			memset( wandev_name, 0, NAMELEN );
+			strncpy( wandev_name, devname, NAMELEN-1 );
+		}
+	} else
+	if( !_strcmp(args[0],"expire") ){
+		int newval;
+
+		if( nth < 2 )
+			return count;
+		/*
+ 		* MIN : 1 
+ 		* MAX : 20 seconds 
+ 		*/
+		newval = simple_strtoul( args[1] , (char **)&(args[1]) , 10 );
+		if( !((newval >= 1) && (newval <= 20)) )
+			return count;
+
+		natbyp_db->expiry_max_count = newval;
+	} else
+	if( !_strcmp(args[0],"show") ){
+
+		int msglen = 0;
+		u8 msg[ 256 ];
+
+		/* zeroing */
+		memset( msg , 0 , 256 );
+
+		if( !natbyp_activated ) {
+			goto _natbyp_write_epilogue;
+		} else {
+			msglen = sprintf(msg, "NATBYP >> \n");
+		}
+	
+		/* log */	
+		msglen += sprintf(msg+msglen , "\tLOG=");
+		switch( natbyp_verbose ){
+		case NATBYP_VERBOSE_NONE:	msglen += sprintf( msg+msglen , "NONE\n"); break;
+		case NATBYP_VERBOSE_STATE:	msglen += sprintf( msg+msglen , "STATE\n"); break;
+		case NATBYP_VERBOSE_PACKET:	msglen += sprintf( msg+msglen , "PACKET\n"); break;
+		default: msglen += sprintf( msg+msglen , "UNKONWN\n"); break;
+		}
+
+		msglen += sprintf(msg+msglen , "\tCOUNTER=%d\n", natbyp_db->expiry_max_count);
+
+		/* Window */
+		msglen += sprintf(msg+msglen , "\tWINDOW=");
+		if( natbyp_db->window_update ) {
+			msglen += sprintf(msg+msglen , "%d \n", natbyp_db->window_scale );
+		} else {
+			msglen += sprintf(msg+msglen , "NONE \n");
+		}
+
+		/* ACK */
+		msglen += sprintf(msg+msglen , "\tACK=%s\n",natbyp_db->ack_handle ? "YES" : "NO" );
+
+	_natbyp_write_epilogue:
+
+		/* console print */
+		printk(KERN_INFO "%s \n", msg);
+	}
+		
+
+	return count;
+}
+
+static const struct proc_ops proc_natbyp_ops = {
+	.proc_open 		= proc_natbyp_open,
+	.proc_read 		= seq_read,
+	.proc_lseek		= seq_lseek,
+	.proc_write		= proc_natbyp_write,
+	.proc_release	= seq_release,
+};
+
+
+/*
+ * network device event handler
+ */
+static int natbyp_netdev_event(struct notifier_block *this, unsigned long event, void *ptr)
+{
+	struct net_device *netdev = netdev_notifier_info_to_dev(ptr);
+
+	switch( event ) {
+	case NETDEV_UP:
+		/* get it more picky */
+		/* TODO */
+		if (!_strcmp( netdev->name, wandev_name )) {
+			/* WAN */
+			natbyp_print(STATE, "DEVICE(%s) UP >> WAN\n",netdev->name);
+			wandev_db = netdev;
+		} else {
+			/* LAN */
+			int index;
+
+			for (index = 0; index < MAX_LAN_DEV; index++) {
+				if (!_strcmp( netdev->name, landev_name[ index ] )) {
+					natbyp_print(STATE, "DEVICE(%s) UP >> LAN\n",netdev->name);
+					landev_db[ index ] = netdev;
+					break;
+				}
+			}
+
+			if (index == MAX_LAN_DEV) {
+				return -EIO; /* No device */ 
+			}
+		}
+		natbyp_dev_insert( netdev );
+		break;
+	case NETDEV_DOWN:
+		/* TODO */
+		if (!_strcmp( netdev->name, wandev_name )) {
+			/* WAN */
+			natbyp_print(STATE, "DEVICE(%s) DN >> !WAN\n",netdev->name);
+			wandev_db = NULL;
+		} else {
+			/* LAN */
+			int index;
+
+			for (index = 0; index < MAX_LAN_DEV; index++) {
+				if (!_strcmp( netdev->name, landev_name[ index ] )) {
+					natbyp_print(STATE, "DEVICE(%s) DN >> !LAN\n",netdev->name);
+					landev_db[ index ] = NULL;
+					break;
+				}
+			}
+
+			if (index == MAX_LAN_DEV) {
+				return -EIO; /* No device */ 
+			}
+		}
+		natbyp_dev_delete( netdev );
+		break;
+	default:
+		break;
+	}
+
+	return NOTIFY_DONE;
+}
+
+/* device event handler */
+static struct notifier_block natbyp_netdev_notifier = {
+	.notifier_call = natbyp_netdev_event
+};
+
+/* init */
+static int __init natbyp_init( void )
+{
+	int index;
+
+	/* create db structure */
+	natbyp_db = (natbyp_db_t *)kmalloc( sizeof(natbyp_db_t) , GFP_KERNEL );
+	if (!natbyp_db) {
+		natbyp_errmsg("KMALLOC( sizeof(natbyp_db_t) )::failed\n");
+		return -ENOMEM;
+	}
+	memset( (char *)natbyp_db, 0, sizeof(natbyp_db_t) ); /* zeroing */
+
+	/* create flow structure */
+	natbyp_flow = (natbyp_flow_t *)kmalloc( sizeof(natbyp_flow_t)*MAX_NATBYP_FLOW , GFP_KERNEL );
+	if (!natbyp_flow) {
+		natbyp_errmsg("KMALLOC( sizeof(natbyp_flow_t) * %d )::failed\n",MAX_NATBYP_FLOW);
+		kfree( natbyp_db );
+		return -ENOMEM;
+	}
+	memset( (char *)natbyp_flow, 0, sizeof(natbyp_flow_t)*MAX_NATBYP_FLOW ); /* zeroing */
+
+	/* initialize device name */
+	memset( wandev_name, 0, NAMELEN );
+	for( index = 0; index < MAX_LAN_DEV; index++ )
+		memset( landev_name[ index ], 0, NAMELEN );
+
+	/* initialize natbyp_flow structure */
+	for( index = 0; index < MAX_NATBYP_FLOW; index++ ) {
+		/* found !! */
+		natbyp_flow[ index ].magic = NATBYP_MAGIC_4BYTES;
+		natbyp_flow[ index ].index = index;
+		natbyp_flow[ index ].used = 0;
+	}
+
+	/* timer init */
+	timer_setup( &(natbyp_db->timer), natbyp_flow_mon, 0 );
+
+	natbyp_db->expiry_max_count = DEFAULT_NATBYP_SLOT_MAX_EXPIRY;
+
+	/* Window size update Feature */
+	natbyp_db->window_update = 0;
+	natbyp_db->window_scale = DEFAULT_WINDOW_SIZE;
+
+	/* ACK packet */
+	/* TCP ACK packet does not belong to the candidates 
+ 	to be NAT bypassed */
+	natbyp_db->ack_handle = 0;  
+
+	/* device drop number - UL/DL */
+	natbyp_db->pkt_drop[ NATBYP_UL ] = 0;
+	natbyp_db->pkt_drop[ NATBYP_DL ] = 0;
+
+	/* number of pkts - UL/DL */
+	natbyp_db->pkt_count[ NATBYP_UL ] = 0;
+	natbyp_db->pkt_count[ NATBYP_DL ] = 0;
+
+	/* network devices */
+	for( index = 0; index < MAX_NATBYP_DEVS; index++ ) {
+		natbyp_db->ndevs[ index ].used = 0;
+		natbyp_db->ndevs[ index ].dev = NULL;
+	}
+			
+	/* proc structure */
+	proc_create("natbyp", S_IRUGO | S_IFREG | S_IWUSR, NULL, &proc_natbyp_ops);
+
+	/* network device notifier */
+	if ( register_netdevice_notifier(&natbyp_netdev_notifier) ) {
+		natbyp_errmsg("[ERR] NATBYP - failed in register_netdevice_notifier() \n");
+		kfree( natbyp_db );
+		kfree( natbyp_flow );
+		return -EIO;
+	}
+
+	/* packet forwarder - UL/DL */	
+	for( index = 0; index < NUM_FLOW_TYPE; index++) {
+
+		/* kernel thread */
+		init_waitqueue_head( &natbyp_db->flow_waitq[ index ] );
+
+		/* task parameter */
+		natbyp_db->flow_task_id[ index ] = index; /* type : UL/DL */
+
+		natbyp_db->flow_task[ index ] =
+				kthread_create(natbyp_forwarder, &(natbyp_db->flow_task_id[ index ]),
+					(index == NATBYP_UL)?"natbyp-pktf/ul":"natbyp-pktf/dl");
+		if (IS_ERR(natbyp_db->flow_task[ index ])) {
+			natbyp_errmsg("[ERR] kthread_create(pktf-%s)::failed\n", dirname(index));
+			kfree( natbyp_db );
+			kfree( natbyp_flow );
+			return -EIO;
+		}
+		wake_up_process( natbyp_db->flow_task[ index ] );
+	}
+
+	/* Logo */
+	printk(KERN_INFO "NATBYP module %s\n", NATBYP_VERSION );
+
+	return 0;
+}
+
+/* exit */
+static void __exit natbyp_exit( void )
+{ /* nothing done */
+}
+
+module_init(natbyp_init);
+module_exit(natbyp_exit);
+
+MODULE_DESCRIPTION("NAT Bypass Control Module");
+MODULE_AUTHOR("Todd HWang");
+MODULE_LICENSE("GPL");
+
+#endif /* defined(CONFIG_NETFILTER) */
diff -uNr linux-rpi-5.15.y-orig/net/core/skbuff.c linux-rpi-5.15.y/net/core/skbuff.c
--- linux-rpi-5.15.y-orig/net/core/skbuff.c	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/net/core/skbuff.c	2023-05-14 14:17:07.297109859 -0700
@@ -71,6 +71,9 @@
 #include <net/mpls.h>
 #include <net/mptcp.h>
 #include <net/page_pool.h>
+#ifdef CONFIG_NET_NATBYP
+#include <net/natbyp.h>
+#endif
 
 #include <linux/uaccess.h>
 #include <trace/events/skb.h>
@@ -212,6 +215,13 @@
 	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
 	atomic_set(&shinfo->dataref, 1);
 
+#ifdef CONFIG_NET_NATBYP
+	skb->natbyp_bypassed = 0;
+	skb->natbyp_etime = 0;
+	skb->natbyp_flow = NULL;
+	skb->natbyp_wlist = NULL;
+#endif
+
 	skb_set_kcov_handle(skb, kcov_common_handle());
 }
 
@@ -1079,6 +1089,13 @@
 	C(truesize);
 	refcount_set(&n->users, 1);
 
+#ifdef CONFIG_NET_NATBYP
+	n->natbyp_bypassed = 0;
+	n->natbyp_etime = 0;
+	n->natbyp_flow = NULL;
+	n->natbyp_wlist = NULL;
+#endif
+
 	atomic_inc(&(skb_shinfo(skb)->dataref));
 	skb->cloned = 1;
 
diff -uNr linux-rpi-5.15.y-orig/net/Kconfig linux-rpi-5.15.y/net/Kconfig
--- linux-rpi-5.15.y-orig/net/Kconfig	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/net/Kconfig	2023-05-14 14:17:07.297109859 -0700
@@ -174,6 +174,13 @@
 
 if NETFILTER
 
+config NET_NATBYP
+	bool "NAT Bypass"
+	depends on NETFILTER
+	default y
+	help 
+	  Skipping NAT FILTER framework but driver-to-driver packet transfer 
+	  
 config NETFILTER_ADVANCED
 	bool "Advanced netfilter configuration"
 	depends on NETFILTER
diff -uNr linux-rpi-5.15.y-orig/net/netfilter/nf_conntrack_proto_tcp.c linux-rpi-5.15.y/net/netfilter/nf_conntrack_proto_tcp.c
--- linux-rpi-5.15.y-orig/net/netfilter/nf_conntrack_proto_tcp.c	2023-02-08 08:47:50.000000000 -0800
+++ linux-rpi-5.15.y/net/netfilter/nf_conntrack_proto_tcp.c	2023-05-14 14:17:07.297109859 -0700
@@ -1156,11 +1156,20 @@
 		break;
 	}
 
+#ifndef CONFIG_NET_NATBYP
+	/* This is very important thing...
+	NAT bypass mode enforces connection tracking module keep silent
+	due to its driver-driver copying. Connection tracker may clean up the
+	channel due to the absense of packets. This patch will prevent such 
+	a sudden connection shutdown.
+	*/
+
 	if (!tcp_in_window(ct, dir, index,
 			   skb, dataoff, th, state)) {
 		spin_unlock_bh(&ct->lock);
 		return -NF_ACCEPT;
 	}
+#endif
      in_window:
 	/* From now on we have got in-window packets */
 	ct->proto.tcp.last_index = index;
