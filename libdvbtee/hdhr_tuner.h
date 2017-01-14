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

#ifndef __HDHR_TUNER_H__
#define __HDHR_TUNER_H__

#include "tune.h"

#if !defined(DVBTEE_FEED_LEGACY)
#define hdhr_tuner hdhrTuner
#endif

typedef std::map<uint16_t, int> filtered_pid_map; /* pid, fd */
typedef std::map<unsigned int, bool> channel_map; /* channel, found? */

class HdhrPullFeeder : public dvbtee::feed::PullFeeder
{
public:
	HdhrPullFeeder(feed_pull_iface&);
	virtual void close();
};

class hdhr_tuner_device;

class hdhr_tuner: public tune, public tsfilter_iface, public feed_pull_iface
{
public:
	hdhr_tuner();
	~hdhr_tuner();

	hdhr_tuner(const hdhr_tuner&);
	hdhr_tuner& operator= (const hdhr_tuner&);

	/* use set_hdhr_id to use a specific hdhomerun device, this is optional, the fallback is auto-discovery */
	bool set_hdhr_id(uint32_t device_id = 0, uint32_t device_ip = 0, unsigned int tuner = 0, bool use_pid_filter = true);
	bool set_hdhr_id(unsigned int tuner, bool use_pid_filter = true) { return set_hdhr_id(0, 0, tuner, use_pid_filter); }
	bool set_hdhr_id(const char *device_str, bool use_pid_filter = true);

	int open_fe();
	int close_fe();

	void stop_feed();
	int start_feed();

	bool __tune_channel(dvbtee_fe_modulation_t, unsigned int);

	const char *get_name();

	bool check();

	void addfilter(uint16_t pid);

	int pull();
private:
#if !defined(DVBTEE_FEED_LEGACY)
	HdhrPullFeeder pullFeeder;
#endif
	void add_filter(uint16_t);
	void clear_filters();
	static void clear_filters(void *);

	dvbtee_fe_status_t fe_status();

	void hdhr_status();

	filtered_pid_map filtered_pids;

	class hdhr_tuner_device *dev;

#define HDHR_TUNER_MAX_TUNERS 2
	int open_available_tuner(unsigned int max_tuners = HDHR_TUNER_MAX_TUNERS);
};

#endif /*__HDHR_TUNER_H__ */
