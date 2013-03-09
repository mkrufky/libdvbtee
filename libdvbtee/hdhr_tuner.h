/*****************************************************************************
 * Copyright (C) 2011-2013 Michael Krufky
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

#ifndef __HDHR_TUNER_H__
#define __HDHR_TUNER_H__

#include "hdhomerun.h"

#include "tune.h"

typedef std::map<uint16_t, int> filtered_pid_map; /* pid, fd */
typedef std::map<unsigned int, bool> channel_map; /* channel, found? */

class hdhr_tuner: public tune
{
public:
	hdhr_tuner();
	~hdhr_tuner();

	hdhr_tuner(const hdhr_tuner&);
	hdhr_tuner& operator= (const hdhr_tuner&);

	bool set_hdhr_id(uint32_t device_id = 0, uint32_t device_ip = 0, unsigned int tuner = 0/*, bool kernel_pid_filter = true*/);
	bool set_hdhr_id(unsigned int tuner/*, bool kernel_pid_filter = true*/) { return set_hdhr_id(0, 0, tuner); }
	bool set_hdhr_id(const char *device_str/*, bool kernel_pid_filter = true*/);

	int open_fe();
	int close_fe();

	void stop_feed();
	int start_feed();

	bool tune_channel(fe_modulation_t, unsigned int);

	bool check();
private:
	fe_status_t fe_status();

	struct hdhomerun_tuner_status_t _hdhr_status;
	struct hdhomerun_tuner_status_t *hdhr_status();

	filtered_pid_map filtered_pids;

	int hdhr_pull_callback();
	static int hdhr_pull_callback(void*);

	struct hdhomerun_device_t *hdhr_dev;
	struct hdhomerun_debug_t *hdhr_dbg;
};

#endif /*__HDHR_TUNER_H__ */
