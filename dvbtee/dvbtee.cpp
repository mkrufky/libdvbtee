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

#include <inttypes.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "feed.h"
#include "hlsfeed.h"

#include "dvbtee_config.h"
#ifdef USE_LINUXTV
#include "linuxtv_tuner.h"
#endif
#ifdef USE_HDHOMERUN
#include "hdhr_tuner.h"
#endif
#include "serve.h"

typedef std::map<uint8_t, tune*> map_tuners;

struct dvbtee_context
{
	int channel;
	bool b_read_dvr;
	bool b_scan;
	bool scan_epg;
	bool b_output_file;
	bool b_output_stdout;
	bool b_serve;
	bool b_kernel_pid_filters;
	bool b_help;
	bool b_json;
	bool b_bitrate_stats;
	bool b_hdhr;

#ifdef USE_LINUXTV
	/* LinuxDVB context: */
	int dvb_adap; /* ID X, /dev/dvb/adapterX/ */
	int fe_id;    /* ID Y, /dev/dvb/adapterX/frontendY */
	int demux_id; /* ID Y, /dev/dvb/adapterX/demuxY */
	int dvr_id;   /* ID Y, /dev/dvb/adapterX/dvrY */
#endif

	unsigned int serv_flags;
	unsigned int scan_flags;
	unsigned int scan_min;
	unsigned int scan_max;
	unsigned int scan_method;

	int num_tuners;
	unsigned int timeout;

	unsigned int wait_event;
	int eit_limit;

	tune *tuner;

	enum output_options out_opt;

	char filename[256];
	char outfilename[256];
	char tcpipfeedurl[2048];
	char service_ids[64];
	char channel_list[256];
	char hdhrname[256];

//////////////////////////////////////////////////////////////////

	dvbtee::feed::Feeder* _file_feeder;
	map_tuners tuners;
	serve *server;

	dvbtee_context() {
		channel = 0;
		b_read_dvr = false;
		b_scan     = false;
		scan_epg   = false;
		b_output_file   = false;
		b_output_stdout = false;
		b_serve    = false;
		b_kernel_pid_filters = false;
		b_help     = false;
		b_json     = false;
		b_bitrate_stats = false;
		b_hdhr     = false;

#ifdef USE_LINUXTV
		/* LinuxDVB context: */
		dvb_adap = -1; /* ID X, /dev/dvb/adapterX/ */
		fe_id    = -1; /* ID Y, /dev/dvb/adapterX/frontendY */
		demux_id = 0; /* ID Y, /dev/dvb/adapterX/demuxY */
		dvr_id   = 0; /* ID Y, /dev/dvb/adapterX/dvrY */
#endif

		serv_flags  = 0;
		scan_flags  = 0;
		scan_min    = 0;
		scan_max    = 0;
		scan_method = 0;

		num_tuners           = -1;
		timeout     = 0;

		wait_event  = 0;
		eit_limit            = -1;

		tuner = NULL;

		out_opt = (enum output_options)-1;

		memset(&filename, 0, sizeof(filename));
		memset(&outfilename, 0, sizeof(outfilename));
		memset(&tcpipfeedurl, 0, sizeof(tcpipfeedurl));
		memset(&service_ids, 0, sizeof(service_ids));
		memset(&channel_list, 0, sizeof(channel_list));
		memset(&hdhrname, 0, sizeof(hdhrname));
	}
};

struct dvbtee_context* ctxt;

class write_feed : public curlhttpget_iface
{
public:
	void write_data(void *buffer, size_t size, size_t nmemb)
	{
		feeder.push(size * nmemb, (const uint8_t*)buffer);
	}
private:
	dvbtee::feed::PushFeeder feeder;
};


void cleanup_tuners(struct dvbtee_context* context, bool quick = false)
{
#if 0
	bool erased = false; /* FIXME: fix this function to call itself recoursively until all tuners are destroyed  */
#endif
	for (map_tuners::const_iterator iter = context->tuners.begin(); iter != context->tuners.end(); ++iter) {

		if (quick) {
			iter->second->feeder.stop_without_wait();
			iter->second->feeder.close_file();
		} else
			iter->second->stop_feed();

		iter->second->close_fe();
		iter->second->feeder.parser.cleanup();
#if 0
		/* this causes a bad crash when enabled */
		delete iter->second;
#endif
	}
	context->tuners.clear();
}

