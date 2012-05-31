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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>

#include "tune.h"
#include "log.h"

#define dprintf(fmt, arg...) __dprintf(DBG_TUNE, fmt, ##arg)

typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;

static map_chan_to_ts_id channels;

tune::tune()
  : f_kill_thread(false)
  , adap_id(-1)
  , fe_fd(-1)
  , demux_fd(-1)
  , fe_id(-1)
  , demux_id(-1)
  , dvr_id(-1)
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
}

tune& tune::operator= (const tune& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

//	channels.clear();
	feeder.parser.cleanup();

	return *this;
}

bool tune::set_device_ids(int adap, int fe, int demux, int dvr)
{
	dprintf("(%d, %d, %d, %d)", adap, fe, demux, dvr);

	adap_id  = adap;
	fe_id    = fe;
	demux_id = demux;
	dvr_id   = dvr;

	return ((adap >= 0) && (fe >= 0) && (demux >= 0) && (dvr >= 0)); /* TO DO: -1 should signify auto/search */
}

int tune::close_fe() {
	close(fe_fd);
	fe_fd = -1;
	return fe_fd;
}

int tune::close_demux() {
	close(demux_fd);
	demux_fd = -1;
	return demux_fd;
}


int tune::open_fe()
{
	struct dvb_frontend_info fe_info;
	char filename[32];

	fe_fd = -1;

	sprintf(filename, "/dev/dvb/adapter%i/frontend%i", adap_id, fe_id);

	if ((fe_fd = open(filename, O_RDWR)) < 0) {
		sprintf(filename, "/dev/dvb%i.frontend%i", adap_id, fe_id);
		if ((fe_fd = open(filename, O_RDWR)) < 0) {
			return fe_fd;
		}
	}

	if (ioctl(fe_fd, FE_GET_INFO, &fe_info) < 0) {
		fprintf(stderr, "open_frontend: FE_GET_INFO failed\n");
		goto fail;
	}

	fprintf(stderr, "%s: using %s\n", __func__, filename);

	switch (fe_info.type) {
	case FE_ATSC:
	case FE_OFDM:
		struct dvb_frontend_parameters fe_params;

		memset(&fe_params, 0, sizeof(struct dvb_frontend_parameters));

		if (ioctl(fe_fd, FE_GET_FRONTEND, &fe_params) < 0) {
			fprintf(stderr, "open_frontend: FE_GET_FRONTEND failed\n");
			goto fail;
		}
		fe_type = fe_info.type;
		return fe_fd;
	default:
		fprintf(stderr, "frontend is not a supported device type!\n");
	}
fail:
	return close_fe();
}

fe_status_t tune::fe_status()
{
	fe_status_t status = (fe_status_t)0;

	if (ioctl(fe_fd, FE_READ_STATUS, &status) < 0) {
		perror("FE_READ_STATUS failed");
		return (fe_status_t)0;
	}

	fprintf(stderr, "%s%s%s%s%s ",
		(status & FE_HAS_SIGNAL)  ? "S" : "",
		(status & FE_HAS_CARRIER) ? "C" : "",
		(status & FE_HAS_VITERBI) ? "V" : "",
		(status & FE_HAS_SYNC)    ? "Y" : "",
		(status & FE_HAS_LOCK)    ? "L" : "");

	return status;
}

bool tune::wait_for_lock_or_timeout(unsigned int time_ms)
{
	unsigned int status = (fe_status_t)0;
	time_t start_time = time(NULL);
	while ((0 == ((status |= fe_status()) & FE_HAS_LOCK)) && ( (time(NULL) - start_time) < ((int)time_ms / 1000) ))
		usleep(200*1000);
	if ((status & (FE_HAS_LOCK | FE_HAS_SYNC)) == FE_HAS_SYNC) {
		start_time = time(NULL);
		while ((0 == ((status |= fe_status()) & FE_HAS_LOCK)) && ( (time(NULL) - start_time) < (2 * (int)time_ms / 1000) ))
			usleep(200*1000);
	}
	return ((status & FE_HAS_LOCK) == FE_HAS_LOCK);
}

void tune::stop_feed()
{
	feeder.stop();
	feeder.close_file();

	close_demux();
}

