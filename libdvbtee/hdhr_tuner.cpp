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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>

#include "hdhr_tuner.h"
#include "log.h"
#define CLASS_MODULE "hdhr_tuner"

#define dprintf(fmt, arg...) __dprintf(DBG_TUNE, fmt, ##arg)

typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;

static map_chan_to_ts_id channels;

hdhr_tuner::hdhr_tuner()
  : hdhr_dev(NULL)
  , hdhr_dbg(NULL)
{
	dprintf("()");
	memset(&filtered_pids, 0, sizeof(filtered_pids));
	filtered_pids.clear();
}

hdhr_tuner::~hdhr_tuner()
{
	dprintf("()");
	stop_feed();
	close_fe();
	filtered_pids.clear();
	if (hdhr_dev)
		hdhomerun_device_destroy(hdhr_dev);
	if (hdhr_dbg)
		hdhomerun_debug_destroy(hdhr_dbg);
}

hdhr_tuner::hdhr_tuner(const hdhr_tuner&)
{
	dprintf("(copy)");

	feeder.parser.cleanup();
	memset(&filtered_pids, 0, sizeof(filtered_pids));
	filtered_pids.clear();
	hdhr_dev = NULL;
	hdhr_dbg = NULL;
}

hdhr_tuner& hdhr_tuner::operator= (const hdhr_tuner& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	feeder.parser.cleanup();
	memset(&filtered_pids, 0, sizeof(filtered_pids));
	filtered_pids.clear();
	hdhr_dev = NULL;
	hdhr_dbg = NULL;

	return *this;
}

#if 0
	if (kernel_pid_filter)
		feeder.parser.set_addfilter_callback(add_filter, this);
#endif

bool hdhr_tuner::set_hdhr_id(uint32_t device_id, uint32_t device_ip, unsigned int tuner/*, bool kernel_pid_filter = true*/)
{
	hdhr_dbg = hdhomerun_debug_create();
	hdhomerun_debug_enable(hdhr_dbg);
	hdhr_dev = hdhomerun_device_create((device_id) ? device_id : HDHOMERUN_DEVICE_ID_WILDCARD, device_ip, tuner, hdhr_dbg);
	return (hdhr_dev) ? true : false;
}

bool hdhr_tuner::set_hdhr_id(const char *device_str/*, bool kernel_pid_filter = true*/)
{
	hdhr_dbg = hdhomerun_debug_create();
	hdhomerun_debug_enable(hdhr_dbg);
	hdhr_dev = hdhomerun_device_create_from_str(device_str, hdhr_dbg);
	return (hdhr_dev) ? true : false;
}

bool hdhr_tuner::check()
{
	if (!hdhr_dev)
		dprintf("tuner not configured!");
	else {
		dprintf("(name: %s, id: %04x, tuner: %d, ip: %04x) state:%s%s%s%s%s",
			hdhomerun_device_get_name(hdhr_dev),
			hdhomerun_device_get_device_id(hdhr_dev),
			hdhomerun_device_get_tuner(hdhr_dev),
			hdhomerun_device_get_device_ip(hdhr_dev),
			is_idle() ? " idle" : "",
			is_open() ? " open" : "",
			is_lock() ? " lock" : "",
			is_scan() ? " scan" : "",
			is_feed() ? " feed" : "");
		if (cur_chan) {
			hdhr_status();
			//dprintf("tuned to channel: %d, %s, snr: %d.%d", cur_chan, (status & FE_HAS_LOCK) ? "LOCKED" : "NO LOCK", snr / 10, snr % 10);
			last_touched();
		}
	}

	return true;
}

int hdhr_tuner::open_fe()
{
	state |= TUNE_STATE_OPEN;
	return 0;
}

struct hdhomerun_tuner_status_t *hdhr_tuner::hdhr_status()
{
	hdhomerun_device_wait_for_lock(hdhr_dev, &_hdhr_status);

	dprintf("%s\n%s\n%s\n%s\n%s\nsignal strength: %d\nsnq: %d\nseq: %d\nrbps: %d\npps: %d\n",
		_hdhr_status.channel, _hdhr_status.lock_str,
		(_hdhr_status.signal_present) ? "signal present" : "",
		(_hdhr_status.lock_supported) ? "lock supported" : "",
		(_hdhr_status.lock_unsupported) ? "lock unsupported" : "",
		_hdhr_status.signal_strength, _hdhr_status.signal_to_noise_quality, _hdhr_status.symbol_error_quality,
		_hdhr_status.raw_bits_per_second, _hdhr_status.packets_per_second);

	state &= ~TUNE_STATE_LOCK;
	if (_hdhr_status.lock_supported)
		state |= TUNE_STATE_LOCK;

	return &_hdhr_status;
}

fe_status_t hdhr_tuner::fe_status()
{
	hdhr_status();
	return (fe_status_t)((_hdhr_status.lock_supported) ? FE_HAS_LOCK : (_hdhr_status.signal_present) ? FE_HAS_SIGNAL : 0); // FIXME
}

void hdhr_tuner::stop_feed()
{
	tune::stop_feed();
	hdhomerun_device_stream_flush(hdhr_dev);
	hdhomerun_device_stream_stop(hdhr_dev);
}

int hdhr_tuner::hdhr_pull_callback(void *p_this, int len, const uint8_t *p)
{
	return static_cast<hdhr_tuner*>(p_this)->hdhr_pull_callback(len, p);
}

int hdhr_tuner::hdhr_pull_callback(int len, const uint8_t *p)
{
	size_t actual;
	const uint8_t *q = hdhomerun_device_stream_recv(hdhr_dev, (p) ? len : ((4096/188)*188), &actual);
	if (p)
		memcpy((uint8_t*)p, q, actual);
	else
		feeder.push(actual, q);

	return actual;
}

int hdhr_tuner::start_feed()
{
	hdhomerun_device_stream_start(hdhr_dev);
	return feeder.pull(this, hdhr_pull_callback);
}

bool hdhr_tuner::tune_channel(fe_modulation_t modulation, unsigned int channel)
{
	char channelno[4] = { 0 };
	snprintf(channelno, sizeof(channelno), "%d", channel);

	return (1 == hdhomerun_device_set_tuner_channel(hdhr_dev, &channelno[0]));
}
