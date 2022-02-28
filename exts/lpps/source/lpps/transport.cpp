/**
 * @file transport.c
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
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <arpa/inet.h>

#include "transport.h"

//
//
// P R I V A T E     F U N C T I O N s
//
//
static uint8_t transport_byte_csum(lpps_pkt_t *msg, int len)
{
	uint16_t csum = 0;
	uint16_t *p;
	int i;

	p = (uint16_t *)&(msg->data[0]);  // data checksum - long time job 

	for (i = 0; i < len; i += sizeof(uint16_t) ) {
		csum += *p;
		++ p;
	}

	// csum - 16 bits 

	// folding to a half - low 8 bits
	csum = (csum >> 8) + (csum & 0xFF);
	csum = (csum >> 8) + csum;
	csum = ~csum;

	// return low 8 bits 
	return (uint8_t)csum;
}

//
//
// P U B L I C       F U N C T I O N s
//
//
int transport_close(lpps_transport_t *t, int fd)
{
	return t->close(t, fd);
}

int transport_open(lpps_transport_t *t, const char *iname, int *fd)
{
	return t->open(t, iname, fd);
}

int transport_recv(lpps_transport_t *t, int fd, lpps_pkt_t *msg)
{
	int cnt;
	uint8_t csum;
	struct address addr;

	cnt = t->recv(t, fd, msg, sizeof(msg->data), &addr);
	if (cnt <= 0)
		return cnt;
	
	csum = transport_byte_csum(msg, ntohl(msg->hdr.len));

	if (csum != msg->hdr.csum)
		ERR("CSUM ERR %02x:%02x \n",csum,msg->hdr.csum);

	return (csum == msg->hdr.csum) ? cnt : -1;

}

int transport_send(lpps_transport_t *t, int fd,
		   enum transport_event event, lpps_pkt_t *msg)
{
	int len = ntohl(msg->hdr.len);

	// checksum !! 
	msg->hdr.csum = transport_byte_csum(msg, len);

	return t->send(t, fd, event, msg, len+sizeof(lpps_pkt_hdr_t), NULL);
}

int transport_physical_addr(lpps_transport_t *t, uint8_t *addr)
{
	if (t->physical_addr) 
		return t->physical_addr(t, addr);
	return 0;
}

int transport_protocol_addr(lpps_transport_t *t, uint8_t *addr)
{
	if (t->protocol_addr) 
		return t->protocol_addr(t, addr);
	return 0;
}

enum transport_type transport_type(lpps_transport_t *t)
{
	return t->type;
}

lpps_transport_t *transport_create(enum transport_type type, parameters_t * param)
{
	lpps_transport_t *t = NULL;

	switch (type) {
	case TRANS_UDP_IPV4:
		t = udp_transport_create( param );
		break;
	case TRANS_UDP_IPV6:
		t = udp6_transport_create( param );
		break;
	case TRANS_IEEE_802_3:
		t = raw_transport_create( param );
		break;
	}

	if (t)
		t->type = type;
	return t;
}

void transport_destroy(lpps_transport_t *t)
{
	t->release(t);
}