int tune::start_feed()
{
	char filename[80]; // max path length??

	dprintf("()");
	demux_fd = -1;

	sprintf(filename, "/dev/dvb/adapter%i/demux%i", adap_id, demux_id);
	if ((demux_fd = open(filename, O_RDWR)) < 0) {
		// try flat dvb dev structure if this fails
		sprintf(filename, "/dev/dvb%i.demux%i", adap_id, demux_id);
		if ((demux_fd = open(filename, O_RDWR)) < 0) {
			fprintf(stderr,"%s: failed to open %s\n", __func__, filename);
			goto fail_demux;
		}
	}
	fprintf(stderr, "%s: using %s\n", __func__, filename);
#if 0
	int buffersize = 188*((384*1024)/188);
	if (ioctl(stream->demux_fd, DMX_SET_BUFFER_SIZE, buffersize) < 0)
		perror("DMX_SET_BUFFER_SIZE failed");
#endif
	struct dmx_pes_filter_params pesfilter;

	pesfilter.pid = 0x2000;
	pesfilter.input = DMX_IN_FRONTEND;
	pesfilter.output = DMX_OUT_TS_TAP;
	pesfilter.pes_type = DMX_PES_OTHER;
	pesfilter.flags = DMX_IMMEDIATE_START;

	if (ioctl(demux_fd, DMX_SET_PES_FILTER, &pesfilter) < 0) {
		perror("DMX_SET_PES_FILTER failed");
		goto fail_filter;
	}

	sleep(1); // FIXME

	sprintf(filename, "/dev/dvb/adapter%i/dvr%i", adap_id, dvr_id);
	if (feeder.open_file(filename) < 0) {
		// try flat dvb dev structure if this fails
		sprintf(filename, "/dev/dvb%i.dvr%i", adap_id, dvr_id);
		if (feeder.open_file(filename) < 0) {
			fprintf(stderr, "failed to open %s\n", filename);
			goto fail_dvr;
		}
	}

	if (0 == feeder.start())
		return 0;
fail_dvr:
	feeder.close_file();
fail_filter:
	close_demux();
fail_demux:
	return -1;
}

bool tune::tune_channel(fe_modulation_t modulation, unsigned int channel)
{
	switch (fe_type) {
	case FE_ATSC:
		return tune_atsc(modulation, channel);
	case FE_OFDM:
		return tune_dvbt(channel);
	default:
		fprintf(stderr, "unsupported FE TYPE\n");
		return false;
	}
}

bool tune::tune_atsc(fe_modulation_t modulation, unsigned int channel)
{
	struct dvb_frontend_parameters fe_params;

	memset(&fe_params, 0, sizeof(struct dvb_frontend_parameters));

	switch (modulation) {
	case VSB_8:
	case VSB_16:
		fe_params.frequency = atsc_vsb_chan_to_freq(channel);
		break;
	case QAM_64:
	case QAM_256:
		fe_params.frequency = atsc_qam_chan_to_freq(channel);
		break;
	default:
		fprintf(stderr, "modulation not supported!\n");
		return false;
	}
	fe_params.u.vsb.modulation = modulation;

	if (ioctl(fe_fd, FE_SET_FRONTEND, &fe_params) < 0) {
		fprintf(stderr, "tune: FE_SET_FRONTEND failed\n");
		return false;
	}
	else fprintf(stderr, "tuned to %d\n", fe_params.frequency);

	return true;
}

bool tune::tune_dvbt(unsigned int channel)
{
	struct dvb_frontend_parameters fe_params;

	memset(&fe_params, 0, sizeof(struct dvb_frontend_parameters));

	fe_params.frequency = dvbt_chan_to_freq(channel);
	fe_params.u.ofdm.bandwidth             = ((channel <= 12) ? BANDWIDTH_7_MHZ : BANDWIDTH_8_MHZ);
	fe_params.u.ofdm.code_rate_HP          = FEC_AUTO;
	fe_params.u.ofdm.code_rate_LP          = FEC_AUTO;
	fe_params.u.ofdm.constellation         = QAM_AUTO;
	fe_params.u.ofdm.transmission_mode     = TRANSMISSION_MODE_AUTO;
	fe_params.u.ofdm.guard_interval        = GUARD_INTERVAL_AUTO;
	fe_params.u.ofdm.hierarchy_information = HIERARCHY_AUTO;

	if (ioctl(fe_fd, FE_SET_FRONTEND, &fe_params) < 0) {
		fprintf(stderr, "tune: FE_SET_FRONTEND failed\n");
		return false;
	}
	else fprintf(stderr, "tuned to %d\n", fe_params.frequency);

	return true;
}

