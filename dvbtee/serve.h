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

	bool   command(char*);
	bool __command(char*);

	int sock_fd;
	uint16_t port;

	tuner_map tuners;
};

#endif /*__SERVE_H__ */
