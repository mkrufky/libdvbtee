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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>

#include "tune.h"
#include "log.h"
#define CLASS_MODULE "tune"

#define dprintf(fmt, arg...) __dprintf(DBG_TUNE, fmt, ##arg)

typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;

static map_chan_to_ts_id channels;

tune::tune()
  : h_thread((pthread_t)NULL)
  , f_kill_thread(false)
  , state(TUNE_STATE_IDLE)
  , cur_chan(0)
  , time_touched((time_t)0)
  , scan_mode(0)
  , scan_epg(false)
  , scan_complete(false)
  , fe_type(DVBTEE_FE_OFDM)
  , last_query((time_t)0)
  , m_iface(NULL)
{
	dprintf("()");
//	channels.clear();
}

tune::~tune()
{
	dprintf("()");
	stop_feed();
	close_fe();
//	channels.clear();
}

tune::tune(const tune&)
{
	dprintf("(copy)");

//	channels.clear();
	feeder.parser.cleanup();
	h_thread = (pthread_t)NULL;
	f_kill_thread = false;
	cur_chan = 0;
	state = TUNE_STATE_IDLE;
	time_touched = (time_t)0;
	scan_mode = 0;
	scan_epg = false;
	scan_complete = false;
	fe_type = DVBTEE_FE_OFDM;
	last_query = (time_t)0;
	m_iface = NULL;
}

tune& tune::operator= (const tune& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

//	channels.clear();
	feeder.parser.cleanup();
	h_thread = (pthread_t)NULL;
	f_kill_thread = false;
	cur_chan = 0;
	state = TUNE_STATE_IDLE;
	time_touched = (time_t)0;
	scan_mode = 0;
	scan_epg = false;
	scan_complete = false;
	fe_type = DVBTEE_FE_OFDM;
	last_query = (time_t)0;
	m_iface = NULL;

	return *this;
}

int tune::close_fe() {
	cur_chan = 0;
	state &= ~TUNE_STATE_OPEN;
	state &= ~TUNE_STATE_LOCK;
	return 0;
}

bool tune::wait_for_lock_or_timeout(unsigned int time_ms)
{
	unsigned int status = (dvbtee_fe_status_t)0;
	time_t start_time = time(NULL);
	while ((0 == ((status |= fe_status()) & DVBTEE_FE_HAS_LOCK)) && ( (time(NULL) - start_time) < ((int)time_ms / 1000) ))
		usleep(200*1000);
	if ((status & (DVBTEE_FE_HAS_LOCK | DVBTEE_FE_HAS_SYNC)) == DVBTEE_FE_HAS_SYNC) {
		start_time = time(NULL);
		while ((0 == ((status |= fe_status()) & DVBTEE_FE_HAS_LOCK)) && ( (time(NULL) - start_time) < (2 * (int)time_ms / 1000) ))
			usleep(200*1000);
	}
	return ((status & DVBTEE_FE_HAS_LOCK) == DVBTEE_FE_HAS_LOCK);
}

bool tune::tune_channel(dvbtee_fe_modulation_t modulation, unsigned int channel)
{
	bool ret = __tune_channel(modulation, channel);

	feeder.parser.reset();

	return ret;
}

void tune::stop_feed()
{
	feeder.stop();
	state &= ~TUNE_STATE_FEED;
	feeder.close_file();
}

time_t tune::last_touched() // sec_ago
{
	time_t timenow;
	time(&timenow);
	time_t ret = timenow - time_touched;
	if (timenow - last_query)
		dprintf("last touched %ld seconds ago", ret);
	time(&last_query);
	return ret;
}

//static
void* tune::scan_thread(void *p_this)
{
	return static_cast<tune*>(p_this)->scan_thread();
}

