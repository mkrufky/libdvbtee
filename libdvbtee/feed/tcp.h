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

#ifndef TCP_H
#define TCP_H

#include "fdfeeder.h"

namespace dvbtee {

namespace feed {

class TcpFeeder : public FdFeeder, public socket_listen_iface
{
public:
	TcpFeeder();
	virtual ~TcpFeeder();

	virtual void accept_socket(int sock) { add_tcp_feed(sock); }

	void add_tcp_feed(int socket);
private:
	void        *tcp_feed_thread();
	static void *tcp_feed_thread(void*);
};

class TcpListener : public TcpFeeder
{
public:
	TcpListener();
	virtual ~TcpListener();

	virtual int start();

	int setPort(uint16_t port_requested) { return m_port = port_requested; }
private:
	uint16_t m_port;
	socket_listen listener;

	int startTcpListener(uint16_t port_requested);
};

}

}

#endif // TCP_H
