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

#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>

#include "log.h"
#include "serve.h"
#include "text.h"

unsigned int dbg_serve = (dbg & DBG_SERVE) ? DBG_SERVE : 0;

#define CLASS_MODULE "server"

#define dprintf(fmt, arg...) __dprintf(DBG_SERVE, fmt, ##arg)

	tuner_map  tuners;
	feeder_map feeders;

	bool any_cli = false;

static tune *find_first_idle_tuner()
{
	for (tuner_map::iterator iter = tuners.begin(); iter != tuners.end(); ++iter)
		if ((iter->second->is_idle()) || (!iter->second->feeder.parser.check())) {
			dprintf("tuner %d is available", iter->first);
			return iter->second;
		}
	return NULL;
}

static tune *find_idle_tuner()
{
	tune *tuner = NULL;
	time_t last_touched = 0;
	for (tuner_map::iterator iter = tuners.begin(); iter != tuners.end(); ++iter)
		if ((iter->second->is_idle()) || (!iter->second->feeder.parser.check())) {
			dprintf("tuner %d is available", iter->first);
			if ((!tuner) || (iter->second->last_touched() > last_touched)) {
				tuner = iter->second;
				last_touched = iter->second->last_touched();
			}
		}
	return tuner;
}

static tune *find_tuned_tuner(unsigned int phy)
{
	for (tuner_map::iterator iter = tuners.begin(); iter != tuners.end(); ++iter)
		if ((iter->second->is_lock()) && (phy == iter->second->get_channel())) {
			dprintf("tuner %d is locked to physical channel %d", iter->first, phy);
			return iter->second;
		}
	return NULL;
}

static inline const char *data_fmt_str(unsigned int data_fmt)
{
	const char *fmt;
	switch (data_fmt) {
	default:
	case SERVE_DATA_FMT_NONE:
		fmt = "NONE";
		break;
	case SERVE_DATA_FMT_HTML:
		fmt = "HTML";
		break;
	case SERVE_DATA_FMT_BIN:
		fmt = "BIN";
		break;
	case SERVE_DATA_FMT_JSON:
		fmt = "JSON";
		break;
	case SERVE_DATA_FMT_CLI:
		fmt = "CLI";
		break;
	case SERVE_DATA_FMT_XML:
		fmt = "XML";
		break;
	}
	return fmt;
}

void serve::add_feeder(feed *new_feeder)
{
	unsigned int new_id = feeders.size();
	while (feeders.count(new_id)) new_id++;
	feeders[new_id] = new_feeder;
}

bool serve::add_tuner(tune *new_tuner)
{
	unsigned int new_id = tuners.size();
	while (tuners.count(new_id)) new_id++;
	tuners[new_id] = new_tuner;
	return true;
}

bool serve_client::list_feeders()
{
	cli_print("%d feeders.\n", feeders.size());
	for (feeder_map::iterator iter = feeders.begin(); iter != feeders.end(); ++iter)
		if (iter->second->check())
			cli_print("feeder %d:\t%s\n", iter->first, iter->second->get_filename());
	return true;
}

bool serve_client::list_tuners()
{
	cli_print("%d tuners.\n", tuners.size());
	for (tuner_map::iterator iter = tuners.begin(); iter != tuners.end(); ++iter)
		if (iter->second->check()) {
			unsigned int cur_chan = iter->second->get_channel();
			const char *tuner_name = iter->second->get_name();
			cli_print("tuner %d:\tchannel %d, state:%s%s%s%s%s\t%s\n",
				  iter->first, cur_chan,
				  iter->second->is_idle() ? " idle" : "",
				  iter->second->is_open() ? " open" : "",
				  iter->second->is_lock() ? " lock" : "",
				  iter->second->is_scan() ? " scan" : "",
				  iter->second->is_feed() ? " feed" : "",
				  tuner_name);
		}
	return true;
}

bool serve_client::list_clients()
{
	serve_client_map *client_map = server->get_client_map();
	cli_print("%d clients.\n", client_map->size());
	for (serve_client_map::iterator iter = client_map->begin(); iter != client_map->end(); ++iter)
		if (iter->second.check())
			cli_print("client %d:\tformat = %s\n", iter->first, data_fmt_str(iter->second.get_data_fmt()));
	return true;
}

bool serve::get_channels(parse_iface *iface, unsigned int tuner_id)
{
	tune* tuner = (tuners.count(tuner_id)) ? tuners[tuner_id] : NULL;
	if (!tuner) {
		dprintf("NO TUNER!\n");
		return false;
	}

	/* channels verified during this session */
	tuner->get_scan_results(false, iface);
	/* load remaining channels that we saved previously but havent seen during this session */
	cmd_config_channels_conf_load(tuner, iface);

	return true;
}

bool serve::get_epg(decode_report *iface)
{
	unsigned int tuner_id = 0; // FIXME
	tune* tuner = (tuners.count(tuner_id)) ? tuners[tuner_id] : NULL;
	if (!tuner) {
		dprintf("NO TUNER!\n");
		return false;
	}

	tuner->feeder.parser.epg_dump(iface);

	return true;
}

bool serve::scan(unsigned int flags, tune_iface *t_iface, parse_iface *p_iface, unsigned int tuner_id)
{
	tune* tuner = (tuners.count(tuner_id)) ? tuners[tuner_id] : NULL;
	if (!tuner) {
		dprintf("NO TUNER!\n");
		return false;
	}

	bool wait_for_results = (p_iface != NULL) ? true : false;

	dprintf("scanning for services...");

	return cmd_tuner_scan(tuner, NULL, false, wait_for_results, flags, t_iface, p_iface);
}

void serve::set_scan_flags(tune *p_tuner, unsigned int flags)
{
	scan_flags[p_tuner] = flags;
}

unsigned int serve::get_scan_flags(tune *p_tuner)
{
	return scan_flags.count(p_tuner) ? scan_flags[p_tuner] : 0;
}

/*****************************************************************************/

#define CRLF "\r\n"

#define MAX_SOCKETS 4
#define HTTP_200_OK  "HTTP/1.1 200 OK"
#define CONTENT_TYPE "Content-type: "
#define TEXT_HTML    "text/html"
#define TEXT_PLAIN   "text/plain"
#define ENC_CHUNKED  "Transfer-Encoding: chunked"
#define CONN_CLOSE   "Connection: close"

