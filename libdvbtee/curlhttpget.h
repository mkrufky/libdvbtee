/*****************************************************************************
 * Copyright (C) 2013-2016 Michael Ira Krufky
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

#ifndef CURLHTTPGET_H
#define CURLHTTPGET_H

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif /* HAVE_LIBCURL */

class curlhttpget_iface
{
public:
	virtual void write_data(void *buffer, size_t size, size_t nmemb) = 0;
};

typedef struct {
  double total_time;
} curlhttpget_info_t;

class curlhttpget
{
public:
  explicit curlhttpget(const char *url = NULL, curlhttpget_iface *iface = NULL,
		       curlhttpget_info_t *info = NULL);

#ifdef HAVE_LIBCURL
private:
  CURL *curl_handle;

  curlhttpget_iface *m_iface;

  void getinfo(curlhttpget_info_t *info);

  static size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
  size_t __write_data(void *buffer, size_t size, size_t nmemb);
#endif /* HAVE_LIBCURL */
};

#endif // CURLHTTPGET_H
