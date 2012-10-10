/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
 *
 * Author: Michael Krufky <mkrufky@linuxtv.org>
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

typedef std::map<uint8_t, tune*> tuner_map;

class serve;

class serve_client
{
public:
	serve_client();
	~serve_client();
#if 1
	serve_client(const serve_client&);
	serve_client& operator= (const serve_client&);
#endif
	void setup(serve *parent, int sock) { server = parent; sock_fd = sock; };

	int start();
	void stop();

	bool socket_active() { return (sock_fd >= 0); };
	bool check();
private:
	pthread_t h_thread;
	bool f_kill_thread;

	serve *server;

	int sock_fd;
	FILE *channels_conf_file;
#define SERVE_DATA_FMT_NONE 0
#define SERVE_DATA_FMT_HTML 1
#define SERVE_DATA_FMT_BIN  2
#define SERVE_DATA_FMT_JSON 4
#define SERVE_DATA_FMT_CLI  8
#define SERVE_DATA_FMT_TEXT (SERVE_DATA_FMT_HTML | SERVE_DATA_FMT_JSON)
	unsigned int data_fmt;

	void stop_without_wait() { f_kill_thread = true; };
	void close_socket();

	void *client_thread();
	static void *client_thread(void*);

	bool   command(char*);
	bool __command(char*);

	bool cmd_tuner_stop(tune*);
	bool cmd_tuner_channel(tune*, int, unsigned int);
	bool cmd_tuner_scan(tune*, char*, bool, bool, unsigned int);
	bool cmd_tuner_scan_channels_save(tune* tuner);
	bool cmd_config_channels_conf_load();

	decode_report *reporter;

	void streamback(const uint8_t*, size_t);
	static void streamback(void*, const uint8_t*, size_t);
	static void streamback(void*, const char*);

	bool streamback_started;
	bool streamback_newchannel;

	void epg_header_footer_callback(bool header, bool channel);
	void epg_event_callback(const char * channel_name,
					uint16_t chan_major,
					uint16_t chan_minor,
					//
					uint16_t event_id,
					time_t start_time,
					uint32_t length_sec,
					const char * name,
					const char * text);
	static void epg_header_footer_callback(void * context, bool header, bool channel);
	static void epg_event_callback(void * context,
					const char * channel_name,
					uint16_t chan_major,
					uint16_t chan_minor,
					//
					uint16_t event_id,
					time_t start_time,
					uint32_t length_sec,
					const char * name,
					const char * text);

	const char * chandump(bool save_to_disk,
		     uint16_t lcn, uint16_t major, uint16_t minor,
		     uint16_t physical_channel, uint32_t freq, const char *modulation,
		     unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number);
	static const char * chandump(void *context,
		     uint16_t lcn, uint16_t major, uint16_t minor,
		     uint16_t physical_channel, uint32_t freq, const char *modulation,
		     unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number);
	static const char * chandump_to_disk(void *context,
		     uint16_t lcn, uint16_t major, uint16_t minor,
		     uint16_t physical_channel, uint32_t freq, const char *modulation,
		     unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number);

	void cli_print(const char *, ...);
	static void cli_print(void *, const char *, ...);
};

typedef std::map<int, serve_client> serve_client_map;

class serve
{
public:
	serve();
	~serve();

	int start(uint16_t port_requested = SERVE_DEFAULT_PORT);
	void stop();

	bool add_tuner(tune *new_tuner) /*{ tuners[tuners.size()] = new_tuner; }*/;

	void set_scan_flags(unsigned int tuner_id, unsigned int flags) { scan_flags = flags; };
	unsigned int get_scan_flags(unsigned int tuner_id) { return scan_flags; };

	bool is_running() { return listener.is_running(); };

	bool check();

	feed_server_map feed_servers;
private:
	socket_listen listener;
	serve_client_map client_map;

	void add_client(int);
	static void add_client(void*, int);

	unsigned int scan_flags;
};

#endif /*__SERVE_H__ */