/*
static char http200ok[] =
	 HTTP_200_OK
	 CRLF
	 CRLF;

static char http_response[] =
	 HTTP_200_OK
	 CRLF
	 CONTENT_TYPE TEXT_HTML
	 CRLF
#if 0
	 "Content-length: 0"
#else
	 ENC_CHUNKED
#endif
#if 0
	 CRLF
	 "Cache-Control: no-cache,no-store,private"
	 CRLF
	 "Expires: -1"
	 CRLF
	 CONN_CLOSE
#endif
	 CRLF
	 CRLF;

static char text_response[] =
	 HTTP_200_OK
	 CRLF
	 CONTENT_TYPE TEXT_PLAIN
	 CRLF
#if 0
	 "Content-length: 0"
#else
	 ENC_CHUNKED
#endif
#if 0
	 CRLF
	 "Cache-Control: no-cache,no-store,private"
	 CRLF
	 "Expires: -1"
	 CRLF
	 CONN_CLOSE
#endif
	 CRLF
	 CRLF;
*/

static char http_conn_close[] =
	 CONN_CLOSE
	 CRLF
	 CRLF;

/*****************************************************************************/

serve_client::serve_client()
  : h_thread((pthread_t)NULL)
  , f_kill_thread(false)
  , server(NULL)
  , tuner(NULL)
  , feeder(NULL)
  , sock_fd(-1)
  , channels_conf_file(NULL)
  , data_fmt(SERVE_DATA_FMT_NONE)
  , streamback_started(false)
  , streamback_newchannel(false)
{
	dprintf("()");
	services.clear();
}

serve_client::~serve_client()
{
	dprintf("(%d)", sock_fd);
	stop();
}

#if 1
serve_client::serve_client(const serve_client&)
{
	dprintf("(copy)");
	h_thread = (pthread_t)NULL;
	f_kill_thread = false;
	server = NULL;
	tuner = NULL;
	feeder = NULL;
	sock_fd = -1;
	channels_conf_file = NULL;
	data_fmt = SERVE_DATA_FMT_NONE;
	streamback_started = false;
	streamback_newchannel = false;
	services.clear();
}

serve_client& serve_client::operator= (const serve_client& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	h_thread = (pthread_t)NULL;
	f_kill_thread = false;
	server = NULL;
	tuner = NULL;
	feeder = NULL;
	sock_fd = -1;
	channels_conf_file = NULL;
	data_fmt = SERVE_DATA_FMT_NONE;
	streamback_started = false;
	streamback_newchannel = false;
	services.clear();
	services.clear();

	return *this;
}
#endif

void serve_client::close_socket()
{
	dprintf("(%d)", sock_fd);

	if (sock_fd >= 0) {
		close(sock_fd);
		sock_fd = -1;
	}
}

void serve_client::stop()
{
	dprintf("(%d)", sock_fd);

	stop_without_wait();

	while (-1 != sock_fd)
		usleep(20*1000);

	return;
}

int serve_client::start()
{
	dprintf("(%d)", sock_fd);

	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, client_thread, this);
	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

//static
void* serve_client::client_thread(void *p_this)
{
	return static_cast<serve_client*>(p_this)->client_thread();
}

void* serve_client::client_thread()
{
	struct sockaddr_in tcpsa;
	socklen_t salen = sizeof(tcpsa);
	char buf[1024];
	char hostname[16] = { 0 };
	char cli_prompt[sizeof(hostname)+3] = { 0 };
	int rxlen;
	bool http, httpget, httphead;
	char *tmpbuf, *cmdbuf, *save;

	gethostname(hostname, sizeof(hostname));
	if (!strlen(hostname))
		strncpy(hostname, "darkwing", sizeof(hostname));

	if (0 != getpeername(sock_fd, (struct sockaddr*)&tcpsa, &salen))
		perror("getpeername() failed");

	snprintf(cli_prompt, sizeof(cli_prompt), "%s> ", hostname);

	dprintf("(%d)", sock_fd);
#if 0
	data_fmt = SERVE_DATA_FMT_NONE;
#endif
	while (!f_kill_thread) {

		rxlen = recv(sock_fd, buf, sizeof(buf)-1, MSG_DONTWAIT);
		if (rxlen > 0) {
			buf[rxlen] = '\0';
			dprintf("(%d): %s", sock_fd, buf);

			http = (strstr(buf, "HTTP")) ? true : false;
			httpget = ((http) && (strstr(buf, "GET")));
			httphead = ((http) && (strstr(buf, "HEAD")));
			if (httpget) {
				data_fmt =	(strstr(buf, "stream/")) ? SERVE_DATA_FMT_BIN :
						(strstr(buf, "json/")) ? SERVE_DATA_FMT_JSON :
						(strstr(buf, "xml/")) ? SERVE_DATA_FMT_XML :
						(strstr(buf, "html/")) ? SERVE_DATA_FMT_HTML :
						(strstr(buf, "xmltv")) ? SERVE_DATA_FMT_XML :
									SERVE_DATA_FMT_HTML;
				tmpbuf = strtok_r(buf, " ", &save);
				if (strstr(tmpbuf, "GET")) {
					cmdbuf = strtok_r(NULL, " ", &save);
					if (!cmdbuf)
						cmdbuf = buf;
				} else
					cmdbuf = buf;
			} else if (server->is_cli_enabled()) {
				data_fmt = SERVE_DATA_FMT_CLI;
				cmdbuf = buf;
			} else {
				/* terminate thread if CLI isn't enabled */
				stop_without_wait();
				break;
			}

			if (httphead) {
				/* send http 200 ok, do not process commands (FIXME) and close connection */
				std::string str = http_response(MIMETYPE_NONE);
				socket_send(sock_fd, str.c_str(), str.length(), 0);
			} else {
				/* httpget - process commands */
				command(cmdbuf);
			}

			if (data_fmt == SERVE_DATA_FMT_CLI)
				socket_send(sock_fd, cli_prompt, sizeof(cli_prompt), 0);

			if (http) {
				/* terminate thread after processing the HTTP input buffer */
				stop_without_wait();

				if (data_fmt == SERVE_DATA_FMT_BIN) {
					/* disconnect socket from the server process
					   as it's now attached to the output process */
					sock_fd = -1;
				}
			}

		} else if ( (rxlen == 0) || ( (rxlen == -1) && (errno != EAGAIN) ) ) {
			stop_without_wait();
		} else
			usleep(20*1000);
	}

	close_socket();
	pthread_exit(NULL);
}

