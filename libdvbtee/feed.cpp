/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
 *
 * Author: Michael Krufky <mkrufky@linuxtv.org>
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

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "feed.h"
#include "log.h"
#define CLASS_MODULE "feed"

#define FEED_BUFFER 0

#define dprintf(fmt, arg...) __dprintf(DBG_FEED, fmt, ##arg)

unsigned int dbg = 0;

void libdvbtee_set_debug_level(unsigned int debug)
{
	dbg = debug;
}

#define BUFSIZE ((4096/188)*188)

feed::feed()
  : f_kill_thread(false)
  , fd(-1)
  , ringbuffer()
{
	dprintf("()");

	memset(filename, 0, sizeof(filename));
#if FEED_BUFFER
	ringbuffer.set_capacity(BUFSIZE*4);
#endif
}

feed::~feed()
{
	dprintf("()");

	close_file();
}

void feed::set_filename(char* new_file)
{
	dprintf("(%s)", new_file);

	strcpy(filename, new_file);
};

int feed::open_file()
{
	dprintf("()");

	fd = -1;

	if ((fd = open(filename, O_RDONLY )) < 0)
		fprintf(stderr, "failed to open %s\n", filename);
	else
		fprintf(stderr, "%s: using %s\n", __func__, filename);

	return fd;
}

void feed::close_file()
{
	dprintf("()");

	close(fd);
	fd = -1;
}

//static
void* feed::feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->feed_thread();
}

//static
void* feed::file_feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->file_feed_thread();
}

//static
void* feed::stdin_feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->stdin_feed_thread();
}

//static
void* feed::tcp_listen_feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->tcp_listen_feed_thread();
}

//static
void* feed::udp_listen_feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->udp_listen_feed_thread();
}

