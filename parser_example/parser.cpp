/*****************************************************************************
 * Copyright (C) 2011-2013 Michael Krufky
 *
 * Author: Michael Krufky <mkrufky@linuxtv.org>
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
#include "tune.h"
#include "serve.h"

#include "atsctext.h"

typedef std::map<uint8_t, tune> map_tuners;

struct dvbtee_context
{
	feed feeder;
	serve *server;
};
typedef std::map<pid_t, struct dvbtee_context*> map_pid_to_context;

map_pid_to_context context_map;


void stop_server(struct dvbtee_context* context);

void cleanup(struct dvbtee_context* context, bool quick = false)
{
	if (context->server)
		stop_server(context);

	if (quick) {
		context->feeder.stop_without_wait();
		context->feeder.close_file();
	} else {
		context->feeder.stop();
	}

	context->feeder.close_file();
#if 1 /* FIXME */
	ATSCMultipleStringsDeInit();
#endif
}


void signal_callback_handler(int signum)
{
	struct dvbtee_context* context = context_map[getpid()];
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
	case SIGHUP:  /* Hangup */
		signal_desc = "SIGHUP";
		break;
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


void stop_server(struct dvbtee_context* context)
{
	if (!context->server)
		return;

	context->server->stop();

	delete context->server;
	context->server = NULL;

	return;
}

int start_server(struct dvbtee_context* context, int port = 0, int eavesdropping_port = 0)
{
	context->server = new serve;
	context->server->add_feeder(&context->feeder);

	if (eavesdropping_port)
		context->feeder.parser.out.add_http_server(eavesdropping_port);

	return (port) ? context->server->start(port) : 0;
}

int main(int argc, char **argv)
{
	int opt;
	dvbtee_context context;
	unsigned int timeout = 0;
	char filename[256];

	context.server = NULL;

	while ((opt = getopt(argc, argv, "F:t:d::")) != -1) {
		switch (opt) {
		case 'F': /* Filename */
			strcpy(filename, optarg);
			break;

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

	context_map[getpid()] = &context;

	signal(SIGINT,  signal_callback_handler); /* Program interrupt. (ctrl-c) */
	signal(SIGABRT, signal_callback_handler); /* Process detects error and reports by calling abort */
	signal(SIGFPE,  signal_callback_handler); /* Floating-Point arithmetic Exception */
	signal(SIGILL,  signal_callback_handler); /* Illegal Instruction */
	signal(SIGSEGV, signal_callback_handler); /* Segmentation Violation */
	signal(SIGTERM, signal_callback_handler); /* Termination */
	signal(SIGHUP,  signal_callback_handler); /* Hangup */
#if 1 /* FIXME */
	ATSCMultipleStringsInit();
#endif
	context.feeder.parser.limit_eit(-1);

	start_server(&context, 62080, 62081);

	if (strlen(filename)) {
		/* first, try to open it as a file */
		if (0 <= context.feeder.open_file(filename)) {
			if (0 == context.feeder.start()) {
				context.feeder.wait_for_streaming_or_timeout(timeout);
				context.feeder.stop();
			}
			context.feeder.close_file();
		} else
		/* next, try to open it as a url */
		if (0 <= context.feeder.start_socket(filename)) {
			context.feeder.wait_for_streaming_or_timeout(timeout);
			context.feeder.stop();
			context.feeder.close_file();
		}
		goto exit;
	}

	/* if we're not feeding a file or url then read from stdin */
	if (NULL == freopen(NULL, "rb", stdin)) {
		fprintf(stderr, "failed to open stdin!\n");
		goto exit;
	}

#define BUFSIZE ((4096/188)*188)
	if (context.server) {
		/* 100 is the magic number to signify:
		   "do not call setpriority on feed thread"
		   otherwise, set a value between -20 and 19
		   **prio only works if the feed class is
		   built with double-buffer support.
		 */
		context.feeder.setup_feed(100);
		while (context.server->is_running()) {
			ssize_t r;
			unsigned char q[BUFSIZE];
			int available;

			available = sizeof(q);

			available = (available < BUFSIZE) ? available : BUFSIZE;
			if ((r = fread(q, 188, available / 188, stdin)) < (available / 188)) {
				if (ferror(stdin)) {
					fprintf(stderr, "%s: error reading stdin!\n", __func__);
					usleep(50*1000);
					clearerr(stdin);
				}
				if (feof(stdin)) {
					fprintf(stderr, "%s: EOF\n", __func__);
					stop_server(&context);
				}
			}
			/* push data into the library */
			context.feeder.push(r * 188, q);
		}
	}
exit:
	if (context.server) stop_server(&context);
//	cleanup(&context);
#if 1 /* FIXME */
	ATSCMultipleStringsDeInit();
#endif
	return 0;
}