serve::serve()
  : feed_server_iface()
  , f_kill_thread(false)
  , f_reclaim_resources(true)
  , f_cli_enabled(true)
{
	dprintf("()");
	tuners.clear();
	feed_servers.clear();
	scan_flags.clear();
	start_monitor();
}

serve::~serve()
{
	dprintf("()");
	stop_monitor();
	stop();

	tuners.clear();
	feed_servers.clear();
	scan_flags.clear();
}

#if 0
serve::serve(const serve&)
{
	dprintf("(copy)");
}

serve& serve::operator= (const serve& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	return *this;
}
#endif

int serve::start_monitor()
{
	dprintf("()");

	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, monitor_thread, this);
	if (0 != ret)
		perror("pthread_create() failed");

	return ret;
}

//static
void* serve::monitor_thread(void *p_this)
{
	return static_cast<serve*>(p_this)->monitor_thread();
}

void* serve::monitor_thread()
{
	while (!f_kill_thread) {
		check();
#if 0
		for (feeder_map::iterator iter = feeders.begin(); iter != feeders.end(); ++iter)
			if (iter->second->check())
				cli_print("feeder %d:\t%s\n", iter->first, iter->second->get_filename());
#endif
		sleep(1*15); // sleep for 15 seconds between monitor iterations
	}
	pthread_exit(NULL);
}


void serve_client::streamback(const uint8_t *str, size_t length)
{
	stream_http_chunk(sock_fd, str, length);
}


void serve_client::epg_header_footer(bool header, bool channel)
{
	dprintf("()");
	if ((header) && (!channel)) streamback_started = true;
	if (!streamback_started) return;
	if ((header) && (channel)) streamback_newchannel = true;
	if (data_fmt & SERVE_DATA_FMT_TEXT) {
		std::string str;
		switch (data_fmt) {
		default: // FIXME
		case SERVE_DATA_FMT_HTML:
			str = html_dump_epg_header_footer_callback(this, header, channel);
			break;
		case SERVE_DATA_FMT_JSON:
			str = json_dump_epg_header_footer_callback(this, header, channel);
			break;
		case SERVE_DATA_FMT_XML:
			str = "";//xml_dump_epg_header_footer_callback(this, header, channel);
			break;
		}
		streamback((const uint8_t *)str.c_str(), str.length());
	}
	if ((!header) && (!channel)) fflush(stdout);
	return;
}

static inline const char *month(int x)
{
	const char *ret = NULL;
	switch (x) {
	case  0: ret = "Jan"; break;
	case  1: ret = "Feb"; break;
	case  2: ret = "Mar"; break;
	case  3: ret = "Apr"; break;
	case  4: ret = "May"; break;
	case  5: ret = "Jun"; break;
	case  6: ret = "Jul"; break;
	case  7: ret = "Aug"; break;
	case  8: ret = "Sep"; break;
	case  9: ret = "Oct"; break;
	case 10: ret = "Nov"; break;
	case 11: ret = "Dec"; break;
	}
	return ret;
}

static inline const char *weekday(int x)
{
	const char *ret = NULL;
	switch (x) {
	case  0: ret = "Sun"; break;
	case  1: ret = "Mon"; break;
	case  2: ret = "Tue"; break;
	case  3: ret = "Wed"; break;
	case  4: ret = "Thu"; break;
	case  5: ret = "Fri"; break;
	case  6: ret = "Sat"; break;
	}
	return ret;
}

void serve_client::epg_event(decoded_event_t &e)
{
	dprintf("()");
	if (!streamback_started) return;
#if 1
	if (streamback_newchannel) {
		if (data_fmt == SERVE_DATA_FMT_CLI)
			cli_print("\n%d.%d-%s\n", e.chan_major, e.chan_minor, e.channel_name.c_str());
		if (data_fmt == SERVE_DATA_FMT_HTML) {
			decoded_event_t ee;
			//memset(&ee, 0, sizeof(ee));
			ee.name.clear();
			ee.text.clear();
			ee.channel_name.assign(e.channel_name);
			ee.chan_major    = e.chan_major;
			ee.chan_minor    = e.chan_minor;
			ee.chan_physical = e.chan_physical;
			ee.chan_svc_id   = e.chan_svc_id;
			//const char *str = html_dump_epg_event_callback(this, channel_name, chan_major, chan_minor, 0, 0, 0, NULL, NULL);
			std::string str = html_dump_epg_event_callback(this, &ee);
			streamback((const uint8_t *)str.c_str(), str.length());
		}
		streamback_newchannel = false;
		fflush(stdout);
	}
#endif
	if (data_fmt == SERVE_DATA_FMT_CLI) {
		time_t end_time = e.start_time + e.length_sec;
		struct tm tms = *localtime( &e.start_time );
		struct tm tme = *localtime( &end_time );

		char time_str[26] = { 0 };

		snprintf(time_str, sizeof(time_str), "%s %s %02d %02d:%02d-%02d:%02d",
			 weekday(tms.tm_wday), month(tms.tm_mon), tms.tm_mday,
			 tms.tm_hour, tms.tm_min,
			 tme.tm_hour, tme.tm_min);

		cli_print("%s\t %s\n", time_str, e.name.c_str());
	}
	if (data_fmt & SERVE_DATA_FMT_TEXT) {
		decoded_event_t ee;
		//memset(&ee, 0, sizeof(ee));
		ee.name.clear();
		ee.text.clear();
		ee.event_id = e.event_id;
		ee.start_time = e.start_time;
		ee.length_sec = e.length_sec;
		ee.name.assign(e.name);
		ee.text.assign(e.text);

		std::string str;
		switch (data_fmt) {
		default: // FIXME
		case SERVE_DATA_FMT_HTML:
			str = html_dump_epg_event_callback(this, &ee);
			break;
		case SERVE_DATA_FMT_JSON:
			str = json_dump_epg_event_callback(this, &e);
			break;
		case SERVE_DATA_FMT_XML:
			str = xml_dump_epg_event_callback(this, &e);
			break;
		}
		streamback((const uint8_t *)str.c_str(), str.length());
	}
	return;
}