int feed::start_feed()
{
#if 0
	f_kill_thread = false;
#endif
	int ret = pthread_create(&h_feed_thread, NULL, feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

int feed::start()
{
	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, file_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");
#if FEED_BUFFER
	else
		start_feed();
#endif
	return ret;
}

void feed::stop()
{
	dprintf("()");

	stop_without_wait();

	parser.stop();

	while (-1 != fd) {
		usleep(20*1000);
	}
}

void *feed::feed_thread()
{
	unsigned char *data = NULL;
	int size, read_size;

	dprintf("()");
	while (!f_kill_thread) {
		size = ringbuffer.get_size();
		if (size >= 188) {
			if (size != (size/188)*188) fprintf(stderr,"%s: ringbuf has unaligned data %d -> %d\t", __func__, size, (size/188)*188);
			size = (size/188)*188;
			read_size = ringbuffer.get_read_ptr((void**)&data, size);
			//if ((read_size != size) && (size != (size/read_size)*read_size)) fprintf(stderr,"%s: read size doesnt match ringbuffer size, shouldnt be %d != %d\n", __func__, read_size, size);
			parser.feed(read_size, data);
			ringbuffer.put_read_ptr(read_size);
		} else
			usleep(20*1000);
	}
	pthread_exit(NULL);
}

void *feed::file_feed_thread()
{
	ssize_t r;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dprintf("(fd=%d)", fd);

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < BUFSIZE) ? available : BUFSIZE;
		if ((r = read(fd, q, available)) <= 0) {

			if (!r) {
				f_kill_thread = true;
				continue;
			}
			switch (errno) {
			case EAGAIN:
				break;
			case EOVERFLOW:
				fprintf(stderr, "%s: r = %d, errno = EOVERFLOW\n", __func__, (int)r);
				break;
			case EBADF:
				fprintf(stderr, "%s: r = %d, errno = EBADF\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EFAULT:
				fprintf(stderr, "%s: r = %d, errno = EFAULT\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EINTR: /* maybe ok? */
				fprintf(stderr, "%s: r = %d, errno = EINTR\n", __func__, (int)r);
				//f_kill_thread = true;
				break;
			case EINVAL:
				fprintf(stderr, "%s: r = %d, errno = EINVAL\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EIO: /* maybe ok? */
				fprintf(stderr, "%s: r = %d, errno = EIO\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			case EISDIR:
				fprintf(stderr, "%s: r = %d, errno = EISDIR\n", __func__, (int)r);
				f_kill_thread = true;
				break;
			default:
				fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, errno);
				break;
			}
			continue;
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr(r);
#else
		parser.feed(r, q);
#endif
	}
	close_file();
	pthread_exit(NULL);
}

void *feed::stdin_feed_thread()
{
	ssize_t r;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dprintf("()");

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < BUFSIZE) ? available : BUFSIZE;
		if ((r = fread(q, 188, available / 188, stdin)) < (available / 188)) {
			if (ferror(stdin)) {
				fprintf(stderr, "%s: error reading stdin!\n", __func__);
				usleep(50*1000);
				clearerr(stdin);
			}
			if (feof(stdin)) {
				fprintf(stderr, "%s: EOF\n", __func__);
				f_kill_thread = true;
			}
			continue;
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr(r * 188);
#else
		parser.feed(r * 188, q);
#endif
	}
	pthread_exit(NULL);
}

void *feed::tcp_listen_feed_thread()
{
	struct sockaddr_in tcpsa;
	socklen_t salen = sizeof(tcpsa);
	int i, rxlen = 0;
#define MAX_SOCKETS 1
	int sock[MAX_SOCKETS];
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dprintf("(sock_fd=%d)", fd);

	for (i = 0; i < MAX_SOCKETS; i++)
		sock[i] = -1;

	while (!f_kill_thread) {
		int d = accept(fd, (struct sockaddr*)&tcpsa, &salen);
		if (d != -1) {
			for (i = 0; i < MAX_SOCKETS; i++)
				if (sock[i] == -1) {
					sock[i] = d;
					break;
				}
			if (sock[i] != d)
				perror("couldn't attach to socket");
		}
		for (i = 0; i < MAX_SOCKETS; i++)
			if (sock[i] != -1) {

#if FEED_BUFFER
				available = ringbuffer.get_write_ptr(&q);
#else
				available = sizeof(q);
#endif
				available = (available < (BUFSIZE)) ? available : (BUFSIZE);
				rxlen = recv(sock[i], q, available, MSG_WAITALL);
				if (rxlen > 0) {
					if (rxlen != available) fprintf(stderr, "%s: %d bytes != %d\n", __func__, rxlen, available);
					getpeername(sock[i], (struct sockaddr*)&tcpsa, &salen);
#if !FEED_BUFFER
					parser.feed(rxlen, q);
#endif
				} else if ( (rxlen == 0) || ( (rxlen == -1) && (errno != EAGAIN) ) ) {
					close(sock[i]);
					sock[i] = -1;
				} else if ( (rxlen == -1) /*&& (errno == EAGAIN)*/ ) {
					usleep(50*1000);
				}
#if FEED_BUFFER
				ringbuffer.put_write_ptr((rxlen > 0) ? rxlen : 0);
#endif
			}
	}
	close_file();
	pthread_exit(NULL);
}

void *feed::udp_listen_feed_thread()
{
//	struct sockaddr_in udpsa;
//	socklen_t salen = sizeof(udpsa);
	int rxlen = 0;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[188*7];
#endif
	int available;

	dprintf("(sock_fd=%d)", fd);

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < (188*7)) ? available : (188*7);
		//ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
		rxlen = recvfrom(fd, q, available, MSG_WAITALL, NULL, NULL);//(struct sockaddr*) &ip_addr, sizeof(ip_addr));
		if (rxlen > 0) {
			if (rxlen != available) fprintf(stderr, "%s: %d bytes != %d\n", __func__, rxlen, available);
//			getpeername(fd, (struct sockaddr*)&udpsa, &salen);
#if !FEED_BUFFER
			parser.feed(rxlen, q);
#endif
		} else if ( (rxlen == 0) || ( (rxlen == -1) && (errno != EAGAIN) ) ) {
//			close(sock[i]);
//			sock[i] = -1;
		} else if ( (rxlen == -1) /*&& (errno == EAGAIN)*/ ) {
			usleep(50*1000);
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr((rxlen > 0) ? rxlen : 0);
#endif
	}
	close_file();
	pthread_exit(NULL);
}

int feed::start_stdin()
{
	dprintf("()");

	if (NULL == freopen(NULL, "rb", stdin)) {
		fprintf(stderr, "failed to open stdin!\n");
		return -1;
	}
	fprintf(stderr, "%s: using STDIN\n", __func__);

	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, stdin_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");
#if FEED_BUFFER
	else
		start_feed();
#endif
	return ret;
}

int feed::start_socket(char* source)
{
	dprintf("()");
#if 0
	struct sockaddr_in ip_addr;
#endif
	char *ip, *portnum;
	uint16_t port = 0;
	bool b_tcp = false;
	bool b_udp = false;
	int ret;

	dprintf("(<--%s)", source);

	if (strstr(source, ":")) {
		ip = strtok(source, ":");
		if (strstr(ip, "tcp"))
			b_tcp = true;
		else
			if (strstr(ip, "udp"))
				b_udp = true;

		if ((b_tcp) || (b_udp)) {
			ip = strtok(NULL, ":");
			if (strstr(ip, "//") == ip)
				ip += 2;
		}
		// else ip = proto;
		portnum = strtok(NULL, ":");
		if (portnum)
			port = atoi(portnum);

		if (!port) {
			port = atoi(ip);
			ip = NULL;
		}
	} else {
		// assuming UDP
		ip = NULL;
		port = atoi(source);
	}
#if 0
	if (fd >= 0)
		close(fd);

	fd = socket(AF_INET, (b_tcp) ? SOCK_STREAM : SOCK_DGRAM, (b_tcp) ? IPPROTO_TCP : IPPROTO_UDP);
	if (fd >= 0) {
#endif
#if 0
		memset(&ip_addr, 0, sizeof(ip_addr));
		ip_addr.sin_family = AF_INET;
		ip_addr.sin_port   = htons(port);
		if (inet_aton(ip, &ip_addr.sin_addr) == 0) {
			perror("ip address translation failed");
			return -1;
		} else
			ringbuffer.reset();
#endif
		ret = (b_tcp) ? start_tcp_listener(port) : start_udp_listener(port);
#if 0
	} else {
		perror("socket failed");
		return -1;
	}
#endif
	dprintf("~(-->%s)", source);
	return ret;
}

int feed::start_tcp_listener(uint16_t port_requested)
{
	struct sockaddr_in tcp_sock;

	dprintf("(%d)", port_requested);

	memset(&tcp_sock, 0, sizeof(tcp_sock));

	f_kill_thread = false;

	fd = -1;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("open socket failed");
		return fd;
	}

	int reuse = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		perror("setting reuse failed");
		return -1;
	}

	tcp_sock.sin_family = AF_INET;
	tcp_sock.sin_port = htons(port_requested);
	tcp_sock.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (struct sockaddr*)&tcp_sock, sizeof(tcp_sock)) < 0) {
		perror("bind to local interface failed");
		return -1;
	}
	//	port = port_requested;
