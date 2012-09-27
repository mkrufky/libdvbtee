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

const char * json_dump_epg_header_footer_callback(void *, bool header, bool channel)
{
	//fprintf(stderr, "%s(%s, %s)\n", __func__, (header) ? "header" : "footer", (channel) ? "channel" : "body");
	const char *str = NULL;
	if (header)
		if (channel)
			str = "{\"Entries\":\n[";
		else
			str = "[";
	else
		if (channel)
			str = "]}";
		else
			str = "]";
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

const char * json_dump_epg_event_callback(void * context,
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
	time_t end_time = start_time + length_sec;
	char time_str[15] = { 0 };
	//fprintf(stderr, "%s()\n", __func__);
	std::string str;
	str.clear();
	str.append("{");

	str.append("\"StartTime\":");
#if 1
	snprintf(time_str, sizeof(time_str), "%lld", (long long int)start_time);
	str.append(time_str);
#else
	str.append(ctime(&start_time));
#endif
	str.append(",");
	str.append("\"EndTime\":");
#if 1
	snprintf(time_str, sizeof(time_str), "%lld", (long long int)end_time); // "%jd", (intmax_t)end_time);
	str.append(time_str);
#else
	str.append(ctime(&end_time));
#endif
	str.append(",");
	str.append("\"Title\":\"");
	str.append(name);
	str.append("\",");
	str.append("\"ShortDescription\":\"");
	//str.append(text);
	str.append("\"");

	str.append("}");
//	fprintf(stderr, "%s", str.c_str());
	return str.c_str();
}

const char * html_dump_channels(void *context,
			       uint16_t lcn, uint16_t major, uint16_t minor,
			       uint16_t physical_channel, uint32_t freq, const char *modulation,
			       unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number)
{
	std::string str;
        str.clear();
	char channelno[7]; /* XXX.XXX */
	if (major + minor > 1)
		sprintf(channelno, "%d.%d", major, minor);
	else if (lcn)
		sprintf(channelno, "%d", lcn);
	else
		sprintf(channelno, "%d", physical_channel);

	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		service_name,
		freq,//iter_vct->second.carrier_freq,
		modulation,
		vpid, apid, program_number);

	char phy_chan[4] = { 0 };
	char svc_id[6] = { 0 };

	sprintf(phy_chan, "%d", physical_channel);
	sprintf(svc_id, "%d", program_number);

	str.append("<table>");
	str.append("<tr>");
	str.append("<td>");
	str.append("<a href='/tune/stop");
	str.append("&service=");
	str.append(svc_id);
	str.append("&channel=");
	str.append(phy_chan);
	str.append("'>");
	str.append(channelno);
	str.append(": ");
	str.append((const char *)service_name);
	str.append("</a>");
#if 0
	str.append("</td>");
	str.append("</tr>");
	str.append("<tr>");
	str.append("<td>");
	str.append("<hr>");
#endif
	str.append("</td>");
	str.append("</tr>");
	str.append("</table>");

	return str.c_str();
}

const char * json_dump_channels(void *context,
			       uint16_t lcn, uint16_t major, uint16_t minor,
			       uint16_t physical_channel, uint32_t freq, const char *modulation,
			       unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number)
{
	std::string str;
	str.clear();
	char channelno[7] = { 0 }; /* XXX.XXX */
	char chan_major[3] = { 0 };
	char chan_minor[3] = { 0 };
	if (major + minor > 1) {
		sprintf(channelno, "%d.%d", major, minor);
		sprintf(chan_major, "%d", major);
		sprintf(chan_minor, "%d", minor);
	} else if (lcn) {
		sprintf(channelno, "%d", lcn);
		sprintf(chan_major, "%d", lcn);
		sprintf(chan_minor, "%d", 0);
	} else {
		sprintf(channelno, "%d", physical_channel);
		sprintf(chan_major, "%d", physical_channel);
		sprintf(chan_minor, "%d", program_number);
	}

	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		service_name,
		freq,//iter_vct->second.carrier_freq,
		modulation,
		vpid, apid, program_number);

	char phy_chan[4] = { 0 };
	char svc_id[6] = { 0 };

	sprintf(phy_chan, "%d", physical_channel);
	sprintf(svc_id, "%d", program_number);

	str.append("{");
	str.append("\"Id\":\"");
	str.append("tune");
	str.append("&channel=");
	str.append(phy_chan);
	str.append("&service=");
	str.append(svc_id);
	str.append("\"");
	str.append(",");
	str.append("\"DisplayName\":\"");
	str.append((const char *)service_name);
	str.append("\"");
	str.append(",");
	str.append("\"MajorChannelNo\":\"");
	str.append(chan_major);
	str.append("\"");
	str.append(",");
	str.append("\"MinorChannelNo\":\"");
	str.append(chan_minor);
	str.append("\"");
	str.append("}");

	str.append(",");

	return str.c_str();
}

const char * html_playing_video(void *)
{
	std::string str;
	str.clear();
	str.append("<html><body>");
//	str.append("<video height=\"1280\" width=\"720\" controls>");
	str.append("<div style=\"position:relative;width:604px;height:256px;float:center;\">");
	str.append("<video controls=\"controls\"  autoplay=\"autoplay\" poster=\"http://easyhtml5video.com/images/happyfit2.jpg\" width=\"604\" height=\"256\" onclick=\"if(/Android/.test(navigator.userAgent))this.play();\">");
//	str.append("<video controls>");
	str.append("<source src=\"/stream/\"");
	str.append(" type='video/mp2ts");
	str.append("; codecs=\"ac-3\"");
	str.append("'>");
	str.append("<source src=\"http://easyhtml5video.com/images/happyfit2.mp4\" type=\"video/mp4\">");
	str.append("</video>");
	str.append("</div>");
	str.append("</body></html>");
	return str.c_str();
}
