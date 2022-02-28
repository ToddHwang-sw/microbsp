/**
 * @file udp.c
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
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "transport.h"

typedef struct{
	lpps_transport_t t;
	struct address ip;
	struct address mac;

	char *ip0;
	char *ip1;
	int port;  // actually short 

	//
	// mc_addr[0] - my multicasy address
	// mc_addr[1] - friend's multicast address
	//
	struct in_addr mc_addr[2];
}__attribute__((aligned)) lpps_udp_t;

static int mcast_bind(int fd, int index)
{
	int err;
	struct ip_mreqn req;
	memset(&req, 0, sizeof(req));
	req.imr_ifindex = index;
	err = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, &req, sizeof(req));
	if (err) {
		pr_err("setsockopt IP_MULTICAST_IF failed: %m");
		return -1;
	}
	return 0;
}

static int mcast_join(int fd, int index, const struct sockaddr_in *sa)
{
	int err;
	struct ip_mreqn req;

	memset(&req, 0, sizeof(req));
	memcpy(&req.imr_multiaddr, &sa->sin_addr, sizeof(struct in_addr));
	req.imr_ifindex = index;
	err = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req));
	if (err) {
		pr_err("setsockopt IP_ADD_MEMBERSHIP failed: %m");
		return -1;
	}
	return 0;
}

static int udp_close(lpps_transport_t *t, int fd)
{
	close(fd);
	return 0;
}

static int open_socket(lpps_udp_t *udp, const char *name, short port, int ttl)
{
	struct sockaddr_in addr;
	int fd = 0, index, on = 1, loop = 0;

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		pr_err("socket failed: %m");
		return -1;
	}
	index = sk_interface_index(fd, name);
	if (index < 0)
		goto no_option;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		pr_err("setsockopt SO_REUSEADDR failed: %m");
		goto no_option;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr))) {
		pr_err("bind failed: %m");
		goto no_option;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, name, strlen(name))) {
		pr_err("setsockopt SO_BINDTODEVICE failed: %m");
		goto no_option;
	}

	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl))) {
		pr_err("setsockopt IP_MULTICAST_TTL failed: %m");
		goto no_option;
	}

	loop = 0;
	if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop))) {
		pr_err("setsockopt IP_MULTICAST_LOOP failed: %m");
		goto no_option;
  	}

	// IP0 - me 
	if (!inet_aton( udp->ip0, &udp->mc_addr[0] ))
		return -1;

	addr.sin_addr = udp->mc_addr[0];
	if (mcast_join(fd, index, &addr)) {
		pr_err("mcast_join ip0 failed");
		goto no_option;
	}

	// IP1 - friend 
	if (!inet_aton( udp->ip1, &udp->mc_addr[1] ))
		return -1;

	addr.sin_addr = udp->mc_addr[1];
	if (mcast_join(fd, index, &addr)) {
		pr_err("mcast_join ip1 failed");
		goto no_option;
	}

	if (mcast_bind(fd, index)) {
		goto no_option;
	}

	return fd;
no_option:
	close(fd);
	return -1;
}

static int udp_open(lpps_transport_t *t, const char *name, int *fd)
{
	lpps_udp_t*udp = container_of(t, lpps_udp_t, t);
	int efd, ttl;

	ttl = 64;

	udp->mac.len = 0;

	sk_interface_macaddr(name, &udp->mac);

	udp->ip.len = 0;
	sk_interface_addr(name, AF_INET, &udp->ip);

	efd = open_socket(udp, name, udp->port , ttl);
	if (efd < 0)
		return -1;

	*fd = efd;

	return 0;
}

//
// TODO - Make a thread watching an interface to receive packets be cancellable, 
//      select() should include timeout and upper control handles the timeout case. 
//
//
static int udp_recv(lpps_transport_t *t, int fd, void *buf, int buflen, struct address *_addr)
{
	struct sockaddr from;
	size_t addrlen;
	fd_set r;
	int pr; // packet ready 

	FD_ZERO(&r);
	FD_SET(fd,&r);

	// multiplexing...
	pr = select(fd+1,&r,NULL,NULL,NULL);
	if (pr <= 0) {
		ERR(" select(...) - error\n");
		return -1; // error/signal ???
	}

	if (FD_ISSET(fd,&r)) {
		addrlen = (size_t) sizeof(from);
		return recvfrom(fd, buf, buflen, 0, &from, (socklen_t *)&addrlen );
	} else
		return -1; // error/signal ???
}

static int udp_send(lpps_transport_t *t, int fd,
		    enum transport_event event, void *buf, int len, struct address *_addr)
{
	lpps_udp_t*udp = container_of(t, lpps_udp_t, t);
	ssize_t cnt;
	struct sockaddr_in addr;

	memset( (char *)&addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_addr = udp->mc_addr[1];  // friend 
	addr.sin_port = htons( udp->port );

	cnt = sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
	if (cnt < 1) {
		pr_err("sendto failed: ");
		return -errno;
	}
	return cnt;
}

static void udp_release(lpps_transport_t *t)
{
	lpps_udp_t*udp = container_of(t, lpps_udp_t, t);
	free(udp);
}

static int udp_physical_addr(lpps_transport_t *t, uint8_t *addr)
{
	lpps_udp_t*udp = container_of(t, lpps_udp_t, t);
	int len = 0;

	if (udp->mac.len) {
		len = MAC_LEN;
		memcpy(addr, udp->mac.sll.sll_addr, len);
	}
	return len;
}

static int udp_protocol_addr(lpps_transport_t *t, uint8_t *addr)
{
	lpps_udp_t*udp = container_of(t, lpps_udp_t, t);
	int len = 0;

	if (udp->ip.len) {
		len = sizeof(udp->ip.sin.sin_addr.s_addr);
		memcpy(addr, &udp->ip.sin.sin_addr.s_addr, len);
	}
	return len;
}

lpps_transport_t *udp_transport_create(parameters_t *param)
{
	lpps_udp_t*udp;

	if (!param)
		return NULL;

	udp = (lpps_udp_t *)malloc(sizeof(lpps_udp_t));
	if (!udp)
		return NULL;

	// copying parameters
	udp->ip0 = param->ip0;
	udp->ip1 = param->ip1;
	udp->port = param->port;
	
	udp->t.close = udp_close;
	udp->t.open  = udp_open;
	udp->t.recv  = udp_recv;
	udp->t.send  = udp_send;
	udp->t.release = udp_release;
	udp->t.physical_addr = udp_physical_addr;
	udp->t.protocol_addr = udp_protocol_addr;

	return &udp->t;
}

