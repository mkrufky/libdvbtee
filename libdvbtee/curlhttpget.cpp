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

#include "dvbtee_config.h"
#ifdef HAVE_LIBCURL
#include "curlhttpget.h"

curlhttpget::curlhttpget(const char *url, curlhttpget_iface *iface, curlhttpget_info_t *info)
  : curl_handle(curl_easy_init())
  , m_iface(iface)
{
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);
//curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libdvbtee");

  if (m_iface) {
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
  }

  CURLcode res = curl_easy_perform(curl_handle);
  if (res != CURLE_OK) {
    fprintf(stderr, "%s: curl_easy_perform() failed: %s\nURL: %s\n\n", __func__, curl_easy_strerror(res), url);
    goto fail;
  }

  if (info) getinfo(info);

fail:
  curl_easy_cleanup(curl_handle);
}

void curlhttpget::getinfo(curlhttpget_info_t *info)
{
  info->total_time = 0.0;
  CURLcode res = curl_easy_getinfo(curl_handle, CURLINFO_TOTAL_TIME, &info->total_time);
  if (res == CURLE_OK)
    fprintf(stderr, "%s: total time: %f\n", __func__, info->total_time);
}

//static
size_t curlhttpget::write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
  return static_cast<curlhttpget*>(userp)->__write_data(buffer, size, nmemb);
}

size_t curlhttpget::__write_data(void *buffer, size_t size, size_t nmemb)
{
  if (m_iface) m_iface->write_data(buffer, size, nmemb);
  return size * nmemb;
}
#endif
