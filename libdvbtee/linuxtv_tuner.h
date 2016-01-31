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

#ifndef __LINUXTV_TUNER_H__
#define __LINUXTV_TUNER_H__

#include "tune.h"
#include "feed/file.h"

#if 0
#include <map>
typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;
static map_chan_to_ts_id channels;
#endif

typedef std::map<uint16_t, int> filtered_pid_map; /* pid, fd */
typedef std::map<unsigned int, bool> channel_map; /* channel, found? */

class linuxtv_tuner: public tune, public tsfilter_iface
{
public:
	linuxtv_tuner();
	~linuxtv_tuner();

	linuxtv_tuner(const linuxtv_tuner&);
	linuxtv_tuner& operator= (const linuxtv_tuner&);

	bool set_device_ids(int adap, int fe, int demux, int dvr, bool kernel_pid_filter = true);

	int open_fe();
	int close_fe();

	void stop_feed();
	int start_feed();

	bool __tune_channel(dvbtee_fe_modulation_t, unsigned int);

	virtual const char *get_name() { return fileFeeder.getUri().c_str(); }

	bool check();

	void addfilter(uint16_t);
private:
	dvbtee::feed::FileFeeder fileFeeder;

	void add_filter(uint16_t);
	void clear_filters();
	static void clear_filters(void *);

	int  adap_id;

	int    fe_fd;
	int demux_fd;

	int    fe_id;
	int demux_id;
	int   dvr_id;

	dvbtee_fe_status_t fe_status();
	uint16_t get_snr();

	int close_demux();
#if 0
	fe_type_t fe_type;
#endif
	bool tune_atsc(dvbtee_fe_modulation_t, unsigned int);
	bool tune_dvbt(unsigned int);

	filtered_pid_map filtered_pids;

	int open_available_tuner(unsigned int max_adap = 8, unsigned int max_fe = 3);
};

#endif /*__LINUXTV_TUNER_H__ */
