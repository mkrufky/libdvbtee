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

#ifndef CURLHTTPGET_H
#define CURLHTTPGET_H

#include <curl/curl.h>

typedef void (*hls_curl_http_get_data_callback)(void *context, void *buffer, size_t size, size_t nmemb);

typedef struct {
  double total_time;
} curlhttpget_info_t;

class curlhttpget
{
public:
  explicit curlhttpget(const char *url = NULL, hls_curl_http_get_data_callback data_callback = NULL, void *data_context = NULL,
                       curlhttpget_info_t *info = NULL);

private:
  CURL *curl_handle;

  hls_curl_http_get_data_callback data_cb;
  void *data_ctxt;

  void getinfo(curlhttpget_info_t *info);

  static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
  size_t __write_data(void *buffer, size_t size, size_t nmemb);
};

#endif // CURLHTTPGET_H
