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

#ifndef __FEED_H__
#define __FEED_H__

#include <pthread.h>
#include <unistd.h>
#include "parse.h"
#include "rbuf.h"

void libdvbtee_set_debug_level(unsigned int debug);

class feed
{
public:
	feed();
	~feed();

	int open_file(char* new_file) { set_filename(new_file); return open_file(); };
	int open_file(int new_fd) { fd = new_fd; return fd; }; /* assumes already open */

	void stop_without_wait() { f_kill_thread = true; };
	void stop();
	int start();
	int start_stdin();
	int start_tcp_listener(uint16_t);

	void close_file();

	parse parser;

#define FEED_EVENT_PSIP 1
#define FEED_EVENT_EPG  2
	bool wait_for_event_or_timeout(unsigned int timeout, unsigned int wait_event);

	inline bool wait_for_streaming_or_timeout(unsigned int timeout) { return wait_for_event_or_timeout(timeout, 0); }

	inline bool wait_for_psip(unsigned int time_ms) { return wait_for_event_or_timeout(time_ms / 1000, FEED_EVENT_PSIP); }

	inline bool wait_for_epg(unsigned int time_ms) { return wait_for_event_or_timeout(time_ms / 1000, FEED_EVENT_EPG); }

private:
	pthread_t h_thread;
	pthread_t h_feed_thread;
	bool f_kill_thread;

	char filename[256];
	int fd;

	rbuf ringbuffer;

	void            *feed_thread();
	void       *file_feed_thread();
	void      *stdin_feed_thread();
	void *tcp_listen_feed_thread();
	static void            *feed_thread(void*);
	static void       *file_feed_thread(void*);
	static void      *stdin_feed_thread(void*);
	static void *tcp_listen_feed_thread(void*);

	void set_filename(char*);
	int  open_file();
	int start_feed();
};

#endif /*__FEED_H__ */
