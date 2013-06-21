/*****************************************************************************
 * Copyright (C) 2013 Michael Krufky
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

hlsfeed::hlsfeed(const char *url, hls_curl_http_get_data_callback data_pump_callback, void *data_pump_context)
  : toplevel(url)
  , Url(url)
  , datapump_cb(data_pump_callback)
  , datapump_ctxt(data_pump_context)
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

void hlsfeed::walk(uint8_t *buffer)
{
  char *save;
  char *playlist = (char *)buffer;
  char *line = strtok_r(playlist, "\n", &save);
  double duration;
  while (line) {
    //if (line[0] == '#')
    if (strstr(line, "#EXTINF:")) {
      char *saveToo;
      char *durationText = strtok_r(line, ":", &saveToo);
      durationText = strtok_r(NULL, ",", &saveToo);
      duration = convertToDouble(std::string(durationText));
    } else if (strstr(line, ".ts"))
      curlhttpget Curl(line, curl_push_callback, this);
    else if (strstr(line, ".m3u8"))
      curlhttpget Curl(line, curl_walk_callback, this);
    else if (!strstr(line, "#EXT"))
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

		if (datapump_cb)
			datapump_cb(datapump_ctxt, data, 188, buf_size / 188);


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
  if (!push_buffer.write(buffer, size * nmemb))
    while (nmemb)
      if (push_buffer.write(buffer, nmemb)) {
        buffer += size;
        nmemb--;
      } else {
        fprintf(stderr, "%s: FAILED: %zu packets dropped\n", __func__, nmemb);
        return;
      }
#else
  if (datapump_cb)
    datapump_cb(datapump_ctxt, buffer, size, nmemb);
#endif
}

void hlsfeed::walk(uint8_t *buffer, size_t size, size_t nmemb)
{
#if WALK_THREAD
  if (!walk_buffer.write(buffer, size * nmemb))
    while (nmemb)
      if (walk_buffer.write(buffer, nmemb)) {
        buffer += size;
        nmemb--;
      } else {
        fprintf(stderr, "%s: FAILED: %zu bytes dropped\n", __func__, size * nmemb);
        return;
      }
#else
  walk(buffer);
#endif
}

void hlsfeed::curl_push_callback(void *context, void *buffer, size_t size, size_t nmemb)
{
  return static_cast<hlsfeed*>(context)->push((uint8_t*)buffer, size, nmemb);
}

void hlsfeed::curl_walk_callback(void *context, void *buffer, size_t size, size_t nmemb)
{
  return static_cast<hlsfeed*>(context)->walk((uint8_t*)buffer, size, nmemb);
}
