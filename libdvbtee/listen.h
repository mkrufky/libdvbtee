/*****************************************************************************
 * Copyright (C) 2011-2013 Michael Krufky
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

#ifndef __LISTEN_H__
#define __LISTEN_H__

#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef void (*accept_socket_callback)(void *, int);

class socket_listen
{
public:
	socket_listen();
	~socket_listen();

	socket_listen(const socket_listen&);
	socket_listen& operator= (const socket_listen&);

	void set_callback(void *priv, accept_socket_callback cb) { accept_socket_data = priv; accept_socket_cb = cb; };

	int start(uint16_t port_requested);
	void stop();

	bool is_running() { return ((f_kill_thread == false) && (sock_fd >= 0) /* && (port) */ ); };
private:
	pthread_t h_thread;
	bool f_kill_thread;

	void *listen_thread();
	static void *listen_thread(void*);

	void stop_without_wait() { f_kill_thread = true; };
	void close_socket();

	int sock_fd;
	uint16_t port;

	accept_socket_callback accept_socket_cb;
	void *accept_socket_data;
};

#endif /* __LISTEN_H__ */
