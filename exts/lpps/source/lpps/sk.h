/**
 * @file sk.h
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
#ifndef HAVE_SK_H
#define HAVE_SK_H

#include "transport.h"

/**
 * Obtains a socket suitable for use with sk_interface_index().
 * @return  An open socket on success, -1 otherwise.
 */
int sk_interface_fd(void);

/**
 * Obtain the numerical index from a network interface by name.
 * @param fd      An open socket.
 * @param device  The name of the network interface of interest.
 * @return        The result from the SIOCGIFINDEX ioctl.
 */
int sk_interface_index(int fd, const char *device);

/**
 * Obtain the MAC address of a network interface.
 * @param name  The name of the interface
 * @param mac   Buffer to hold the result
 * @return      Zero on success, non-zero otherwise.
 */
int sk_interface_macaddr(const char *name, struct address *mac);

/**
 * Obtains the first IP address assigned to a network interface.
 * @param name   The name of the interface
 * @param family The family of the address to get: AF_INET or AF_INET6
 * @param addr   Buffer to hold the result
 * @return       The number of bytes written to addr on success, -1 otherwise.
 */
int sk_interface_addr(const char *name, int family, struct address *addr);

/**
 * Set DSCP value for socket.
 * @param fd    An open socket.
 * @param dscp  The desired DSCP code.
 * @return Zero on success, negative on failure
 */
int sk_set_priority(int fd, uint8_t dscp);

#endif
