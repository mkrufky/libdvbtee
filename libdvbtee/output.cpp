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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

#include "output.h"
#include "log.h"
#define CLASS_MODULE "out"

#define dprintf(fmt, arg...) __dprintf(DBG_OUTPUT, fmt, ##arg)

#define DOUBLE_BUFFER 0
#define PREVENT_RBUF_DEADLOCK 0
#define NON_BLOCKING_TCP_SEND 1

#define HTTP_200_OK  "HTTP/1.1 200 OK"
#define CONTENT_TYPE "Content-type: "
#define TEXT_HTML    "text/html"
#define TEXT_PLAIN   "text/plain"
#define OCTET_STREAM "application/octet-stream"
#define ENC_CHUNKED  "Transfer-Encoding: chunked"
#define CONN_CLOSE   "Connection: close"
#define CRLF         "\r\n"

#if 0
static char http_conn_close[] =
	CONN_CLOSE
	CRLF
	CRLF;
#endif

static const char * __http_response(const char *mimetype)
{
	std::string str;
	str.clear();

	str.append(HTTP_200_OK
		   CRLF);
	if (mimetype) {
		str.append(CONTENT_TYPE);
		str.append(mimetype);
		str.append(CRLF
			   ENC_CHUNKED
			   CRLF);
	}
	str.append(CRLF);

	return str.c_str();
}

const char * http_response(enum output_mimetype mimetype)
{
	const char *str;

	switch (mimetype) {
	default:
	case MIMETYPE_OCTET_STREAM:
		str = OCTET_STREAM;
		break;
	case MIMETYPE_TEXT_PLAIN:
		str = TEXT_PLAIN;
		break;
	case MIMETYPE_TEXT_HTML:
		str = TEXT_HTML;
		break;
	case MIMETYPE_NONE:
		str = NULL;
		break;
	}

	return __http_response(str);
}

ssize_t socket_send(int sockfd, const void *buf, size_t len, int flags,
		    const struct sockaddr *dest_addr, socklen_t addrlen)
{
	if (sockfd < 0)
		return sockfd;
	int ret = 0;
	while (0 >= ret) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);

		struct timeval sel_timeout = {0, 10000};

		ret = select(sockfd + 1, NULL, &fds, NULL, &sel_timeout);
		if (ret == -1) {
			perror("error sending data to socket!");
			/*
			switch (errno) {
			case EBADF:  // invalid file descriptor
			case EINTR:  // signal caught
			case EINVAL: // negative fd or invalid timeout
			case ENOMEM: // unable to allocate memory
			}
			*/
			break;
		}
	}
	return (ret > 0) ?
#if 0
		((dest_addr) && (addrlen)) ?
			sendto(sockfd, buf, len, flags, dest_addr, addrlen) :
			send(sockfd, buf, len, flags) :
#else
		sendto(sockfd, buf, len, flags|MSG_NOSIGNAL, dest_addr, addrlen) :
#endif
		ret;
}

static inline ssize_t stream_crlf(int socket)
{
	return socket_send(socket, CRLF, 2, 0);
}

int stream_http_chunk(int socket, const uint8_t *buf, size_t length, const bool send_zero_length)
{
	if (socket < 0)
		return socket;
#if DBG
	dprintf("(length:%d)", (int)length);
#endif
	if ((length) || (send_zero_length)) {
		int ret = 0;
		char sz[7] = { 0 };
		sprintf(sz, "%x\r\n", (unsigned int)length);

		ret = socket_send(socket, sz, strlen(sz), 0);
		if (ret < 0)
			return ret;
#if 0
		ret = stream_crlf(socket);
		if (ret < 0)
			return ret;
#endif
		if (length) {
			ret = socket_send(socket, buf, length, 0);
			if (ret < 0)
				return ret;

			ret = stream_crlf(socket);
			if (ret < 0)
				return ret;
		}
	}
	return 0;
}


output_stream::output_stream()
  : f_kill_thread(false)
  , f_streaming(false)
  , sock(-1)
  , mimetype(MIMETYPE_OCTET_STREAM)
  , ringbuffer()
  , stream_method(OUTPUT_STREAM_UDP)
  , count_in(0)
  , count_out(0)
  , stream_cb(NULL)
  , stream_cb_priv(NULL)
{
	dprintf("()");
	memset(&ringbuffer, 0, sizeof(ringbuffer));
	memset(&name, 0, sizeof(name));
}

