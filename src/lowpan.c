/*
 *
 *  Wireless PAN (802.15.4) daemon for Linux
 *
 *  Copyright (C) 2017 CESAR. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>

#include <linux/if_arp.h>
#include <linux/rtnetlink.h>

#include <ell/ell.h>

#include "lowpan.h"

static struct l_netlink *rtnl = NULL;

static void rtnl_link_notify(uint16_t type, const void *data, uint32_t len,
							void *user_data)
{
	/*
	 * ip link add link wpan0 name lowpan0 type lowpan
	 * Callback called when virtual link is created or deleted.
	 */
	const struct ifinfomsg *ifi = data;

	if (ifi->ifi_type != ARPHRD_6LOWPAN)
		return;

	switch (type) {
	case RTM_NEWLINK:
		l_info("RTNL_NEWLINK");
		break;
	case RTM_DELLINK:
		l_info("RTM_DELLINK");
		break;
	}
}

bool lowpan_init(void)
{
	l_info("6LoWPAN init");

	rtnl = l_netlink_new(NETLINK_ROUTE);
	if (!rtnl) {
		l_error("Failed to open netlink route socket");
		return false;
	}

	if (!l_netlink_register(rtnl, RTNLGRP_LINK,
				rtnl_link_notify, NULL, NULL)) {
		l_error("Failed to register RTNL link notifications");
		l_netlink_destroy(rtnl);
		return false;
	}

	return true;
}

void lowpan_exit(void)
{
	l_info("6LoWPAN exit");

	l_netlink_destroy(rtnl);
}
