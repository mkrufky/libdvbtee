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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#include <unistd.h>

#include <fstream>

#include "feed.h"
#include "log.h"
#define CLASS_MODULE "feed"

#define FEED_BUFFER 0

#define dPrintf(fmt, arg...) __dPrintf(DBG_FEED, fmt, ##arg)

#define BUFSIZE ((4096/188)*188)

feed::feed()
  :
#if !defined(_WIN32)
    h_thread((pthread_t)NULL)
  , h_feed_thread((pthread_t)NULL)
  ,
#endif
    f_kill_thread(false)
  , fd(-1)
#if FEED_BUFFER
  , feed_thread_prio(100)
  , ringbuffer()
#endif
  , m_pull_iface(NULL)
{
	dPrintf("()");

	memset(filename, 0, sizeof(filename));
#if FEED_BUFFER
	ringbuffer.set_capacity(BUFSIZE*4);
#endif
}

feed::~feed()
{
	dPrintf("(%s)", strlen(filename) ? filename : "");

	close_file();
}

feed::feed(const feed&)
{
	dPrintf("(copy)");
#if !defined(_WIN32)
	h_thread = (pthread_t)NULL;
	h_feed_thread = (pthread_t)NULL;
#endif
	f_kill_thread = false;
	fd = -1;
#if FEED_BUFFER
	feed_thread_prio = 100;
#endif
	m_pull_iface = NULL;
}

feed& feed::operator= (const feed& cSource)
{
	dPrintf("(operator=)");

	if (this == &cSource)
		return *this;

#if !defined(_WIN32)
	h_thread = (pthread_t)NULL;
	h_feed_thread = (pthread_t)NULL;
#endif
	f_kill_thread = false;
	fd = -1;
#if FEED_BUFFER
	feed_thread_prio = 100;
#endif
	m_pull_iface = NULL;

	return *this;
}

void feed::set_filename(char* new_file)
{
	dPrintf("(%s)", new_file);

	size_t len = strlen(new_file);
	strncpy(filename, new_file, sizeof(filename)-1);
	filename[len < sizeof(filename) ? len : sizeof(filename)-1] = '\0';
}

int feed::_open_file(int flags)
{
	dPrintf("()");

	fd = -1;

#if !USE_IOS_READ
	if ((fd = open(filename, O_RDONLY|flags )) < 0)
		fprintf(stderr, "failed to open %s\n", filename);
	else
		__log_printf(stderr, "%s: using %s\n", __func__, filename);

	return fd;
#else
	return 0;
#endif
}

void feed::close_file()
{
	dPrintf("()");

	if (fd >= 0) {
		close(fd);
		fd = -1;
	}
}

bool feed::check()
{
	dPrintf("(%d, %s) %s", fd, filename, (f_kill_thread) ? "stopping" : "running");
	return true; //FIXME
}

#if FEED_BUFFER
//static
void* feed::feed_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->feed_thread();
}
#endif