void serve::accept_socket(int socket)
{
	if (socket < 0) {
		fprintf(stderr, "not attaching to invalid socket, %d\n", socket);
		return;
	}

	/* check for old clients & clean them up */
	reclaim_server_resources();

	client_map[socket].setup(this, socket);
	client_map[socket].start();
}

int serve::start(struct libdvbtee_server_config *cfg)
{
	f_cli_enabled = !cfg->cli_disabled;
	return (cfg->port_requested > 0) ? start(cfg->port_requested) : start();
}

int serve::start(uint16_t port_requested)
{
	dprintf("(%d)", port_requested);

	listener.set_interface(this);

	return listener.start(port_requested);
}

void serve::stop()
{
	dprintf("()");

	listener.stop();

	return;
}

bool serve_client::check()
{
	bool ret = socket_active();
	if (!ret)
		dprintf("(%d) socket idle!", sock_fd);
	else {
		if (data_fmt == SERVE_DATA_FMT_CLI) any_cli = true;

		dprintf("(%d) format = %s", sock_fd, data_fmt_str(data_fmt));
	}
	return ret;
}

void serve::reclaim_server_resources()
{
	dprintf("()");

	bool erased = false;

	for (serve_client_map::iterator iter = client_map.begin(); iter != client_map.end(); ++iter)
		if (!iter->second.check()) {
			dprintf("erasing idle client...");
			client_map.erase(iter->first);
			/* stop the loop if we erased any targets */
			erased = true;
			break;
		}

	/* if we erased a target, restart the above by re-calling this function recursively */
	if (erased)
		reclaim_server_resources();
}

void serve::reclaim_tuner_resources()
{
	dprintf("()");

	for (tuner_map::iterator iter = tuners.begin(); iter != tuners.end(); ++iter)
		if (iter->second->check()) {
			/* if the tuner is feeding, check to see if it is streaming to any output.
			   if not streaming, stop this tuner */
			if ((!iter->second->feeder.parser.check()) &&
			    ((f_reclaim_resources) && (!any_cli)) &&
			    ((iter->second->is_feed()) && (iter->second->last_touched() > 15)) ) {
				dprintf("reclaiming idle resource:");
				dprintf("stopping data feed...");
				iter->second->stop_feed();
				dprintf("closing frontend...");
				iter->second->close_fe();
			}
		}
}

bool serve::check()
{
	dprintf("()");

	any_cli = false;

	reclaim_server_resources();
	reclaim_tuner_resources();

	return true;
}


void serve_client::print(const char *fmt, ...)
{
	if (data_fmt != SERVE_DATA_FMT_CLI) return;

	va_list args;

	va_start(args, fmt);

	cli_print(fmt, args);

	va_end(args);
}

void serve_client::cli_print(const char *fmt, ...)
{
	char buf[256] = { 0 };
	int bufsize;
	va_list args;

	va_start(args, fmt);

	vsprintf(buf, fmt, args);

	bufsize = sizeof(buf);

	if (bufsize) {
		if (dbg_serve & DBG_SERVE)
			fprintf(stderr, "server::%s: %s", __func__, buf);

		if ((data_fmt == SERVE_DATA_FMT_CLI) && (sock_fd >= 0))
			socket_send(sock_fd, buf, bufsize, 0);
	}
	va_end(args);
}

/*****************************************************************************/

#if 1//def PRETTY_URLS
#define CHAR_CMD_SEP "&/"
#define CHAR_CMD_SET "="
#else
#define CHAR_CMD_SEP ";"
#define CHAR_CMD_SET "/"
#endif

#define USE_JSON(data_fmt) (data_fmt == SERVE_DATA_FMT_JSON)
#define USE_HTML(data_fmt) (data_fmt == SERVE_DATA_FMT_HTML)
#define USE_XML(data_fmt)  (data_fmt == SERVE_DATA_FMT_XML)

bool serve_client::command(char* cmdline)
{
	char *save;
	bool ret = false;
	char *item = strtok_r(cmdline, CHAR_CMD_SEP, &save);
	bool stream_http_headers = (data_fmt & SERVE_DATA_FMT_TEXT) ? true : false;
#if 1
	streamback_newchannel = false;
	streamback_started = false;

	if (stream_http_headers) {
		std::string str;
		if ((USE_JSON(data_fmt)) || (USE_XML(data_fmt)))
			str = http_response(MIMETYPE_TEXT_PLAIN);
		else
			str = http_response(MIMETYPE_TEXT_HTML);
		socket_send(sock_fd, str.c_str(), str.length(), 0);
	}
#endif
	if (item) while (item) {

		ret = __command(item);
		if (!ret)
			goto exit;

		item = strtok_r(NULL, CHAR_CMD_SEP, &save);
	} else
		ret = __command(cmdline);
exit:
#if 1
	if (stream_http_headers) {
		stream_http_chunk(sock_fd, (uint8_t *)"", 0, true);
		socket_send(sock_fd, http_conn_close, strlen(http_conn_close), 0);
//		close_socket();
	}

#endif
	return ret;
}

class serve_parser_iface : public parse_iface
{
public:
	serve_parser_iface(serve_client&, bool to_disk = false);
	virtual void chandump(parsed_channel_info_t*);
private:
	serve_client& m_serve_client;
	bool m_to_disk;
};

serve_parser_iface::serve_parser_iface(serve_client &client, bool to_disk)
 : m_serve_client(client)
 , m_to_disk(to_disk)
{
	//
}

