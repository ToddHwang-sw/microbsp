/**
 * @file transport.h
 * @brief Defines an abstract transport layer.
 * @note Copyright (C) 2011 Richard Cochran <richardcochran@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef HAVE_TRANSPORT_H
#define HAVE_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <net/if_arp.h>
#include "os.h"
#include "sk.h"

#define EUI48 6
#define EUI64 8

#define MAC_LEN  EUI48
#define GUID_LEN EUI64

#define GUID_OFFSET 36

typedef uint8_t eth_addr[MAC_LEN];

struct eth_hdr {
        eth_addr dst;
        eth_addr src;
        uint16_t type;
} __attribute__((packed));

#define VLAN_HLEN 4

struct vlan_hdr {
        eth_addr dst;
        eth_addr src;
        uint16_t tpid;
        uint16_t tci;
        uint16_t type;
} __attribute__((packed));

#define OFF_ETYPE (2 * sizeof(eth_addr))

struct address {
        socklen_t len;
        union {
                struct sockaddr_storage ss;
                struct sockaddr_ll sll;
                struct sockaddr_in sin;
                struct sockaddr_in6 sin6;
                struct sockaddr_un sun;
                struct sockaddr sa;
        };
};

// 
// network driven packet...
//
typedef struct {
	uint8_t type; // type 
	uint8_t sid;  // source id
	uint8_t did;  // destination id
	uint8_t csum; // checksum
	uint32_t len; // message length 
}__attribute__((packed)) lpps_pkt_hdr_t;

#define MAX_TRANSPORT_LEN	1500
#define PAYLOAD_LENGTH		(MAX_TRANSPORT_LEN - sizeof(lpps_pkt_hdr_t))

typedef struct {
	lpps_pkt_hdr_t hdr;
	uint8_t data[ PAYLOAD_LENGTH ];
}__attribute__((packed)) lpps_pkt_t;

enum transport_type {
	TRANS_UDP_IPV4 = 0,
	TRANS_UDP_IPV6,
	TRANS_IEEE_802_3,
};

enum transport_event {
	TRANS_LPPS_MESSAGE = 100,	

	// Message 
	TRANS_LPPS_MKDIR   = TRANS_LPPS_MESSAGE + 0,
	TRANS_LPPS_RMDIR   = TRANS_LPPS_MESSAGE + 1,
	TRANS_LPPS_MKFILE  = TRANS_LPPS_MESSAGE + 2,
	TRANS_LPPS_RMFILE  = TRANS_LPPS_MESSAGE + 3,

	// Control 
	TRANS_LPPS_CONTROL = TRANS_LPPS_MESSAGE + 32,
};

//
// Default values...
//
// Currently registered multicast IP addresses - 
//       https://www.iana.org/assignments/multicast-addresses/multicast-addresses.xhtml
//
// Currently registered port numbers - 
//       https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.txt
//       [ 224.0.2.15 - 224.0.2.63 ] - Unassigned (Ad-hoc Block)
//
//
#define DEFAULT_ID		0
#define DEFAULT_PORT		20582
#define DEFAULT_IP		"224.0.2.15:224.0.2.16"
#define DEFAULT_DEV		"eth0"
#define DEFAULT_TRANSPORT	TRANS_UDP_IPV4
#define DEFAULT_LEVEL		1

typedef struct __lpps_transport{
        enum transport_type type;

        int (*close)(struct __lpps_transport *t, int fd);
        int (*open)(struct __lpps_transport *t, const char *iname, int *fd);
        int (*recv)(struct __lpps_transport *t, int fd, void *buf, int buflen, struct address *addr);
        int (*send)(struct __lpps_transport *t, int fd, enum transport_event event, void *buf, int buflen, struct address *addr);
        void (*release)(struct __lpps_transport *t);
        int (*physical_addr)(struct __lpps_transport *t, uint8_t *addr);
        int (*protocol_addr)(struct __lpps_transport *t, uint8_t *addr);
}__attribute__((aligned)) lpps_transport_t;

//
// Used as global container 
//
typedef struct {
	unsigned int nmid;
	char *device;
	char *ip;   
	char *ip0;  // multicast IP
	char *ip1;  // multicast IP
	unsigned int port;
	enum transport_type transport;
	unsigned int level;
	lpps_transport_t * network;
	int fd;
}__attribute__((aligned)) parameters_t;
//extern parameters_t params;

extern int transport_close(lpps_transport_t *t, int fd);

extern int transport_open(lpps_transport_t *t, const char *iname, int *fd);

extern int transport_recv(lpps_transport_t *t, int fd, lpps_pkt_t *msg);

extern int transport_send(lpps_transport_t *t, int fd, enum transport_event event, lpps_pkt_t *msg);

enum transport_type transport_type(lpps_transport_t *t);

#define TRANSPORT_ADDR_LEN 16

/**
 * Gets the transport's physical address.
 * @param t    The transport.
 * @param addr The address will be written to this buffer.
 * @return     The number of bytes written to the buffer. Will be 0-16
 *             bytes
 */
extern int transport_physical_addr(lpps_transport_t *t, uint8_t *addr);

/**
 * Gets the transport's protocol address.
 * @param t    The transport.
 * @param addr The address will be written to this buffer.
 * @return     The number of bytes written to the buffer. Will be 0-16
 *             bytes
 */
extern int transport_protocol_addr(lpps_transport_t *t, uint8_t *addr);

/**
 * Allocate an instance of the specified transport.
 * @param config Pointer to the configuration database.
 * @param type  Which transport to obtain.
 * @return      Pointer to a transport instance on success, NULL otherwise.
 */
extern lpps_transport_t *transport_create(enum transport_type type, parameters_t * param );

/**
 * Free an instance of a transport.
 * @param t Pointer obtained by calling transport_create().
 */
extern void transport_destroy(lpps_transport_t *t);

// 
// should be placed here ~ 
//
#ifndef container_of
#define container_of(ptr, type, member) ({                              \
                        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
                        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#define pr_err(fmt,args...) ERR(" " fmt "\n",##args)

extern lpps_transport_t *udp_transport_create(  parameters_t *param );
extern lpps_transport_t *udp6_transport_create( parameters_t *param );
extern lpps_transport_t *raw_transport_create(  parameters_t *param );

#endif