//static
void* tune::scan_thread(void *p_this)
{
	return static_cast<tune*>(p_this)->scan_thread();
}

void* tune::scan_thread()
{
	feeder.parser.set_scan_mode(true);
	feeder.parser.set_epg_mode(scan_epg);

	for (unsigned int channel = scan_min; channel <= scan_max; channel++) {

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

		if ((!f_kill_thread) && ((tune_channel((scan_mode == SCAN_VSB) ? VSB_8 : QAM_256, channel)) && (wait_for_lock_or_timeout(2000)))) {

			if (f_kill_thread)
				break;

			switch (fe_type) {
			default:
			case FE_ATSC:
				feeder.parser.set_channel_info(channel,
							       (scan_mode == SCAN_VSB) ? atsc_vsb_chan_to_freq(channel) : atsc_qam_chan_to_freq(channel),
							       (scan_mode == SCAN_VSB) ? "8VSB" : "QAM256");
				break;
			case FE_OFDM:
				feeder.parser.set_channel_info(channel, dvbt_chan_to_freq(channel),
							       ((channel <= 12) ?
								"INVERSION_AUTO:BANDWIDTH_7_MHZ:FEC_AUTO:FEC_AUTO:QAM_AUTO:TRANSMISSION_MODE_AUTO:GUARD_INTERVAL_AUTO:HIERARCHY_AUTO" :
								"INVERSION_AUTO:BANDWIDTH_8_MHZ:FEC_AUTO:FEC_AUTO:QAM_AUTO:TRANSMISSION_MODE_AUTO:GUARD_INTERVAL_AUTO:HIERARCHY_AUTO"));
				break;
			}
			if (0 == start_feed()) {
				int timeout = (scan_epg) ? 16 : (fe_type == FE_ATSC) ? 4 : 8;
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
			feeder.parser.reset();
		}
	}
	close_fe();
	scan_complete = true;
	pthread_exit(NULL);
}

int tune::start_scan(unsigned int mode, unsigned int min, unsigned int max, bool epg)
{
	//channels.clear();

	if (mode != SCAN_QAM)
		mode = SCAN_VSB;

	scan_mode = mode;

	scan_min = min;
	scan_max = max;

	scan_epg = epg;

	fe_fd = open_fe();
	if (fe_fd < 0)
		return fe_fd;

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

	scan_complete = false;
	f_kill_thread = false;

	int ret = pthread_create(&h_thread, NULL, scan_thread, this);
	if (0 != ret) {
		perror("pthread_create() failed");
		close_fe();
	}
	return ret;
}

unsigned int tune::get_scan_results(bool wait)
{
	if (wait) wait_for_scan_complete();
#if 0
	return feeder.parser.xine_dump();
#else
	unsigned int ret = feeder.parser.xine_dump();

	if (scan_epg) feeder.parser.epg_dump();

	return ret;
#endif
};

int tune::scan_for_services(unsigned int mode, unsigned int min, unsigned int max, bool epg)
{
	unsigned int count = 0;
	unsigned int total_count = 0;

	if (!mode)
		mode = SCAN_VSB;

	for (scan_mode = SCAN_VSB; scan_mode <= SCAN_QAM; scan_mode++) if (mode & scan_mode) {

		count = 0;

		if (0 != start_scan(scan_mode, min, max, epg))
			return -1;

		count += get_scan_results(true);
		total_count += count;
#if 0
		for (map_chan_to_ts_id::const_iterator iter = channels.begin(); iter != channels.end(); ++iter)
			fprintf(stderr, "found ts_id %05d on channel %d\n", iter->second, iter->first);
		channels.clear(); //
#endif
		fprintf(stderr, "found %d services\n", count);
	}
	if (count != total_count)
		fprintf(stderr, "found %d services in total\n", total_count);
	return 0;
}