void serve_parser_iface::chandump(parsed_channel_info_t *c)
{
	std::string str;
	char channelno[7] = { 0 }; /* XXX.XXX */
	if (c->major + c->minor > 1)
		sprintf(channelno, "%d.%d", c->major, c->minor);
	else if (c->lcn)
		sprintf(channelno, "%d", c->lcn);
	else
		sprintf(channelno, "%d", c->physical_channel);

	m_serve_client.cli_print("%s-%s:%d:%s:%d:%d:%d\t tune=%d~%d\n",
		  channelno,
		  c->service_name,
		  c->freq,//iter_vct->second.carrier_freq,
		  c->modulation,
		  c->vpid, c->apid, c->program_number,
		  c->physical_channel,
		  c->program_number);

	if (m_to_disk) {
		//char diskbuf[96] = { 0 };
		//snprintf(diskbuf, 96,
		fprintf(m_serve_client.channels_conf_file,
			"%s-%s:%d:%s:%d:%d:%d\n",
			channelno,
			c->service_name,
			c->freq,//iter_vct->second.carrier_freq,
			c->modulation,
			c->vpid, c->apid, c->program_number);

		//if (channels_fd >= 0)
		//	write(channels_fd, (const void *)diskbuf, sizeof(diskbuf));
	} else
	if (m_serve_client.data_fmt & SERVE_DATA_FMT_TEXT) {

		str =
		  (USE_JSON(m_serve_client.data_fmt)) ?
			json_dump_channels(this, c) :
		  (USE_XML(m_serve_client.data_fmt)) ?
			xml_dump_channels(this, c) :
			html_dump_channels(this, c);

		m_serve_client.streamback((const uint8_t *)str.c_str(), str.length());
	}

	return;
}

bool serve_client::cmd_tuner_stop()
{
	cli_print("stopping data feed...\n");
	tuner->stop_feed();
	cli_print("closing frontend...\n");
	tuner->close_fe();
	return true;
}

bool serve_client::cmd_tuner_channel(int channel, unsigned int flags)
{
	if (channel > 0) {
		cli_print("TUNE to channel %d... ", channel);
		if (tuner->open_fe() < 0) {
			cli_print("open_fe() failed!\n");
			return false;
		}
		if (!flags)
			flags = SCAN_VSB;

		if (tuner->tune_channel((flags == SCAN_VSB) ? DVBTEE_VSB_8 : DVBTEE_QAM_256, channel)) {

			if (!tuner->wait_for_lock_or_timeout(2000)) {
				tuner->close_fe();
				cli_print("no lock!\n");
				return false; /* NO LOCK! */
			} else
				cli_print("LOCK!\n");
			tuner->feeder.parser.set_channel_info(channel,
							     (flags == SCAN_VSB) ? atsc_vsb_chan_to_freq(channel) :
										   atsc_qam_chan_to_freq(channel),
							     (flags == SCAN_VSB) ? "8VSB" : "QAM_256");
			tuner->start_feed();

			return true;
		} else
			cli_print("tune_channel() failed!\n");
	} else
		cli_print("invalid channel number.\n");

	return false;
}

bool serve::cmd_tuner_scan(tune* tuner, char *arg, bool scanepg, bool wait_for_results, unsigned int flags,
			   tune_iface *t_iface, parse_iface *p_iface)
{
	if (!flags)
		flags = SCAN_VSB;

	if ((arg) && strlen(arg))
		tuner->scan_for_services(flags, arg, scanepg, t_iface, p_iface, wait_for_results);
	else
		tuner->scan_for_services(flags, 0, 0, scanepg, t_iface, p_iface, wait_for_results);

	return true;
}

static uint16_t derive_physical_channel(uint32_t freq, const char *modulation)
{
	uint16_t ret = 0;

	if (modulation) {
		if (strstr(modulation, "VSB"))
			ret = atsc_vsb_freq_to_chan(freq);
		else if (strstr(modulation, "QAM_256"))
			ret = atsc_qam_freq_to_chan(freq);
		else /* DVB-T */
			ret = dvbt_freq_to_chan(freq);
	}
	return ret;
}

bool serve::cmd_config_channels_conf_load(tune* tuner, parse_iface *iface)
{
	char *homedir = getenv ("HOME");
	const char *subdir = "/.dvbtee";
	const char *slashchannelsconf = "/channels.conf";
	char dir[/*strlen(homedir)+strlen(subdir)*/64] = { 0 };
	char filepath[/*strlen(dir)+strlen(slashchannelsconf)*/78] = { 0 };

	//snprintf(dir, strlen(dir), "%s%s", homedir, subdir);
	memcpy(dir, homedir, strlen(homedir));
	memcpy(dir + strlen(homedir), subdir, strlen(subdir) + 1);
	//snprintf(filepath, strlen(filepath), "%s%s", dir, slashchannelsconf);
	memcpy(filepath, dir, strlen(dir));
	memcpy(filepath + strlen(dir), slashchannelsconf, strlen(slashchannelsconf) + 1);

	FILE *channels_conf = fopen(filepath, "r");
	if (channels_conf) {
		dprintf("reading %s...", filepath);

		char line[256] = { 0 };
		while (fgets(line, sizeof(line), channels_conf)) {
#if 0
			cli_print("%s", line);
#endif
			parsed_channel_info_t c;
			char *save, *temp, *chan, *name = strtok_r(line, ":", &save);

			if (!name) {
				memset(line, 0, sizeof(line));
				continue;
			}

			temp = strtok_r(NULL, ":", &save);
			c.freq = ((temp) && strlen(temp)) ? strtoul(temp, NULL, 0) : 0;
			temp = strtok_r(NULL, ":", &save);
			c.modulation = ((temp) && strlen(temp)) ? temp : "";

			if (!temp) {
				memset(line, 0, sizeof(line));
				continue;
			}

			if (strstr(temp, "INVERSION")) {
				//quick hack for DVB-T
				bool bw7mhz = false;

				temp = strtok_r(NULL, ":", &save);
				if (strstr(temp, "BANDWIDTH_7_MHZ"))
					bw7mhz = true;

				temp = strtok_r(NULL, ":", &save); // FEC_AUTO
				temp = strtok_r(NULL, ":", &save); // FEC_AUTO
				temp = strtok_r(NULL, ":", &save); // QAM_AUTO
				temp = strtok_r(NULL, ":", &save); // TRANSMISSION_MODE_AUTO
				temp = strtok_r(NULL, ":", &save); // GUARD_INTERVAL_AUTO
				temp = strtok_r(NULL, ":", &save); // HIERARCHY_AUTO

				c.modulation = (bw7mhz) ?
					"INVERSION_AUTO:BANDWIDTH_7_MHZ:FEC_AUTO:FEC_AUTO:QAM_AUTO:TRANSMISSION_MODE_AUTO:GUARD_INTERVAL_AUTO:HIERARCHY_AUTO" :
					"INVERSION_AUTO:BANDWIDTH_8_MHZ:FEC_AUTO:FEC_AUTO:QAM_AUTO:TRANSMISSION_MODE_AUTO:GUARD_INTERVAL_AUTO:HIERARCHY_AUTO";
			}

			c.physical_channel = derive_physical_channel(c.freq, c.modulation);

			uint16_t ts_id = tuner->feeder.parser.get_ts_id(c.physical_channel);
			if (ts_id) { /* dont load data for channels we already know about */
				memset(line, 0, sizeof(line));
				continue;
			}

			temp = strtok_r(NULL, ":", &save);
			c.vpid = ((temp) && strlen(temp)) ? strtoul(temp, NULL, 0) : 0;
			temp = strtok_r(NULL, ":", &save);
			c.apid = ((temp) && strlen(temp)) ? strtoul(temp, NULL, 0) : 0;
			temp = strtok_r(NULL, ":", &save);
			c.program_number = ((temp) && strlen(temp)) ? strtoul(temp, NULL, 0) : 0;

			chan = strtok_r(name, "-", &save);
			temp = strtok_r(NULL, "-", &save);
			sprintf((char*)c.service_name, "%s", (unsigned char *)(((temp) && strlen(temp)) ? temp : chan));

			temp = strtok_r(chan, ".", &save);
			if (temp) {
				c.major = strtoul(temp, NULL, 0);
				temp = strtok_r(NULL, ".", &save);
				c.minor = ((temp) && strlen(temp)) ? strtoul(temp, NULL, 0): 0;
			} else {
				c.major = 0;
				c.minor = 0;
			}
			c.lcn = c.major;

			if (iface) iface->chandump(&c);
			memset(line, 0, sizeof(line));
		}
		fclose(channels_conf);
		return true;
	}
	return false;
}

