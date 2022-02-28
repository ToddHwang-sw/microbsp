/**
 * @file sk.c
 * @brief Implements protocol independent socket methods.
 * @note Copyright (C) 2012 Richard Cochran <richardcochran@gmail.com>
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
#include <stdlib.h>
#include <errno.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <poll.h>
#include "transport.h"

/* public methods */

int sk_interface_fd(void)
{
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		pr_err("socket failed:");
		return -1;
	}
	return fd;
}

int sk_interface_index(int fd, const char *name)
{
	struct ifreq ifreq;
	int err;

	memset(&ifreq, 0, sizeof(ifreq));
	strncpy(ifreq.ifr_name, name, sizeof(ifreq.ifr_name) - 1);
	err = ioctl(fd, SIOCGIFINDEX, &ifreq);
	if (err < 0) {
		pr_err("ioctl SIOCGIFINDEX failed: %d",fd);
		return err;
	}
	return ifreq.ifr_ifindex;
}

static int sk_interface_guidaddr(const char *name, unsigned char *guid)
{
	char file_name[64], buf[64], addr[8];
	FILE *f;
	char *err;
	int res;

	snprintf(file_name, sizeof buf, "/sys/class/net/%s/address", name);
	f = fopen(file_name, "r");
	if (!f) {
		pr_err("failed to open %s: ", buf);
		return -1;
	}

	/* Set the file position to the beginning of the GUID */
	res = fseek(f, GUID_OFFSET, SEEK_SET);
	if (res) {
		pr_err("fseek failed: ");
		goto error;
	}

	err = fgets(buf, sizeof buf, f);
	if (err == NULL) {
		pr_err("fseek failed: ");
		goto error;
	}

	res = sscanf(buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			   &addr[0], &addr[1], &addr[2], &addr[3],
			   &addr[4], &addr[5], &addr[6], &addr[7]);
	if (res != GUID_LEN) {
		pr_err("sscanf failed: ");
		goto error;
	}

	memcpy(guid, addr, GUID_LEN);
	fclose(f);

	return 0;

error:
	fclose(f);
	return -1;
}

int sk_interface_macaddr(const char *name, struct address *mac)
{
	struct ifreq ifreq;
	int err, fd, type;

	memset(&ifreq, 0, sizeof(ifreq));
	strncpy(ifreq.ifr_name, name, sizeof(ifreq.ifr_name) - 1);

	fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		pr_err("socket failed: ");
		return -1;
	}

	err = ioctl(fd, SIOCGIFHWADDR, &ifreq);
	if (err < 0) {
		pr_err("ioctl SIOCGIFHWADDR failed: ");
		close(fd);
		return -1;
	}

	close(fd);

	/* Get interface type */
	type = ifreq.ifr_hwaddr.sa_family;
	switch (type) {
		case ARPHRD_INFINIBAND:
			err = sk_interface_guidaddr(name, mac->sll.sll_addr);
			if (err) {
				pr_err("fail to get address using sysfs: ");
				return -1;
			}
			mac->sll.sll_halen = EUI64;
			break;
		default:
			memcpy(mac->sll.sll_addr, &ifreq.ifr_hwaddr.sa_data, MAC_LEN);
			mac->sll.sll_halen = EUI48;
	}

	mac->sll.sll_family = AF_PACKET;
	mac->len = sizeof(mac->sll);
	return 0;
}

int sk_interface_addr(const char *name, int family, struct address *addr)
{
	struct ifaddrs *ifaddr, *i;
	int result = -1;

	if (getifaddrs(&ifaddr) == -1) {
		pr_err("getifaddrs failed: ");
		return -1;
	}
	for (i = ifaddr; i; i = i->ifa_next) {
		if (i->ifa_addr && family == i->ifa_addr->sa_family &&
			strcmp(name, i->ifa_name) == 0)
		{
			switch (family) {
			case AF_INET:
				addr->len = sizeof(addr->sin);
				memcpy(&addr->sin, i->ifa_addr, addr->len);
				break;
			case AF_INET6:
				addr->len = sizeof(addr->sin6);
				memcpy(&addr->sin6, i->ifa_addr, addr->len);
				break;
			default:
				continue;
			}
			result = 0;
			break;
		}
	}
	freeifaddrs(ifaddr);
	return result;
}

int sk_set_priority(int fd, uint8_t dscp)
{
	int tos;
	socklen_t tos_len;

	tos_len = sizeof(tos);
	if (getsockopt(fd, SOL_IP, IP_TOS, &tos, &tos_len) < 0) {
		tos = 0;
	}

	/* clear old DSCP value */
	tos &= ~0xFC;

	/* set new DSCP value */
	tos |= dscp<<2;
	tos_len = sizeof(tos);
	if (setsockopt(fd, SOL_IP, IP_TOS, &tos, tos_len) < 0) {
		return -1;
	}

	return 0;
}

