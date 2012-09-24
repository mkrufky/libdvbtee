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

#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>

#include "serve.h"
#include "html.h"

#define CRLF "\r\n"

//FIXME:
#define DBG_SERVE 1

unsigned int dbg_serve = DBG_SERVE;

#define __printf(fd, fmt, arg...) fprintf(fd, fmt, ##arg)

#define __dprintf(lvl, fmt, arg...) do {			  \
    if (dbg_serve & lvl)						  \
      __printf(stderr, "server::%s: " fmt "\n", __func__, ##arg);	  \
  } while (0)

#define dprintf(fmt, arg...) __dprintf(DBG_SERVE, fmt, ##arg)

	tuner_map tuners;

bool serve::add_tuner(tune *new_tuner) { tuners[tuners.size()] = new_tuner; };

/*****************************************************************************/

static inline ssize_t stream_crlf(int socket)
{
	return send(socket, CRLF, 2, 0);
}

static int stream_http_chunk(int socket, const uint8_t *buf, size_t length, const bool send_zero_length = false)
{
	dprintf("(length:%d)", (int)length);

	if (socket < 0)
		return socket;

	if ((length) || (send_zero_length)) {
		int ret = 0;
		char sz[5] = { 0 };
		sprintf(sz, "%x", (unsigned int)length);

		ret = send(socket, sz, strlen(sz), 0);
		if (ret < 0)
			return ret;

		ret = stream_crlf(socket);
		if (ret < 0)
			return ret;

		if (length) {
			ret = send(socket, buf, length, 0);
			if (ret < 0)
				return ret;

			ret = stream_crlf(socket);
			if (ret < 0)
				return ret;
		}
	}
	return 0;
}

/*****************************************************************************/

serve_client::serve_client()
  : f_kill_thread(false)
  , sock_fd(-1)
  , data_fmt(SERVE_DATA_FMT_NONE)
  , reporter(NULL)
{
	dprintf("()");
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
	f_kill_thread = false;
	sock_fd = -1;
	data_fmt = SERVE_DATA_FMT_NONE;
}

