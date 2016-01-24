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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

#include "dvbtee_config.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include "output.h"
#include "log.h"
#define CLASS_MODULE "out"

/* MSG_NOSIGNAL does not exists on OS X / Windows */
#if defined(__APPLE__) || defined(__MACH__) || defined(_WIN32)
# if defined(_WIN32)
#  ifndef SO_NOSIGPIPE
#   define SO_NOSIGPIPE 0
#  endif
# endif
# ifndef MSG_NOSIGNAL
#   define MSG_NOSIGNAL SO_NOSIGPIPE
# endif
#endif

#define dPrintf(fmt, arg...) __dPrintf(DBG_OUTPUT, fmt, ##arg)

#define DOUBLE_BUFFER 0
#define PREVENT_RBUF_DEADLOCK 0
#define NON_BLOCKING_TCP_SEND 1

#define HTTP_200_OK  "HTTP/1.1 200 OK"
#define CONTENT_TYPE "Content-type: "
#define TEXT_HTML    "text/html"
#define TEXT_PLAIN   "text/plain"
#define OCTET_STREAM "application/octet-stream"
#define ENC_CHUNKED  "Transfer-Encoding: chunked"
#define CONN_CLOSE   "Connection: close"
#define CRLF         "\r\n"

#if 0
static char http_conn_close[] =
	CONN_CLOSE
	CRLF
	CRLF;
#endif

int hostname_to_ip(char *hostname, char *ip, size_t sizeof_ip = 0)
{
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in *h;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(hostname, NULL, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return -1;
	}

	/* loop through all the results and connect to the first we can */
	for (p = servinfo; p != NULL; p = p->ai_next) {
		h = (struct sockaddr_in*) p->ai_addr;
		if (sizeof_ip)
			strncpy(ip, inet_ntoa(h->sin_addr), sizeof_ip);
		else
			strcpy(ip, inet_ntoa(h->sin_addr));
	}

	freeaddrinfo(servinfo);
	return 0;
}


static const std::string __http_response(const char *mimetype)
{
	std::string str;
	str.clear();

	str.append(HTTP_200_OK
		   CRLF);
	if (mimetype) {
		str.append(CONTENT_TYPE);
		str.append(mimetype);
		str.append(CRLF
			   ENC_CHUNKED
			   CRLF);
	}
	str.append(CRLF);

	return str;
}

const std::string http_response(enum output_mimetype mimetype)
{
	const char *str;

	switch (mimetype) {
	default:
	case MIMETYPE_OCTET_STREAM:
		str = OCTET_STREAM;
		break;
	case MIMETYPE_TEXT_PLAIN:
		str = TEXT_PLAIN;
		break;
	case MIMETYPE_TEXT_HTML:
		str = TEXT_HTML;
		break;
	case MIMETYPE_NONE:
		str = NULL;
		break;
	}

	return __http_response(str);
}

ssize_t socket_send(int sockfd, const void *buf, size_t len, int flags,
		    const struct sockaddr *dest_addr, socklen_t addrlen)
{
	if (sockfd < 0)
		return sockfd;
	int ret = 0;
	while (0 >= ret) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);

		struct timeval sel_timeout = {0, 10000};

		ret = select(sockfd + 1, NULL, &fds, NULL, &sel_timeout);
		if (ret == -1) {
			perror("error sending data to socket!");
			/*
			switch (errno) {
			case EBADF:  // invalid file descriptor
			case EINTR:  // signal caught
			case EINVAL: // negative fd or invalid timeout
			case ENOMEM: // unable to allocate memory
			}
			*/
			break;
		}
	}
	return (ret > 0) ?
#if 0
		((dest_addr) && (addrlen)) ?
			sendto(sockfd, buf, len, flags, dest_addr, addrlen) :
			send(sockfd, buf, len, flags) :
#else
		sendto(sockfd, (const char *)buf, len, flags|MSG_NOSIGNAL, dest_addr, addrlen) :
