/*****************************************************************************
 * Copyright (C) 2011-2014 Michael Ira Krufky
 *
 * Author: Michael Ira Krufky <mkrufky@linuxtv.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef __SERVE_H__
#define __SERVE_H__

#include <pthread.h>
#include <stdint.h>

#include "tune.h"
#include "listen.h"

#define SERVE_DEFAULT_PORT 64080

typedef std::map<unsigned int, tune*> tuner_map;
typedef std::map<unsigned int, feed*> feeder_map;

class serve;

class serve_client
{
	friend class serve_parser_iface;
public:
	serve_client();
	~serve_client();
#if 1
	serve_client(const serve_client&);
	serve_client& operator= (const serve_client&);
#endif
	void setup(serve *parent, int sock) { server = parent; sock_fd = sock; }

	int start();
	void stop();

	bool socket_active() { return (sock_fd >= 0); }
	bool check();

	unsigned int get_data_fmt() { return data_fmt; }
private:
	pthread_t h_thread;
	bool f_kill_thread;

	serve *server;
	tune *tuner;
	feed *feeder;

	int sock_fd;
	FILE *channels_conf_file;
#define SERVE_DATA_FMT_NONE 0
#define SERVE_DATA_FMT_HTML 1
#define SERVE_DATA_FMT_JSON 2
#define SERVE_DATA_FMT_XML  4
#define SERVE_DATA_FMT_BIN  8
#define SERVE_DATA_FMT_CLI  16
#define SERVE_DATA_FMT_TEXT (SERVE_DATA_FMT_HTML | SERVE_DATA_FMT_JSON | SERVE_DATA_FMT_XML)
	unsigned int data_fmt;

	void stop_without_wait() { f_kill_thread = true; }
	void close_socket();

	void *client_thread();
	static void *client_thread(void*);

	bool   command(char*);
	bool __command(char*);

	bool cmd_tuner_stop();
	bool cmd_tuner_channel(int, unsigned int);
	bool cmd_tuner_scan_channels_save();

	bool list_feeders();
	bool list_tuners();
	bool list_clients();

	decode_report *reporter;

	void streamback(const uint8_t*, size_t);
	static void streamback(void*, const uint8_t*, size_t);
	static void streamback(void*, const char*);

	bool streamback_started;
	bool streamback_newchannel;

	void epg_header_footer_callback(bool header, bool channel);
	void epg_event_callback(decoded_event_t *e);
	static void epg_header_footer_callback(void * context, bool header, bool channel);
	static void epg_event_callback(void * context, decoded_event_t *e);

	const char * chandump(bool save_to_disk, parsed_channel_info_t *c);
	static const char * chandump(void *context, parsed_channel_info_t *c);
	static const char * chandump_to_disk(void *context, parsed_channel_info_t *c);

	void cli_print(const char *, ...);
	static void cli_print(void *, const char *, ...);

	std::string services;
};

typedef std::map<int, serve_client> serve_client_map;

struct libdvbtee_server_config {
	uint16_t port_requested;
	bool cli_disabled;
};

class serve
{
public:
	serve();
	~serve();

	int start(struct libdvbtee_server_config *cfg);
	int start(uint16_t port_requested = SERVE_DEFAULT_PORT);
	void stop();

	bool add_tuner(tune *new_tuner);
	bool add_feeder(feed *new_feeder);
	static bool add_feeder(void*, feed*);
	static bool add_feeder_static(void *v, feed *f) { return add_feeder(v, f); }

	bool get_epg(dump_epg_header_footer_callback epg_signal_cb,
		     dump_epg_event_callback epg_event_cb, void *epgdump_context);
	bool get_channels(parse_iface *iface, unsigned int tuner_id = 0);
	bool scan(unsigned int flags,
		  tune_iface *t_iface = NULL,
		  parse_iface *p_iface = NULL,
		  unsigned int tuner_id = 0);
	bool scan(unsigned int flags,
		  parse_iface *iface,
		  unsigned int tuner_id = 0)
		{ return scan(flags, NULL, iface, tuner_id); }
	bool scan(unsigned int flags,
		  tune_iface *iface,
		  unsigned int tuner_id = 0)
		{ return scan(flags, iface, NULL, tuner_id); }


	void set_scan_flags(tune* p_tuner, unsigned int flags);
	unsigned int get_scan_flags(tune* p_tuner);

	bool is_running() { return listener.is_running(); }
	bool is_cli_enabled() { return f_cli_enabled; }

	void reclaim_server_resources();
	void reclaim_tuner_resources();
	bool check();

	void reclaim_resources(bool enable = true) { f_reclaim_resources = enable; }

	/* FIXME: move to private */
	bool cmd_tuner_scan(tune* tuner, char *arg, bool scanepg, bool wait_for_results, unsigned int flags,
			    tune_iface *t_iface, parse_iface *p_iface);
	bool cmd_config_channels_conf_load(tune* tuner, parse_iface *iface);

	feed_server_map feed_servers;

	serve_client_map* get_client_map() { return &client_map; }
private:
	socket_listen listener;
	serve_client_map client_map;

	pthread_t h_thread;
	bool f_kill_thread;

	void *monitor_thread();
	static void *monitor_thread(void*);

	int start_monitor();
	void stop_monitor() { f_kill_thread = true; }

	void add_client(int);
	static void add_client(void*, int);

	std::map<tune*, unsigned int> scan_flags;

	bool f_reclaim_resources;
	bool f_cli_enabled;
};

#endif /*__SERVE_H__ */
