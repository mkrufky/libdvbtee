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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

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

void* serve::serve_thread()
{
	close_socket();
	pthread_exit(NULL);
}

int serve::start()
{
	dprintf("()");

	f_kill_thread = false;

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