#endif
		ret;
}

static inline ssize_t stream_crlf(int socket)
{
	return socket_send(socket, CRLF, 2, 0);
}

int stream_http_chunk(int socket, const uint8_t *buf, size_t length, const bool send_zero_length)
{
	if (socket < 0)
		return socket;
#if DBG
	dPrintf("(length:%d)", (int)length);
#endif
	if ((length) || (send_zero_length)) {
		int ret = 0;
		char sz[8] = { 0 };
		sprintf(sz, "%x\r\n", (unsigned int)length);

		ret = socket_send(socket, sz, strlen(sz), 0);
		if (ret < 0)
			return ret;
#if 0
		ret = stream_crlf(socket);
		if (ret < 0)
			return ret;
#endif
		if (length) {
			ret = socket_send(socket, buf, length, 0);
			if (ret < 0)
				return ret;

			ret = stream_crlf(socket);
			if (ret < 0)
				return ret;
		}
	}
	return 0;
}

static inline size_t write_stdout(uint8_t* p_data, int size) {
	return 188 * fwrite(p_data, 188, size / 188, stdout);
}

class output_stream_priv
{
	output_stream_priv()
	{
		reset();
	}

	void reset()
	{
		memset(&ip_addr, 0, sizeof(ip_addr));
	}

friend class output_stream;
	struct sockaddr_in ip_addr;
};

output_stream::output_stream()
  :
#if !defined(_WIN32)
    h_thread((pthread_t)NULL)
  ,
#endif
    f_kill_thread(false)
  , f_streaming(false)
  , sock(-1)
  , mimetype(MIMETYPE_OCTET_STREAM)
  , ringbuffer()
  , stream_method(OUTPUT_STREAM_UDP)
  , count_in(0)
  , count_out(0)
  , m_iface(NULL)
  , stream_cb(NULL)
  , stream_cb_priv(NULL)
  , have_pat(false)
  , priv(NULL)
{
	dPrintf("()");
	memset(&name, 0, sizeof(name));
	pids.clear();
}

