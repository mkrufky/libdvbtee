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

#include "udp.h"

#include "log.h"
#define CLASS_MODULE "UdpFeeder"

#define FEED_BUFFER 0

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

using namespace dvbtee::feed;

#define BUFSIZE ((4096/188)*188)

UdpFeeder::UdpFeeder()
{
  //
}

UdpFeeder::~UdpFeeder()
{
  //
}

int UdpFeeder::startUdpListener(uint16_t port_requested)
{
	struct sockaddr_in udp_sock;

	dPrintf("(%d)", port_requested);
	sprintf(m_uri, "UDPLISTEN: %d", port_requested);

	memset(&udp_sock, 0, sizeof(udp_sock));

	m_fd = -1;

	m_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_fd < 0) {
		perror("open socket failed");
		return m_fd;
	}

#if defined(_WIN32)
	if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, "1", 1) < 0) {
#else
	int reuse = 1;
	if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
#endif
		perror("setting reuse failed");
		return -1;
	}

	udp_sock.sin_family = AF_INET;
	udp_sock.sin_port = htons(port_requested);
	udp_sock.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_fd, (struct sockaddr*)&udp_sock, sizeof(udp_sock)) < 0) {
		perror("bind to local interface failed");
		return -1;
	}
	//	port = port_requested;
#if 0
	socket_set_nbio(m_fd);
#endif
	return 0;
}

int UdpFeeder::start()
{
	int ret = startUdpListener(m_port);
	if (0 != ret) {
		perror("startUdpListener() failed");
		return ret;
	}

	f_kill_thread = false;

	ret = pthread_create(&h_thread, NULL, udp_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

//static
void* UdpFeeder::udp_feed_thread(void *p_this)
{
	return static_cast<UdpFeeder*>(p_this)->udp_feed_thread();
}

void *UdpFeeder::udp_feed_thread()
{
	int rxlen = 0;
#if FEED_BUFFER
	void *q = NULL;
#else
	char q[188*7];
#endif
	int available;

	dPrintf("(sock_fd=%d)", m_fd);

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < (188*7)) ? available : (188*7);
		//ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
		rxlen = recvfrom(m_fd, q, available, MSG_WAITALL, NULL, NULL);//(struct sockaddr*) &ip_addr, sizeof(ip_addr));
		if (rxlen > 0) {
#if 0
			if (rxlen != available) fprintf(stderr, "%s: %d bytes != %d\n", __func__, rxlen, available);
#endif
//			getpeername(m_fd, (struct sockaddr*)&udpsa, &salen);
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