#if 0
	int fl = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) {
		perror("set non-blocking failed");
		return -1;
	}
#endif
	listen(fd, 1);

	int ret = pthread_create(&h_thread, NULL, tcp_listen_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");
#if FEED_BUFFER
	else
		start_feed();
#endif
	return ret;
}

int feed::start_udp_listener(uint16_t port_requested)
{
	struct sockaddr_in udp_sock;

	dprintf("(%d)", port_requested);

	memset(&udp_sock, 0, sizeof(udp_sock));

	f_kill_thread = false;

	fd = -1;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		perror("open socket failed");
		return fd;
	}

	int reuse = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		perror("setting reuse failed");
		return -1;
	}

	udp_sock.sin_family = AF_INET;
	udp_sock.sin_port = htons(port_requested);
	udp_sock.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (struct sockaddr*)&udp_sock, sizeof(udp_sock)) < 0) {
		perror("bind to local interface failed");
		return -1;
	}
	//	port = port_requested;
#if 0
	int fl = fcntl(fd, F_GETFL, 0);
	if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) {
		perror("set non-blocking failed");
		return -1;
	}
#endif
	int ret = pthread_create(&h_thread, NULL, udp_listen_feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");
#if FEED_BUFFER
	else
		start_feed();
#endif
	return ret;
}

bool feed::wait_for_event_or_timeout(unsigned int timeout, unsigned int wait_event) {
	time_t start_time = time(NULL);
	while ((!f_kill_thread) &&
	       ((timeout == 0) || (time(NULL) - start_time) < ((int)timeout) )) {

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