output_stream::~output_stream()
{
	dPrintf("(%d)", sock);

	stop();

	dPrintf("(stream) %lu packets in, %lu packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
}

#if 1
output_stream::output_stream(const output_stream&)
{
	dPrintf("(copy)");
#if !defined(_WIN32)
	h_thread = (pthread_t)NULL;
#endif
	f_kill_thread = false;
	f_streaming = false;
	m_iface = NULL;
	stream_cb = NULL;
	stream_cb_priv = NULL;
	count_in = 0;
	count_out = 0;
	sock = -1;
	mimetype = MIMETYPE_OCTET_STREAM;
	stream_method = OUTPUT_STREAM_UDP;
	memset(&name, 0, sizeof(name));
	if (priv) priv = NULL;
	pids.clear();
	have_pat = false;
}

output_stream& output_stream::operator= (const output_stream& cSource)
{
	dPrintf("(operator=)");

	if (this == &cSource)
		return *this;

#if !defined(_WIN32)
	h_thread = (pthread_t)NULL;
#endif
	f_kill_thread = false;
	f_streaming = false;
	stream_cb = NULL;
	stream_cb_priv = NULL;
	count_in = 0;
	count_out = 0;
	sock = -1;
	mimetype = MIMETYPE_OCTET_STREAM;
	stream_method = OUTPUT_STREAM_UDP;
	memset(&name, 0, sizeof(name));
	if (priv) priv = NULL;
	pids.clear();
	have_pat = false;

	return *this;
}
#endif

//static
void* output_stream::output_stream_thread(void *p_this)
{
	return static_cast<output_stream*>(p_this)->output_stream_thread();
}

#define OUTPUT_STREAM_PACKET_SIZE ((stream_method == OUTPUT_STREAM_UDP) ? 188*7 : 188*21)
void* output_stream::output_stream_thread()
{
	uint8_t *data = NULL;
	int buf_size, ret = 0;

	dPrintf("(%d)", sock);
#if 1
	switch (stream_method) {
	case OUTPUT_STREAM_HTTP:
		std::string str = http_response(mimetype);
		ret = socket_send(sock, str.c_str(), str.length(), 0);
		break;
	}
	if (ret < 0) {
		perror("stream header failed");
		goto fail;
	}

#endif
	f_streaming = true;

	/* push data from output_stream buffer to target */
	while (!f_kill_thread) {

		buf_size = ringbuffer.get_size();
		if (buf_size < OUTPUT_STREAM_PACKET_SIZE) {
			usleep(1000);
			continue;
		}

		buf_size = ringbuffer.get_read_ptr((void**)&data, OUTPUT_STREAM_PACKET_SIZE);
		buf_size /= 188;
		buf_size *= 188;

#if PREVENT_RBUF_DEADLOCK
		{
			uint8_t newdata[buf_size];
			memcpy(newdata, data, buf_size);
			ringbuffer.put_read_ptr(buf_size);
			stream(newdata, buf_size);
		}
#else
		stream(data, buf_size);

		ringbuffer.put_read_ptr(buf_size);
#endif
		count_out += buf_size;
#if 0
		dPrintf("(thread-stream) %d packets in, %d packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
#endif
	}

	f_streaming = false;
#if 0
	switch (stream_method) {
	case OUTPUT_STREAM_HTTP:
		if (stream_http_chunk(sock, (uint8_t *)"", 0, true) < 0)
			perror("stream empty http chunk failed");
		else if (socket_send(sock, http_conn_close, strlen(http_conn_close), 0) < 0)
			perror("stream http connection close failed");
		break;
	}
#endif
fail:
	close_file();
	pthread_exit(NULL);
}

int output_stream::start()
{
	if (f_streaming) {
		dPrintf("(%d) already streaming", sock);
		return 0;
	}
	if ((sock < 0) &&
	    ((stream_method != OUTPUT_STREAM_FUNC) &&
	     (stream_method != OUTPUT_STREAM_INTF) &&
	     (stream_method != OUTPUT_STREAM_STDOUT)))
		return sock;

	dPrintf("(%d)", sock);

	ringbuffer.set_capacity(OUTPUT_STREAM_BUF_SIZE);

	int ret = pthread_create(&h_thread, NULL, output_stream_thread, this);
	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

bool output_stream::drain()
{
	dPrintf("(%d)", sock);

	if (!f_streaming)
		return false;

	while ((f_streaming) && (ringbuffer.get_capacity()))
		usleep(20*1000);

#if !defined(_WIN32)
	fsync(sock);
#endif

	return (!f_streaming);
}

void output_stream::stop()
{
	dPrintf("(%d)", sock);

	stop_without_wait();

	while (f_streaming)
		usleep(20*1000);

	return;
}

bool output_stream::check()
{
	bool ret = (f_streaming | ((0 == count_in + count_out)));
	if (!ret)
		dPrintf("(%d: %s) not streaming!", sock, name);
	else {
		dPrintf("(%d: %s) %s %lu in, %lu out",
			sock, name,
			(stream_method == OUTPUT_STREAM_UDP) ? "UDP" :
			(stream_method == OUTPUT_STREAM_TCP) ? "TCP" :
			(stream_method == OUTPUT_STREAM_HTTP) ? "HTTP" :
			(stream_method == OUTPUT_STREAM_FILE) ? "FILE" :
			(stream_method == OUTPUT_STREAM_FUNC) ? "FUNC" :
			(stream_method == OUTPUT_STREAM_INTF) ? "INTF" :
			(stream_method == OUTPUT_STREAM_STDOUT) ? "STDOUT" : "UNKNOWN",
			count_in / 188, count_out / 188);
#if 1//DBG
		if (pids.size()) {
			dPrintf("(%d: %s) subscribed to the following pids:", sock, name);
			for (map_pidtype::const_iterator iter = pids.begin(); iter != pids.end(); ++iter)
				fprintf(stderr, "%d, ", iter->first);
			fprintf(stderr, "\n");
		}
#endif
	}
	ringbuffer.check();

	return ret;
}

bool output_stream::push(uint8_t* p_data, int size)
{
#if TUNER_RESOURCE_SHARING
	if ((0 == (((p_data[1] & 0x1f) << 8) | p_data[2])) && (188 == size)) {
		if (!have_pat) {
			memcpy(pat_pkt, p_data, 188);
			have_pat = true;
		}
		if (ringbuffer.write(pat_pkt, 188)) {
			count_in += 188;
			return true;
		} else {
			fprintf(stderr, "%s> FAILED: PAT Table (%d bytes) dropped\n", __func__, 188);
		}
	} else if (want_pkt(p_data)) {
#else
	if ((0 == (((p_data[1] & 0x1f) << 8) | p_data[2])) || (want_pkt(p_data))) {
#endif
	/* push data into output_stream buffer */
	if (!ringbuffer.write(p_data, size))
		while (size >= 188)
			if (ringbuffer.write(p_data, 188)) {
				p_data += 188;
				size -= 188;
				count_in += 188;
			} else {
				fprintf(stderr, "%s> FAILED: %d bytes dropped\n", __func__, size);
#if 0
				dPrintf("(push-false-stream) %d packets in, %d packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
#endif
				return false;
			}
	else count_in += size;
#if 0
	dPrintf("(push-true-stream) %d packets in, %d packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
#endif
	}
	return true;
}

int output_stream::stream(uint8_t* p_data, int size)
{
	int ret = -1;

	if (!priv)
		dPrintf("no priv - should never happen!!!");
	else if ((!p_data) || (!size))
		dPrintf("no data to stream!!!");
	/* stream data to target */
	else switch (stream_method) {
	case OUTPUT_STREAM_UDP:
		ret = socket_send(sock, p_data, size, 0, (struct sockaddr*) &priv->ip_addr, sizeof(priv->ip_addr));
		break;
	case OUTPUT_STREAM_TCP:
		ret = socket_send(sock, p_data, size, 0);
		if (ret < 0) {
			stop_without_wait();
			perror("tcp streaming failed");
		}
		break;
	case OUTPUT_STREAM_FILE:
		ret = write(sock, p_data, size);
		if (ret < 0) {
			stop_without_wait();
			perror("file streaming failed");
		}
		break;
	case OUTPUT_STREAM_HTTP:
		ret = stream_http_chunk(sock, p_data, size);
		if (ret < 0) {
			stop_without_wait();
			perror("http streaming failed");
		}
		break;
	case OUTPUT_STREAM_FUNC:
		if (stream_cb)
			ret = stream_cb(stream_cb_priv, p_data, size);
		if (ret < 0) {
			stop_without_wait();
			perror("streaming via callback failed");
		}
		break;
	case OUTPUT_STREAM_INTF:
		if (m_iface)
			ret = m_iface->stream(p_data, size);
		if (ret < 0) {
			stop_without_wait();
			perror("streaming via interface failed");
		}
		break;
	case OUTPUT_STREAM_STDOUT:
		ret = write_stdout(p_data, size);
		if (ret != size) {
			stop_without_wait();
			perror("dump to stdout failed");
		}
		break;
	}
	return ret;
}

void output_stream::close_file()
{
	dPrintf("(%d, %s)", sock, name);

	if (sock >= 0) {
		close(sock);
		sock = -1;
	}
}

int output_stream::add(void* priv, stream_callback callback, map_pidtype &pids)
{
	stream_cb = callback;
	stream_cb_priv = priv;

	ringbuffer.reset();
	stream_method = OUTPUT_STREAM_FUNC;
	strncpy(name, "FUNC", sizeof(name));
	return set_pids(pids);
}

int output_stream::add(output_stream_iface *iface, map_pidtype &pids)
{
	m_iface = iface;
	ringbuffer.reset();
	stream_method = OUTPUT_STREAM_INTF;
	strncpy(name, "INTF", sizeof(name));
	return set_pids(pids);
}

int output_stream::add(int socket, unsigned int method, map_pidtype &pids)
{
	sock = socket;
	stream_method = method;
	strncpy(name, "SOCKET", sizeof(name));

#if NON_BLOCKING_TCP_SEND
#if (defined(_WIN32) && !defined(HAVE_FCNTL))
	unsigned long mode = 1;  /* 1 to enable non-blocking socket */
	ioctlsocket(sock, FIONBIO, &mode);
#else
	int fl = fcntl(sock, F_GETFL, 0);
	if (fcntl(socket, F_SETFL, fl | O_NONBLOCK) < 0)
		perror("set non-blocking failed");
#endif
#endif
	ringbuffer.reset();
	return set_pids(pids);
}

int output_stream::add_stdout(map_pidtype &pids)
{
	dPrintf("dumping to stdout...");
	ringbuffer.reset();
	stream_method = OUTPUT_STREAM_STDOUT;
	strncpy(name, "STDOUT", sizeof(name));
	return set_pids(pids);
}

int output_stream::add(char* target, map_pidtype &pids)
{
	char *save;
	char *ip = NULL;
	uint16_t port = 0;
	bool b_tcp = false;
	bool b_udp = false;
	bool b_file = false;

	dPrintf("(-->%s)", target);

	size_t len = strlen(target);
	strncpy(name, target, sizeof(name)-1);
	name[len < sizeof(name) ? len : sizeof(name)-1] = '\0';

	if ((0 == strcmp(target, "-")) ||
	    (0 == strcmp(target, "fd://0")) ||
	    (0 == strcmp(target, "fd:/0")))
		return add_stdout(pids);
	else
	if (strstr(target, ":")) {
		ip = strtok_r(target, ":", &save);
		if (strstr(ip, "tcp"))
			b_tcp = true;
		else
		if (strstr(ip, "udp"))
			b_udp = true;
		else
		if (strstr(ip, "file"))
			b_file = true;

		if ((b_tcp) || (b_udp) || (b_file)) {
			ip = strtok_r(NULL, ":", &save);
			if (strstr(ip, "//") == ip)
				ip += 2;
		}
		// else ip = proto;
		if (!b_file)
			port = atoi(strtok_r(NULL, ":", &save));
	} else {
		b_tcp = false;
		ip = target;
		port = 1234;
	}

	if (sock >= 0)
		close(sock);

	if (b_file) {
		dPrintf("opening %s...", ip);
		if ((sock = open(ip, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU)) < 0) {
			perror("file failed");
			return -1;
		} else {
			ringbuffer.reset();
			stream_method = OUTPUT_STREAM_FILE;
			return set_pids(pids);
		}
	}

	sock = socket(AF_INET, (b_tcp) ? SOCK_STREAM : SOCK_DGRAM, (b_tcp) ? IPPROTO_TCP : IPPROTO_UDP);
	if (sock >= 0) {

#if (defined(_WIN32) && !defined(HAVE_FCNTL))
		unsigned long mode = 1;  /* 1 to enable non-blocking socket */
		ioctlsocket(sock, FIONBIO, &mode);
#else
		int fl = fcntl(sock, F_GETFL, 0);
		if (fcntl(sock, F_SETFL, fl | O_NONBLOCK) < 0)
			perror("set non-blocking failed");
#endif

		char resolved_ip[16] = { 0 };
		if (0 == hostname_to_ip(ip, resolved_ip, sizeof(resolved_ip)))
			ip = &resolved_ip[0];

		if (!priv) priv = new output_stream_priv;
		priv->ip_addr.sin_family = AF_INET;
		priv->ip_addr.sin_port   = htons(port);
		if (inet_aton(ip, &priv->ip_addr.sin_addr) == 0) {

			perror("ip address translation failed");
			return -1;
		} else
			ringbuffer.reset();

		if (b_tcp) {
			if ((connect(sock, (struct sockaddr *) &priv->ip_addr, sizeof(priv->ip_addr)) < 0) && (errno != EINPROGRESS)) {
				perror("failed to connect to server");
				return -1;
			}
			stream_method = OUTPUT_STREAM_TCP;
		} else {
			stream_method = OUTPUT_STREAM_UDP;
		}
	} else {
		perror("socket failed");
		return -1;
	}
	dPrintf("~(-->%s)", target);

	return set_pids(pids);
}

int output_stream::set_pids(map_pidtype &new_pids)
{
	for (map_pidtype::const_iterator iter = new_pids.begin(); iter != new_pids.end(); ++iter)
		pids[iter->first] = iter->second;
	return 0;
}

int output_stream::get_pids(map_pidtype &result)
{
	for (map_pidtype::const_iterator iter = pids.begin(); iter != pids.end(); ++iter)
		result[iter->first] = iter->second;
	return 0;
}

/* ----------------------------------------------------------------- */

output::output()
  :
#if !defined(_WIN32)
    h_thread((pthread_t)NULL)
  ,
#endif
    f_kill_thread(false)
  , f_streaming(false)
  , ringbuffer()
  , num_targets(0)
  , options(OUTPUT_NONE)
  , count_in(0)
  , count_out(0)
{
	dPrintf("()");

	output_streams.clear();
}

output::~output()
{
	dPrintf("()");

	stop();

	output_streams.clear();

	dPrintf("(intermediate) %lu packets in, %lu packets out, %d packets remain in rbuf", count_in / 188, count_out / 188, ringbuffer.get_size() / 188);
}

#if 1
output::output(const output&)
{
	dPrintf("(copy)");

#if !defined(_WIN32)
	h_thread = (pthread_t)NULL;
#endif
	f_kill_thread = false;
	f_streaming = false;
	num_targets = 0;
	options = OUTPUT_NONE;
	count_in = 0;
	count_out = 0;

	memset(&ringbuffer, 0, sizeof(ringbuffer));

	output_streams.clear();
}

output& output::operator= (const output& cSource)
{
	dPrintf("(operator=)");

	if (this == &cSource)
		return *this;

#if !defined(_WIN32)
	h_thread = (pthread_t)NULL;
#endif
	f_kill_thread = false;
	f_streaming = false;
	num_targets = 0;
	options = OUTPUT_NONE;
	count_in = 0;
	count_out = 0;

	memset(&ringbuffer, 0, sizeof(ringbuffer));

	output_streams.clear();

	return *this;
}
#endif

//static
void* output::output_thread(void *p_this)
{
	return static_cast<output*>(p_this)->output_thread();
}

void* output::output_thread()
{
	int buf_size;
#if !PREVENT_RBUF_DEADLOCK
	uint8_t *data = NULL;
#endif

	f_streaming = true;

	/* push data from main output buffer into output_stream buffers */
	while (!f_kill_thread) {

		buf_size = ringbuffer.get_size();
		//data = NULL;
		if (buf_size >= 188) {
#if !PREVENT_RBUF_DEADLOCK
			buf_size = ringbuffer.get_read_ptr((void**)&data, buf_size);
			buf_size /= 188;
			buf_size *= 188;
#else
			uint8_t data[buf_size];
			buf_size = ringbuffer.read(data, buf_size);
#endif
			if (buf_size)
			for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter) {
				if (iter->second.is_streaming())
					iter->second.push(data, buf_size);
#if 0
				else {
					dPrintf("erasing idle output stream...");
					output_streams.erase(iter->first);
					dPrintf("garbage collection complete");
				}
#endif
			}
#if !PREVENT_RBUF_DEADLOCK
			ringbuffer.put_read_ptr(buf_size);
#endif
			count_out += buf_size;
		} else {
			usleep(1000);
		}

	}
	f_streaming = false;
	pthread_exit(NULL);
}

void output::add_http_client(int socket)
{
	if (0 > add(socket, OUTPUT_STREAM_HTTP))
		perror("output.add(socket, OUTPUT_STREAM_HTTP) failed");
	else if (0 != start())
		perror("output.start() failed");
	return;
}

int output::add_http_server(int port)
{
	dPrintf("(%d)", port);
	listener.set_interface(this);
	return listener.start(port);
}

int output::get_pids(map_pidtype &result)
{
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.get_pids(result);
	return 0;
}

bool output::check()
{
	dPrintf("()");
	unsigned int dead = 0;
	bool ret = false;

	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter) {
		bool streaming = iter->second.check();
		ret |= streaming;
		if (!streaming)
			dead++;
	}
	if (dead) {
		dPrintf("%d dead streams found", dead);
		reclaim_resources();
	}

#if 1//DBG
	map_pidtype pids;
	get_pids(pids);
	if (pids.size()) {
		dPrintf("subscribed to the following pids:");
		for (map_pidtype::const_iterator iter = pids.begin(); iter != pids.end(); ++iter)
			fprintf(stderr, "%d, ", iter->first);
		fprintf(stderr, "\n");
	}
#endif
	ringbuffer.check();

	return ret;
}

void output::reclaim_resources()
{
	dPrintf("()");
	bool erased = false;

	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter) {
		if (!iter->second.check()) {
			dPrintf("erasing idle output stream...");
			output_streams.erase(iter->first);
			/* stop the loop if we erased any targets */
			erased = true;
			break;
		}
	}
	/* if we erased a target, restart the above by re-calling this function recursively */
	if (erased)
		reclaim_resources();
}

int output::start()
{
	dPrintf("()");

#if DOUBLE_BUFFER
	int ret = 0;

	if (output_streams.size() <= 1)
		goto nobuffer;

	if (f_streaming) {
		dPrintf("() already streaming!");
		goto nobuffer;
	}
	ret = pthread_create(&h_thread, NULL, output_thread, this);

	if (0 != ret) {
		perror("pthread_create() failed");
		goto fail;
	}

	/* allocates out buffer if and only if we have begun streaming output */
	if (ringbuffer.get_capacity() <= 0)
		ringbuffer.set_capacity(OUTPUT_STREAM_BUF_SIZE*2);
nobuffer:
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.start();
fail:
	return ret;
#else
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.start();
	return 0;
#endif
}

void output::stop()
{
	dPrintf("()");

	stop_without_wait();

	/* call stop_without_wait() on everybody first before we call stop() on everybody, which is a blocking function */
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.stop_without_wait();

	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		iter->second.stop();

	while (f_streaming)
		usleep(20*1000);

	return;
}

void output::stop(int id)
{
	dPrintf("(%d)", id);

	if (output_streams.count(id))
		output_streams[id].stop();
	else
		dPrintf("no such stream id: %d", id);

	return;
}

bool output::push(uint8_t* p_data, int size)
{
	bool ret = true;

	if (ringbuffer.get_capacity()) {

		/* push data into output buffer */
		ret = ringbuffer.write(p_data, size);
		if (!ret)
			fprintf(stderr, "%s: FAILED: %d bytes dropped\n", __func__, size);
	} else for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter) {
		if (iter->second.is_streaming())
			iter->second.push(p_data, size);
#if 0
		else {
			dPrintf("erasing idle output stream...");
			output_streams.erase(iter->first);
			dPrintf("garbage collection complete");
		}
#endif
	}
	count_in += size;

	return ret;
}

bool output::push(uint8_t* p_data, enum output_options opt)
{
	return (((!options) || (!opt)) || (opt & options)) ? push(p_data, 188) : false;
}

int output::add_stdout(map_pidtype &pids)
{
	int target_id = num_targets;
	/* push data into output buffer */
	int ret = output_streams[target_id].add_stdout(pids);
	if (ret == 0)
		num_targets++;
	else
		dPrintf("failed to add target #%d", target_id);

	dPrintf("~(%d->STDOUT)", target_id);

	return (ret == 0) ? target_id : ret;
}

int output::add(void* priv, stream_callback callback, map_pidtype &pids)
{
	if ((callback) && (priv)) {

		int search_id = search(priv, callback);
		if (search_id >= 0) {
			dPrintf("target callback already exists #%d", search_id);
			return search_id;
		}
		int target_id = num_targets;
		/* push data into output buffer */
		int ret = output_streams[target_id].add(priv, callback, pids);
		if (ret == 0)
			num_targets++;
		else
			dPrintf("failed to add target #%d", target_id);

		dPrintf("~(%d->FUNC)", target_id);

		return (ret == 0) ? target_id : ret;
	}
	return -1;
}

int output::add(output_stream_iface *iface, map_pidtype &pids)
{
	if (iface) {

		int search_id = search(iface);
		if (search_id >= 0) {
			dPrintf("target interface already exists #%d", search_id);
			return search_id;
		}
		int target_id = num_targets;
		/* push data into output buffer */
		int ret = output_streams[target_id].add(iface, pids);
		if (ret == 0)
			num_targets++;
		else
			dPrintf("failed to add target #%d", target_id);

		dPrintf("~(%d->INTF)", target_id);

		return (ret == 0) ? target_id : ret;
	}
	return -1;
}

int output::add(int socket, unsigned int method, map_pidtype &pids)
{
	if (socket >= 0) {

		int search_id = search(socket, method);
		if (search_id >= 0) {
			dPrintf("target socket already exists #%d", search_id);
			return search_id;
		}
		int target_id = num_targets;
		/* push data into output buffer */
		int ret = output_streams[num_targets].add(socket, method, pids);
		if (ret == 0)
			num_targets++;
		else
			dPrintf("failed to add target #%d", target_id);

		dPrintf("~(%d->SOCKET[%d])", target_id, socket);

		return (ret == 0) ? target_id : ret;
	}
	return -1;
}

#define CHAR_CMD_COMMA ","

int output::add(char* target, map_pidtype &pids)
{
	char *save;
	int ret = -1;
	char *item = strtok_r(target, CHAR_CMD_COMMA, &save);
	if (item) while (item) {

		ret = __add(item, pids);
		if (ret < 0)
			return ret;

		item = strtok_r(NULL, CHAR_CMD_COMMA, &save);
	} else
		ret = __add(target, pids);

	return ret;
}

int output::__add(char* target, map_pidtype &pids)
{
	int search_id = search(target);
	if (search_id >= 0) {
		dPrintf("target already exists #%d: %s", search_id, target);
		return search_id;
	}
	int target_id = num_targets;

	dPrintf("(%d->%s)", target_id, target);

	/* push data into output buffer */
	int ret = output_streams[target_id].add(target, pids);
	if (ret == 0)
		num_targets++;
	else
		dPrintf("failed to add target #%d: %s", target_id, target);

	dPrintf("~(%d->%s)", target_id, target);

	return (ret == 0) ? target_id : ret;
}

void output::reset_pids(int target_id)
{
	if (output_streams.count(target_id))
		output_streams[target_id].reset_pids();
	else if (-1 == target_id)
		for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
			iter->second.reset_pids();
}

int output::search(void* priv, stream_callback callback)
{
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		if (iter->second.verify(priv, callback)) return iter->first;
	return -1;
}

int output::search(output_stream_iface *iface)
{
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		if (iter->second.verify(iface)) return iter->first;
	return -1;
}

int output::search(int socket, unsigned int method)
{
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		if (iter->second.verify(socket, method)) return iter->first;
	return -1;
}

int output::search(char* target)
{
	for (output_stream_map::iterator iter = output_streams.begin(); iter != output_streams.end(); ++iter)
		if (iter->second.verify(target)) return iter->first;
	return -1;
}