serve_client& serve_client::operator= (const serve_client& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	f_kill_thread = false;
	sock_fd = -1;
	data_fmt = SERVE_DATA_FMT_NONE;

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
	char buf[1024] = { 0 };
	int rxlen;

	getpeername(sock_fd, (struct sockaddr*)&tcpsa, &salen);
	dprintf("(%d)", sock_fd);
#if 0
	data_fmt = SERVE_DATA_FMT_NONE;
#endif
	while (!f_kill_thread) {
		rxlen = recv(sock_fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (rxlen > 0) {
			fprintf(stderr, "%s: %s\n", __func__, buf);

			if ((strstr(buf, "HTTP")) && (strstr(buf, "GET"))) {
				data_fmt =	(strstr(buf, "stream/")) ? SERVE_DATA_FMT_BIN :
						(strstr(buf, "json/")) ? SERVE_DATA_FMT_JSON : SERVE_DATA_FMT_HTML;
			}

			command(buf); /* process */
		} else if ( /*(rxlen == 0) ||*/ ( (rxlen == -1) && (errno != EAGAIN) ) ) {
			if (data_fmt != SERVE_DATA_FMT_BIN)
				stop_without_wait();
		} else
			usleep(20*1000);
	}

	close_socket();
	pthread_exit(NULL);
}

serve::serve()
{
	dprintf("()");
	tuners.clear();
}

serve::~serve()
{
	dprintf("()");
	tuners.clear();

	stop();
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

//static
void serve_client::streamback(void *p_this, const char *str)
{
	return static_cast<serve_client*>(p_this)->streamback((uint8_t *)str, strlen(str));
}

//static
void serve_client::streamback(void *p_this, const uint8_t *str, size_t length)
{
	return static_cast<serve_client*>(p_this)->streamback(str, length);
}

void serve_client::streamback(const uint8_t *str, size_t length)
{
	stream_http_chunk(sock_fd, str, length);
}

#define MAX_SOCKETS 4
#define HTTP_200_OK  "HTTP/1.1 200 OK"
#define CONTENT_TYPE "Content-type: "
#define TEXT_HTML    "text/html"
#define TEXT_PLAIN   "text/plain"
#define ENC_CHUNKED  "Transfer-Encoding: chunked"
#define CONN_CLOSE   "Connection: close"

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

static char json_response[] =
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

static char http_conn_close[] =
	 CONN_CLOSE
	 CRLF
	 CRLF;

void serve_client::epg_header_footer_callback(void *context, bool header, bool channel)
{
	return static_cast<serve_client*>(context)->epg_header_footer_callback(header, channel);
}


void serve_client::epg_header_footer_callback(bool header, bool channel)
{
	dprintf("()");
	if ((header) && (!channel)) streamback_started = true;
	if (!streamback_started) return;
	if ((header) && (channel)) streamback_newchannel = true;
	if (data_fmt & SERVE_DATA_FMT_TEXT) {
		const char *str;
		switch (data_fmt) {
		default: // FIXME
		case SERVE_DATA_FMT_HTML:
			str = html_dump_epg_header_footer_callback(this, header, channel);
			break;
		case SERVE_DATA_FMT_JSON:
			str = json_dump_epg_header_footer_callback(this, header, channel);
			break;
		}
		streamback((const uint8_t *)str, strlen(str));
	}
	if ((!header) && (!channel)) fflush(stdout);
	return;
}

void serve_client::epg_event_callback(void * context,
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
	return static_cast<serve_client*>(context)->epg_event_callback(channel_name, chan_major, chan_minor, event_id, start_time, length_sec, name, text);
}

void serve_client::epg_event_callback(
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
	dprintf("()");
	if (!streamback_started) return;
#if 1
	if (streamback_newchannel) {
		if (data_fmt == SERVE_DATA_FMT_HTML) {
			const char *str = html_dump_epg_event_callback(this, channel_name, chan_major, chan_minor, 0, 0, 0, NULL, NULL);
			streamback((const uint8_t *)str, strlen(str));
		}
		streamback_newchannel = false;
		fflush(stdout);
	}
#endif
	if (data_fmt & SERVE_DATA_FMT_TEXT) {
		const char *str;
		switch (data_fmt) {
		default: // FIXME
		case SERVE_DATA_FMT_HTML:
			str = html_dump_epg_event_callback(this, NULL, 0, 0, event_id, start_time, length_sec, name, text);
			break;
		case SERVE_DATA_FMT_JSON:
			str = json_dump_epg_event_callback(this, NULL, 0, 0, event_id, start_time, length_sec, name, text);
			break;
		}
		streamback((const uint8_t *)str, strlen(str));
	}
	return;
}


//static
void serve::add_client(void *p_this, int socket)
{
	return static_cast<serve*>(p_this)->add_client(socket);
}

void serve::add_client(int socket)
{
	if (socket < 0) {
		dprintf("not attaching to invalid socket, %d", socket);
		return;
	}

	/* check for old clients & clean them up */
	for (serve_client_map::iterator iter = client_map.begin(); iter != client_map.end(); ++iter)
		if (!iter->second.socket_active())
			client_map.erase(iter->first);

	client_map[socket].setup(this, socket);
	client_map[socket].start();
}

int serve::start(uint16_t port_requested)
{
	dprintf("()");

	listener.set_callback(this, add_client);

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
		const char *fmt;
		switch (data_fmt) {
		case SERVE_DATA_FMT_NONE:
			fmt = "NONE";
			break;
		case SERVE_DATA_FMT_HTML:
			fmt = "HTTP";
			break;
		case SERVE_DATA_FMT_BIN:
			fmt = "BIN";
			break;
		case SERVE_DATA_FMT_JSON:
			fmt = "JSON";
			break;
		}
		dprintf("(%d) format = %s", sock_fd, fmt);
	}
	return ret;
}

bool serve::check()
{
	dprintf("()");

	for (serve_client_map::iterator iter = client_map.begin(); iter != client_map.end(); ++iter)
		if (!iter->second.check())
			client_map.erase(iter->first);

	return true;
}

#if 1//def PRETTY_URLS
#define CHAR_CMD_SEP "&/"
#define CHAR_CMD_SET "="
#else
#define CHAR_CMD_SEP ";"
#define CHAR_CMD_SET "/"
#endif

#define USE_JSON (data_fmt & SERVE_DATA_FMT_JSON)

bool serve_client::command(char* cmdline)
{
	char *save;
	bool ret = false;
	char *item = strtok_r(cmdline, CHAR_CMD_SEP, &save);
	bool stream_http_headers = (data_fmt & SERVE_DATA_FMT_TEXT) ? true : false;
#if 1
	if (stream_http_headers) {
		streamback_newchannel = false;
		streamback_started = false;
		reporter = new decode_report;
		reporter->set_dump_epg_cb(this,
				epg_header_footer_callback,
				epg_event_callback);
		if (USE_JSON)
			send(sock_fd, json_response, strlen(json_response), 0);
		else
			send(sock_fd, http_response, strlen(http_response), 0);
	}
#endif
	if (item) while (item) {
		if (!item)
			item = cmdline;

		ret = __command(item);
		if (!ret)
			return ret;

		item = strtok_r(NULL, CHAR_CMD_SEP, &save);
	} else
		ret = __command(cmdline);
#if 1
	if (stream_http_headers) {
		stream_http_chunk(sock_fd, (uint8_t *)"", 0, true);
		send(sock_fd, http_conn_close, strlen(http_conn_close), 0);
//		close_socket();
		delete reporter;
		reporter = NULL;
	}
#endif
	return ret;
}

const char * serve_client::chandump(void *context,
		     uint16_t lcn, uint16_t major, uint16_t minor,
		     uint16_t physical_channel, uint32_t freq, const char *modulation,
		     unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number)
{
	return static_cast<serve_client*>(context)->chandump(lcn, major, minor,
		physical_channel, freq, modulation, service_name, vpid, apid, program_number);
}

const char * serve_client::chandump(
		     uint16_t lcn, uint16_t major, uint16_t minor,
		     uint16_t physical_channel, uint32_t freq, const char *modulation,
		     unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number)
{
	char channelno[7]; /* XXX.XXX */
	if (major + minor > 1)
		sprintf(channelno, "%d.%d", major, minor);
	else if (lcn)
		sprintf(channelno, "%d", lcn);
	else
		sprintf(channelno, "%d", physical_channel);
#if 0
	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		service_name,
		freq,//iter_vct->second.carrier_freq,
		modulation,
		vpid, apid, program_number);
#else
	fprintf(stdout, "%s-%s: service/%d;channel/%d\n",
		channelno,
		service_name,
		program_number,
		physical_channel);
#endif
	const char *str = (USE_JSON) ?
		json_dump_channels(this, lcn, major, minor, physical_channel, freq, modulation, service_name, vpid, apid, program_number) :
		html_dump_channels(this, lcn, major, minor, physical_channel, freq, modulation, service_name, vpid, apid, program_number);

	streamback((const uint8_t *)str, strlen(str));

	return str;
}

