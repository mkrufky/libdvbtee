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

#ifndef __DVBTEE_CONFIG_H__
#define __DVBTEE_CONFIG_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LINUX_DVB_FRONTEND_H
#if HAVE_LINUX_DVB_FRONTEND_H
#define USE_LINUXTV
#include <linux/dvb/dmx.h>
#ifndef DMX_DQBUF
#include <stdint.h>
struct dmx_buffer {
	uint32_t			index;
	uint32_t			bytesused;
	uint32_t			offset;
	uint32_t			length;
	uint32_t			flags;
	uint32_t			count;
};
struct dmx_requestbuffers {
	uint32_t			count;
	uint32_t			size;
};
struct dmx_exportbuffer {
	uint32_t		index;
	uint32_t		flags;
	uint32_t		fd;
};
enum dmx_buffer_flags {
	DMX_BUFFER_FLAG_HAD_CRC32_DISCARD		= 1 << 0,
	DMX_BUFFER_FLAG_TEI				= 1 << 1,
	DMX_BUFFER_PKT_COUNTER_MISMATCH			= 1 << 2,
	DMX_BUFFER_FLAG_DISCONTINUITY_DETECTED		= 1 << 3,
	DMX_BUFFER_FLAG_DISCONTINUITY_INDICATOR		= 1 << 4,
};
#define DMX_REQBUFS              _IOWR('o', 60, struct dmx_requestbuffers)
#define DMX_QUERYBUF             _IOWR('o', 61, struct dmx_buffer)
#define DMX_EXPBUF               _IOWR('o', 62, struct dmx_exportbuffer)
#define DMX_QBUF                 _IOWR('o', 63, struct dmx_buffer)
#define DMX_DQBUF                _IOWR('o', 64, struct dmx_buffer)
#endif
#endif
#endif

#ifdef HAVE_LIBHDHOMERUN_HDHOMERUN_H
#if HAVE_LIBHDHOMERUN_HDHOMERUN_H
#define USE_HDHOMERUN
#endif
#endif

#ifdef HAVE_LIBHDHOMERUN
#if HAVE_LIBHDHOMERUN
#define USE_HDHOMERUN
#endif
#endif

#if defined(_WIN32)
#include <ws2tcpip.h>

#ifndef HAVE_STRTOK_R
#define strtok_r(a,b,c) strtok(a,b)
#endif
#endif

#ifndef USE_WSTRING_CONVERT
#define USE_WSTRING_CONVERT 0
#endif

#ifndef USE_DVBTEE_WSTRIP
#define USE_DVBTEE_WSTRIP 0
#if USE_WSTRING_CONVERT
#undef USE_DVBTEE_WSTRIP
#define USE_DVBTEE_WSTRIP 1
#endif /* USE_WSTRING_CONVERT */
#if defined(_WIN32)
#undef USE_DVBTEE_WSTRIP
#define USE_DVBTEE_WSTRIP 1
#endif /* defined(_WIN32) */
#endif /* USE_DVBTEE_WSTRIP */

#endif //__DVBTEE_CONFIG_H__
