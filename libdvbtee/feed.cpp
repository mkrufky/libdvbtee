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

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "feed.h"
#include "log.h"

#define dprintf(fmt, arg...) __dprintf(DBG_FEED, fmt, ##arg)

unsigned int dbg = 0;

void libdvbtee_set_debug_level(unsigned int debug)
{
	dbg = debug;
}

feed::feed()
  : f_kill_thread(false)
  , fd(-1)
{
	dprintf("()");

	memset(filename, 0, sizeof(filename));
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

	if ((fd = open(filename, O_RDONLY | O_NONBLOCK )) < 0)
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
void* feed::stdin_feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->stdin_feed_thread();
}

//static
void* feed::tcp_listen_feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->tcp_listen_feed_thread();
}

int feed::start()
{
	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, feed_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

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

#define BUFSIZE (188 * 312)
void *feed::feed_thread()
{
	unsigned char buf[BUFSIZE];
	ssize_t r;

	dprintf("(fd=%d)", fd);

	while (!f_kill_thread) {

		if ((r = read(fd, buf, BUFSIZE)) <= 0) {

			if (!r) {
				f_kill_thread = true;
				continue;
			}
			// FIXME: handle (r < 0) errror cases
			//if (ret <= 0) switch (errno) {
			switch (errno) {
			case EAGAIN:
				break;
			case EOVERFLOW:
				fprintf(stderr, "%s: r = %d, errno = EOVERFLOW\n", __func__, (int)r);
				break;
			case EBADF:
				fprintf(stderr, "%s: r = %d, errno = EBADF\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			case EFAULT:
				fprintf(stderr, "%s: r = %d, errno = EFAULT\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			case EINTR: /* maybe ok? */
				fprintf(stderr, "%s: r = %d, errno = EINTR\n", __func__, (int)r);
				//f_kill_thread = true;
				break;
			case EINVAL:
				fprintf(stderr, "%s: r = %d, errno = EINVAL\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			case EIO: /* maybe ok? */
				fprintf(stderr, "%s: r = %d, errno = EIO\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			case EISDIR:
				fprintf(stderr, "%s: r = %d, errno = EISDIR\n", __func__, (int)r);
				f_kill_thread = true;
				continue;
			default:
				fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, errno);
				break;
			}

			usleep(50*1000);
			continue;
		}
		parser.feed(r, buf);
	}
	close_file();
	pthread_exit(NULL);
}

void *feed::stdin_feed_thread()
{
	unsigned char buf[BUFSIZE];
	ssize_t r;

	dprintf("()");

	while (!f_kill_thread) {

		if ((r = fread(buf, 188, BUFSIZE / 188, stdin)) < (BUFSIZE / 188)) {
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
		parser.feed(r * 188, buf);
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
	unsigned char buf[/*BUFSIZE*/188*175];

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

				rxlen = recv(sock[i], buf, sizeof(buf), MSG_WAITALL);
				if (rxlen > 0) {
					if (rxlen != sizeof(buf)) fprintf(stderr, "%s: %d bytes != %d\n", __func__, rxlen, sizeof(buf));
					getpeername(sock[i], (struct sockaddr*)&tcpsa, &salen);
					parser.feed(rxlen, buf);
				} else if ( (rxlen == 0) || ( (rxlen == -1) && (errno != EAGAIN) ) ) {
					close(sock[i]);
					sock[i] = -1;
				} else if ( (rxlen == -1) /*&& (errno == EAGAIN)*/ ) {
					usleep(50*1000);
				}
			}
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
