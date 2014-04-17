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

#ifndef __TUNE_H__
#define __TUNE_H__

#include <pthread.h>
#include <sys/ioctl.h>
#include <time.h>

#include "channels.h"
#include "feed.h"

#if 0
#include <map>
typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;
static map_chan_to_ts_id channels;
#endif

typedef std::map<uint16_t, int> filtered_pid_map; /* pid, fd */
typedef std::map<unsigned int, bool> channel_map; /* channel, found? */

typedef struct {
	unsigned int total;
	unsigned int current;
	unsigned int physical_channel;
} scan_progress_t;

typedef enum {
	DVBTEE_FE_QPSK,
	DVBTEE_FE_QAM,
	DVBTEE_FE_OFDM,
	DVBTEE_FE_ATSC
} dvbtee_fe_type_t;

typedef enum {
	DVBTEE_QAM_64,
	DVBTEE_QAM_256,
	DVBTEE_VSB_8,
	DVBTEE_VSB_16
} dvbtee_fe_modulation_t;

typedef enum {
	DVBTEE_FE_IS_IDLE    = 0,
	DVBTEE_FE_HAS_SIGNAL = 1,
	DVBTEE_FE_HAS_SYNC   = 2,
	DVBTEE_FE_HAS_LOCK   = 4,
} dvbtee_fe_status_t;

typedef void (*scan_progress_callback)(void *context, scan_progress_t *p);

#if 0
#define vrtdbg fprintf(stderr, "%s: virtual function undefined!\n", __func__)
#else
#define vrtdbg {}
#endif

class tune
{
public:
	tune();
	~tune();

	tune(const tune&);
	tune& operator= (const tune&);

	virtual int open_fe() { vrtdbg; return -1; }
	virtual int close_fe();

	virtual void stop_feed();
	virtual int start_feed() { vrtdbg; return -1; }

	bool wait_for_lock_or_timeout(unsigned int);

	bool tune_channel(dvbtee_fe_modulation_t, unsigned int);
	unsigned int get_channel() { return cur_chan; }
	time_t last_touched();

	virtual const char *get_name() { return feeder.get_filename(); }

	virtual bool check() { vrtdbg; return false; }

#define SCAN_VSB 1
#define SCAN_QAM 2
	int scan_for_services(unsigned int, char *, bool epg = false, scan_progress_callback progress_cb = NULL, void* progress_context = NULL, chandump_callback chandump_cb = NULL, void* chandump_context = NULL, bool wait_for_results = true);
	int scan_for_services(unsigned int, unsigned int, unsigned int, bool epg = false, scan_progress_callback progress_cb = NULL, void* progress_context = NULL, chandump_callback chandump_cb = NULL, void* chandump_context = NULL, bool wait_for_results = true);
	/* FIXME: deprecate start_scan & move to private */
	int start_scan(unsigned int, char *, bool epg = false, scan_progress_callback progress_cb = NULL, void* progress_context = NULL);
	int start_scan(unsigned int, unsigned int, unsigned int, bool epg = false, scan_progress_callback progress_cb = NULL, void* progress_context = NULL);
	void wait_for_scan_complete() { while (!scan_complete) usleep(20*1000); }
	unsigned int get_scan_results(bool wait = true, chandump_callback chandump_cb = NULL, void* chandump_context = NULL);
	void stop_scan() { f_kill_thread = true; }

	feed feeder;
#define TUNE_STATE_IDLE 0
#define TUNE_STATE_OPEN 1
#define TUNE_STATE_LOCK 2
#define TUNE_STATE_SCAN 4
#define TUNE_STATE_FEED 8
	inline bool is_idle() { return (state ==TUNE_STATE_IDLE); }
	inline bool is_open() { return (state & TUNE_STATE_OPEN); }
	inline bool is_lock() { return (state & TUNE_STATE_LOCK); }
	inline bool is_scan() { return (state & TUNE_STATE_SCAN); }
	inline bool is_feed() { return (state & TUNE_STATE_FEED); }
protected:
	static void *scan_thread(void*);

	pthread_t h_thread;
	bool f_kill_thread;

	unsigned int state;
	unsigned int cur_chan;
	time_t time_touched;

	int          scan_mode;
	channel_map  scan_channel_list;
	bool         scan_epg;
	bool         scan_complete;

	dvbtee_fe_type_t fe_type;
private:
	void *scan_thread();

	time_t last_query;

	//map_chan_to_ts_id channels;

	virtual bool __tune_channel(dvbtee_fe_modulation_t, unsigned int) { vrtdbg; return false; }
	virtual dvbtee_fe_status_t fe_status() { vrtdbg; return (dvbtee_fe_status_t)0; } // FIXME
#if 0
	uint16_t get_snr();
#endif
	int start_scan(unsigned int, bool epg, scan_progress_callback, void*);

	scan_progress_callback scan_progress_cb;
	void* scan_progress_context;
};

#endif /*__TUNE_H__ */
