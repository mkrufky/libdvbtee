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
	pthread_exit(NULL);
}

int output_stream::push(uint8_t* p_data)
{
	return 0;
}
/* ----------------------------------------------------------------- */

output::output()
  : f_kill_thread(false)
{
	dprintf("()");
}

output::~output()
{
	dprintf("()");
}

#if 0
output::output(const output&)
{
	dprintf("(copy)");
}

output& output::operator= (const output& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

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
	pthread_exit(NULL);
}

int output::push(uint8_t* p_data)
{
	return 0;
}
