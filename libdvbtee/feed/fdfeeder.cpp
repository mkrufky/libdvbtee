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

#include "fdfeeder.h"

#include "log.h"
#define CLASS_MODULE "FdFeeder"

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

using namespace dvbtee::feed;

UriFeeder::UriFeeder()
{
	memset(m_uri, 0, sizeof(m_uri));
}

UriFeeder::~UriFeeder()
{
	dPrintf("(%s)", strlen(m_uri) ? m_uri : "");
}

FdFeeder::FdFeeder()
  : m_fd(-1)
{
	//
}

FdFeeder::~FdFeeder()
{
	closeFd();
}

void FdFeeder::stop()
{
	dPrintf("()");

	stop_without_wait();

	dPrintf("waiting...");

	while (-1 != m_fd) {
		usleep(20*1000);
	}

	dPrintf("done");
}

void FdFeeder::closeFd()
{
	dPrintf("()");

	if (m_fd >= 0) {
		::close(m_fd);
		m_fd = -1;
	}
}

int FdFeeder::openFile(int new_fd)
{
	closeFd();
	m_fd = new_fd;
	return m_fd;
}