void stop_server(struct dvbtee_context* context);

void cleanup(struct dvbtee_context* context, bool quick = false)
{
	if (context->server)
		stop_server(context);

#if 0
	if (quick)
		context->_file_feeder->stop_without_wait();
	else
#endif
		context->_file_feeder->stop();

	context->_file_feeder->close_file();
	context->_file_feeder->parser.cleanup();

	cleanup_tuners(context, quick);
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

	exit(signum);
}


static void bitrate_stats(void *priv, stats_map &bitrates, stats_map &discontinuities, uint64_t tei_count, bool per_sec)
{
	(void)priv;
	/* display the bitrates for each pid, followed by special pid 0x2000 for the full TS */
	for (stats_map::const_iterator iter = bitrates.begin(); iter != bitrates.end(); ++iter) {
		char a[16];
		char b[16];
		fprintf(stderr, "pid %04x %5" PRIu64 " p%s  %sb%s  %sbit\n",
			iter->first, iter->second / 188, (per_sec) ? "/s" : "",
			stats_scale_unit(a, sizeof(a), iter->second), (per_sec) ? "/s" : "",
			stats_scale_unit(b, sizeof(b), iter->second * 8));
	}
	for (stats_map::const_iterator iter = discontinuities.begin(); iter != discontinuities.end(); ++iter)
		fprintf(stderr, "pid %04x\t%" PRIu64 " discontinuities (%" PRIu64 "%%)\n", iter->first, iter->second, ((!iter->second) || (!bitrates[iter->first])) ? 0 : (!bitrates.count(iter->first)) ? 0 : (100 * iter->second / (bitrates[iter->first] / 188)));

	if (tei_count)
		fprintf(stderr, "tei count: %" PRIu64 " (%" PRIu64 "%%)\n", tei_count, (!bitrates[0x2000]) ? 0 : (18800 * tei_count / bitrates[0x2000]));

	fprintf(stderr,"\n");
}


void stop_server(struct dvbtee_context* context)
{
	if (!context->server)
		return;

	if (context->server->is_running())
		context->server->stop();

	delete context->server;
	context->server = NULL;

	return;
}

int start_server(struct dvbtee_context* context, unsigned int flags)
{
	context->server = new serve;

	if (!context->tuners.size())
		context->server->add_feeder(context->_file_feeder);

	for (map_tuners::const_iterator iter = context->tuners.begin(); iter != context->tuners.end(); ++iter) {
		context->server->add_tuner(iter->second);
		context->server->set_scan_flags(iter->second, flags >> 2);

		if (flags & 2)
			iter->second->feeder.parser.out.add_http_server(SERVE_DEFAULT_PORT+1+iter->first);
	}

	return context->server->start();
}

