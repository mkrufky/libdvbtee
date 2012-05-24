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

class tune
{
public:
	tune();
	~tune();

	tune(const tune&);
	tune& operator= (const tune&);

	bool set_device_ids(int, int, int, int);

	int open_fe();
	int close_fe();
	int close_demux();

	void stop_feed();
	int start_feed();

	bool wait_for_lock_or_timeout(unsigned int);

	bool tune_atsc(fe_modulation_t, unsigned int);

#define SCAN_VSB 1
#define SCAN_QAM 2
	int scan_for_services(unsigned int, unsigned int, unsigned int, bool epg = false);
	int start_scan(unsigned int, unsigned int, unsigned int, bool epg = false);
	void wait_for_scan_complete() { while (!scan_complete) usleep(20*1000); };
	unsigned int get_scan_results(bool wait = true);

	feed feeder;
private:
	pthread_t h_thread;
	bool f_kill_thread;

	void *scan_thread();
	static void *scan_thread(void*);

	int  adap_id;

	int    fe_fd;
	int demux_fd;

	int    fe_id;
	int demux_id;
	int   dvr_id;

	int          scan_mode;
	unsigned int scan_min;
	unsigned int scan_max;
	bool         scan_epg;
	bool         scan_complete;

	//map_chan_to_ts_id channels;

	fe_status_t fe_status();
};

#endif /*__TUNE_H__ */
