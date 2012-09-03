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

#include <stdio.h>
#include <string>
#include "html.h"

const char * html_dump_epg_header_footer_callback(void *, bool header, bool channel)
{
	//fprintf(stderr, "%s(%s, %s)\n", __func__, (header) ? "header" : "footer", (channel) ? "channel" : "body");
	const char *str = NULL;
	if (header)
		if (channel)
			str = "<tr>";
		else
			str = "<html><body><table border=1>";
	else
		if (channel)
			str = "</tr>";
		else
			str = "</table></body></html>";
//	fprintf(stderr, "%s", str);
	return str;
}

const char * html_dump_epg_event_callback(void * context,
					  const char * channel_name,
					  uint16_t chan_major,
					  uint16_t chan_minor,
					  //
					  uint16_t event_id,
					  time_t start_time,
					  uint32_t length_sec,
					  const char * name,
					  const char * text)
{
	//fprintf(stderr, "%s()\n", __func__);
	std::string str;
	str.clear();
	str.append("<td>");
	//str.append("channel: ");
	if (channel_name) {
		char chan_nbr[12] = { 0 };

		snprintf(chan_nbr, sizeof(chan_nbr), "%d.%d: ", chan_major, chan_minor);

		str.append("<nobr>");
		str.append(chan_nbr);
		str.append(channel_name);
		str.append("</nobr>");
	}
	//str.append("show: ");
	if (name) {
		time_t end_time = start_time + length_sec;
		struct tm tms = *localtime( &start_time );
		struct tm tme = *localtime( &end_time );

		char time_str[14] = { 0 };

		str.append(name);

		snprintf(time_str, sizeof(time_str), "%02d:%02d - %02d:%02d", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min);

		str.append("<hr><nobr>");
		str.append(time_str);
		str.append("</nobr>");
	}
	str.append("</td>");
//	fprintf(stderr, "%s", str.c_str());
	return str.c_str();
}
