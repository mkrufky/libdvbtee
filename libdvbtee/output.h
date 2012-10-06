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

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <map>

#include "listen.h"
#include "rbuf.h"

ssize_t socket_send(int sockfd, const void *buf, size_t len, int flags,
		    const struct sockaddr *dest_addr = NULL, socklen_t addrlen = 0);

enum output_options {
	OUTPUT_NONE    = 0,
	OUTPUT_PATPMT  = 1,
	OUTPUT_PES     = 2,
	OUTPUT_PSIP    = 4,
};

#define OUTPUT_AV (OUTPUT_PATPMT | OUTPUT_PES)

#define OUTPUT_STREAM_BUF_SIZE 188*7*198

typedef int (*stream_callback)(void *, const uint8_t *, size_t);

class output_stream
{
public:
	output_stream();
	~output_stream();
#if 1
	output_stream(const output_stream&);
	output_stream& operator= (const output_stream&);
#endif
	bool is_streaming() { return f_streaming; };
	void stop_without_wait() { f_kill_thread = true; };

	int start();
	void stop();
	void close_file();

	bool push(uint8_t*, int);
	int add(char*);
	int add(int, unsigned int);
	int add(void*, stream_callback);

	bool check();
private:
	pthread_t h_thread;
	bool f_kill_thread;
	bool f_streaming;
	int sock;

	char name[21];

	rbuf ringbuffer;

	void *output_stream_thread();
	static void *output_stream_thread(void*);

	struct sockaddr_in  ip_addr;

	int stream(uint8_t*, int);
#define OUTPUT_STREAM_UDP  0
#define OUTPUT_STREAM_TCP  1
#define OUTPUT_STREAM_FILE 2
#define OUTPUT_STREAM_FUNC 3
#define OUTPUT_STREAM_HTTP 4
	unsigned int stream_method;

	unsigned int count_in, count_out;

	stream_callback stream_cb;
	void *stream_cb_priv;
};

typedef std::map<int, output_stream> output_stream_map;

class output
{
public:
	output();
	~output();
#if 1
	output(const output&);
	output& operator= (const output&);
#endif
	int start();
	void stop();

	bool push(uint8_t* p_data, int size);
	bool push(uint8_t* p_data, enum output_options opt = OUTPUT_NONE);
	int add(char*);
	int add(int, unsigned int);
	int add(void*, stream_callback);
	int add_http_server(int);

	void set_options(enum output_options opt = OUTPUT_NONE) { options = opt; }

	bool check();
private:
	output_stream_map output_streams;

	pthread_t h_thread;
	bool f_kill_thread;
	bool f_streaming;

	rbuf ringbuffer;

	void *output_thread();
	static void *output_thread(void*);

	void add_http_client(int);
	static void add_http_client(void*, int);

	void stop_without_wait() { f_kill_thread = true; };

	int __add(char*);

	unsigned int num_targets;

	enum output_options options;

	unsigned int count_in, count_out;

	socket_listen listener;
};

#endif /*__OUTPUT_H__ */
