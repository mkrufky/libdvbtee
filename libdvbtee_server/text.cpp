/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "functions.h"
#include "text.h"
#include "dvbtee_config.h"

const std::string html_dump_epg_header_footer_callback(void *, bool header, bool channel)
{
	//fprintf(stderr, "%s(%s, %s)\n", __func__, (header) ? "header" : "footer", (channel) ? "channel" : "body");
	std::string str;
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

const std::string json_dump_channels_header_footer_callback(void *, bool header, bool channel)
{
	//fprintf(stderr, "%s(%s, %s)\n", __func__, (header) ? "header" : "footer", (channel) ? "channel" : "body");
	std::string str;
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

const std::string json_dump_epg_header_footer_callback(void *, bool header, bool channel)
{
	//fprintf(stderr, "%s(%s, %s)\n", __func__, (header) ? "header" : "footer", (channel) ? "channel" : "body");
	std::string str;
	if (header)
		if (channel)
			str = "{\"Entries\":\n[";
		else
			str = "[";
	else
		if (channel)
			str = "{}]},";
		else
			str = "{}]";
//	fprintf(stderr, "%s", str);
	return str;
}

const std::string xml_dump_epg_header_footer_callback(void *, bool header, bool channel)
{
	//fprintf(stderr, "%s(%s, %s)\n", __func__, (header) ? "header" : "footer", (channel) ? "channel" : "body");
	std::string str;
	if (header)
		if (channel)
			str = "";
		else
			str = "<tv generator-info-name='dvbtee v" LIBDVBTEE_VERSION "'>\n";
	else
		if (channel)
			str = "";
		else
			str = "</tv>\n";
//	fprintf(stderr, "%s", str);
	return str;
}

const std::string html_dump_epg_event_callback(void *, decoded_event_t *e)
{
	//fprintf(stderr, "%s()\n", __func__);
	std::string str;
	str.clear();
	str.append("<td>");
	//str.append("channel: ");
	if (e->channel_name.length()) {
		char chan_nbr[12] = { 0 };

		snprintf(chan_nbr, sizeof(chan_nbr), "%d.%d: ", e->chan_major, e->chan_minor);

		str.append("<nobr>");
		str.append(chan_nbr);
#if 0
		char *hstr = url_encode((char *)e->channel_name);
		str.append(hstr);
		free(hstr);
#else
		str.append(e->channel_name);
#endif
		str.append("</nobr>");
	}
	//str.append("show: ");
	if (e->name.length()) {
		time_t end_time = e->start_time + e->length_sec;
		struct tm tms = *localtime( &e->start_time );
		struct tm tme = *localtime( &end_time );

		char time_str[14] = { 0 };
#if 0
		char *hstr = url_encode((char *)e->name);
		str.append(hstr);
		free(hstr);
#else
		str.append(e->name);
#endif
		if (e->text.length()) {
			str.append(": <br><font size=-3>");
#if 0
			hstr = url_encode((char *)e->text);
			str.append(hstr);
			free(hstr);
#else
			str.append(e->text);
#endif
			str.append("</font>");
		}
		snprintf(time_str, sizeof(time_str), "%02d:%02d - %02d:%02d", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min);

		str.append("<hr><nobr>");
		str.append(time_str);
		str.append("</nobr>");
	}
	str.append("</td>");
//	fprintf(stderr, "%s", str.c_str());
	return str;
}

const std::string json_dump_epg_event_callback(void *, decoded_event_t *e)
{
	time_t end_time = e->start_time + e->length_sec;
	char time_str[15] = { 0 };
	//fprintf(stderr, "%s()\n", __func__);
	std::string str;
	str.clear();
	str.append("{");

	str.append("\"StartTime\":");
#if 1
	snprintf(time_str, sizeof(time_str), "%lld", (long long int)e->start_time);
	str.append(time_str);
#else
	str.append(ctime(&e->start_time));
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
#if 0
	char *hstr = url_encode((char *)e->name);
	str.append(hstr);
	free(hstr);
#else
	str.append(e->name);
#endif
	str.append("\",");
	str.append("\"ShortDescription\":\"");
#if 0
	hstr = url_encode((char *)e->text);
	str.append(hstr);
	free(hstr);
#else
	str.append(e->text);
#endif
	str.append("\"");

	str.append("},");
//	fprintf(stderr, "%s", str.c_str());
	return str;
}


const char * bcd_time_str(const time_t *the_time, char *time_str, size_t str_len)
{
	struct tm tm_time;
	if (!time_str)
		return NULL;

#ifndef HAVE_LOCALTIME_R
	/* as per:
	 * http://stackoverflow.com/questions/18551409/localtime-r-support-on-mingw
	 * localtime_r is not supported, but localtime is supported.
	 * localtime is thread safe but not reentrant.
	 */
	tm_time = *localtime(the_time);
#else
	localtime_r(the_time, &tm_time);
#endif
	snprintf(time_str, str_len, "%04d%02d%02d%02d%02d",
		 1900 + tm_time.tm_year,
		 1 + tm_time.tm_mon, tm_time.tm_mday,
		 tm_time.tm_hour, tm_time.tm_min);

	return time_str;
}


const std::string xml_dump_epg_event_callback(void *, decoded_event_t *e)
{
	time_t end_time = e->start_time + e->length_sec;
	char time_str[15] = { 0 };
	char chan_nbr[12] = { 0 };
	//fprintf(stderr, "%s()\n", __func__);
	std::string str;

	str.clear();
	str.append("<programme start='");

	memset(time_str, 0, sizeof(time_str));
	str.append(bcd_time_str(&e->start_time, time_str, sizeof(time_str)));

	str.append("' stop='");

	memset(time_str, 0, sizeof(time_str));
	str.append(bcd_time_str(&end_time, time_str, sizeof(time_str)));

	str.append("' channel='");
#if 0
	snprintf(chan_nbr, sizeof(chan_nbr), "%d.%d: ", e->chan_major, e->chan_minor);

	str.append(chan_nbr);
	str.append(" ");
	str.append(e->channel_name);
#else
	snprintf(chan_nbr, sizeof(chan_nbr), "%d+%d", e->chan_physical, e->chan_svc_id);

	str.append(chan_nbr);
#endif
	str.append("'>\n");
	str.append("<title lang='en'>");
#if 1
	char *hstr = url_encode(e->name.c_str());
	str.append(hstr);
	free(hstr);
#else
	str.append(e->name);
#endif
	str.append("</title>\n");
#if 1
	str.append("<desc lang='en'>");
#if 1
	hstr = url_encode(e->text.c_str());
	str.append(hstr);
	free(hstr);
#else
	str.append(e->text);
#endif
	str.append("</desc>\n");
#endif
	str.append("</programme>\n");

	return str;
}

const std::string html_dump_channels(void *, parsed_channel_info_t *c)
{
	std::string str;
	str.clear();
	char channelno[8]; /* XXX.XXX */
	if (c->major + c->minor > 1)
		sprintf(channelno, "%d.%d", c->major, c->minor);
	else if (c->lcn)
		sprintf(channelno, "%d", c->lcn);
	else
		sprintf(channelno, "%d", c->physical_channel);

	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		c->service_name,
		c->freq,//iter_vct->second.carrier_freq,
		c->modulation,
		c->vpid, c->apid, c->program_number);

	char phy_chan[4] = { 0 };
	char svc_id[6] = { 0 };

	sprintf(phy_chan, "%d", c->physical_channel);
	sprintf(svc_id, "%d", c->program_number);

	str.append("<table>");
	str.append("<tr>");
	str.append("<td>");
	str.append("<a href='/tune=");
	str.append(phy_chan);
	str.append("~");
	str.append(svc_id);
	str.append("&channels'>");
	str.append(channelno);
	str.append(": ");
#if 0
	char *hstr = url_encode((char *)c->service_name);
	str.append(hstr);
	free(hstr);
#else
	str.append((const char *)c->service_name);
#endif
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

	return str;
}

const std::string json_dump_channels(void *, parsed_channel_info_t *c)
{
	std::string str;
	str.clear();
	char channelno[8] = { 0 }; /* XXX.XXX */
	char chan_major[4] = { 0 };
	char chan_minor[4] = { 0 };
	if (c->major + c->minor > 1) {
		sprintf(channelno, "%d.%d", c->major, c->minor);
		sprintf(chan_major, "%d", c->major);
		sprintf(chan_minor, "%d", c->minor);
	} else if (c->lcn) {
		sprintf(channelno, "%d", c->lcn);
		sprintf(chan_major, "%d", c->lcn);
		sprintf(chan_minor, "%d", 0);
	} else {
		sprintf(channelno, "%d", c->physical_channel);
		sprintf(chan_major, "%d", c->physical_channel);
		sprintf(chan_minor, "%d", c->program_number);
	}

	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		c->service_name,
		c->freq,//iter_vct->second.carrier_freq,
		c->modulation,
		c->vpid, c->apid, c->program_number);

	char phy_chan[4] = { 0 };
	char svc_id[6] = { 0 };

	sprintf(phy_chan, "%d", c->physical_channel);
	sprintf(svc_id, "%d", c->program_number);

	str.append("{");
	str.append("\"Id\":\"");
	str.append(phy_chan);
	str.append("~");
	str.append(svc_id);
	str.append("\"");
	str.append(",");
	str.append("\"DisplayName\":\"");
#if 0
	char *hstr = url_encode((char *)c->service_name);
	str.append(hstr);
	free(hstr);
#else
	str.append((const char *)c->service_name);
#endif
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

	return str;
}

