/*****************************************************************************
 * Copyright (C) 2013-2014 Michael Ira Krufky
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

#include "hlsfeed.h"

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

#define HLS_BUFSIZE_MB(x) ((1024 * 1024 * x) / 188) * 188
#define HLS_BUFSIZE HLS_BUFSIZE_MB(8)

#define PUSH_THREAD 1
#define WALK_THREAD 1

class BadConversion : public std::runtime_error {
public:
  BadConversion(std::string const& s)
    : std::runtime_error(s)
  { }
};

inline double convertToDouble(std::string const& s)
{
  std::istringstream i(s);
  double x;
  if (!(i >> x))
    throw BadConversion("convertToDouble(\"" + s + "\")");
  return x;
}

hlsfeed::hlsfeed(const char *url, curlhttpget_iface *iface)
  : toplevel(url)
  , Url(url)
  , m_iface(iface)
  , push_buffer()
  , walk_buffer()
  , f_kill_thread(false)
{
  int ret;
#if PUSH_THREAD
  push_buffer.set_capacity(HLS_BUFSIZE);
  push_buffer.reset();
  ret = pthread_create(&h_push_thread, NULL, push_thread, this);
  if (0 != ret) {
    perror("pthread_create() failed to create push_thread");
    return;
  }
#endif
#if WALK_THREAD
  walk_buffer.set_capacity(1024 * 1024 * 1);
  walk_buffer.reset();
#endif
  walk((uint8_t*)Url.c_str());
#if WALK_THREAD
  sleep(1);
  ret = pthread_create(&h_walk_thread, NULL, walk_thread, this);
  if (0 != ret) {
    perror("pthread_create() failed to create walk_thread");
    return;
  }
  while (walk_buffer.get_size()) sleep(1);
#endif
}

class pushwalk_iface : public curlhttpget_iface
{
public:
#define PUSHWALK_WALK 0
#define PUSHWALK_PUSH 1
	pushwalk_iface(hlsfeed &parent, int pushwalk) : curlhttpget_iface(), m_parent(parent), m_pushwalk(pushwalk) {}

	void write_data(void *buffer, size_t size, size_t nmemb)
	{
		switch(m_pushwalk) {
		case PUSHWALK_WALK:
			m_parent.walk((uint8_t*)buffer, size, nmemb);
			break;
		case PUSHWALK_PUSH:
			m_parent.push((uint8_t*)buffer, size, nmemb);
			break;
		}
	}
private:
	hlsfeed &m_parent;
	int m_pushwalk;
};

void hlsfeed::walk(uint8_t *buffer)
{
  char *save;
  char *playlist = (char *)buffer;
  char *line = strtok_r(playlist, "\n", &save);
  double duration;
  curlhttpget_info_t info = { 0 };
  pushwalk_iface push_iface(*this, PUSHWALK_PUSH);
  pushwalk_iface walk_iface(*this, PUSHWALK_WALK);

  while (line) {
    //if (line[0] == '#')
    if (strstr(line, "#EXTINF:")) {
      char *saveToo;
      char *durationText = strtok_r(line, ":", &saveToo);
      durationText = strtok_r(NULL, ",", &saveToo);
      try {
	duration = convertToDouble(std::string(durationText));
      }
      catch (BadConversion& e) {
	perror(e.what());
	duration = 0;
      }
      fprintf(stderr, "%s: playback duration: %f\n", __func__, duration);
    } else if (strstr(line, ".ts")) {
      curlhttpget(line, &push_iface, &info);
    } else if (strstr(line, ".m3u8")) {
      curlhttpget(line, &walk_iface);
    } else if (!strstr(line, "#EXT"))
      fprintf(stderr, "%s: invalid line: '%s'\n", __func__, line);

    line = strtok_r(NULL, "\n", &save);
  }
}

//static
void* hlsfeed::push_thread(void *p_this)
{
	return static_cast<hlsfeed*>(p_this)->push_thread();
}

//static
void* hlsfeed::walk_thread(void *p_this)
{
	return static_cast<hlsfeed*>(p_this)->walk_thread();
}

void* hlsfeed::push_thread()
{
	uint8_t *data = NULL;
	int buf_size;

	/* push data from hlsfeed buffer */
	while (!f_kill_thread) {

		buf_size = push_buffer.get_size();
		if (!buf_size) {
			usleep(200);
			continue;
		}

		buf_size = push_buffer.get_read_ptr((void**)&data, buf_size);
		buf_size /= 188;
		buf_size *= 188;

		if (m_iface) m_iface->write_data(data, 188, buf_size / 188);

		push_buffer.put_read_ptr(buf_size);
	}
	pthread_exit(NULL);
}

void* hlsfeed::walk_thread()
{
	uint8_t *data = NULL;
	int buf_size;

	/* push data from hlsfeed buffer */
	while (!f_kill_thread) {

		buf_size = walk_buffer.get_size();
		if (!buf_size) {
			usleep(200);
			continue;
		}

		buf_size = walk_buffer.get_read_ptr((void**)&data, buf_size);

		walk(data);

		walk_buffer.put_read_ptr(buf_size);
	}
	pthread_exit(NULL);
}

void hlsfeed::push(uint8_t *buffer, size_t size, size_t nmemb)
{
#if PUSH_THREAD
  if (!push_buffer.write(buffer, size * nmemb)) {
    while (nmemb) {
      if (push_buffer.write(buffer, nmemb)) {
	buffer += size;
	nmemb--;
      } else {
	fprintf(stderr, "%s: FAILED: %zu packets dropped\n", __func__, nmemb);
	return;
      }
    }
  }
#else
  if (datapump_cb)
    datapump_cb(datapump_ctxt, buffer, size, nmemb);
#endif
}

void hlsfeed::walk(uint8_t *buffer, size_t size, size_t nmemb)
{
#if WALK_THREAD
  if (!walk_buffer.write(buffer, size * nmemb)) {
    while (nmemb) {
      if (walk_buffer.write(buffer, nmemb)) {
	buffer += size;
	nmemb--;
      } else {
	fprintf(stderr, "%s: FAILED: %zu bytes dropped\n", __func__, size * nmemb);
	return;
      }
    }
  }
#else
  walk(buffer);
#endif
}