void multiscan(struct dvbtee_context* context, unsigned int scan_method,
	       unsigned int scan_flags, unsigned int scan_min, unsigned int scan_max, bool scan_epg)
{
	int count = 0;
	int partial_redundancy = 0;
	int channels_to_scan = scan_max - scan_min + 1;
	size_t num_tuners = context->tuners.size();

	switch (scan_method) {
	case 1: /* speed */
		for (map_tuners::const_iterator iter = context->tuners.begin(); iter != context->tuners.end(); ++iter) {
			int i = iter->first;
			int scan_start = scan_min + ((0 + i) * (unsigned int)channels_to_scan/num_tuners);
			int scan_end   = scan_min + ((1 + i) * (unsigned int)channels_to_scan/num_tuners);
			fprintf(stderr, "speed scan: tuner %d scanning from %d to %d\n", i, scan_start, scan_end);
			iter->second->tune::start_scan(scan_flags, scan_start, scan_end, scan_epg);
			sleep(1); // FIXME
		}
		break;
	case 2: /* redundancy */
		for (map_tuners::const_iterator iter = context->tuners.begin(); iter != context->tuners.end(); ++iter) {
			int i = iter->first;
			fprintf(stderr, "redundancy scan: tuner %d scanning from %d to %d\n", i, scan_min, scan_max);
			iter->second->tune::start_scan(scan_flags, scan_min, scan_max, scan_epg);
			sleep(5); // FIXME
		}
		break;
	case 4: /* speed AND partial redundancy */
		partial_redundancy = (num_tuners > 2) ? (num_tuners - 2) : 0;
		/* FALL-THRU */
	default:
	case 3: /* speed AND redundancy */
		for (int j = 0; j < ((int) num_tuners) - partial_redundancy; j++) {
			for (map_tuners::const_iterator iter = context->tuners.begin(); iter != context->tuners.end(); ++iter) {
				int i = iter->first;
				if (j) {
					iter->second->wait_for_scan_complete();
				}
				int scan_start, scan_end;
				if (i + j < ((int) num_tuners)) {
					scan_start = scan_min + ((0 + (i + j)) * (unsigned int)channels_to_scan/num_tuners);
					scan_end   = scan_min + ((1 + (i + j)) * (unsigned int)channels_to_scan/num_tuners);
				} else {
					scan_start = scan_min + ((0 + ((i + j) - num_tuners)) * (unsigned int)channels_to_scan/num_tuners);
					scan_end   = scan_min + ((1 + ((i + j) - num_tuners)) * (unsigned int)channels_to_scan/num_tuners);
				}
				fprintf(stderr, "speed & %sredundancy scan: pass %d of %zu, tuner %d scanning from %d to %d\n",
					(partial_redundancy) ? "partial " : "",
					j + 1, num_tuners - partial_redundancy,
					i, scan_start, scan_end);
				iter->second->tune::start_scan(scan_flags, scan_start, scan_end, scan_epg);
				sleep(1); // FIXME
			}
		}
		break;
	}
	for (map_tuners::const_iterator iter = context->tuners.begin(); iter != context->tuners.end(); ++iter)
		iter->second->wait_for_scan_complete();
#if 1
	for (map_tuners::const_iterator iter = context->tuners.begin(); iter != context->tuners.end(); ++iter) {
		int _count = iter->second->feeder.parser.xine_dump();
		count += _count;
		fprintf(stderr, "tuner %d found %d services\n", iter->first, _count);
	}
#else
	count += tuners[0].feeder.parser.xine_dump();
#endif
	fprintf(stderr, "found %d services in total\n", count);
}

void configure_output(struct dvbtee_context* context)
{
	if (context->out_opt > 0) {
		context->_file_feeder->parser.out.set_options(context->out_opt);
	}
	if (context->b_bitrate_stats) {
		context->_file_feeder->parser.statistics.set_statistics_callback(bitrate_stats, &context);
	}
	if (context->b_output_file) {
		context->_file_feeder->parser.add_output(context->outfilename);
	}
	if (context->b_output_stdout) {
		context->_file_feeder->parser.add_stdout();
	}
}