const std::string xml_dump_channels(void *, parsed_channel_info_t *c)
{
	std::string str;
	str.clear();
	char channelno[8] = { 0 }; /* XXX.XXX */
	char chan_major[4] = { 0 };
	char chan_minor[4] = { 0 };
	if (c->major + c->minor > 1) {
		sprintf(channelno, "%d.%d", c->major, c->minor);
		sprintf(chan_major, "%d", c->major);
		sprintf(chan_minor, "%d", c->minor);
	} else if (c->lcn) {
		sprintf(channelno, "%d", c->lcn);
		sprintf(chan_major, "%d", c->lcn);
		sprintf(chan_minor, "%d", 0);
	} else {
		sprintf(channelno, "%d", c->physical_channel);
		sprintf(chan_major, "%d", c->physical_channel);
		sprintf(chan_minor, "%d", c->program_number);
	}

	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		c->service_name,
		c->freq,//iter_vct->second.carrier_freq,
		c->modulation,
		c->vpid, c->apid, c->program_number);

	char phy_chan[4] = { 0 };
	char svc_id[6] = { 0 };

	sprintf(phy_chan, "%d", c->physical_channel);
	sprintf(svc_id, "%d", c->program_number);

	str.append("<channel id='");
	str.append(phy_chan);
	str.append("~");
	str.append(svc_id);
	str.append("'>\n");
	str.append("<display-name>");
	str.append(channelno);
	str.append(" ");
#if 1
	char *hstr = url_encode((const char *)c->service_name);
	str.append(hstr);
	free(hstr);
#else
	str.append((const char *)c->service_name);
#endif
	str.append("</display-name>\n");
	str.append("<display-name>");
	str.append(channelno);
	str.append("</display-name>\n");
	str.append("<display-name>");
#if 1
	hstr = url_encode((const char *)c->service_name);
	str.append(hstr);
	free(hstr);
#else
	str.append((const char *)c->service_name);
#endif
	str.append("</display-name>\n");
	str.append("</channel>\n");

	return str;
}

const std::string html_playing_video(void *)
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
	return str;
}
