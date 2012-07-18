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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "output.h"
#include "log.h"
#define CLASS_MODULE "out"

#define dprintf(fmt, arg...) __dprintf(DBG_OUTPUT, fmt, ##arg)

output_stream::output_stream()
  : f_kill_thread(false)
  , f_streaming(false)
  , sock(-1)
  , ringbuffer()
  , stream_method(OUTPUT_STREAM_UDP)
  , count_in(0)
  , count_out(0)
{
	dprintf("()");
	memset(&ringbuffer, 0, sizeof(ringbuffer));
	ringbuffer.set_capacity(OUTPUT_STREAM_BUF_SIZE);
}

output_stream::~output_stream()
{
	dprintf("()");

	stop();

	dprintf("(stream) %d packets in, %d packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
}

#if 1
output_stream::output_stream(const output_stream&)
{
	dprintf("(copy)");
	memset(&ringbuffer, 0, sizeof(ringbuffer));
	ringbuffer.set_capacity(OUTPUT_STREAM_BUF_SIZE);
	f_kill_thread = false;
	f_streaming = false;
	sock = -1;
}

output_stream& output_stream::operator= (const output_stream& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	memset(&ringbuffer, 0, sizeof(ringbuffer));
	ringbuffer.set_capacity(OUTPUT_STREAM_BUF_SIZE);
	f_kill_thread = false;
	f_streaming = false;
	sock = -1;

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
	int buf_size;

	dprintf("()");

	f_streaming = true;

	/* push data from output_stream buffer to target */
	while (!f_kill_thread) {

		buf_size = ringbuffer.get_size();
		if (buf_size < OUTPUT_STREAM_PACKET_SIZE) {
			usleep(50*1000);
			continue;
		}

		buf_size = ringbuffer.get_read_ptr((void**)&data, OUTPUT_STREAM_PACKET_SIZE);
		buf_size /= 188;
		buf_size *= 188;
		stream(data, buf_size);
		ringbuffer.put_read_ptr(buf_size);
		count_out += buf_size;
	}
	f_streaming = false;
	pthread_exit(NULL);
}

int output_stream::start()
{
	dprintf("()");

	int ret = pthread_create(&h_thread, NULL, output_stream_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

void output_stream::stop()
{
	dprintf("(%d)", sock);

	stop_without_wait();

	while (f_streaming)
		usleep(20*1000);

	return;
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
				return false;
			}
	else count_in += size;

	return true;
}

int output_stream::stream(uint8_t* p_data, int size)
{
	int ret = -1;

	/* stream data to target */
	switch (stream_method) {
	case OUTPUT_STREAM_UDP:
		ret = sendto(sock, p_data, size, 0, (struct sockaddr*) &ip_addr, sizeof(ip_addr));
		break;
	case OUTPUT_STREAM_TCP:
		while (0 >= ret) {
		  fd_set fds;
		  FD_ZERO(&fds);
		  FD_SET(sock, &fds);

		  struct timeval sel_timeout = {0, 10000};

		  ret = select(sock + 1, NULL, &fds, NULL, &sel_timeout);
		  if (ret < 0)
		    perror("error!");
		  usleep(20*1000);
		}
		ret = send(sock, p_data, size, 0);
		break;
	case OUTPUT_STREAM_FILE:
		ret = write(sock, p_data, size);
		break;
	}
	return ret;
}

int output_stream::add(char* target)
{
	char *ip;
	uint16_t port = 0;
	bool b_tcp = false;
	bool b_udp = false;
	bool b_file = false;

	dprintf("(-->%s)", target);

	if (strstr(target, ":")) {
		ip = strtok(target, ":");
		if (strstr(ip, "tcp"))
			b_tcp = true;
		else
		if (strstr(ip, "udp"))
			b_udp = true;
		else
		if (strstr(ip, "file"))
			b_file = true;

		if ((b_tcp) || (b_udp) || (b_file)) {
			ip = strtok(NULL, ":");
			if (strstr(ip, "//") == ip)
				ip += 2;
		}
		// else ip = proto;
		if (!b_file)
			port = atoi(strtok(NULL, ":"));
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

	dprintf("(intermediate) %d packets in, %d packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
}

#if 1
output::output(const output&)
{
	dprintf("(copy)");

	memset(&output_streams, 0, sizeof(output_streams));

	output_streams.clear();
}

output& output::operator= (const output& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

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
	uint8_t *data = NULL;

	f_streaming = true;

	/* push data from main output buffer into output_stream buffers */
	while (!f_kill_thread) {

		buf_size = ringbuffer.get_size();
		//data = NULL;
		if (buf_size >= 188) {
			buf_size = ringbuffer.get_read_ptr((void**)&data, buf_size);
			buf_size /= 188;
			buf_size *= 188;

			for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
				iter->second.push(data, buf_size);
			ringbuffer.put_read_ptr(buf_size);
			count_out += buf_size;
		} else {
			usleep(50*1000);
		}

	}
	f_streaming = false;
	pthread_exit(NULL);
}

int output::start()
{
	dprintf("()");

	int ret = pthread_create(&h_thread, NULL, output_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.start();

	return ret;
}

void output::stop()
{
	dprintf("()");

	/* call stop_without_wait() on everybody first before we call stop() on everybody, which is a blocking function */
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.stop_without_wait();

	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.stop();

	stop_without_wait();

	while (f_streaming)
		usleep(20*1000);

	return;
}

bool output::push(uint8_t* p_data, int size)
{
	if (!ringbuffer.get_capacity())
		return false;
	/* push data into output buffer */
	bool ret = ringbuffer.write(p_data, size);
	if (!ret)
		fprintf(stderr, "%s: FAILED: %d bytes dropped\n", __func__, size);

	count_in += size;

	return ret;
}

bool output::push(uint8_t* p_data, enum output_options opt)
{
	return (((!options) || (!opt)) || (opt & options)) ? push(p_data, 188) : false;
}

int output::add(char* target)
{
	dprintf("(%d->%s)", num_targets, target);

	/* allocates out buffer if and only if we have at least one target */
	if (ringbuffer.get_capacity() <= 0)
		ringbuffer.set_capacity(OUTPUT_STREAM_BUF_SIZE*2);

	/* push data into output buffer */
	int ret = output_streams[num_targets].add(target);
	if (ret == 0)
		num_targets++;
	else
		dprintf("failed to add target #%d: %s", num_targets, target);

	dprintf("~(%d->%s)", (ret == 0) ? num_targets - 1 : num_targets, target);

	return ret;
}