bool serve_client::cmd_tuner_scan_channels_save()
{
	char cmd_buf[32] = { 0 };
	char *homedir = getenv ("HOME");
	const char *subdir = "/.dvbtee";
	const char *slashchannelsconf = "/channels.conf";
	char dir[/*strlen(homedir)+strlen(subdir)*/64] = { 0 };
	char filepath[/*strlen(dir)+strlen(slashchannelsconf)*/78] = { 0 };
	struct stat st;
	serve_parser_iface chandump_disk_iface(*this, true);

	//snprintf(dir, strlen(dir), "%s%s", homedir, subdir);
	memcpy(dir, homedir, strlen(homedir));
	memcpy(dir + strlen(homedir), subdir, strlen(subdir) + 1);
	//snprintf(filepath, strlen(filepath), "%s%s", dir, slashchannelsconf);
	memcpy(filepath, dir, strlen(dir));
	memcpy(filepath + strlen(dir), slashchannelsconf, strlen(slashchannelsconf) + 1);

	cli_print("%ssaving scanned channel list to %s... \n",
		  tuner->is_scan() ?
		  "waiting for channel scan to complete and " : "",
		  filepath);

	if (stat(dir, &st) != 0) {
		sprintf(cmd_buf, "mkdir -p %s", dir);
		if (system(cmd_buf) < 0) {
			perror("could not create ~/.dvbtee/");
			cli_print("error: could not create %s!\n", dir);
		}
	}

	int channels_fd = creat(filepath, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (channels_fd < 0) {
		perror("could not create channels.conf");
		cli_print("error %d: could not create %s!\n", channels_fd, filepath);
		return false;
	}
	channels_conf_file = fdopen(channels_fd, "w");
	tuner->get_scan_results(true, &chandump_disk_iface);
	fclose(channels_conf_file);
	channels_conf_file = NULL;
	close(channels_fd);
	cli_print("done\n");
	return true;
}

bool serve_client::__command(char* cmdline)
{
	unsigned int scan_flags = 0;
	serve_parser_iface parser_iface(*this, false);
	char *arg, *save;
	char *cmd = strtok_r(cmdline, CHAR_CMD_SET, &save);

	int feeder_id = -1;
	int tuner_id  = -1;

	if (!cmd)
		cmd = cmdline;
	arg = strtok_r(NULL, CHAR_CMD_SET, &save);

	if (strstr(cmd, "tuner")) {
		if ((arg) && strlen(arg)) {
			tuner_id = strtoul(arg, NULL, 0);
			if (tuner_id >= 0) {
				tuner = (tuners.count(tuner_id)) ? tuners[tuner_id] : NULL;
				if (tuner) feeder = &tuner->feeder;
			}
		} else {
			list_tuners();
		}
		return true;
	} else
	if (strstr(cmd, "feeder")) {
		if ((arg) && strlen(arg)) {
			feeder_id = strtoul(arg, NULL, 0);
			if (feeder_id >= 0) {
				feeder = (feeders.count(feeder_id)) ? feeders[feeder_id] : NULL;
				if (feeder) tuner = NULL;
			}
		} else {
			list_feeders();
		}
		return true;
	}

	if ((tuner) && (!feeder))
		feeder = &tuner->feeder;

	if ((!feeder) && (tuners.size())) {
#if 0
		tuner = find_idle_tuner();
#else
		tuner_id = 0;
		tuner = (tuners.count(tuner_id)) ? tuners[tuner_id] : NULL;
#endif
		if (tuner) feeder = &tuner->feeder;
	}

	if ((!feeder) && (feeders.size())) {
		feeder_id = 0;
		feeder = (feeders.count(feeder_id)) ? feeders[feeder_id] : NULL;
		if (feeder) tuner = NULL;
	}

	if (tuner)
		scan_flags = server->get_scan_flags(tuner);

	if (!feeder) {
		cli_print("NO TUNER / FEEDER!\n");
		return false;
	} else
	if (strstr(cmd, "scan")) {
		if (!tuner) {
			cli_print("NO TUNER!\n");
			return false;
		}
		cli_print("scanning for services...\n");
		server->cmd_tuner_scan(tuner, arg,
				      (strstr(cmd, "scanepg")) ? true : false,
				      (strstr(cmd, "startscan")) ? false : true,
				      scan_flags, NULL, &parser_iface);

	} else if (strstr(cmd, "open")) {
		if ((arg) && strlen(arg)) {
			cli_print("opening %s...", arg);
			if (strstr(cmd, "openfile")) {
				if (feeder->open_file(arg) < 0) {
					cli_print("ERROR: open_file(%s) failed.\n", arg);
					return false;
				}
				if (!feeder->start()) {
					cli_print("ERROR: start() failed.\n");
					return false;
				}
			} else
			if (strstr(cmd, "opensock")) {
				feeder->start_socket(arg);
			} else if (strstr(cmd, "opensock")) {
				cli_print("ERROR: http stream not yet supported in this mode\n");
			} else
			cli_print("ERROR: don't know how to open %s. Try opensock / openfile\n", arg);
		}
	} else if (strstr(cmd, "tune")) {
		char *cmdtune, *ser = NULL;
		unsigned int phy = 0;
		unsigned int cur = 0;
		bool tuned = false;

		if (!tuner) {
			cli_print("NO TUNER!\n");
			return false;
		}
		if ((arg) && strlen(arg)) {
			phy = strtoul(strtok_r(arg, ".-+~", &cmdtune), NULL, 0);
			ser = strtok_r(NULL, ".-+~", &cmdtune);
		}

		cli_print("preparing to tune to physical channel %d, service %s...\n", phy, (!ser) ? "?" : ser);

		/* see if tuner has the right physical channel, if not
		 * then find another tuner that does or change it */
		cur = tuner->get_channel();
		if ((cur) && (cur != phy)) {
#if TUNER_RESOURCE_SHARING
			tune *old_tuner = tuner;
			tune *new_tuner = find_tuned_tuner(phy);
			if (!new_tuner) new_tuner = find_idle_tuner();

			if (new_tuner) {
				tuner = new_tuner;
				feeder = &tuner->feeder;
				cur = tuner->get_channel();
			} else {
				cmd_tuner_stop();
				tuner->feeder.parser.reset_output_pids();
			}
			if ((tuner == new_tuner) && (tuner != old_tuner))
				cli_print("found another tuner more suitable for physical channel %d.\n", phy);
#else
			cmd_tuner_stop();
			tuner->feeder.parser.reset_output_pids();
#endif
		}
		if (!tuner) {
			cli_print("NO TUNER!\n");
			return false;
		}
		if (cur == phy) /* (cur) */ {
			cli_print("already tuned to physical channel %d.\n", phy);
			tuned = true;
		} else if (arg) {
			tuned = cmd_tuner_channel(strtoul(arg, NULL, 0), scan_flags);
		} // else tuned = false;

		if (tuned) {
			/* set service, if any */
			services.clear();
			if ((ser) && strlen(ser)) {
				cli_print("selecting service id (%s)...\n", ser);
				tuner->feeder.parser.set_service_ids(ser);
				services.append(ser);
			}
#if 0
			else
				tuner->feeder.parser.set_service_ids(NULL);
#endif
		}

	} else if (strstr(cmd, "channels")) {
		if (!tuner) {
			cli_print("NO TUNER!\n");
			return false;
		}
		cli_print("dumping channel list...\n");

		std::string str;

		if (data_fmt == SERVE_DATA_FMT_XML) {
			str = xml_dump_epg_header_footer_callback(this, true, false);
			streamback((const uint8_t *)str.c_str(), str.length());
		}
		if (data_fmt == SERVE_DATA_FMT_JSON) {
			str = json_dump_epg_header_footer_callback(this, true, false);
			streamback((const uint8_t *)str.c_str(), str.length());
		}

		/* channels verified during this session */
		tuner->get_scan_results(false, &parser_iface);
		/* load remaining channels that we saved previously but havent seen during this session */
		server->cmd_config_channels_conf_load(tuner, &parser_iface);

		if (data_fmt == SERVE_DATA_FMT_XML) {
			str = xml_dump_epg_header_footer_callback(this, false, false);
			streamback((const uint8_t *)str.c_str(), str.length());
		}
		if (data_fmt == SERVE_DATA_FMT_JSON) {
			str = " {} ";
			streamback((const uint8_t *)str.c_str(), str.length());
			str = json_dump_epg_header_footer_callback(this, false, false);
			streamback((const uint8_t *)str.c_str(), str.length());
		}

	} else if (strstr(cmd, "channel")) {
		if (!tuner) {
			cli_print("NO TUNER!\n");
			return false;
		}
		if ((arg) && strlen(arg))
			cmd_tuner_channel(strtoul(arg, NULL, 0), scan_flags);
		else
			cli_print("missing channel number?\n");

	} else if (strstr(cmd, "service")) {
		cli_print("selecting service id...\n");
		services.clear();
		if ((arg) && strlen(arg)) {
			feeder->parser.set_service_ids(arg);
			services.append(arg);
		} else
			feeder->parser.set_service_ids(NULL);

	} else if (strstr(cmd, "stream")) {
#if TUNER_RESOURCE_SHARING
		cli_print("wait for psip...\n");
		feeder->wait_for_psip(1000); /* FIXME */
#endif
		cli_print("adding stream target id#%4d...\n",
			  ((arg) && strlen(arg)) ?
			  feeder->parser.add_output(arg, (char*)services.c_str()) :
			  feeder->parser.add_output(sock_fd, OUTPUT_STREAM_HTTP, (char*)services.c_str()));

	} else if (strstr(cmd, "video")) {
		if (data_fmt == SERVE_DATA_FMT_HTML) {
			cli_print("streaming video via html5...\n");
			std::string str = html_playing_video(this);
			streamback((const uint8_t *)str.c_str(), str.length());
		}
	} else if (strstr(cmd, "epg")) {
		cli_print("dumping epg...\n");

		std::string str;

		if (data_fmt == SERVE_DATA_FMT_XML) {
			str = xml_dump_epg_header_footer_callback(this, true, false);
			streamback((const uint8_t *)str.c_str(), str.length());
		}

		feeder->parser.epg_dump(this);

		if (data_fmt == SERVE_DATA_FMT_XML) {
			str = xml_dump_epg_header_footer_callback(this, false, false);
			streamback((const uint8_t *)str.c_str(), str.length());
		}

	} else if (strstr(cmd, "xmltv")) {
		if (!tuner) {
			cli_print("NO TUNER!\n");
			return false;
		}
		cli_print("dumping xmltv...\n");
		std::string str;

		str = xml_dump_epg_header_footer_callback(this, true, false);
		streamback((const uint8_t *)str.c_str(), str.length());

		/* channels verified during this session */
		tuner->get_scan_results(false, &parser_iface);
		/* load remaining channels that we saved previously but havent seen during this session */
		server->cmd_config_channels_conf_load(tuner, &parser_iface);
		/* and finally, the epg data */
		feeder->parser.epg_dump(this);

		str = xml_dump_epg_header_footer_callback(this, false, false);
		streamback((const uint8_t *)str.c_str(), str.length());

	} else if (strstr(cmd, "info")) {
		char *cmdinfo;
		uint16_t ser = 0;
		unsigned int phy = 0;
#if 0
		if (!tuner) {
			cli_print("NO TUNER!\n");
			return false;
		}
#endif
		if ((arg) && strlen(arg)) {
			phy = strtoul(strtok_r(arg, ".-+~", &cmdinfo), NULL, 0);
			ser = strtoul(strtok_r(NULL, ".-+~", &cmdinfo), NULL, 0);
		}

		cli_print("dumping stream info for physical channel %d, service %d...\n", phy, ser);
		parsed_channel_info_t c;
		decoded_event_t e[2];

		if (feeder->parser.get_stream_info(phy, ser, &c, &e[0], &e[1])) {

			std::string str;

			if (data_fmt == SERVE_DATA_FMT_XML) {
				str = xml_dump_epg_header_footer_callback(this, true, false);
				streamback((const uint8_t *)str.c_str(), str.length());
			}
			if (data_fmt == SERVE_DATA_FMT_JSON) {
				str = json_dump_epg_header_footer_callback(this, true, false);
				streamback((const uint8_t *)str.c_str(), str.length());
			}

			streamback_started = true;
			parser_iface.chandump(&c);

			if (data_fmt == SERVE_DATA_FMT_HTML) {
				str = html_dump_epg_header_footer_callback(this, true, false);
				streamback((const uint8_t *)str.c_str(), str.length());
			}

			epg_event(e[0]);

			if (data_fmt == SERVE_DATA_FMT_HTML) {
				str = html_dump_epg_header_footer_callback(this, false, false);
				streamback((const uint8_t *)str.c_str(), str.length());
				str = html_dump_epg_header_footer_callback(this, true, false);
				streamback((const uint8_t *)str.c_str(), str.length());
			}
			if (data_fmt == SERVE_DATA_FMT_JSON) {
				str = ",";
				streamback((const uint8_t *)str.c_str(), str.length());
			}

			epg_event(e[1]);

			if (data_fmt == SERVE_DATA_FMT_HTML) {
				str = html_dump_epg_header_footer_callback(this, false, false);
				streamback((const uint8_t *)str.c_str(), str.length());
			}

			streamback_started = false;

			if (data_fmt == SERVE_DATA_FMT_XML) {
				str = xml_dump_epg_header_footer_callback(this, false, false);
				streamback((const uint8_t *)str.c_str(), str.length());
			}
			if (data_fmt == SERVE_DATA_FMT_JSON) {
//				str = " {} ";
//				streamback((const uint8_t *)str.c_str(), str.length());
				str = json_dump_epg_header_footer_callback(this, false, false);
				streamback((const uint8_t *)str.c_str(), str.length());
			}
		}

	} else if (strstr(cmd, "stop")) {
		bool stop_output = (strstr(cmd, "stopoutput")) ? true : false;
		int output_id = ((arg) && strlen(arg)) ? strtoul(arg, NULL, 0) : -1;

		if ((!stop_output) || (output_id == -1)) {
			if (tuner)
				cmd_tuner_stop();
			else
				feeder->stop();
		}
		if (stop_output) {
			if (output_id == -1) {
				cli_print("stopping output...\n");
				feeder->parser.stop();
			} else {
				cli_print("stopping output #%d...\n", output_id);
				feeder->parser.stop(output_id);
			}
		}
	} else if (strstr(cmd, "check")) {
		cli_print("checking server status...\n");
		server->check();
		list_clients();
		if (tuner) {
			cli_print("checking tuner status...\n");
			tuner->check();
		}
		list_tuners();
		cli_print("checking feeder status...\n");
		feeder->check();
		list_feeders();
		cli_print("checking parser / output status...\n");
		feeder->parser.check();
	} else if (strstr(cmd, "debug")) {
		cli_print("setting debug level...\n");
		if ((arg) && strlen(arg))
			libdvbtee_set_debug_level(strtoul(arg, NULL, 0));
		else
			libdvbtee_set_debug_level(255);
	} else if (strstr(cmd, "parser")) {
		if ((arg) && strlen(arg))
			feeder->parser.enable((strtoul(arg, NULL, 0)) ? true : false);
		cli_print("parser is %sabled.\n", (feeder->parser.is_enabled()) ? "en" : "dis");
	} else if (strstr(cmd, "listen")) {
		if ((arg) && strlen(arg)) {
			int portnum = strtoul(arg, NULL, 0);
			int ret;
			if (strstr(cmd, "udp")) {
				cli_print("starting TS listener on UDP port %d... ", portnum);
				ret = server->feed_servers[portnum].start_udp_listener(portnum, server);
			} else {
				cli_print("starting TS listener on TCP port %d... ", portnum);
				ret = server->feed_servers[portnum].start_tcp_listener(portnum, server);
			}
			cli_print("%s!\n", (ret < 0) ? "FAILED" : "SUCCESS");
		}
	} else if (strstr(cmd, "save")) {
		if (!tuner) {
			cli_print("NO TUNER!\n");
			return false;
		}
		cmd_tuner_scan_channels_save();

	} else if (strstr(cmd, "quit")) {
		cli_print("stopping server...\n");
		server->stop();
		/* disable CLI prompt */
		if (data_fmt == SERVE_DATA_FMT_CLI)
			data_fmt = SERVE_DATA_FMT_NONE;
		/* prevent further command processing */
		return true;
	}

	return true;
}
