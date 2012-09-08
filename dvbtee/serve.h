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

#ifndef __SERVE_H__
#define __SERVE_H__

#include <pthread.h>
#include <stdint.h>

#include "tune.h"

#define SERVE_DEFAULT_PORT 64080

typedef std::map<uint8_t, tune*> tuner_map;

class serve
{
public:
	serve();
	~serve();

	int start(uint16_t port_requested = SERVE_DEFAULT_PORT);
	void stop();
#if 0
	int push(uint8_t* p_data);
#endif
	bool add_tuner(tune *new_tuner) { tuners[tuners.size()] = new_tuner; };
private:
	pthread_t h_thread;
	bool f_kill_thread;

	void *serve_thread();
	static void *serve_thread(void*);
	void stop_without_wait() { f_kill_thread = true; };
	void close_socket();

	void streamback(const uint8_t*, size_t);
	static void streamback(void*, const uint8_t*, size_t);
	static void streamback(void*, const char*);
	int streamback_socket;
	bool streamback_started;
	bool streamback_newchannel;

	bool   command(int, char*);
	bool __command(int, char*);

	int sock_fd;
	uint16_t port;

	tuner_map tuners;

	const char * epg_header_footer_callback(bool header, bool channel);
	const char * epg_event_callback(const char * channel_name,
					uint16_t chan_major,
					uint16_t chan_minor,
					//
					uint16_t event_id,
					time_t start_time,
					uint32_t length_sec,
					const char * name,
					const char * text);
	static const char * epg_header_footer_callback(void * context, bool header, bool channel);
	static const char * epg_event_callback(void * context,
					const char * channel_name,
					uint16_t chan_major,
					uint16_t chan_minor,
					//
					uint16_t event_id,
					time_t start_time,
					uint32_t length_sec,
					const char * name,
					const char * text);

};

#endif /*__SERVE_H__ */
