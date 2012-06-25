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

#include <pthread.h>
#include <stdint.h>

class output_stream
{
public:
	output_stream();
	~output_stream();

	output_stream(const output_stream&);
	output_stream& operator= (const output_stream&);

	int push(uint8_t* p_data);

private:
	pthread_t h_thread;
	bool f_kill_thread;

	void *output_stream_thread();
	static void *output_stream_thread(void*);
};

class output
{
public:
	output();
	~output();

	int push(uint8_t* p_data);

private:
	pthread_t h_thread;
	bool f_kill_thread;

	void *output_thread();
	static void *output_thread(void*);
};

#endif /*__OUTPUT_H__ */
