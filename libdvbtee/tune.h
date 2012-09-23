/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
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

#ifndef __TUNE_H__
#define __TUNE_H__

#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#include "channels.h"
#include "feed.h"

#if 0
#include <map>
typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;
static map_chan_to_ts_id channels;
#endif

typedef std::map<uint16_t, int> filtered_pid_map; /* pid, fd */
typedef std::map<unsigned int, bool> channel_map; /* channel, found? */

class tune
{
public:
	tune();
	~tune();

	tune(const tune&);
	tune& operator= (const tune&);

	bool set_device_ids(int adap, int fe, int demux, int dvr, bool kernel_pid_filter = true);

	int open_fe();
	int close_fe();
	int close_demux();

	void stop_feed();
	int start_feed();

	bool wait_for_lock_or_timeout(unsigned int);

	bool tune_channel(fe_modulation_t, unsigned int);

	bool check();

#define SCAN_VSB 1
#define SCAN_QAM 2
	int scan_for_services(unsigned int, char *, bool epg = false, chandump_callback chandump_cb = NULL, void* chandump_context = NULL);
	int scan_for_services(unsigned int, unsigned int, unsigned int, bool epg = false, chandump_callback chandump_cb = NULL, void* chandump_context = NULL);
	int start_scan(unsigned int, char *, bool epg = false);
	int start_scan(unsigned int, unsigned int, unsigned int, bool epg = false);
	void wait_for_scan_complete() { while (!scan_complete) usleep(20*1000); };
	unsigned int get_scan_results(bool wait = true, chandump_callback chandump_cb = NULL, void* chandump_context = NULL);

	feed feeder;
private:
	pthread_t h_thread;
	bool f_kill_thread;

	void *scan_thread();
	static void *scan_thread(void*);

	void add_filter(uint16_t);
	static void add_filter(void *, uint16_t);
	void clear_filters();
	static void clear_filters(void *);

	int  adap_id;

	int    fe_fd;
	int demux_fd;

	int    fe_id;
	int demux_id;
	int   dvr_id;

	unsigned int cur_chan;

	int          scan_mode;
	channel_map  scan_channel_list;
	bool         scan_epg;
	bool         scan_complete;

	//map_chan_to_ts_id channels;

	fe_status_t fe_status();
	uint16_t get_snr();

	fe_type_t fe_type;

	bool tune_atsc(fe_modulation_t, unsigned int);
	bool tune_dvbt(unsigned int);

	filtered_pid_map filtered_pids;

	int start_scan(unsigned int, bool epg = false);
};

#endif /*__TUNE_H__ */
