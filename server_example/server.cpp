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
#include "linuxtv_tuner.h"
#include "serve.h"

#include "atsctext.h"

typedef std::map<uint8_t, tune> map_tuners;

struct dvbtee_context
{
	linuxtv_tuner tuner;
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

int start_server(struct dvbtee_context* context, unsigned int flags, int port, int eavesdropping_port = 0)
{
	context->server = new serve;
	context->server->add_tuner(&context->tuner);

	if (eavesdropping_port)
		context->tuner.feeder.parser.out.add_http_server(eavesdropping_port);

	context->server->set_scan_flags(0, flags);

	return context->server->start(port);
}


//FIXME: we return const char * for no reason - this will be converted to bool, int or void
const char * chandump(void *context, parsed_channel_info_t *c)
{
	char channelno[7]; /* XXX.XXX */
	if (c->major + c->minor > 1)
		sprintf(channelno, "%d.%d", c->major, c->minor);
	else if (c->lcn)
		sprintf(channelno, "%d", c->lcn);
	else
		sprintf(channelno, "%d", c->physical_channel);

	/* xine format */
	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		c->service_name,
		c->freq,//iter_vct->second.carrier_freq,
		c->modulation,
		c->vpid, c->apid, c->program_number);

	/* link to http stream */
	fprintf(stdout, "<a href='/tune=%d+%d&stream/here'>%s: %s</a>",
		c->physical_channel, c->program_number, channelno, c->service_name);

	return NULL;
}

bool list_channels(serve *server)
{
	if (!server)
		return false;

	return server->get_channels(chandump, NULL);
}

bool start_async_channel_scan(serve *server, unsigned int flags = 0)
{
	return server->scan(flags);
}

bool channel_scan_and_dump(serve *server, unsigned int flags = 0)
{
	return server->scan(flags, chandump, NULL);
}


void epg_header_footer(void *context, bool header, bool channel)
{
	const char *noun = (channel) ? "channel" : "epg table";
	const char *adj  = (header)  ? "start" : "end";
	fprintf(stdout, "receiving %s of %s\n", adj, noun);
}

void epg_event(void *context, decoded_event_t *e)
{
	fprintf(stdout, "received event id: %d on channel name: %s, major: %d, minor: %d, physical: %d, service id: %d, title: %s, desc: %s, start time (time_t) %ld, duration (sec) %d\n",
		e->event_id, e->channel_name, e->chan_major, e->chan_minor, e->chan_physical, e->chan_svc_id, e->name, e->text, e->start_time, e->length_sec);
}

bool request_epg(serve *server)
{
	if (!server)
		return false;

	return server->get_epg(epg_header_footer, epg_event, NULL);
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

        while ((opt = getopt(argc, argv, "a:A:f:d::")) != -1) {
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

	start_server(&context, scan_flags, 62080, 62081);

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
