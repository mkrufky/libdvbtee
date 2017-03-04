/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
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

#include "log.h"

#include <sys/timeb.h>

unsigned int dbg = 0;
unsigned int dbg_info = 0;


int libdvbtee_set_debug_level(unsigned int debug, unsigned int debug_info)
{
	dbg = debug;
	dbg_info = debug_info;
	if (debug)
		__printf(stderr, "%d LOG::%s: (0x%x)\n",
			(int)time(NULL), __func__, debug);
	return 0;
}


dbgFn::dbgFn(const char *str)
 : m_str(str ? str : "")
{
        log("+++");
}
dbgFn::~dbgFn()
{
        log("---");
}

void dbgFn::log(const char *pre)
{
        if (m_str.length()) {
            __printf(stderr, "%d %s %s\n", (int)time(NULL), pre, m_str.c_str());
        }
}
