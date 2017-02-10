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

#include "dvbtee_config.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
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

int UdpFeeder::startUdpListener(uint16_t port_requested, const char *ip)
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
	udp_sock.sin_addr.s_addr = (ip) ? inet_addr(ip) : INADDR_ANY;

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

int UdpFeeder::startUdpListener(uint16_t port_requested, const char *ip, const char *net_if)
{
	dPrintf("(%d)", port_requested);
	snprintf(m_uri, sizeof(m_uri), "UDPLISTEN: %s:%d %s", ip, port_requested, net_if);

	m_fd = -1;

	m_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_fd < 0) {
		perror("open socket failed");
		return m_fd;
	}

	struct sockaddr_in sock;
	memset(&sock, 0, sizeof(sock));

	struct ip_mreq imreq;

	imreq.imr_multiaddr.s_addr = inet_addr(ip);

#ifdef HAVE_IFADDRS_H
	char host[NI_MAXHOST] = { 0 };
	struct ifaddrs *ifaddr, *ifa;
	int s;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs failed");
		return -1;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		s = getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
		if((strcmp(ifa->ifa_name, net_if) == 0) && (ifa->ifa_addr->sa_family == AF_INET)) {
			if (s != 0) {
				perror("unable to get network interface");
				return -1;
			}
			break;
		}
	}

	freeifaddrs(ifaddr);

	imreq.imr_interface.s_addr = inet_addr(host);
#endif

	sock.sin_family = AF_INET;
	sock.sin_port = htons(port_requested);
	sock.sin_addr.s_addr = inet_addr(ip);

#if defined(_WIN32)
	if (setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&imreq, sizeof(imreq)) < 0) {
#else
	if (setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imreq, sizeof(imreq)) < 0) {
#endif
		perror("setting IPPROTO_IP / IP_ADD_MEMBERSHIP failed");
		return -1;
	}

	if (bind(m_fd, (struct sockaddr*)&sock, sizeof(sock)) < 0) {
		perror("bind to specified interface failed");
		return -1;
	}
#if 0
	socket_set_nbio(m_fd);
#endif
	return 0;
}

int UdpFeeder::start()
{
	return start(NULL);
}

int UdpFeeder::start(const char *ip)
{
	int ret = startUdpListener(m_port, ip);
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

int UdpFeeder::start(const char *ip, const char *net_if)
{
	int ret = startUdpListener(m_port, ip, net_if);
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

int UdpFeeder::setPort(uint16_t port_requested)
{
	return m_port = port_requested;
}

//static
void* UdpFeeder::udp_feed_thread(void *p_this)
{
	return static_cast<UdpFeeder*>(p_this)->udp_feed_thread();
}

void *UdpFeeder::udp_feed_thread()
{
//	struct sockaddr_in udpsa;
//	socklen_t salen = sizeof(udpsa);
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
		rxlen = recvfrom(m_fd, q, available, MSG_DONTWAIT, NULL, NULL);//(struct sockaddr*) &ip_addr, sizeof(ip_addr));
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
			usleep(40*1000);
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr((rxlen > 0) ? rxlen : 0);
#endif
	}
	closeFd();
	pthread_exit(NULL);
}
