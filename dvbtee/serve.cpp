/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
 *
 * Author: Michael Krufky <mkrufky@linuxtv.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>

#include "serve.h"

//FIXME:
#define DBG_SERVE 1

unsigned int dbg_serve = DBG_SERVE;

#define __printf(fd, fmt, arg...) fprintf(fd, fmt, ##arg)

#define __dprintf(lvl, fmt, arg...) do {			  \
    if (dbg_serve & lvl)						  \
      __printf(stderr, "%s: " fmt "\n", __func__, ##arg);	  \
  } while (0)

#define dprintf(fmt, arg...) __dprintf(DBG_SERVE, fmt, ##arg)

serve::serve()
  : f_kill_thread(false)
  , sock_fd(-1)
  , port(0)
{
	dprintf("()");
}

serve::~serve()
{
	dprintf("()");

	close_socket();
}

void serve::close_socket()
{
	dprintf("()");

	stop();

	if (sock_fd >= 0) {
		close(sock_fd);
		sock_fd = -1;
	}
	port = 0;
}

#if 0
serve::serve(const serve&)
{
	dprintf("(copy)");
}

serve& serve::operator= (const serve& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	return *this;
}
#endif

//static
void* serve::serve_thread(void *p_this)
{
	return static_cast<serve*>(p_this)->serve_thread();
}

#define MAX_SOCKETS 4

void* serve::serve_thread()
{
	struct sockaddr_in tcpsa;
	socklen_t salen = sizeof(tcpsa);
	int i, rxlen = 0;
	int sock[MAX_SOCKETS];

	dprintf("(sock_fd=%d)", sock_fd);

	for (i = 0; i < MAX_SOCKETS; i++)
		sock[i] = -1;

	while (!f_kill_thread) {
		int d = accept(sock_fd, (struct sockaddr*)&tcpsa, &salen);
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
				char buf[256] = { 0 };
				rxlen = recv(sock[i], buf, sizeof(buf), MSG_DONTWAIT);
				if (rxlen > 0) {
					getpeername(sock[i], (struct sockaddr*)&tcpsa, &salen);
					fprintf(stderr, "%s: %s\n", __func__, buf);
					//process
				} else if ( (rxlen == 0) || ( (rxlen == -1) && (errno != EAGAIN) ) ) {
					close(sock[i]);
					sock[i] = -1;
				}
			}
	}

	close_socket();
	pthread_exit(NULL);
}

int serve::start(uint16_t port_requested)
{
	struct sockaddr_in tcp_sock;

	dprintf("()");

	memset(&tcp_sock, 0, sizeof(tcp_sock));

	f_kill_thread = false;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("open socket failed");
		return sock_fd;
	}

	int reuse = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		perror("setting reuse failed");
		return -1;
	}

	tcp_sock.sin_family = AF_INET;
	tcp_sock.sin_port = htons(port_requested);
	tcp_sock.sin_addr.s_addr = INADDR_ANY;


	if (bind(sock_fd, (struct sockaddr*)&tcp_sock, sizeof(tcp_sock)) < 0) {
		perror("bind to local interface failed");
		return -1;
	}
	port = port_requested;

	int fl = fcntl(sock_fd, F_GETFL, 0);
	if (fcntl(sock_fd, F_SETFL, fl | O_NONBLOCK) < 0) {
		perror("set non-blocking failed");
		return -1;
	}
	listen(sock_fd, 4);

	int ret = pthread_create(&h_thread, NULL, serve_thread, this);

	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

void serve::stop()
{
	dprintf("()");

	stop_without_wait();

	while (-1 != sock_fd) {
		usleep(20*1000);
	}
	return;
}

#if 0
int serve::push(uint8_t* p_data)
{
	return 0;
}
#endif

bool serve::command(char* cmdline)
{
	char* cmd = strtok(cmdline, ":");
	if (!cmd)
		return false;

	char* arg = strtok(cmdline, ":");
#if 0
	if (strstr(cmd, "channel")) {
		int channel = atoi(arg);
		fprintf(stderr, "TUNE to channel %d...\n", channel);
		if (tuner.open_fe() < 0)
			return false;
		if (!scan_flags)
			scan_flags = SCAN_VSB;

		if (tuner.tune_channel((scan_flags == SCAN_VSB) ? VSB_8 : QAM_256, channel)) {

			if (!tuner.wait_for_lock_or_timeout(2000)) {
				tuner.close_fe();
				return false; /* NO LOCK! */
			}
			tuner.feeder.parser.set_channel_info(channel,
							     (scan_flags == SCAN_VSB) ? atsc_vsb_chan_to_freq(channel) : atsc_qam_chan_to_freq(channel),
							     (scan_flags == SCAN_VSB) ? "8VSB" : "QAM256");
			tuner.start_feed();
		}

	} else if (strstr(cmd, "stream")) {
		tuner.feeder.parser.add_output(arg);
	} else if (strstr(cmd, "stopfeed")) {
		tuner.stop_feed();
		tuner.close_fe();
	}
#endif
	return true;
}
