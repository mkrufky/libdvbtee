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
{
//walk((void*)url);
  walk((void*)Url.c_str());
}

void hlsfeed::walk(void *buffer)
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

void hlsfeed::push(void *buffer, size_t size, size_t nmemb)
{
  if (datapump_cb)
    datapump_cb(datapump_ctxt, buffer, size, nmemb);
}

void hlsfeed::curl_push_callback(void *context, void *buffer, size_t size, size_t nmemb)
{
  return static_cast<hlsfeed*>(context)->push(buffer, size, nmemb);
}

void hlsfeed::curl_walk_callback(void *context, void *buffer, size_t size, size_t nmemb)
{
  return static_cast<hlsfeed*>(context)->walk(buffer);
}
