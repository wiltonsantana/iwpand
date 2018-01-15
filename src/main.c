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
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <ell/ell.h>
#include "phy.h"
#include "dbus.h"

#define NL802154_GENL_NAME "nl802154"

static struct l_timeout *timeout;
static bool terminating;
static uint8_t channel = 0xff;
static uint8_t page = 0xff;

static void main_loop_quit(struct l_timeout *timeout, void *user_data)
{
	l_main_quit();
}

static void terminate(void)
{
	if (terminating)
		return;

	terminating = true;

	timeout = l_timeout_create(1, main_loop_quit, NULL, NULL);
}

static void signal_handler(struct l_signal *signal, uint32_t signo,
							void *user_data)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		terminate();
		break;
	}
}

static void nl802154_appeared(void *user_data)
{
	if (terminating)
		return;

	l_debug("nl802154 appeared");
	phy_init(user_data, page, channel);
}

static void nl802154_vanished(void *user_data)
{
	l_debug("nl802154 vanished");
	phy_exit(user_data);
}

static void usage(void)
{
	printf("iwpand - Wireless PAN daemon\n"
		"Usage:\n");
	printf("\tiwpand [options]\n");
	printf("Options:\n"
		"\t-c, --channel          Radio channel to use\n"
		"\t-p, --page		  Radio channel page to use\n"
		"\t-h, --help             Show help options\n");
}
static const struct option main_options[] = {
	{ "version",		no_argument,       NULL, 'v' },
	{ "page",		required_argument, NULL, 'p' },
	{ "channel",		required_argument, NULL, 'c' },
	{ "help",		no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	struct l_genl *genl;
	struct l_genl_family *nl802154;
	struct l_signal *sig;
	sigset_t mask;
	int ret = EXIT_FAILURE;
	int opt;

	for (;;) {
		opt = getopt_long(argc, argv, "c:p:", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'c':
			channel = atoi(optarg);
			break;
		case 'p':
			page = atoi(optarg);
			break;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (argc - optind > 0) {
		fprintf(stderr, "Invalid command line parameters\n");
		return EXIT_FAILURE;
	}

	if (!l_main_init())
		return EXIT_FAILURE;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	sig = l_signal_create(&mask, signal_handler, NULL, NULL);

	l_log_set_stderr();
	l_debug_enable("*");
	l_info("Wireless PAN daemon version %s", VERSION);

	if (!dbus_init(false)) {
		l_error("D-Bus init fail");
		goto fail_dbus;
	}

	genl = l_genl_new_default();
	if (!genl) {
		l_error("Generic Netlink fail");
		goto fail_genl;
	}

	nl802154 = l_genl_family_new(genl, NL802154_GENL_NAME);
	if (!nl802154) {
		l_error("Failed to open nl802154 interface");
		goto fail_nl802154;
	}

	if (!l_genl_family_set_watches(nl802154,
				  nl802154_appeared,
				  nl802154_vanished,
				  nl802154, NULL)) {
		l_error("Failed to add NL watch");
		goto fail_watch;
	}

	ret = 0;

	l_main_run();

fail_watch:
	l_genl_family_unref(nl802154);

fail_nl802154:
	l_genl_unref(genl);

fail_genl:
	dbus_exit();

fail_dbus:
	l_signal_remove(sig);
	l_main_exit();

	return ret;
}
