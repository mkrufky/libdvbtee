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

#ifndef HLSFEEDER_H
#define HLSFEEDER_H

#include "feed.h"
#include "curlhttpget.h"

class hlsfeed
{
public:
  explicit hlsfeed(const char *url = NULL, hls_curl_http_get_data_callback data_pump_callback = NULL, void *data_pump_context = NULL);

private:
  const char *toplevel;
  std::string Url;
  hls_curl_http_get_data_callback datapump_cb;
  void *datapump_ctxt;

  void push(void *buffer, size_t size, size_t nmemb);
  void walk(void *buffer);

  feed feeder;

  static void curl_push_callback(void *context, void *buffer, size_t size, size_t nmemb);
  static void curl_walk_callback(void *context, void *buffer, size_t size, size_t nmemb);
};

#endif // HLSFEEDER_H
