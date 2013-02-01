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

#include <stdio.h>

#include "demux.h"

#include "log.h"
#define CLASS_MODULE "demux"

#define DBG_DEMUX DBG_PARSE

#define dprintf(fmt, arg...) __dprintf(DBG_DEMUX, fmt, ##arg)

demux::demux()
{
	dprintf("()");
	memset(&out, 0, sizeof(out));
	out.clear();
}

demux::~demux()
{
	dprintf("()");
	sleep(2);
	for (map_output::const_iterator iter = out.begin(); iter != out.end(); ++iter)
		((output_stream)(iter->second)).stop_after_drain();
	out.clear();
}

int demux::push(uint16_t pid, uint8_t *p)
{
	if (!out.count(pid)) {

		char newfile[16] = { 0 };
		snprintf(newfile, sizeof(newfile), "file://%04x.ts", pid);

		int ret = out[pid].add(newfile);
		if (ret < 0)
			return ret;
		out[pid].start();
	}
	return (out[pid].push(p, 188)) ? 0 : -1;
}
