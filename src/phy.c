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

#include <ell/ell.h>
#include "dbus.h"
#include "phy.h"

#define PHY_INTERFACE		"net.connman.iwpand.Adapter"

static bool phy_property_get_powered(struct l_dbus *dbus,
				     struct l_dbus_message *msg,
				     struct l_dbus_message_builder *builder,
				     void *user_data)
{
	bool value = false;

	l_dbus_message_builder_append_basic(builder, 'b', &value);
	l_info("GetProperty(Powered = %d)", value);

	return true;
}

static struct l_dbus_message *phy_property_set_powered(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_iter *new_value,
					l_dbus_property_complete_cb_t complete,
					void *user_data)
{
        bool value;

	if (!l_dbus_message_iter_get_variant(new_value, "b", &value))
		return dbus_error_invalid_args(message);

	l_info("SetProperty(Powered = %d)", value);

	return NULL;
}

static bool phy_property_get_name(struct l_dbus *dbus,
				  struct l_dbus_message *msg,
				  struct l_dbus_message_builder *builder,
				  void *user_data)
{
	const char value[] = "lowpan0";

	l_dbus_message_builder_append_basic(builder, 's', value);
	l_info("GetProperty(Name = %s)", value);

	return true;
}

static void setup_phy_interface(struct l_dbus_interface *interface)
{
	if (!l_dbus_interface_property(interface, "Powered", 0, "b",
				       phy_property_get_powered,
				       phy_property_set_powered))
		l_error("Can't add 'Powered' property");

	if (!l_dbus_interface_property(interface, "Name", 0, "s",
				       phy_property_get_name,
				       NULL))
		l_error("Can't add 'Name' property");
}

int phy_init(void)
{
	if (!l_dbus_register_interface(dbus_get_bus(),
				       PHY_INTERFACE,
				       setup_phy_interface,
				       NULL, false)) {
		l_error("Unable to register %s interface", PHY_INTERFACE);
		return false;
	}

	if (!l_dbus_object_add_interface(dbus_get_bus(),
					 "/wpan0",
					 PHY_INTERFACE,
					 NULL))
		l_error("'/wpan0': Unable to register %s interface",
							PHY_INTERFACE);

	if (!l_dbus_object_add_interface(dbus_get_bus(),
					 "/wpan0",
					 L_DBUS_INTERFACE_PROPERTIES,
					 NULL))
		l_error("'/wpan0': Unable to register %s interface",
						L_DBUS_INTERFACE_PROPERTIES);

	return 0;
}

void phy_exit(void)
{

}
