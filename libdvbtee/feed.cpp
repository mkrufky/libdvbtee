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
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
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
	__dprintf(debug, "(0x%x)", debug);
}

#define BUFSIZE ((4096/188)*188)

feed::feed()
  : f_kill_thread(false)
  , fd(-1)
  , feed_thread_prio(100)
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

feed::feed(const feed&)
{
	dprintf("(copy)");
	f_kill_thread = false;
	fd = -1;
	feed_thread_prio = 100;
}

feed& feed::operator= (const feed& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	f_kill_thread = false;
	fd = -1;
	feed_thread_prio = 100;

	return *this;
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

bool feed::check()
{
	dprintf("(%d, %s) %s", fd, filename, (f_kill_thread) ? "stopping" : "running");
	return true; //FIXME
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
void* feed::tcp_client_feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->tcp_client_feed_thread();
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

int feed::setup_feed(int prio)
{
#if FEED_BUFFER
	feed_thread_prio = prio;

	return start_feed();
#else
	return 0;
#endif
}

int feed::push(int size, const uint8_t* data)
{
#if FEED_BUFFER
	return (ringbuffer.write((const void*)data, size)) ? 0 : -1;
#else
	return parser.feed(size, (uint8_t*)data);
#endif
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
#if 0
	parser.stop();
#endif
	listener.stop();

	dprintf("waiting...");

	while (-1 != fd) {
		usleep(20*1000);
	}

	dprintf("done");
}

void *feed::feed_thread()
{
	unsigned char *data = NULL;
	int size, read_size;

	if (feed_thread_prio != 100) {
		pid_t tid = syscall(SYS_gettid);
		dprintf("setting priority from %d to %d", getpriority(PRIO_PROCESS, tid), feed_thread_prio);
		setpriority(PRIO_PROCESS, tid, feed_thread_prio);
	}
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
				f_kill_thread = true;
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
		}
#if FEED_BUFFER
		ringbuffer.put_write_ptr(r * 188);
#else
		parser.feed(r * 188, q);
#endif
	}
	pthread_exit(NULL);
}

void *feed::tcp_client_feed_thread()
{
	struct sockaddr_in tcpsa;
	socklen_t salen = sizeof(tcpsa);
	int rxlen = 0;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dprintf("(sock_fd=%d)", fd);

	getpeername(fd, (struct sockaddr*)&tcpsa, &salen);

	while (!f_kill_thread) {
#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < (BUFSIZE)) ? available : (BUFSIZE);
		rxlen = recv(fd, q, available, MSG_WAITALL);
		if (rxlen > 0) {
			if (rxlen != available) fprintf(stderr, "%s: %d bytes != %d\n", __func__, rxlen, available);
#if !FEED_BUFFER
			parser.feed(rxlen, q);
#endif
		} else if ( (rxlen == 0) || ( (rxlen == -1) && (errno != EAGAIN) ) ) {
			stop_without_wait();
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
#if 0
			if (rxlen != available) fprintf(stderr, "%s: %d bytes != %d\n", __func__, rxlen, available);
#endif
//			getpeername(fd, (struct sockaddr*)&udpsa, &salen);
#if !FEED_BUFFER
			parser.feed(rxlen, q);
#endif
		} else if ( (rxlen == 0) || ( (rxlen == -1) && (errno != EAGAIN) ) ) {
			stop_without_wait();
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
	strcpy(filename, "STDIN");

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
	char *ip, *portnum, *save;
	uint16_t port = 0;
	bool b_tcp = false;
	bool b_udp = false;
	int ret;

	dprintf("(<--%s)", source);
	strcpy(filename, source);

	if (strstr(source, ":")) {
		ip = strtok_r(source, ":", &save);
		if (strstr(ip, "tcp"))
			b_tcp = true;
		else
			if (strstr(ip, "udp"))
				b_udp = true;

		if ((b_tcp) || (b_udp)) {
			ip = strtok_r(NULL, ":", &save);
			if (strstr(ip, "//") == ip)
				ip += 2;
		}
		// else ip = proto;
		portnum = strtok_r(NULL, ":", &save);
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

//static
void feed::add_tcp_feed(void *p_this, int socket)
{
	return static_cast<feed*>(p_this)->add_tcp_feed(socket);
}

void feed::add_tcp_feed(int socket)
{
	dprintf("(%d)", socket);
	if (fd >= 0) {
		dprintf("(%d) this build only supports one tcp input feed connection at a time", socket);
		close(socket);
		return;
	}

	if (!strlen(filename))
		sprintf(filename, "TCPSOCKET: %d", socket);

	fd = socket;

	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, tcp_client_feed_thread, this);

	if (0 != ret) {
		perror("pthread_create() failed");
		close_file();
#if FEED_BUFFER
	} else {
		start_feed();
#endif
	}
	return;
}

int feed::start_tcp_listener(uint16_t port_requested)
{
	dprintf("(%d)", port_requested);
	sprintf(filename, "TCPLISTEN: %d", port_requested);

	f_kill_thread = false;

	fd = -1;

	listener.set_callback(this, add_tcp_feed);

	return listener.start(port_requested);
}

int feed::start_udp_listener(uint16_t port_requested)
{
	struct sockaddr_in udp_sock;

	dprintf("(%d)", port_requested);
	sprintf(filename, "UDPLISTEN: %d", port_requested);

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

/*****************************************************************************/

feed_server::feed_server()
{
	dprintf("()");

	feeders.clear();
}

feed_server::~feed_server()
{
	dprintf("()");

	listener.stop();

	feeders.clear();
}

//static
void feed_server::add_tcp_feed(void *p_this, int socket)
{
	return static_cast<feed_server*>(p_this)->add_tcp_feed(socket);
}

void feed_server::add_tcp_feed(int socket)
{
	if (socket >= 0) {
		dprintf("(%d)", socket);

		feeders[socket].add_tcp_feed(socket);

		if (connection_notify_cb) connection_notify_cb(parent_context, &feeders[socket]);
	}
	return;
}

int feed_server::start_tcp_listener(uint16_t port_requested, feed_server_callback notify_cb, void *context)
{
	dprintf("(%d)", port_requested);

	/* set listener callback to notify us (feed_server) of new connections */
	listener.set_callback(this, add_tcp_feed);

	/* set connection notify callback to notify parent server of new feeds */
	connection_notify_cb = notify_cb;
	parent_context = context;

	return listener.start(port_requested);
}
