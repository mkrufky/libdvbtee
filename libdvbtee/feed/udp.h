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

#ifndef UDP_H
#define UDP_H

#include "fdfeeder.h"

namespace dvbtee {

namespace feed {

class UdpFeeder : public FdFeeder
{
public:
	UdpFeeder();
	virtual ~UdpFeeder();

	virtual int start();
	int start(const char *ip);
	int start(const char *ip, const char *net_if);

	virtual
	int setPort(uint16_t port_requested);

private:
	void        *udp_feed_thread();
	static void *udp_feed_thread(void*);

	uint16_t m_port;

	int startUdpListener(uint16_t port_requested, const char *ip = NULL);
	int startUdpListener(uint16_t port_requested, const char* ip, const char* net_if);
};

}

}

#endif // UDP_H
