/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
 *
 * Author: Michael Ira Krufky <mkrufky@linuxtv.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "feed.h"

#include "feed/stdin.h"

struct dvbtee_context
{
	dvbtee::feed::StdinFeeder feeder;
};

struct dvbtee_context* ctxt;


void stop_server(struct dvbtee_context* context);

void cleanup(struct dvbtee_context* context, bool quick = false)
{
	if (quick) {
		context->feeder.stop_without_wait();
	} else {
		context->feeder.stop();
	}
}


void signal_callback_handler(int signum)
{
	struct dvbtee_context* context = ctxt;
	bool signal_dbg = true;

	const char *signal_desc;
	switch (signum) {
	case SIGINT:  /* Program interrupt. (ctrl-c) */
		signal_desc = "SIGINT";
		signal_dbg = false;
		break;
	case SIGABRT: /* Process detects error and reports by calling abort */
		signal_desc = "SIGABRT";
		break;
	case SIGFPE:  /* Floating-Point arithmetic Exception */
		signal_desc = "SIGFPE";
		break;
	case SIGILL:  /* Illegal Instruction */
		signal_desc = "SIGILL";
		break;
	case SIGSEGV: /* Segmentation Violation */
		signal_desc = "SIGSEGV";
		break;
	case SIGTERM: /* Termination */
		signal_desc = "SIGTERM";
		break;
#if !defined(_WIN32)
	case SIGHUP:  /* Hangup */
		signal_desc = "SIGHUP";
		break;
#endif
	default:
		signal_desc = "UNKNOWN";
		break;
	}
	if (signal_dbg)
		fprintf(stderr, "%s: caught signal %d: %s\n", __func__, signum, signal_desc);

	cleanup(context, true);

	context->feeder.parser.cleanup();

	exit(signum);
}


int main(int argc, char **argv)
{
	int opt;
	dvbtee_context context;
	ctxt = &context;

	unsigned int timeout = 0;
	char filename[256];

	memset(&filename, 0, sizeof(filename));

	while ((opt = getopt(argc, argv, "t:d::")) != -1) {
		switch (opt) {
		case 't': /* timeout */
			timeout = strtoul(optarg, NULL, 0);
			break;
		case 'd':
			if (optarg)
				libdvbtee_set_debug_level(strtoul(optarg, NULL, 0));
			else
				libdvbtee_set_debug_level(255);
			break;
		default:  /* bad cmd line option */
			return -1;
		}
	}

	signal(SIGINT,  signal_callback_handler); /* Program interrupt. (ctrl-c) */
	signal(SIGABRT, signal_callback_handler); /* Process detects error and reports by calling abort */
	signal(SIGFPE,  signal_callback_handler); /* Floating-Point arithmetic Exception */
	signal(SIGILL,  signal_callback_handler); /* Illegal Instruction */
	signal(SIGSEGV, signal_callback_handler); /* Segmentation Violation */
	signal(SIGTERM, signal_callback_handler); /* Termination */
#if !defined(_WIN32)
	signal(SIGHUP,  signal_callback_handler); /* Hangup */
#endif
	context.feeder.parser.limit_eit(-1);

	int ret = context.feeder.start();
	if (0 == ret) {
		context.feeder.wait_for_streaming_or_timeout(timeout);
		context.feeder.stop();
	}
exit:
	cleanup(&context);
	return 0;
}
