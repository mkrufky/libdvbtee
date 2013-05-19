/*****************************************************************************
 * Copyright (C) 2013 Michael Krufky
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

#ifndef HLSINPUT_H
#define HLSINPUT_H

#include "feed.h"

class hlsinput
{
public:
  explicit hlsinput(bool feed_stdout = false);
  bool get(const char *url);
private:
  bool b_stdout;
  feed feeder;

  static void write_feed(void *context, void *buffer, size_t size, size_t nmemb);
};

#endif // HLSINPUT_H
