/*****************************************************************************
 * Copyright (C) 2011-2014 Michael Ira Krufky
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

#ifndef __TEXT_H__
#define __TEXT_H__

#include <stdint.h>
#include "parse.h"

const std::string html_dump_epg_header_footer_callback(void *, bool, bool);

const std::string json_dump_epg_header_footer_callback(void *, bool, bool);

const std::string xml_dump_epg_header_footer_callback(void *, bool, bool);

const std::string html_dump_epg_event_callback(void * context, decoded_event_t *e);

const std::string json_dump_epg_event_callback(void * context, decoded_event_t *e);

const std::string xml_dump_epg_event_callback(void * context, decoded_event_t *e);

const std::string html_dump_channels(void *context, parsed_channel_info_t *c);

const std::string json_dump_channels(void *context, parsed_channel_info_t *c);

const std::string xml_dump_channels(void *context, parsed_channel_info_t *c);

const std::string html_playing_video(void *);

#endif /* __TEXT_H__ */
