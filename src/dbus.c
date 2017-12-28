/*
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

#include <string.h>
#include <stdbool.h>

#include <ell/ell.h>

#include "dbus.h"

struct l_dbus *g_dbus = NULL;

static void debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	l_info("%s%s", prefix, str);
}

static void request_name_callback(struct l_dbus *dbus, bool success,
					bool queued, void *user_data)
{
	if (!success)
		l_error("Name request failed");
}

static void ready_callback(void *user_data)
{
	l_dbus_name_acquire(g_dbus, "net.connman.iwpand", false, false, true,
						request_name_callback, NULL);

	if (!l_dbus_object_manager_enable(g_dbus))
		l_error("Unable to register the ObjectManager");
}

static void disconnect_callback(void *user_data)
{
	l_info("D-Bus disconnected");
}

bool dbus_init(bool enable_debug)
{
	g_dbus = l_dbus_new_default(L_DBUS_SYSTEM_BUS);

	if (enable_debug)
		l_dbus_set_debug(g_dbus, debug, "[DBUS] ", NULL);

	l_dbus_set_ready_handler(g_dbus, ready_callback, g_dbus, NULL);
	l_dbus_set_disconnect_handler(g_dbus, disconnect_callback, NULL, NULL);

	return true;
}

void dbus_exit(void)
{
	l_dbus_destroy(g_dbus);
	g_dbus = NULL;
}
