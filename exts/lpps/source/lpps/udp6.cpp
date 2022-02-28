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
}__attribute__((aligned)) lpps_udp6_t;

static int udp6_close(lpps_transport_t *t, int fd)
{
	return 0;
}

static int udp6_open(lpps_transport_t *t, const char *iname, int *fd)
{
	return 0;
}

static int udp6_recv(lpps_transport_t *t, int fd, void *buf, int buflen, struct address *addr)
{
	return 0;
}

static int udp6_send(lpps_transport_t *t, int fd,
		    enum transport_event event, void *buf, int len, struct address *addr)
{
	return 0;
}

static void udp6_release(lpps_transport_t *t)
{
	lpps_udp6_t * udp6 = container_of(t, lpps_udp6_t, t);
	free(udp6);
}

static int udp6_physical_addr(lpps_transport_t *t, uint8_t *addr)
{
	return 0;
}

static int udp6_protocol_addr(lpps_transport_t *t, uint8_t *addr)
{
	return 0;
}

lpps_transport_t *udp6_transport_create(parameters_t *param)
{
	lpps_udp6_t * udp6 = (lpps_udp6_t*)calloc(1, sizeof(*udp6));
	if (!udp6)
		return NULL;

	udp6->t.close = udp6_close;
	udp6->t.open  = udp6_open;
	udp6->t.recv  = udp6_recv;
	udp6->t.send  = udp6_send;
	udp6->t.release = udp6_release;
	udp6->t.physical_addr = udp6_physical_addr;
	udp6->t.protocol_addr = udp6_protocol_addr;

	return &udp6->t;
}

