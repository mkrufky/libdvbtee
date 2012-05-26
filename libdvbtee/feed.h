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

#if 0
if ((open_file("filename")) || (open_file("newfilename"))) {
	start();
	...
	stop();
	close_file();
}
#endif

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

	void close_file();

	parse parser;

	inline bool wait_for_streaming_or_timeout(unsigned int timeout)
		{ time_t start_time = time(NULL); while ((!f_kill_thread) && ((timeout == 0) || (time(NULL) - start_time) < ((int)timeout) )) usleep(200*1000); return f_kill_thread; }

	inline bool wait_for_psip(unsigned int time_ms)
		{ time_t start_time = time(NULL); while ((!parser.is_psip_ready()) && ( (time(NULL) - start_time) < ((int)time_ms / 1000) )) usleep(200*1000); return parser.is_psip_ready(); }

	inline bool wait_for_epg(unsigned int time_ms)
		{ time_t start_time = time(NULL); while ((!parser.is_epg_ready()) && ( (time(NULL) - start_time) < ((int)time_ms / 1000) )) usleep(200*1000); return parser.is_epg_ready(); }

private:
	pthread_t h_thread;
	bool f_kill_thread;

	char filename[256];
	int fd;

	void *feed_thread();
	void *stdin_feed_thread();
	static void *feed_thread(void*);
	static void *stdin_feed_thread(void*);

	void set_filename(char*);
	int  open_file();
};

#endif /*__FEED_H__ */
