/*****************************************************************************
 * Copyright (C) 2011-2017 Michael Ira Krufky
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

#ifndef __OUTPUTBASE_H__
#define __OUTPUTBASE_H__

#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <map>
#include <string>

#include "listen.h"
#include "rbuf.h"

#if 1 // moved from parse.h
typedef std::map<uint16_t, uint16_t> map_pidtype;
#endif

enum output_options {
	OUTPUT_NONE    = 0,
	OUTPUT_PATPMT  = 1,
	OUTPUT_PES     = 2,
	OUTPUT_PSIP    = 4,
};

enum output_mimetype {
	MIMETYPE_NONE,
	MIMETYPE_OCTET_STREAM,
	MIMETYPE_TEXT_PLAIN,
	MIMETYPE_TEXT_HTML,
};


#define OUTPUT_AV (OUTPUT_PATPMT | OUTPUT_PES)

#define OUTPUT_STREAM_BUF_SIZE 188*7*198

typedef int (*stream_callback)(void *, const uint8_t *, size_t);

class output_stream_iface
{
public:
	virtual int stream(const uint8_t *, size_t) = 0;
};

class output_base
{
public:
	output_base() {}
	virtual ~output_base() {}

	virtual int start() = 0;
	virtual void stop() = 0;
	virtual void stop(int) = 0;

	virtual bool push(uint8_t* p_data, int size) = 0;
	virtual bool push(uint8_t* p_data, enum output_options opt = OUTPUT_NONE) = 0;

	virtual int add(char* target) = 0;
	virtual int add(int socket, unsigned int method) = 0;
	virtual int add(void* priv, stream_callback callback) = 0;
	virtual int add(output_stream_iface *iface) = 0;
	virtual int add_stdout() = 0;

	virtual int add(char* target, map_pidtype &pids) = 0;
	virtual int add(int socket, unsigned int method, map_pidtype &pids) = 0;
	virtual int add(void* priv, stream_callback callback, map_pidtype &pids) = 0;
	virtual int add(output_stream_iface *iface, map_pidtype &pids) = 0;
	virtual int add_stdout(map_pidtype &pids) = 0;

	virtual int add_http_server(int) = 0;

	virtual void set_options(enum output_options opt = OUTPUT_NONE) = 0;
	virtual void rotate(unsigned long int file, unsigned long int fseq) = 0;

	virtual bool check() = 0;

	virtual int get_pids(map_pidtype&) = 0;
	virtual void reset_pids(int target_id) = 0;
};

class dummy_output: public output_base
{
public:
	virtual int start() { return 0; }
	virtual void stop() {}
	virtual void stop(int) {}

	virtual bool push(uint8_t* p_data, int size) { return true; }
	virtual bool push(uint8_t* p_data, enum output_options opt = OUTPUT_NONE) { return true; }

	virtual int add(char* target) { return 0; }
	virtual int add(int socket, unsigned int method) { return 0; }
	virtual int add(void* priv, stream_callback callback) { return 0; }
	virtual int add(output_stream_iface *iface) { return 0; }
	virtual int add_stdout() { return 0; }

	virtual int add(char* target, map_pidtype &pids) { return 0; }
	virtual int add(int socket, unsigned int method, map_pidtype &pids) { return 0; }
	virtual int add(void* priv, stream_callback callback, map_pidtype &pids) { return 0; }
	virtual int add(output_stream_iface *iface, map_pidtype &pids) { return 0; }
	virtual int add_stdout(map_pidtype &pids) { return 0; }

	virtual int add_http_server(int) { return 0; }

	virtual void set_options(enum output_options opt = OUTPUT_NONE) { }
	virtual void rotate(unsigned long int file, unsigned long int fseq) { }

	virtual bool check() { return 0; }

	virtual int get_pids(map_pidtype&) { return 0; }
	virtual void reset_pids(int target_id) {}
};

#endif /*__OUTPUTBASE_H__ */
