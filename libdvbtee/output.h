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

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <map>
#include <string>

#include "listen.h"
#include "rbuf.h"

#define TUNER_RESOURCE_SHARING 0

#if 1 // moved from parse.h
typedef std::map<uint16_t, uint16_t> map_pidtype;
#endif
ssize_t socket_send(int sockfd, const void *buf, size_t len, int flags,
		    const struct sockaddr *dest_addr = NULL, socklen_t addrlen = 0);

int stream_http_chunk(int socket, const uint8_t *buf, size_t length, const bool send_zero_length = false);

enum output_options {
	OUTPUT_NONE    = 0,
	OUTPUT_PATPMT  = 1,
	OUTPUT_PES     = 2,
	OUTPUT_PSIP    = 4,
};

enum output_mimetype {
	MIMETYPE_NONE,
	MIMETYPE_OCTET_STREAM,
	MIMETYPE_TEXT_PLAIN,
	MIMETYPE_TEXT_HTML,
};

const std::string http_response(enum output_mimetype mimetype);

#define OUTPUT_AV (OUTPUT_PATPMT | OUTPUT_PES)

#define OUTPUT_STREAM_BUF_SIZE 188*7*198

typedef int (*stream_callback)(void *, const uint8_t *, size_t);

class output_stream_iface
{
public:
	virtual int stream(const uint8_t *, size_t) = 0;
};

class output_stream
{
public:
	output_stream();
	~output_stream();
#if 1
	output_stream(const output_stream&);
	output_stream& operator= (const output_stream&);
#endif
	bool is_streaming() { return ((!f_kill_thread) && (f_streaming)); }
	void stop_without_wait() { f_kill_thread = true; }

	int start();
	bool drain();
	void stop();
	inline void stop_after_drain() { if (drain()) stop(); }
	int change_file(char*);
	void close_file();

	bool push(uint8_t*, int);

	int add(char*, map_pidtype&);
	int add(int, unsigned int, map_pidtype&);
	int add(void*, stream_callback, map_pidtype&);
	int add(output_stream_iface *iface, map_pidtype &pids);
	int add_stdout(map_pidtype &);

	bool check();

	int get_pids(map_pidtype&);
	void reset_pids() { pids.clear(); }

	bool verify(void* priv, stream_callback callback) { return ((priv == stream_cb_priv) && (callback == stream_cb)); }
	bool verify(output_stream_iface *iface) { return (m_iface == iface); }
	bool verify(int socket, unsigned int method) { return ((socket == sock) && (method == stream_method)); }
	bool verify(char* target) { return (strcmp(target, name) == 0); }

private:
	pthread_t h_thread;
	bool f_kill_thread;
	bool f_streaming;
	int sock;
	enum output_mimetype mimetype;

	char name[21];
	unsigned int name_index;

	rbuf ringbuffer;

	void *output_stream_thread();
	static void *output_stream_thread(void*);

	struct sockaddr_in  ip_addr;

	int stream(uint8_t*, int);
#define OUTPUT_STREAM_UDP    0
#define OUTPUT_STREAM_TCP    1
#define OUTPUT_STREAM_FILE   2
#define OUTPUT_STREAM_FUNC   3
#define OUTPUT_STREAM_HTTP   4
#define OUTPUT_STREAM_STDOUT 5
#define OUTPUT_STREAM_INTF   6
	unsigned int stream_method;

	unsigned long int count_in, count_out;

	output_stream_iface *m_iface;
	stream_callback stream_cb;
	void *stream_cb_priv;

	map_pidtype pids;

	int set_pids(map_pidtype&);

	bool have_pat;
#if TUNER_RESOURCE_SHARING
	uint8_t pat_pkt[188];

	bool want_pid(uint16_t pid) { return ((!pids.size()) || (pids.count(pid))) ? true : false; }
	bool want_pkt(uint8_t *p) { return ((p) && (want_pid(((p[1] & 0x1f) << 8) | p[2]))); }
#else
	bool want_pid(uint16_t pid) { (void)pid; return true; }
	bool want_pkt(uint8_t *p) { (void)p; return true; }
#endif
};

typedef std::map<unsigned int, output_stream> output_stream_map;

class output : public socket_listen_iface
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
	void stop(int);

	bool push(uint8_t* p_data, int size);
	bool push(uint8_t* p_data, enum output_options opt = OUTPUT_NONE);

	int add(char* target) { map_pidtype pids; return add(target, pids); }
	int add(int socket, unsigned int method) { map_pidtype pids; return add(socket, method, pids); }
	int add(void* priv, stream_callback callback) { map_pidtype pids; return add(priv, callback, pids); }
	int add(output_stream_iface *iface) { map_pidtype pids; return add(iface, pids); }
	int add_stdout() { map_pidtype pids; return add_stdout(pids); }

	int add(char* target, map_pidtype &pids);
	int add(int socket, unsigned int method, map_pidtype &pids);
	int add(void* priv, stream_callback callback, map_pidtype &pids);
	int add(output_stream_iface *iface, map_pidtype &pids);
	int add_stdout(map_pidtype &pids);

	int add_http_server(int);

	void set_options(enum output_options opt = OUTPUT_NONE) { options = opt; }

	bool check();

	int get_pids(map_pidtype&);
	void reset_pids(int target_id);

	void accept_socket(int sock) { add_http_client(sock); }
private:
	output_stream_map output_streams;

	pthread_t h_thread;
	bool f_kill_thread;
	bool f_streaming;

	rbuf ringbuffer;

	void *output_thread();
	static void *output_thread(void*);

	void add_http_client(int);

	void stop_without_wait() { f_kill_thread = true; }

	void reclaim_resources();

	int __add(char* target, map_pidtype &pids);

	unsigned int num_targets;

	enum output_options options;

	unsigned long int count_in, count_out;

	socket_listen listener;

	int search(void* priv, stream_callback callback);
	int search(output_stream_iface *iface);
	int search(int socket, unsigned int method);
	int search(char* target);
};

#endif /*__OUTPUT_H__ */
