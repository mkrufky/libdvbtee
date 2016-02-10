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

#ifndef FEEDER_H
#define FEEDER_H

#include <pthread.h>
#include "parse.h"

namespace dvbtee {

namespace feed {

class Feeder
{
public:
	Feeder();
	virtual ~Feeder();

	parse parser;

	std::string getUri() { return std::string(m_uri); }

	/* for compat */
	__attribute__((deprecated)) const char* get_filename() const { return m_uri; }
	__attribute__((deprecated)) void close_file() { return this->close(); }

protected:
	Feeder(const Feeder&);
	Feeder& operator= (const Feeder&);

	char m_uri[256];
};

class PushFeeder : public Feeder
{
public:
	PushFeeder();
	virtual ~PushFeeder();

	int push(int size, const uint8_t* data);
};

class ThreadFeeder : public PushFeeder
{
public:
	ThreadFeeder();
	virtual ~ThreadFeeder();

	void stop_without_wait();

	virtual void stop() { stop_without_wait(); }
	virtual int start() = 0;

#define FEED_EVENT_PSIP 1
#define FEED_EVENT_EPG  2
	bool wait_for_event_or_timeout(unsigned int timeout, unsigned int wait_event);

	inline bool wait_for_streaming_or_timeout(unsigned int timeout) { return wait_for_event_or_timeout(timeout, 0); }
	inline bool wait_for_psip(unsigned int time_ms) { return wait_for_event_or_timeout(time_ms / 1000, FEED_EVENT_PSIP); }
	inline bool wait_for_epg(unsigned int time_ms) { return wait_for_event_or_timeout(time_ms / 1000, FEED_EVENT_EPG); }

protected:
	pthread_t h_thread;
	bool f_kill_thread;
};

class feed_pull_iface // FIXME: RENAME!!
{
public:
	virtual int pull() = 0;
};

class PullFeeder : public ThreadFeeder
{
public:
	PullFeeder(feed_pull_iface& iface);
	virtual ~PullFeeder();

	virtual int start();

private:
	feed_pull_iface& m_iface;

	void        *pull_thread();
	static void *pull_thread(void*);
};

}

}

#endif // FEEDER_H
