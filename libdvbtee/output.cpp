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
{
	dprintf("()");
}

output_stream::~output_stream()
{
	dprintf("()");
}

output_stream::output_stream(const output_stream&)
{
	dprintf("(copy)");
}

output_stream& output_stream::operator= (const output_stream& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	return *this;
}

//static
void* output_stream::output_stream_thread(void *p_this)
{
	return static_cast<output_stream*>(p_this)->output_stream_thread();
}

void* output_stream::output_stream_thread()
{
	/* push data from output_stream buffer to target */
	while (!f_kill_thread) {
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

int output_stream::push(uint8_t* p_data)
{
	/* push data into output_stream buffer  */
	return 0;
}
/* ----------------------------------------------------------------- */

output::output()
  : f_kill_thread(false)
{
	dprintf("()");

	output_streams.clear();
}

output::~output()
{
	dprintf("()");

	output_streams.clear();
}

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

//static
void* output::output_thread(void *p_this)
{
	return static_cast<output*>(p_this)->output_thread();
}

void* output::output_thread()
{
	/* push data from main output buffer into output_stream buffers */
	while (!f_kill_thread) {
		/* FIXME: read from buffer */
		uint8_t* p_data = 0;
		for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
			iter->second.push(p_data);
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
	/* push data into output buffer  */
	return 0;
}