void usage(bool help, char *myname)
{
	fprintf(stderr, "built against libdvbpsi version %s\n\n", parse_libdvbpsi_version);
	int f_count = parse::count_decoder_factories();
	fprintf(stderr, "\n  "
		"-a\tadapter id\n  "
		"-A\t(1 for ATSC, 2 for ClearQAM)\n  "
		"-b\tdisplay bitrates & statistics\n  "
		"-c\tchannel to tune /\n\tcomma (,) separated list of channels to scan /\n\tscan minimum channel\n  "
		"-C\tchannel to tune /\n\tcomma (,) separated list of channels to scan /\n\tscan maximum channel\n  "
		"-f\tfrontend id\n  "
		"-F\tfilename to use as input\n  "
		"-t\ttimeout\n  "
		"-T\tnumber of tuners (dvb adapters) allowed to use, 0 for all\n  "
		"-s\tscan, optional arg when using multiple tuners: \n\t1 for speed, 2 for redundancy, \n\t3 for speed AND redundancy, \n\t4 for optimized speed / partial redundancy\n  "
		"-S\tserver mode, optional arg 1 for command server, \n\t2 for http stream server, 3 for both\n  "
		"-i\tpull local/remote tcp/udp port for data\n  "
		"-I\trequest a service and its associated PES streams by its service id\n  "
		"-E\tenable EPG scan, optional arg to limit the number of EITs to parse\n  "
		"-o\toutput filtered data, optional arg is a filename / URI, ie udp://127.0.0.1:1234\n  "
		"-O\toutput options: (or-able) 1 = PAT/PMT, 2 = PES, 4 = PSIP\n  "
		"-H\tuse a HdHomeRun device, optional arg to specify the device string\n  "
		"-j\tenable json output of decoded tables & descriptors%s\n  "
		"-d\tdebug level\n  "
		"-h\tdisplay additional help\n\n", f_count ? "" : " (feature disabled)");
	if (help)
		fprintf(stderr,
		"To tune to service id 1 of physical channel 33 and stream it to a udp port:\n  "
		"%s -c33 -I1 -oudp://192.168.1.100:1234\n\n"
		"To tune the second frontend of adapter 1 and stream the full TS of physical channel 44 to a tcp listener:\n  "
		"%s -c44 -otcp://192.168.1.200:5555\n\n"
		"To listen to a TCP port and stream to a UDP port:\n  "
		"%s -itcp://5555 -oudp://192.168.1.100:1234\n\n"
		"To parse a captured file and filter out the PSIP data, saving the PAT/PMT and PES streams to a file:\n  "
		"%s -Finput.ts -O3 -ofile://output.ts\n\n"
		"To parse a UDP stream for ten seconds:\n  "
		"%s -iudp://127.0.0.1:1234 -t10\n\n"
		"To scan for ClearQAM services using 5 tuners optimized for speed and partial redundancy:\n  "
		"%s -A2 -T5 -s4\n\n"
		"To scan for ATSC services using 2 HdHomeRun tuners optimized for speed and redundancy:\n  "
		"%s -A1 -H -T2 -s3\n\n"
		"To start a server using adapter 0:\n  "
		"%s -a0 -S\n\n"
		"To start a server using tuner1 of a specific HdHomeRun device (ex: ABCDABCD):\n  "
		"%s -H ABCDABCD-1 -S\n\n"
		, myname, myname, myname, myname, myname, myname, myname, myname, myname
	);
}