bool serve_client::__command(char* cmdline)
{
	char *arg, *save;
	char *cmd = strtok_r(cmdline, CHAR_CMD_SET, &save);

	if (!cmd)
		cmd = cmdline;
	arg = strtok_r(NULL, CHAR_CMD_SET, &save);

	unsigned int tuner_id, scan_flags = 0;

	if (strstr(cmd, "tuner")) {
		tuner_id = atoi(arg);
		cmd = strtok_r(NULL, CHAR_CMD_SET, &save);
		arg = strtok_r(NULL, CHAR_CMD_SET, &save);
	} else
		tuner_id = 0;

	scan_flags = server->get_scan_flags(tuner_id);

	tune* tuner = (tuners.count(tuner_id)) ? tuners[tuner_id] : NULL;
	if (!tuner) {
		fprintf(stderr, "NO TUNER!\n");
		return false;
	}
//	if (strstr(cmd, "channels")) {
//		fprintf(stderr, "dumping channel list...\n");
	if (strstr(cmd, "scan")) {
		fprintf(stderr, "scanning for services...\n");

		if (!scan_flags)
			scan_flags = SCAN_VSB;

		if ((arg) && strlen(arg))
			tuner->scan_for_services(scan_flags, arg, (strstr(cmd, "epg")) ? true : false, chandump, this);
		else
			tuner->scan_for_services(scan_flags, 0, 0, (strstr(cmd, "epg")) ? true : false, chandump, this);

	} else if (strstr(cmd, "channels")) {
		fprintf(stderr, "dumping channel list...\n");

		tuner->get_scan_results(false, chandump, this);

	} else if (strstr(cmd, "channel")) {
		if ((arg) && strlen(arg)) {

		int channel = atoi(arg);
		fprintf(stderr, "TUNE to channel %d...(%s)\n", channel, arg);
		if (tuner->open_fe() < 0) {
			fprintf(stderr, "open_fe() failed!\n");
			return false;
		}
		if (!scan_flags)
			scan_flags = SCAN_VSB;

		if (tuner->tune_channel((scan_flags == SCAN_VSB) ? VSB_8 : QAM_256, channel)) {

			if (!tuner->wait_for_lock_or_timeout(2000)) {
				tuner->close_fe();
				fprintf(stderr, "no lock!\n");
				return false; /* NO LOCK! */
			}
			tuner->feeder.parser.set_channel_info(channel,
							     (scan_flags == SCAN_VSB) ? atsc_vsb_chan_to_freq(channel) : atsc_qam_chan_to_freq(channel),
							     (scan_flags == SCAN_VSB) ? "8VSB" : "QAM_256");
			tuner->start_feed();
		}} else fprintf(stderr, "missing channel number?\n");

	} else if (strstr(cmd, "service")) {
		fprintf(stderr, "selecting service id...\n");
		if ((arg) && strlen(arg))
			tuner->feeder.parser.set_service_ids(arg);
		else
			tuner->feeder.parser.set_service_ids(NULL);

	} else if (strstr(cmd, "stream")) {
		fprintf(stderr, "adding stream target...\n");
		if ((arg) && strlen(arg))
			tuner->feeder.parser.add_output(arg);
		else {
			tuner->feeder.parser.add_output(sock_fd, OUTPUT_STREAM_HTTP);
			sock_fd = -1; /* disconnect socket from the server process as its attached to the output process */
		}
	} else if (strstr(cmd, "epg")) {
		fprintf(stderr, "dumping epg...\n");
		tuner->feeder.parser.epg_dump(reporter);
	} else if (strstr(cmd, "stop")) {
		fprintf(stderr, "stopping...\n");
		tuner->stop_feed();
		fprintf(stderr, "closing frontend...\n");
		tuner->close_fe();
		if (strstr(cmd, "stopoutput")) {
			fprintf(stderr, "stopping output...\n");
			tuner->feeder.parser.stop();
		}
	} else if (strstr(cmd, "check")) {
		fprintf(stderr, "checking server status...\n");
		server->check();
		fprintf(stderr, "checking tuner status...\n");
		tuner->check();
		fprintf(stderr, "checking feeder status...\n");
		tuner->feeder.check();
		fprintf(stderr, "checking parser / output status...\n");
		tuner->feeder.parser.check();
	} else if (strstr(cmd, "debug")) {
		fprintf(stderr, "setting debug level...\n");
		if ((arg) && strlen(arg))
			libdvbtee_set_debug_level(strtoul(arg, NULL, 0));
		else
			libdvbtee_set_debug_level(255);
	} else if (strstr(cmd, "quit")) {
		fprintf(stderr, "stopping server...\n");
		server->stop();
	}

	return true;
}
