/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
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

#ifndef __HTML_H__
#define __HTML_H__

#include <stdint.h>

const char * html_dump_epg_header_footer_callback(void *, bool, bool);
const char * html_dump_epg_event_callback(void * context,
					  const char * channel_name,
					  uint16_t chan_major,
					  uint16_t chan_minor,
					  //
					  uint16_t event_id,
					  time_t start_time,
					  uint32_t length_sec,
					  const char * name,
					  const char * text);

const char * html_dump_channels(void *context,
				uint16_t lcn, uint16_t major, uint16_t minor,
				uint16_t physical_channel, uint32_t freq, const char *modulation,
				unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number);

#endif /* __HTML_H__ */
