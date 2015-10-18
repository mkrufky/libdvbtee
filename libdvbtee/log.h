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

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <time.h>

#define DBG_DECODE	1
#define DBG_PARSE	2
#define DBG_FEED	4
#define DBG_TUNE	8
#define DBG_OUTPUT	16
#define DBG_SERVE	32
#define DBG_TIME	64
#define DBG_DESC	128
#define DBG_STATS	256
#define DBG_ATSCTEXT	512
#define DBG_DVBPSI	1024

extern unsigned int dbg;

#define __printf(fd, fmt, arg...) fprintf(fd, fmt, ##arg)

#define __dPrintf(lvl, fmt, arg...) do {				\
	if (dbg & lvl)							\
		__printf(stderr, "%d "CLASS_MODULE"::%s: " fmt "\n", 	\
				 (int)time(NULL), __func__, ##arg);	\
} while (0)

#endif /* __LOG_H__ */