int main(int argc, char **argv)
{
#if defined(_WIN32)
/* Initialize Winsock as described here:
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ms738573(v=vs.85).aspx
 */
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
		printf("Error at WSAStartup()\n");
#endif
	dvbtee_context context;
	context.server = NULL;
	ctxt = &context;

	int opt;
	while ((opt = getopt(argc, argv, "a:A:bc:C:f:F:t:T:i:I:js::S::E::o::O:d::H::h?")) != -1) {
		switch (opt) {
		case 'a': /* adapter */
#ifdef USE_LINUXTV
			ctxt->dvb_adap = strtoul(optarg, NULL, 0);
			ctxt->b_read_dvr = true;
#endif
			break;
		case 'A': /* ATSC / QAM */
			ctxt->scan_flags = strtoul(optarg, NULL, 0);
			ctxt->b_read_dvr = true;
			break;
		case 'b': /* bitrates & statistics */
			ctxt->b_bitrate_stats = true;
			break;
		case 'c': /* channel list | channel / scan min */
			if (strstr(optarg, ","))
				strncpy(ctxt->channel_list, optarg, sizeof(ctxt->channel_list)-1);

			/* if a list was provided, use the first item */
			ctxt->channel = strtoul(optarg, NULL, 0);
			ctxt->scan_min = strtoul(optarg, NULL, 0);

			ctxt->b_read_dvr = true;
			break;
		case 'C': /* channel list | channel / scan max */
			if (strstr(optarg, ","))
				strncpy(ctxt->channel_list, optarg, sizeof(ctxt->channel_list));

			/* if a list was provided, use the first item */
			ctxt->scan_max = strtoul(optarg, NULL, 0);

			ctxt->b_read_dvr = true;
			break;
		case 'f': /* frontend */
#ifdef USE_LINUXTV
			ctxt->fe_id = strtoul(optarg, NULL, 0);
			ctxt->b_read_dvr = true;
#endif
			break;
		case 'F': /* Filename */
			strncpy(ctxt->filename, optarg, sizeof(ctxt->filename));
			break;
		case 't': /* timeout */
			ctxt->timeout = strtoul(optarg, NULL, 0);
			break;
		case 'T': /* number of tuners (dvb adapters) allowed to use, 0 for all */
			ctxt->num_tuners = strtoul(optarg, NULL, 0);
			ctxt->b_read_dvr = true;
			break;
		case 's': /* scan, optional arg when using multiple tuners: 1 for speed, 2 for redundancy, 3 for speed AND redundancy, 4 for optimized speed / partial redundancy */
			ctxt->b_scan = true;
			ctxt->scan_method = (optarg) ? strtoul(optarg, NULL, 0) : 0;
			if (ctxt->scan_method)
				fprintf(stderr, "MULTISCAN: %d...\n", ctxt->scan_method);
			break;
		case 'S': /* server mode, optional arg 1 for command server, 2 for http stream server, 3 for both */
			ctxt->b_serve = true;
			ctxt->serv_flags = (optarg) ? strtoul(optarg, NULL, 0) : 0;
			break;
		case 'i': /* pull local/remote tcp/udp port for data */
			strncpy(ctxt->tcpipfeedurl, optarg, sizeof(ctxt->tcpipfeedurl));
			break;
		case 'I': /* request a service by its service id */
			strncpy(ctxt->service_ids, optarg, sizeof(ctxt->service_ids));
			break;
		case 'E': /* enable EPG scan, optional arg to limit the number of EITs to parse */
#if 0
			ctxt->b_scan = true; // FIXME
#endif
			ctxt->scan_epg = true;
			ctxt->wait_event = FEED_EVENT_EPG;
			ctxt->eit_limit = (optarg) ? strtoul(optarg, NULL, 0) : -1;
			if (ctxt->eit_limit >= 0)
				fprintf(stderr, "EIT LIMIT: %d...\n", ctxt->eit_limit);
			break;
		case 'o': /* output filtered data, optional arg is a filename */
			if (optarg) {
				/* outfilename was initialized with 0's */
				strncpy(ctxt->outfilename, optarg, sizeof(ctxt->outfilename)-1);
				ctxt->b_output_file = true;
			} else
				ctxt->b_output_stdout = true;
			break;
		case 'O': /* output options */
			ctxt->out_opt = (enum output_options)strtoul(optarg, NULL, 0);
			break;
		case 'd':
			if (optarg)
				libdvbtee_set_debug_level(strtoul(optarg, NULL, 0));
			else
				libdvbtee_set_debug_level(255);
			break;
		case 'H':
			if (optarg)
				strncpy(ctxt->hdhrname, optarg, sizeof(ctxt->hdhrname));
			ctxt->b_hdhr = true;
			break;
		case 'j':
			ctxt->b_json = true;
			break;
		case 'h':
			ctxt->b_help = true;
			/* fall - thru  */
		default:
			usage(ctxt->b_help, argv[0]);
			return -1;
		}
	}
#define b_READ_TUNER (ctxt->b_read_dvr || ctxt->b_hdhr)

	signal(SIGINT,  signal_callback_handler); /* Program interrupt. (ctrl-c) */
	signal(SIGABRT, signal_callback_handler); /* Process detects error and reports by calling abort */
	signal(SIGFPE,  signal_callback_handler); /* Floating-Point arithmetic Exception */
	signal(SIGILL,  signal_callback_handler); /* Illegal Instruction */
	signal(SIGSEGV, signal_callback_handler); /* Segmentation Violation */
	signal(SIGTERM, signal_callback_handler); /* Termination */
#if !defined(_WIN32)
	signal(SIGHUP,  signal_callback_handler); /* Hangup */
#endif
	ctxt->b_kernel_pid_filters = (strlen(ctxt->service_ids) > 0) ? true : false;

	if (((ctxt->b_scan) && (ctxt->num_tuners == -1)) || b_READ_TUNER) {
#ifdef USE_HDHOMERUN
		if (ctxt->b_hdhr) {
			ctxt->tuner = new hdhr_tuner;

			if ((ctxt->tuner) && (strlen(ctxt->hdhrname))) {
				((hdhr_tuner*)(ctxt->tuner))->set_hdhr_id(ctxt->hdhrname);
			}
		} else {
#else
		{
#endif
#ifdef USE_LINUXTV
			ctxt->tuner = new linuxtv_tuner;

			if ((ctxt->tuner) && ((ctxt->dvb_adap >= 0) || (ctxt->fe_id >= 0))) {
				if (ctxt->dvb_adap < 0)
					ctxt->dvb_adap = 0;
				if (ctxt->fe_id < 0)
					ctxt->fe_id = 0;
				((linuxtv_tuner*)(ctxt->tuner))->set_device_ids(ctxt->dvb_adap, ctxt->fe_id, ctxt->demux_id, ctxt->dvr_id, ctxt->b_kernel_pid_filters);
			}
#endif
		}
		if (ctxt->tuner) {
			context.tuners[context.tuners.size()] = ctxt->tuner;
			ctxt->tuner->feeder.parser.limit_eit(ctxt->eit_limit);
		} else {
			fprintf(stderr, "ERROR allocating tuner %zu\n", context.tuners.size());
#if defined(_WIN32)
			WSACleanup();
#endif
			exit(-1);
		}
	}
#if (defined(USE_HDHOMERUN) | defined(USE_LINUXTV))
	if (ctxt->num_tuners > 0) while (context.tuners.size() < ((unsigned int) ctxt->num_tuners)) {
		tune *new_tuner = NULL;
#ifdef USE_HDHOMERUN
		if (ctxt->b_hdhr) {
			new_tuner = new hdhr_tuner;
		} else
#endif
		{
#ifdef USE_LINUXTV
			new_tuner = new linuxtv_tuner;
#endif
		}
		if (new_tuner) {
			new_tuner->feeder.parser.limit_eit(ctxt->eit_limit);
			context.tuners[context.tuners.size()] = new_tuner;
		} else {
			fprintf(stderr, "ERROR allocating tuner %zu\n", context.tuners.size());
			break;
		}
	}
#endif

	if (b_READ_TUNER) {// FIXME
		for (map_tuners::const_iterator iter = context.tuners.begin(); iter != context.tuners.end(); ++iter) {
			if (ctxt->out_opt > 0) {
				iter->second->feeder.parser.out.set_options(ctxt->out_opt);
			}
			if (ctxt->b_bitrate_stats) {
				iter->second->feeder.parser.statistics.set_statistics_callback(bitrate_stats, &context);
			}
			if (ctxt->b_output_file) {
				iter->second->feeder.parser.add_output(ctxt->outfilename);
			}
			if (ctxt->b_output_stdout) {
				iter->second->feeder.parser.add_stdout();
			}
		}
	}
	if (ctxt->b_serve)
		start_server(&context, ctxt->serv_flags | (ctxt->scan_flags << 2));

	if ((ctxt->scan_min) && (!ctxt->scan_max))
		ctxt->scan_max = ctxt->scan_min;
	else
	if ((ctxt->scan_max) && (!ctxt->scan_min))
		ctxt->channel = ctxt->scan_min = ctxt->scan_max;

	if ((ctxt->b_scan) && (b_READ_TUNER || (ctxt->num_tuners >= 0))) {
		if (ctxt->num_tuners >= 0)
			multiscan(&context, ctxt->scan_method, ctxt->scan_flags, ctxt->scan_min, ctxt->scan_max, ctxt->scan_epg); // FIXME: channel_list
		else {
			if (strlen(ctxt->channel_list)) {
				if (ctxt->tuner) ctxt->tuner->scan_for_services(ctxt->scan_flags, ctxt->channel_list, ctxt->scan_epg);
			} else {
				if (ctxt->tuner) ctxt->tuner->scan_for_services(ctxt->scan_flags, ctxt->scan_min, ctxt->scan_max, ctxt->scan_epg);
			}
		}
		goto exit;
	}

	if (strlen(ctxt->filename)) {
		context._file_feeder = new dvbtee::feed::FileFeeder;
		configure_output(ctxt);
		if (0 <= context._file_feeder->open_file(ctxt->filename)) {
			int ret = context._file_feeder->start();
			if (ctxt->b_serve) goto exit;
			if (0 == ret) {
				context._file_feeder->wait_for_streaming_or_timeout(ctxt->timeout);
				context._file_feeder->stop();
			}
			context._file_feeder->close_file();
		}
		goto exit;
	}

#if 0
	if (strlen(ctxt->tcpipfeedurl)) {
		context._file_feeder = new dvbtee::feed:://FileFeeder;
		configure_output(ctxt);
		if (0 == strncmp(ctxt->tcpipfeedurl, "http", 4)) {
			write_feed iface;
			hlsfeed(ctxt->tcpipfeedurl, &iface);
		} else {
			int ret = context._file_feeder->start_socket(ctxt->tcpipfeedurl);
			if (ctxt->b_serve) goto exit;
			if (0 <= ret) {
				context._file_feeder->wait_for_streaming_or_timeout(ctxt->timeout);
				context._file_feeder->stop();
				context._file_feeder->close_file();
			}
		}
		goto exit;
	}
#endif

	if ((ctxt->tuner) && (ctxt->channel)) {
		fprintf(stderr, "TUNE to channel %d...\n", ctxt->channel);
		int fe_fd = ctxt->tuner->open_fe();
		if (fe_fd < 0)
			return fe_fd;

		if (!ctxt->scan_flags)
			ctxt->scan_flags = SCAN_VSB;

		if (ctxt->tuner->tune_channel(
				(ctxt->scan_flags == SCAN_VSB) ? DVBTEE_VSB_8 : DVBTEE_QAM_256, ctxt->channel)) {
			if (!ctxt->tuner->wait_for_lock_or_timeout(2000)) {
				ctxt->tuner->close_fe();
				goto exit; /* NO LOCK! */
			}
			ctxt->tuner->feeder.parser.set_channel_info(ctxt->channel,
				(ctxt->scan_flags == SCAN_VSB) ? atsc_vsb_chan_to_freq(ctxt->channel) : atsc_qam_chan_to_freq(ctxt->channel),
				(ctxt->scan_flags == SCAN_VSB) ? "8VSB" : "QAM_256");
		}

		if (strlen(ctxt->service_ids) > 0) {
			ctxt->tuner->feeder.parser.set_service_ids(ctxt->service_ids);
			if (ctxt->out_opt < 0)
				ctxt->tuner->feeder.parser.out.set_options((enum output_options)OUTPUT_AV);
		}
	}

	if ((ctxt->tuner) && b_READ_TUNER) {
		/* assume frontend is already streaming,
		   all we have to do is read from the DVR device */
		int ret = ctxt->tuner->start_feed();
		if (ctxt->b_serve) goto exit;
		else {
			if (0 == ret) {
				ctxt->tuner->feeder.wait_for_event_or_timeout(ctxt->timeout, ctxt->wait_event);
				ctxt->tuner->stop_feed();
			}
			if (ctxt->channel) /* if we tuned the frontend ourselves then close it */
				ctxt->tuner->close_fe();
		}
	}
	else
	if (0 == context._file_feeder->parser.get_fed_pkt_count()) {
		fprintf(stderr, "reading from STDIN\n");
		int ret = context._file_feeder->start_stdin();
		if (ctxt->b_serve) goto exit;
		if (0 == ret) {
			context._file_feeder->wait_for_streaming_or_timeout(ctxt->timeout);
			context._file_feeder->stop();
		}
	}

	if (ctxt->b_serve)
		goto exit;

	if ((ctxt->tuner) && (ctxt->b_scan)) // scan channel mode, normal scan would have goto'd to exit
		ctxt->tuner->feeder.parser.xine_dump();
	if ((ctxt->tuner) && (ctxt->scan_epg))
		ctxt->tuner->feeder.parser.epg_dump();
exit:
	if (context.server) {
		while (context.server->is_running()) sleep(1);
		stop_server(&context);
	}
	if (ctxt->b_json) {
		parse::dumpJson();
	}
	cleanup(&context);
#if defined(_WIN32)
	WSACleanup();
#endif
	return 0;
}
