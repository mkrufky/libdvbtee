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

#include <errno.h>

#include "tcp.h"

#include "log.h"
#define CLASS_MODULE "TcpFeeder"

#define FEED_BUFFER 0

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

using namespace dvbtee::feed;

#define BUFSIZE ((4096/188)*188)

TcpListener::TcpListener()
{
	//
}

TcpListener::~TcpListener()
{
	//
}

TcpFeeder::TcpFeeder()
{
	//
}

TcpFeeder::~TcpFeeder()
{
	//
}

int TcpListener::startTcpListener(uint16_t port_requested)
{
	dPrintf("(%d)", port_requested);
	sprintf(m_uri, "TCPLISTEN: %d", port_requested);

	f_kill_thread = false;

	m_fd = -1;

	listener.set_interface(this);

	return listener.start(port_requested);
}

int TcpListener::start()
{
	int ret = startTcpListener(m_port);
	if (0 != ret) {
		perror("startTcpListener() failed");
		return ret;
	}

	return ret;
}

void TcpFeeder::add_tcp_feed(int socket)
{
	struct sockaddr_in tcpsa;
	socklen_t salen = sizeof(tcpsa);

	dPrintf("(%d)", socket);
	if (m_fd >= 0) {
		dPrintf("(%d) this build only supports one tcp input feed connection at a time", socket);
		::close(socket);
		return;
	}

	if (!strlen(m_uri))
		sprintf(m_uri, "TCPSOCKET: %d", socket);

	m_fd = socket;

	f_kill_thread = false;

	if (0 != getpeername(m_fd, (struct sockaddr*)&tcpsa, &salen)) {
		perror("getpeername() failed");
		goto fail_close_file;
	}

	if (0 != pthread_create(&h_thread, NULL, tcp_feed_thread, this)) {
		perror("pthread_create() failed");
		goto fail_close_file;
#if FEED_BUFFER
	} else {
		start_feed();
#endif
	}
	return;
fail_close_file:
	closeFd();
	return;
}


//static
void* TcpFeeder::tcp_feed_thread(void *p_this)
{
	return static_cast<TcpListener*>(p_this)->tcp_feed_thread();
}

void *TcpFeeder::tcp_feed_thread()
{
//	struct sockaddr_in tcpsa;
//	socklen_t salen = sizeof(tcpsa);
	int rxlen = 0;
#if FEED_BUFFER
	void *q = NULL;
#else
	char q[BUFSIZE];
#endif
	int available;

	dPrintf("(sock_fd=%d)", m_fd);

//	getpeername(m_fd, (struct sockaddr*)&tcpsa, &salen);

	while (!f_kill_thread) {
#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < (BUFSIZE)) ? available : (BUFSIZE);
		rxlen = recv(m_fd, q, available, MSG_WAITALL);
		if (rxlen > 0) {
			if (rxlen != available) fprintf(stderr, "%s: %d bytes != %d\n", __func__, rxlen, available);
#if !FEED_BUFFER
			parser.feed(rxlen, (uint8_t*)q);
#endif
		} else if ( (rxlen == 0) || ( (rxlen == -1) && (errno != EAGAIN) ) ) {
			stop_without_wait();
		} else if (rxlen == -1) { //( (rxlen == -1) && (errno == EAGAIN) ) {
			usleep(50*1000);
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr((rxlen > 0) ? rxlen : 0);
#endif
	}
	closeFd();
	pthread_exit(NULL);
}
