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

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <string>

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

void libdvbtee_set_debug_level(unsigned int debug);

#define __printf(fd, fmt, arg...) fprintf(fd, fmt, ##arg)

#define __dPrintf(lvl, fmt, arg...) do {				\
	if (dbg & lvl)							\
		__printf(stderr, "%d %s::%s: " #fmt "\n",		\
			 (int)time(NULL), CLASS_MODULE, __func__, ##arg);	\
} while (0)

class dbgFn {
public:
    dbgFn(const char *str = NULL);
    ~dbgFn();
private:
    void log(const char *pre);

    std::string m_str;
};

#define PRETTY_FILE() ({ \
    const char *__needle = "../"; \
    const char *__fname = __FILE__; \
    const char *__ret; \
    if (__fname == strstr(__fname, __needle)) { \
        __ret = __fname + strlen(__needle); \
    } \
    else { \
        __ret = __fname; \
    } \
    __ret; })

#define DBGFN() \
    std::string __dbgfnstr; \
    char __lineNumb[6] = { 0 }; \
    snprintf(__lineNumb, sizeof(__lineNumb), "%d", __LINE__); \
    __dbgfnstr.append(PRETTY_FILE()); \
    __dbgfnstr.append(":"); \
    __dbgfnstr.append(__lineNumb); \
    __dbgfnstr.append(": "); \
    __dbgfnstr.append(__func__); \
    __dbgfnstr.append("()"); \
    dbgFn DbgFn(__dbgfnstr.c_str())

#endif /* __LOG_H__ */