//static
void* feed::file_feed_thread(void *p_this)
{
#if USE_IOS_READ
	return static_cast<feed*>(p_this)->ios_file_feed_thread();
#else
	return static_cast<feed*>(p_this)->file_feed_thread();
#endif
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

//static
void* feed::pull_thread(void *p_this)
{
	return static_cast<feed*>(p_this)->pull_thread();
}

#if FEED_BUFFER
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
#endif

int feed::setup_feed(int prio)
{
#if FEED_BUFFER
	feed_thread_prio = prio;

	return start_feed();
#else
	(void)prio;
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

int feed::pull(feed_pull_iface *iface)
{
	f_kill_thread = false;

	m_pull_iface = iface;

	strncpy(filename, "PULLCALLBACK", sizeof(filename));

	int ret = pthread_create(&h_thread, NULL, pull_thread, this);

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
	dPrintf("()");

	stop_without_wait();
#if 0
	parser.stop();
#endif
	listener.stop();

	dPrintf("waiting...");

	while (-1 != fd) {
		usleep(20*1000);
	}

	dPrintf("done");
}

#if FEED_BUFFER
void *feed::feed_thread()
{
	unsigned char *data = NULL;
	int size, read_size;

	if (feed_thread_prio != 100) {
		pid_t tid = syscall(SYS_gettid);
		if (tid >= 0) {
			dPrintf("setting priority from %d to %d",
				getpriority(PRIO_PROCESS, tid), feed_thread_prio);
			if (0 > setpriority(PRIO_PROCESS, tid, feed_thread_prio))
				perror("setpriority() failed");
		}
	}
	dPrintf("()");
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
#endif

void *feed::file_feed_thread()
{
	ssize_t r;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;
#ifdef HAVE_SYS_POLL_H
	struct pollfd pfd[1];
#endif

	dPrintf("(fd=%d)", fd);

#ifdef HAVE_SYS_POLL_H
	pfd[0].fd = fd;
	pfd[0].events = POLLIN;
#endif

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < BUFSIZE) ? available : BUFSIZE;
#ifdef HAVE_SYS_POLL_H
		if ((r = poll(pfd, 1, -1)) <= 0) {
			fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, errno);
			f_kill_thread = true;
			break;
		}
		if (pfd[0].revents & (POLLERR|POLLIN)) {
			if ((r = read(fd, q, available)) > 0) {
#if FEED_BUFFER
				ringbuffer.put_write_ptr(r);
#else
				parser.feed(r, q);
#endif
				continue;
			}
#else
		if ((r = read(fd, q, available)) <= 0) {
#endif

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
#ifndef HAVE_SYS_POLL_H
#if FEED_BUFFER
		ringbuffer.put_write_ptr(r);
#else
		parser.feed(r, q);
#endif
#endif
	}
	close_file();
	pthread_exit(NULL);
}

#if USE_IOS_READ
void *feed::ios_file_feed_thread()
{
	ssize_t r;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dPrintf("(ios)");
	std::ifstream infile;
	infile.open(filename, std::ios::binary | std::ios::in);
	if (infile.fail()) {
		switch (errno) {
		case EACCES:
			fprintf(stderr, "%s: r = %d, errno = EACCES\n", __func__, (int)r);
			break;
		case ENOENT:
			fprintf(stderr, "%s: r = %d, errno = ENOENT\n", __func__, (int)r);
			break;
		default:
			fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, errno);
			break;
		}
	} else

	while (!f_kill_thread) {

#if FEED_BUFFER
		available = ringbuffer.get_write_ptr(&q);
#else
		available = sizeof(q);
#endif
		available = (available < BUFSIZE) ? available : BUFSIZE;

		infile.read((char *)q, available);
		r = available;
		if (infile.fail()) {
			r = 0;
			int err = errno;
			switch (err) {
			case EACCES:
				fprintf(stderr, "%s: r = %d, errno = EACCES\n", __func__, (int)r);
				break;
			case ENOENT:
				fprintf(stderr, "%s: r = %d, errno = ENOENT\n", __func__, (int)r);
				break;
			default:
				if (err) fprintf(stderr, "%s: r = %d, errno = %d\n", __func__, (int)r, err);
				break;
			}
			f_kill_thread = true;
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
#endif

void *feed::stdin_feed_thread()
{
	ssize_t r;
#if FEED_BUFFER
	void *q = NULL;
#else
	unsigned char q[BUFSIZE];
#endif
	int available;

	dPrintf("()");

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

void *feed::pull_thread()
{
	dPrintf("()");

	while (!f_kill_thread)
		if ((m_pull_iface) &&
		    (0 >= m_pull_iface->pull()))
			usleep(50*1000);

	pthread_exit(NULL);
}

void *feed::tcp_client_feed_thread()
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

	dPrintf("(sock_fd=%d)", fd);

//	getpeername(fd, (struct sockaddr*)&tcpsa, &salen);

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
	char q[188*7];
#endif
	int available;

	dPrintf("(sock_fd=%d)", fd);

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
	close_file();
	pthread_exit(NULL);
}

int feed::start_stdin()
{
	dPrintf("()");

	if (NULL == freopen(NULL, "rb", stdin)) {
		fprintf(stderr, "failed to open stdin!\n");
		return -1;
	}
	__log_printf(stderr, "%s: using STDIN\n", __func__);
	strncpy(filename, "STDIN", sizeof(filename));

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

int feed::start_socket(char* source, char* net_if)
{
	dPrintf("()");
#if 0
	struct sockaddr_in ip_addr;
#endif
	char *ip, *portnum, *save;
	uint16_t port = 0;
	bool b_tcp = false;
	bool b_udp = false;
	int ret;

	dPrintf("(<--%s)", source);
	size_t len = strlen(source);
	strncpy(filename, source, sizeof(filename)-1);
	filename[len < sizeof(filename) ? len : sizeof(filename)-1] = '\0';

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
		if (inet_pton(AF_INET, ip, &ip_addr.sin_addr) == 0) {
			perror("ip address translation failed");
			return -1;
		} else
			ringbuffer.reset();
#endif
		ret = (b_tcp) ? start_tcp_listener(port) :
		      ((ip) && (net_if)) ? start_udp_listener(port, ip, net_if) :
		      start_udp_unbound_listener(port, ip);
#if 0
	} else {
		perror("socket failed");
		return -1;
	}
#endif
	dPrintf("~(-->%s)", source);
	return ret;
}

void feed::add_tcp_feed(int socket)
{
	struct sockaddr_in tcpsa;
	socklen_t salen = sizeof(tcpsa);

	dPrintf("(%d)", socket);
	if (fd >= 0) {
		dPrintf("(%d) this build only supports one tcp input feed connection at a time", socket);
		close(socket);
		return;
	}

	if (!strlen(filename))
		sprintf(filename, "TCPSOCKET: %d", socket);

	fd = socket;

	f_kill_thread = false;

	if (0 != getpeername(fd, (struct sockaddr*)&tcpsa, &salen)) {
		perror("getpeername() failed");
		goto fail_close_file;
	}

	if (0 != pthread_create(&h_thread, NULL, tcp_client_feed_thread, this)) {
		perror("pthread_create() failed");
		goto fail_close_file;
#if FEED_BUFFER
	} else {
		start_feed();
#endif
	}
	return;
fail_close_file:
	close_file();
	return;
}

int feed::start_tcp_listener(uint16_t port_requested)
{
	dPrintf("(%d)", port_requested);
	sprintf(filename, "TCPLISTEN: %d", port_requested);

	f_kill_thread = false;

	fd = -1;

	listener.set_interface(this);

	return listener.start(port_requested);
}

int feed::start_udp_unbound_listener(uint16_t port_requested, char *ip)
{
	struct sockaddr_in udp_sock;

	dPrintf("(%d)", port_requested);
	sprintf(filename, "UDPLISTEN: %d", port_requested);

	memset(&udp_sock, 0, sizeof(udp_sock));

	f_kill_thread = false;

	fd = -1;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		perror("open socket failed");
		return fd;
	}

#if defined(_WIN32)
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, "1", 1) < 0) {
#else
	int reuse = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
#endif
		perror("setting reuse failed");
		return -1;
	}

	udp_sock.sin_family = AF_INET;
	udp_sock.sin_port = htons(port_requested);
	udp_sock.sin_addr.s_addr = (ip) ? inet_addr(ip) : INADDR_ANY;

	if (bind(fd, (struct sockaddr*)&udp_sock, sizeof(udp_sock)) < 0) {
		perror("bind to local interface failed");
		return -1;
	}
	//	port = port_requested;
#if 0
	socket_set_nbio(fd);
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

int feed::start_udp_listener(uint16_t port_requested, char *ip, char *net_if)
{
	dPrintf("(%d)", port_requested);
	snprintf(filename, sizeof(filename), "UDPLISTEN: %s:%d %s", ip, port_requested, net_if);

	f_kill_thread = false;

	fd = -1;

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		perror("open socket failed");
		return fd;
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
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&imreq, sizeof(imreq)) < 0) {
#else
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imreq, sizeof(imreq)) < 0) {
#endif
		perror("setting IPPROTO_IP / IP_ADD_MEMBERSHIP failed");
		return -1;
	}

	if (bind(fd, (struct sockaddr*)&sock, sizeof(sock)) < 0) {
		perror("bind to specified interface failed");
		return -1;
	}
