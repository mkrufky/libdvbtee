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

#include "output.h"
#include "log.h"

#define dprintf(fmt, arg...) __dprintf(DBG_OUTPUT, fmt, ##arg)

output_stream::output_stream()
  : f_kill_thread(false)
  , sock(-1)
  , ringbuffer()
{
	dprintf("()");
	memset(&ringbuffer, 0, sizeof(ringbuffer));
	ringbuffer.setCapacity(188*7*199+1);
}

output_stream::~output_stream()
{
	dprintf("()");

	stop();
}

#if 1
output_stream::output_stream(const output_stream&)
{
	dprintf("(copy)");
	memset(&ringbuffer, 0, sizeof(ringbuffer));
	ringbuffer.setCapacity(188*7*199+1);
}

output_stream& output_stream::operator= (const output_stream& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	memset(&ringbuffer, 0, sizeof(ringbuffer));
	ringbuffer.setCapacity(188*7*199+1);

	return *this;
}
#endif

//static
void* output_stream::output_stream_thread(void *p_this)
{
	return static_cast<output_stream*>(p_this)->output_stream_thread();
}

#define OUTPUT_STREAM_PACKET_SIZE 188*7
void* output_stream::output_stream_thread()
{
	uint8_t data[OUTPUT_STREAM_PACKET_SIZE];

	dprintf("()");

	/* push data from output_stream buffer to target */
	while (!f_kill_thread) {

		int buf_size = ringbuffer.getSize();
		if (buf_size < OUTPUT_STREAM_PACKET_SIZE) {
			usleep(50*1000);
			continue;
		}
		ringbuffer.read(data, OUTPUT_STREAM_PACKET_SIZE);
		stream(data, OUTPUT_STREAM_PACKET_SIZE);
	}
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
	dprintf("()");

	stop_without_wait();
#if 0
	while (-1 != sock_fd) {
		usleep(20*1000);
	}
#endif
	return;
}

int output_stream::push(uint8_t* p_data, int size)
{
	/* push data into output_stream buffer */
	return ringbuffer.write(p_data, size);
}

int output_stream::stream(uint8_t* p_data, int size)
{
	/* stream data to target */
	return sendto(sock, p_data, size, 0, (struct sockaddr*) &udp_addr, sizeof(udp_addr));
}

int output_stream::add(char* target)
{
	char *ip;
	uint16_t port;

	dprintf("(-->%s)", target);

	if (strstr(target, ":")) {
		ip = strtok(target, ":");
		port = atoi(strtok(NULL, ":"));
	} else {
		ip = target;
		port = 1234;
	}

	if (sock >= 0)
		close(sock);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sock >= 0) {
		int fl = fcntl(sock, F_GETFL, 0);
		if (fcntl(sock, F_SETFL, fl | O_NONBLOCK) < 0)
			perror("set non-blocking failed");
		memset(&udp_addr, 0, sizeof(udp_addr));
		udp_addr.sin_family = AF_INET;
		udp_addr.sin_port   = htons(port);
		if (inet_aton(ip, &udp_addr.sin_addr) == 0) {

			perror("udp ip address translation failed");
			return -1;
		} else
			ringbuffer.reset();
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
  , ringbuffer()
  , num_targets(0)
{
	dprintf("()");

	ringbuffer.setCapacity(188*7*199*2+1);

	memset(&output_streams, 0, sizeof(output_streams));

	output_streams.clear();
}

output::~output()
{
	dprintf("()");

	stop();

	output_streams.clear();
}

#if 0
output::output(const output&)
{
	dprintf("(copy)");

	output_streams.clear();
}

output& output::operator= (const output& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

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
	/* push data from main output buffer into output_stream buffers */
	while (!f_kill_thread) {

		int buf_size = ringbuffer.getSize();
		uint8_t p_data[buf_size];// = { 0 };
		if (buf_size) {
			ringbuffer.read(p_data, buf_size);

			for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
				iter->second.push(p_data, buf_size);
		}
	}
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
#if 0
	while (-1 != sock_fd) {
		usleep(20*1000);
	}
#endif
	return;
}

int output::push(uint8_t* p_data)
{
	/* push data into output buffer */
	return ringbuffer.write(p_data, 188);
}

int output::add(char* target)
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
