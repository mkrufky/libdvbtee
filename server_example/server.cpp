/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
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
	tune tuner;
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
		context->tuner.feeder.stop_without_wait();
		context->tuner.feeder.close_file();
		context->tuner.close_demux();
	} else {
		context->tuner.stop_feed();
	}

	context->tuner.close_fe();
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

	context->tuner.feeder.parser.cleanup();

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

int start_server(struct dvbtee_context* context, unsigned int flags)
{
	context->server = new serve;
	context->server->add_tuner(&context->tuner);

	if (flags & 2)
		context->tuner.feeder.parser.out.add_http_server(SERVE_DEFAULT_PORT+1);

	context->server->set_scan_flags(0, flags >> 2);

	return context->server->start(62080);
}

int main(int argc, char **argv)
{
	int opt;
	dvbtee_context context;

	context.server = NULL;

	/* LinuxDVB context: */
	int dvb_adap = 0; /* ID X, /dev/dvb/adapterX/ */
	int demux_id = 0; /* ID Y, /dev/dvb/adapterX/demuxY */
	int dvr_id   = 0; /* ID Y, /dev/dvb/adapterX/dvrY */
	int fe_id    = 0; /* ID Y, /dev/dvb/adapterX/frontendY */

	unsigned int serv_flags  = 0;
	unsigned int scan_flags  = 0;

        while ((opt = getopt(argc, argv, "a:A:c:C:f:F:t:T:i:I:s::S::E::o::O:d::")) != -1) {
		switch (opt) {
		case 'a': /* adapter */
			dvb_adap = strtoul(optarg, NULL, 0);
			break;
		case 'A': /* ATSC / QAM */
			scan_flags = strtoul(optarg, NULL, 0);
			break;
		case 'f': /* frontend */
			fe_id = strtoul(optarg, NULL, 0);
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
	context.tuner.set_device_ids(dvb_adap, fe_id, demux_id, dvr_id, false);
	context.tuner.feeder.parser.limit_eit(-1);

	start_server(&context, serv_flags | (scan_flags << 2));

	if (context.server) {
		while (context.server->is_running()) sleep(1);
		stop_server(&context);
	}
//	cleanup(&context);
#if 1 /* FIXME */
	ATSCMultipleStringsDeInit();
#endif
	return 0;
}