#if 0
	socket_set_nbio(fd);
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

/*****************************************************************************/

feed_server::feed_server()
  : m_iface(NULL)
{
	dPrintf("()");

	clear_feeders();
}

feed_server::~feed_server()
{
	dPrintf("()");

	listener.stop();

	clear_feeders();
}

void feed_server::add_tcp_feed(int socket)
{
	if (socket >= 0) {
		dPrintf("(%d)", socket);

		if (feeders.count(socket)) delete feeders[socket];
		feeders[socket] = new feed;


		feeders[socket]->add_tcp_feed(socket);

		if (m_iface) m_iface->add_feeder(feeders[socket]);
	}
	return;
}

void feed_server::clear_feeders()
{
	for (feed_map::iterator it = feeders.begin(); it != feeders.end(); ++it)
		delete it->second;

	feeders.clear();
	return;
}

int feed_server::start_tcp_listener(uint16_t port_requested, feed_server_iface *iface)
{
	dPrintf("(%d)", port_requested);

	/* set listener callback to notify us (feed_server) of new connections */
	listener.set_interface(this);

	/* set connection notify callback to notify parent server of new feeds */
	m_iface = iface;

	return listener.start(port_requested);
}

int feed_server::start_udp_listener(uint16_t port_requested, feed_server_iface *iface)
{
	if (feeders.count(0)) delete feeders[0];
	feeders[0] = new feed;

	int ret = feeders[0]->start_udp_unbound_listener(port_requested);
	if (ret < 0)
		goto fail;

	/* call connection notify callback to notify parent server of new feed */
	if (iface) iface->add_feeder(feeders[0]);
fail:
	return ret;
}