output_stream::~output_stream()
{
	dprintf("(%d)", sock);

	stop();

	dprintf("(stream) %lu packets in, %lu packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
}

#if 1
output_stream::output_stream(const output_stream&)
{
	dprintf("(copy)");
	memset(&ringbuffer, 0, sizeof(ringbuffer));
	f_kill_thread = false;
	f_streaming = false;
	stream_cb = NULL;
	stream_cb_priv = NULL;
	count_in = 0;
	count_out = 0;
	sock = -1;
	mimetype = MIMETYPE_OCTET_STREAM;
	memset(&name, 0, sizeof(name));
}

output_stream& output_stream::operator= (const output_stream& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	memset(&ringbuffer, 0, sizeof(ringbuffer));
	f_kill_thread = false;
	f_streaming = false;
	stream_cb = NULL;
	stream_cb_priv = NULL;
	count_in = 0;
	count_out = 0;
	sock = -1;
	mimetype = MIMETYPE_OCTET_STREAM;
	memset(&name, 0, sizeof(name));

	return *this;
}
#endif

//static
void* output_stream::output_stream_thread(void *p_this)
{
	return static_cast<output_stream*>(p_this)->output_stream_thread();
}

#define OUTPUT_STREAM_PACKET_SIZE ((stream_method == OUTPUT_STREAM_UDP) ? 188*7 : 188*21)
void* output_stream::output_stream_thread()
{
	uint8_t *data = NULL;
	int buf_size, ret = 0;

	dprintf("(%d)", sock);
#if 1
	switch (stream_method) {
	case OUTPUT_STREAM_HTTP:
		const char *str = http_response(mimetype);
		ret = socket_send(sock, str, strlen(str), 0);
		break;
	}
	if (ret < 0) {
		perror("stream header failed");
		goto fail;
	}

#endif
	f_streaming = true;

	/* push data from output_stream buffer to target */
	while (!f_kill_thread) {

		buf_size = ringbuffer.get_size();
		if (buf_size < OUTPUT_STREAM_PACKET_SIZE) {
			usleep(1000);
			continue;
		}

		buf_size = ringbuffer.get_read_ptr((void**)&data, OUTPUT_STREAM_PACKET_SIZE);
		buf_size /= 188;
		buf_size *= 188;

#if PREVENT_RBUF_DEADLOCK
		{
			uint8_t newdata[buf_size];
			memcpy(newdata, data, buf_size);
			ringbuffer.put_read_ptr(buf_size);
			stream(newdata, buf_size);
		}
#else
		stream(data, buf_size);

		ringbuffer.put_read_ptr(buf_size);
#endif
		count_out += buf_size;
#if 0
		dprintf("(thread-stream) %d packets in, %d packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
#endif
	}

	f_streaming = false;
#if 0
	switch (stream_method) {
	case OUTPUT_STREAM_HTTP:
		if (stream_http_chunk(sock, (uint8_t *)"", 0, true) < 0)
			perror("stream empty http chunk failed");
		else if (socket_send(sock, http_conn_close, strlen(http_conn_close), 0) < 0)
			perror("stream http connection close failed");
		break;
	}
#endif
fail:
	close_file();
	pthread_exit(NULL);
}

int output_stream::start()
{
	if (f_streaming) {
		dprintf("(%d) already streaming", sock);
		return 0;
	}
	if ((sock < 0) && (stream_method != OUTPUT_STREAM_FUNC))
		return sock;

	dprintf("(%d)", sock);

	ringbuffer.set_capacity(OUTPUT_STREAM_BUF_SIZE);

	int ret = pthread_create(&h_thread, NULL, output_stream_thread, this);
	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

bool output_stream::drain()
{
	dprintf("(%d)", sock);

	if (!f_streaming)
		return false;

	while ((f_streaming) && (ringbuffer.get_capacity()))
		usleep(20*1000);

	fsync(sock);

	return (!f_streaming);
}

void output_stream::stop()
{
	dprintf("(%d)", sock);

	stop_without_wait();

	while (f_streaming)
		usleep(20*1000);

	return;
}

bool output_stream::check()
{
	bool ret = f_streaming;
	if (!ret)
		dprintf("(%d: %s) not streaming!", sock, name);
	else {
		dprintf("(%d: %s) %s %lu in, %lu out",
			sock, name,
			(stream_method == OUTPUT_STREAM_UDP) ? "UDP" :
			(stream_method == OUTPUT_STREAM_TCP) ? "TCP" :
			(stream_method == OUTPUT_STREAM_HTTP) ? "HTTP" :
			(stream_method == OUTPUT_STREAM_FILE) ? "FILE" :
			(stream_method == OUTPUT_STREAM_FUNC) ? "FUNC" : "UNKNOWN",
			count_in / 188, count_out / 188);
	}
	ringbuffer.check();

	return ret;
}

bool output_stream::push(uint8_t* p_data, int size)
{
	/* push data into output_stream buffer */
	if (!ringbuffer.write(p_data, size))
		while (size >= 188)
			if (ringbuffer.write(p_data, 188)) {
				p_data += 188;
				size -= 188;
				count_in += 188;
			} else {
				fprintf(stderr, "%s> FAILED: %d bytes dropped\n", __func__, size);
#if 0
				dprintf("(push-false-stream) %d packets in, %d packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
#endif
				return false;
			}
	else count_in += size;
#if 0
	dprintf("(push-true-stream) %d packets in, %d packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
#endif
	return true;
}

int output_stream::stream(uint8_t* p_data, int size)
{
	int ret = -1;

	if ((!p_data) || (!size))
		dprintf("no data to stream!!!");
	/* stream data to target */
	else switch (stream_method) {
	case OUTPUT_STREAM_UDP:
		ret = socket_send(sock, p_data, size, 0, (struct sockaddr*) &ip_addr, sizeof(ip_addr));
		break;
	case OUTPUT_STREAM_TCP:
		ret = socket_send(sock, p_data, size, 0);
		if (ret < 0) {
			stop_without_wait();
			perror("tcp streaming failed");
		}
		break;
	case OUTPUT_STREAM_FILE:
		ret = write(sock, p_data, size);
		if (ret < 0) {
			stop_without_wait();
			perror("file streaming failed");
		}
		break;
	case OUTPUT_STREAM_HTTP:
		ret = stream_http_chunk(sock, p_data, size);
		if (ret < 0) {
			stop_without_wait();
			perror("http streaming failed");
		}
		break;
	case OUTPUT_STREAM_FUNC:
		if (stream_cb)
			ret = stream_cb(stream_cb_priv, p_data, size);
		if (ret < 0) {
			stop_without_wait();
			perror("streaming via callback failed");
		}
		break;
	}
	return ret;
}

void output_stream::close_file()
{
	dprintf("(%d, %s)", sock, name);

	if (sock >= 0) {
		close(sock);
		sock = -1;
	}
}

int output_stream::add(void* priv, stream_callback callback)
{
	stream_cb = callback;
	stream_cb_priv = priv;

	ringbuffer.reset();
	stream_method = OUTPUT_STREAM_FUNC;
	strcpy(name, "FUNC");
	return 0;
}

int output_stream::add(int socket, unsigned int method)
{
	sock = socket;
	stream_method = method;
	strcpy(name, "SOCKET");

#if NON_BLOCKING_TCP_SEND
	int fl = fcntl(sock, F_GETFL, 0);
	if (fcntl(socket, F_SETFL, fl | O_NONBLOCK) < 0)
		perror("set non-blocking failed");
#endif
	ringbuffer.reset();
	return 0;
}

int output_stream::add(char* target)
{
	char *save;
	char *ip;
	uint16_t port = 0;
	bool b_tcp = false;
	bool b_udp = false;
	bool b_file = false;

	dprintf("(-->%s)", target);

	strcpy(name, target);

	if (strstr(target, ":")) {
		ip = strtok_r(target, ":", &save);
		if (strstr(ip, "tcp"))
			b_tcp = true;
		else
		if (strstr(ip, "udp"))
			b_udp = true;
		else
		if (strstr(ip, "file"))
			b_file = true;

		if ((b_tcp) || (b_udp) || (b_file)) {
			ip = strtok_r(NULL, ":", &save);
			if (strstr(ip, "//") == ip)
				ip += 2;
		}
		// else ip = proto;
		if (!b_file)
			port = atoi(strtok_r(NULL, ":", &save));
	} else {
		b_tcp = false;
		ip = target;
		port = 1234;
	}

	if (sock >= 0)
		close(sock);

	if (b_file) {
		dprintf("opening %s...", ip);
		if ((sock = open(ip, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU)) < 0) {
			perror("file failed");
			return -1;
		} else {
			ringbuffer.reset();
			stream_method = OUTPUT_STREAM_FILE;
			return 0;
		}
	}

	sock = socket(AF_INET, (b_tcp) ? SOCK_STREAM : SOCK_DGRAM, (b_tcp) ? IPPROTO_TCP : IPPROTO_UDP);
	if (sock >= 0) {

		int fl = fcntl(sock, F_GETFL, 0);
		if (fcntl(sock, F_SETFL, fl | O_NONBLOCK) < 0)
			perror("set non-blocking failed");

		memset(&ip_addr, 0, sizeof(ip_addr));
		ip_addr.sin_family = AF_INET;
		ip_addr.sin_port   = htons(port);
		if (inet_aton(ip, &ip_addr.sin_addr) == 0) {

			perror("ip address translation failed");
			return -1;
		} else
			ringbuffer.reset();

		if (b_tcp) {
			if ((connect(sock, (struct sockaddr *) &ip_addr, sizeof(ip_addr)) < 0) && (errno != EINPROGRESS)) {
				perror("failed to connect to server");
				return -1;
			}
			stream_method = OUTPUT_STREAM_TCP;
		} else {
			stream_method = OUTPUT_STREAM_UDP;
		}
	} else {
		perror("socket failed");
		return -1;
	}
	dprintf("~(-->%s)", target);

	return 0;
}

/* ----------------------------------------------------------------- */

output::output()
  : f_kill_thread(false)
  , f_streaming(false)
  , ringbuffer()
  , num_targets(0)
  , options(OUTPUT_NONE)
  , count_in(0)
  , count_out(0)
{
	dprintf("()");

	memset(&output_streams, 0, sizeof(output_streams));

	output_streams.clear();
}

output::~output()
{
	dprintf("()");

	stop();

	output_streams.clear();

	dprintf("(intermediate) %lu packets in, %lu packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
}

#if 1
output::output(const output&)
{
	dprintf("(copy)");

	f_kill_thread = false;
	f_streaming = false;
	num_targets = 0;
	options = OUTPUT_NONE;
	count_in = 0;
	count_out = 0;

	memset(&ringbuffer, 0, sizeof(ringbuffer));
	memset(&output_streams, 0, sizeof(output_streams));

	output_streams.clear();
}

output& output::operator= (const output& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	f_kill_thread = false;
	f_streaming = false;
	num_targets = 0;
	options = OUTPUT_NONE;
	count_in = 0;
	count_out = 0;

	memset(&ringbuffer, 0, sizeof(ringbuffer));
	memset(&output_streams, 0, sizeof(output_streams));

	output_streams.clear();

	return *this;
}
#endif

//static
void* output::output_thread(void *p_this)
{
	return static_cast<output*>(p_this)->output_thread();
}

void* output::output_thread()
{
	int buf_size;
#if !PREVENT_RBUF_DEADLOCK
	uint8_t *data = NULL;
#endif

	f_streaming = true;

	/* push data from main output buffer into output_stream buffers */
	while (!f_kill_thread) {

		buf_size = ringbuffer.get_size();
		//data = NULL;
		if (buf_size >= 188) {
#if !PREVENT_RBUF_DEADLOCK
			buf_size = ringbuffer.get_read_ptr((void**)&data, buf_size);
			buf_size /= 188;
			buf_size *= 188;
#else
			uint8_t data[buf_size];
			buf_size = ringbuffer.read(data, buf_size);
#endif
			if (buf_size)
			for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter) {
				if (iter->second.is_streaming())
					iter->second.push(data, buf_size);
#if 0
				else {
					dprintf("erasing idle output stream...");
					output_streams.erase(iter->first);
					dprintf("garbage collection complete");
				}
#endif
			}
#if !PREVENT_RBUF_DEADLOCK
			ringbuffer.put_read_ptr(buf_size);
#endif
			count_out += buf_size;
		} else {
			usleep(1000);
		}

	}
	f_streaming = false;
	pthread_exit(NULL);
}

//static
void output::add_http_client(void *p_this, int socket)
{
	return static_cast<output*>(p_this)->add_http_client(socket);
}

void output::add_http_client(int socket)
{
	add(socket, OUTPUT_STREAM_HTTP);
	start();
	return;
}

int output::add_http_server(int port)
{
	dprintf("(%d)", port);
	listener.set_callback(this, add_http_client);
	return listener.start(port);
}

bool output::check()
{
	dprintf("()");
	unsigned int dead = 0;
	bool ret = false;

	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter) {
		bool streaming = iter->second.check();
		ret |= streaming;
		if (!streaming)
			dead++;
	}
	if (dead) {
		dprintf("%d dead streams found", dead);
		reclaim_resources();
	}

	ringbuffer.check();

	return ret;
}

void output::reclaim_resources()
{
	dprintf("()");
	bool erased = false;

	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter) {
		if (!iter->second.check()) {
			dprintf("erasing idle output stream...");
			output_streams.erase(iter->first);
			/* stop the loop if we erased any targets */
			erased = true;
			break;
		}
	}
	/* if we erased a target, restart the above by re-calling this function recursively */
	if (erased)
		reclaim_resources();
}

int output::start()
{
	dprintf("()");

#if DOUBLE_BUFFER
	int ret = 0;

	if (output_streams.size() <= 1)
		goto nobuffer;

	if (f_streaming) {
		dprintf("() already streaming!");
		goto nobuffer;
	}
	ret = pthread_create(&h_thread, NULL, output_thread, this);

	if (0 != ret) {
		perror("pthread_create() failed");
		goto fail;
	}

	/* allocates out buffer if and only if we have begun streaming output */
	if (ringbuffer.get_capacity() <= 0)
		ringbuffer.set_capacity(OUTPUT_STREAM_BUF_SIZE*2);
nobuffer:
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.start();
fail:
	return ret;
#else
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.start();
	return 0;
#endif
}

void output::stop()
{
	dprintf("()");

	stop_without_wait();

	/* call stop_without_wait() on everybody first before we call stop() on everybody, which is a blocking function */
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.stop_without_wait();

	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.stop();

	while (f_streaming)
		usleep(20*1000);

	return;
}

bool output::push(uint8_t* p_data, int size)
{
	bool ret = true;

	if (ringbuffer.get_capacity()) {

		/* push data into output buffer */
		ret = ringbuffer.write(p_data, size);
		if (!ret)
			fprintf(stderr, "%s: FAILED: %d bytes dropped\n", __func__, size);
	} else for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter) {
		if (iter->second.is_streaming())
			iter->second.push(p_data, size);
#if 0
		else {
			dprintf("erasing idle output stream...");
			output_streams.erase(iter->first);
			dprintf("garbage collection complete");
		}
#endif
	}
	count_in += size;

	return ret;
}

bool output::push(uint8_t* p_data, enum output_options opt)
{
	return (((!options) || (!opt)) || (opt & options)) ? push(p_data, 188) : false;
}

int output::add(void* priv, stream_callback callback)
{
	if ((callback) && (priv)) {

		/* push data into output buffer */
		int ret = output_streams[num_targets].add(priv, callback);
		if (ret == 0)
			num_targets++;
		else
			dprintf("failed to add target #%d", num_targets);

		dprintf("~(%d->FUNC)", (ret == 0) ? num_targets - 1 : num_targets);

		return ret;
	}
	return -1;
}

int output::add(int socket, unsigned int method)
{
	if (socket >= 0) {

		/* push data into output buffer */
		int ret = output_streams[num_targets].add(socket, method);
		if (ret == 0)
			num_targets++;
		else
			dprintf("failed to add target #%d", num_targets);

		dprintf("~(%d->SOCKET[%d])", (ret == 0) ? num_targets - 1 : num_targets, socket);

		return ret;
	}
	return -1;
}

#define CHAR_CMD_COMMA ","

int output::add(char* target)
{
	char *save;
	int ret = -1;
	char *item = strtok_r(target, CHAR_CMD_COMMA, &save);
	if (item) while (item) {
		if (!item)
			item = target;

		ret = __add(item);
		if (ret < 0)
			return ret;

		item = strtok_r(NULL, CHAR_CMD_COMMA, &save);
	} else
		ret = __add(target);

	return ret;
}

int output::__add(char* target)
{
	dprintf("(%d->%s)", num_targets, target);

	/* push data into output buffer */
	int ret = output_streams[num_targets].add(target);
	if (ret == 0)
		num_targets++;
	else
		dprintf("failed to add target #%d: %s", num_targets, target);

	dprintf("~(%d->%s)", (ret == 0) ? num_targets - 1 : num_targets, target);

	return ret;
}