void* tune::scan_thread()
{
	if (!is_scan()) {

	scan_progress_t progress;

	progress.total = (unsigned int)scan_channel_list.size(),
	progress.current = 0,
	progress.physical_channel = 0,

	state |= TUNE_STATE_SCAN;

	feeder.parser.set_scan_mode(true);
	feeder.parser.set_epg_mode(scan_epg);

	for (channel_map::const_iterator iter = scan_channel_list.begin(); iter != scan_channel_list.end(); ++iter) {
		unsigned int channel = iter->first;
		progress.current++;
		progress.physical_channel = channel;

		if (f_kill_thread)
			break;
#if 0
		map_chan_to_ts_id::const_iterator iter = channels.find(channel);
		if (iter != channels.end()) {
#else
		if (channels.count(channel)) {
#endif
			fprintf(stderr, "ALREADY SCANNED CHANNEL %d\n", channel);
			continue;
		}

		fprintf(stderr, "scan channel %d...\n", channel);

		if (m_iface) m_iface->scan_progress(&progress);

		if ((!f_kill_thread) && ((tune_channel((scan_mode == SCAN_VSB) ? DVBTEE_VSB_8 : DVBTEE_QAM_256, channel)) && (wait_for_lock_or_timeout(2000)))) {

			if (f_kill_thread)
				break;

			switch (fe_type) {
			default:
			case DVBTEE_FE_ATSC:
				feeder.parser.set_channel_info(channel,
							       (scan_mode == SCAN_VSB) ? atsc_vsb_chan_to_freq(channel) : atsc_qam_chan_to_freq(channel),
							       (scan_mode == SCAN_VSB) ? "8VSB" : "QAM_256");
				break;
			case DVBTEE_FE_OFDM:
				feeder.parser.set_channel_info(channel, dvbt_chan_to_freq(channel),
							       ((channel <= 12) ?
								"INVERSION_AUTO:BANDWIDTH_7_MHZ:FEC_AUTO:FEC_AUTO:QAM_AUTO:TRANSMISSION_MODE_AUTO:GUARD_INTERVAL_AUTO:HIERARCHY_AUTO" :
								"INVERSION_AUTO:BANDWIDTH_8_MHZ:FEC_AUTO:FEC_AUTO:QAM_AUTO:TRANSMISSION_MODE_AUTO:GUARD_INTERVAL_AUTO:HIERARCHY_AUTO"));
				break;
			}
			if (0 == start_feed()) {
				int timeout = (scan_epg) ? 16 : (fe_type == DVBTEE_FE_ATSC) ? 4 : 12;
				while ((!f_kill_thread) && (timeout)) {
					if (scan_epg)
						feeder.wait_for_epg(1000);
					else
						feeder.wait_for_psip(1000);
					timeout--;
				}
				stop_feed();
				channels[channel] = feeder.parser.get_ts_id();
			} // else what if we cant start the feed???
		}
	}
	close_fe();
	scan_complete = true;
	state &= ~TUNE_STATE_SCAN;
	}
	pthread_exit(NULL);
}

#define CHAR_CMD_COMMA ","

int tune::start_scan(unsigned int mode, char *channel_list, bool epg, tune_iface *iface)
{
	char *save;
	char *item = strtok_r(channel_list, CHAR_CMD_COMMA, &save);

	scan_channel_list.clear();

	if (item) while (item) {
#if 0
		if (!item)
			item = channel_list;
#endif
		scan_channel_list[atoi(item)] = false;

		item = strtok_r(NULL, CHAR_CMD_COMMA, &save);
	} else
		scan_channel_list[atoi(channel_list)] = false;

	return start_scan(mode, epg, iface);
}

int tune::start_scan(unsigned int mode, unsigned int min, unsigned int max, bool epg, tune_iface *iface)
{
	//channels.clear();
	scan_channel_list.clear();

	unsigned int scan_min = min;
	unsigned int scan_max = max;

	if (mode != SCAN_QAM)
		mode = SCAN_VSB;

	scan_mode = mode;

	switch (scan_mode) {
	default:
	case SCAN_VSB:
		scan_min = (min) ? min : 2;
		scan_max = (max) ? max : 69;
		break;
	case SCAN_QAM:
		scan_min = (min) ? min : 2;
		scan_max = (max) ? max : 133;
		break;
	}

	for (unsigned int channel = scan_min; channel <= scan_max; channel++)
		scan_channel_list[channel] = false; // TODO: set true if channel found

	return start_scan(mode, epg, iface);
}

int tune::start_scan(unsigned int mode, bool epg, tune_iface *iface)
{
	m_iface = iface;

	if (mode != SCAN_QAM)
		mode = SCAN_VSB;

	scan_mode = mode;

	scan_epg = epg;

	int fd = open_fe();
	if (fd < 0)
		return fd;

	scan_complete = false;
	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, scan_thread, this);
	if (0 != ret) {
		perror("pthread_create() failed");
		close_fe();
	}
	return ret;
}

unsigned int tune::get_scan_results(bool wait, chandump_callback chandump_cb, void* chandump_context)
{
	if (wait) wait_for_scan_complete();

	unsigned int ret = feeder.parser.xine_dump(chandump_cb, chandump_context);

	if (scan_epg) feeder.parser.epg_dump();

	return ret;
}

int tune::scan_for_services(unsigned int mode, char *channel_list, bool epg, tune_iface *iface, chandump_callback chandump_cb, void* chandump_context, bool wait_for_results)
{
	unsigned int count = 0;

	if (!mode)
		mode = SCAN_VSB;

	if (0 != start_scan(scan_mode, channel_list, epg, iface))
		return -1;

	if (wait_for_results) {
		count += get_scan_results(true, chandump_cb, chandump_context);
		fprintf(stderr, "found %d services\n", count);
	}
	return 0;
}

int tune::scan_for_services(unsigned int mode, unsigned int min, unsigned int max, bool epg, tune_iface *iface, chandump_callback chandump_cb, void* chandump_context, bool wait_for_results)
{
	unsigned int count = 0;
	unsigned int total_count = 0;

	if (!mode)
		mode = SCAN_VSB;

	for (scan_mode = SCAN_VSB; scan_mode <= SCAN_QAM; scan_mode++) if (mode & scan_mode) {

		count = 0;

		if (0 != start_scan(scan_mode, min, max, epg, iface))
			return -1;

		if (wait_for_results) {

			count += get_scan_results(true, chandump_cb, chandump_context);
			total_count += count;
#if 0
			for (map_chan_to_ts_id::const_iterator iter = channels.begin(); iter != channels.end(); ++iter)
				fprintf(stderr, "found ts_id %05d on channel %d\n", iter->second, iter->first);
			channels.clear(); //
#endif
			fprintf(stderr, "found %d services\n", count);
		}
	}
	if (count != total_count)
		fprintf(stderr, "found %d services in total\n", total_count);
	return 0;
}
