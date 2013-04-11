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

#include "hlsinput.h"
#include "hlsfeed.h"

static void write_stdout(void *context, void *buffer, size_t size, size_t nmemb)
{
  fwrite(buffer, size, nmemb, stdout);
}

hlsinput::hlsinput(bool feed_stdout)
  : b_stdout(feed_stdout)
{
}

bool hlsinput::get(const char *url)
{
  if (b_stdout)
    hlsfeed hlsFeeder(url, write_stdout, this);
  else
    hlsfeed hlsFeeder(url);

  return true;
}
