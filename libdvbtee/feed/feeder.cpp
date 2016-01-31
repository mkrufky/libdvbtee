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

#include "feeder.h"


#include "log.h"
#define CLASS_MODULE "Feeder"

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

using namespace dvbtee::feed;


PushFeeder::PushFeeder()
{
	//
}

PushFeeder::~PushFeeder()
{
	//
}

int PushFeeder::push(int size, const uint8_t* data)
{
	return parser.feed(size, (uint8_t*)data);
}


ThreadFeeder::ThreadFeeder()
  :
#if !defined(_WIN32)
    h_thread((pthread_t)NULL)
  ,
#endif
    f_kill_thread(false)
{
	//
}

ThreadFeeder::~ThreadFeeder()
{
	//
}

void ThreadFeeder::stop_without_wait()
{
	f_kill_thread = true;
}

bool ThreadFeeder::wait_for_event_or_timeout(unsigned int timeout, unsigned int wait_event) {
	time_t start_time = time(NULL);
	while ((!f_kill_thread) &&
	       ((timeout == 0) || ((time(NULL) - start_time) < (time_t)(timeout)) )) {

		switch (wait_event) {
		case FEED_EVENT_PSIP:
			if (parser.is_psip_ready()) return true;
			break;
		case FEED_EVENT_EPG:
			if (parser.is_epg_ready()) return true;
			break;
		default:
			break;
		}
		usleep(200*1000);
	}
	switch (wait_event) {
	case FEED_EVENT_PSIP:
		return parser.is_psip_ready();
	case FEED_EVENT_EPG:
		return parser.is_epg_ready();
	default:
		return f_kill_thread;
	}
}


PullFeeder::PullFeeder(feed_pull_iface* iface)
  : m_iface(iface)
{
  //
}

PullFeeder::~PullFeeder()
{
	//
}

int PullFeeder::start()
{
	f_kill_thread = false;

	//strncpy(m_uri, "PULLCALLBACK", sizeof(m_uri));

	int ret = pthread_create(&h_thread, NULL, pull_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

//static
void* PullFeeder::pull_thread(void *p_this)
{
	return static_cast<PullFeeder*>(p_this)->pull_thread();
}

void *PullFeeder::pull_thread()
{
	dPrintf("()");

	while (!f_kill_thread)
		if (0 >= m_iface->pull())
			usleep(50*1000);

	dPrintf("() exiting...");
	pthread_exit(NULL);
}
