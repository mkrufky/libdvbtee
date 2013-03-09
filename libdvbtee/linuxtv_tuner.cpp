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

#include "linuxtv_tuner.h"
#include "log.h"
#define CLASS_MODULE "linuxtv_tuner"

#define dprintf(fmt, arg...) __dprintf(DBG_TUNE, fmt, ##arg)

typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;

static map_chan_to_ts_id channels;

linuxtv_tuner::linuxtv_tuner()
  : adap_id(-1)
  , fe_fd(-1)
  , demux_fd(-1)
  , fe_id(-1)
  , demux_id(-1)
  , dvr_id(-1)
{
	dprintf("()");
	memset(&filtered_pids, 0, sizeof(filtered_pids));
	filtered_pids.clear();
}

linuxtv_tuner::~linuxtv_tuner()
{
	dprintf("()");
	stop_feed();
	close_fe();
	filtered_pids.clear();
}

linuxtv_tuner::linuxtv_tuner(const linuxtv_tuner&)
{
	dprintf("(copy)");

	feeder.parser.cleanup();
	memset(&filtered_pids, 0, sizeof(filtered_pids));
	filtered_pids.clear();
	adap_id = -1;
	fe_fd = -1;
	demux_fd = -1;
	fe_id = -1;
	demux_id = -1;
	dvr_id = -1;
}

linuxtv_tuner& linuxtv_tuner::operator= (const linuxtv_tuner& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	feeder.parser.cleanup();
	memset(&filtered_pids, 0, sizeof(filtered_pids));
	filtered_pids.clear();
	adap_id = -1;
	fe_fd = -1;
	demux_fd = -1;
	fe_id = -1;
	demux_id = -1;
	dvr_id = -1;

	return *this;
}

bool linuxtv_tuner::set_device_ids(int adap, int fe, int demux, int dvr, bool kernel_pid_filter)
{
	dprintf("(%d, %d, %d, %d)", adap, fe, demux, dvr);

	adap_id  = adap;
	fe_id    = fe;
	demux_id = demux;
	dvr_id   = dvr;

	if (kernel_pid_filter)
		feeder.parser.set_addfilter_callback(add_filter, this);

	return ((adap >= 0) && (fe >= 0) && (demux >= 0) && (dvr >= 0)); /* TO DO: -1 should signify auto/search */
}

int linuxtv_tuner::close_fe() {
	close(fe_fd);
	fe_fd = -1;
	return tune::close_fe();
}

int linuxtv_tuner::close_demux() {
	close(demux_fd);
	demux_fd = -1;
	cur_chan = 0; // FIXME???
	return demux_fd;
}

bool linuxtv_tuner::check()
{
	bool ret = ((adap_id >= 0) && (fe_id >= 0) && (demux_id >= 0) && (dvr_id >= 0));
	if (!ret)
		dprintf("tuner not configured!");
	else {
		dprintf("(adap: %d, fe: %d, demux: %d, dvr: %d) state:%s%s%s%s%s", adap_id, fe_id, demux_id, dvr_id,
			is_idle() ? " idle" : "",
			is_open() ? " open" : "",
			is_lock() ? " lock" : "",
			is_scan() ? " scan" : "",
			is_feed() ? " feed" : "");
		if (cur_chan) {
			fe_status_t status = fe_status();
			uint16_t snr = get_snr();
			dprintf("tuned to channel: %d, %s, snr: %d.%d", cur_chan, (status & FE_HAS_LOCK) ? "LOCKED" : "NO LOCK", snr / 10, snr % 10);
			last_touched();
		}
	}

	return ret;
}


int linuxtv_tuner::open_fe()
{
	struct dvb_frontend_info fe_info;
	char filename[32];

	if ((fe_fd >= 0) || (is_open())) {
		fprintf(stderr, "open_frontend: already open!\n");
		return -1;
	}
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
		state |= TUNE_STATE_OPEN;
		return fe_fd;
	default:
		fprintf(stderr, "frontend is not a supported device type!\n");
	}
fail:
	return close_fe();
}

fe_status_t linuxtv_tuner::fe_status()
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

	state &= ~TUNE_STATE_LOCK;
	if (status & FE_HAS_LOCK)
		state |= TUNE_STATE_LOCK;

	return status;
}

uint16_t linuxtv_tuner::get_snr()
{
	uint16_t snr = 0;

	if (ioctl(fe_fd, FE_READ_SNR, &snr) < 0) {
		perror("FE_READ_SNR failed");
#if 0
	} else {
#endif
	}

	return snr;
}

void linuxtv_tuner::stop_feed()
{
	tune::stop_feed();
	close_demux();
}

int linuxtv_tuner::start_feed()
{
	char filename[80]; // max path length??

	dprintf("()");
	if (demux_fd >= 0) {
		fprintf(stderr, "linuxtv_tuner::start_feed: demux already open!\n");
		return -1;
	}
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
#define BUFSIZE_0x2000 (188*((384*1024)/188))
	if (ioctl(demux_fd, DMX_SET_BUFFER_SIZE, BUFSIZE_0x2000) < 0)
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

	if (0 == feeder.start()) {
		state |= TUNE_STATE_FEED;
		return 0;
	}
fail_dvr:
	feeder.close_file();
fail_filter:
	close_demux();
fail_demux:
	return -1;
}

bool linuxtv_tuner::tune_channel(fe_modulation_t modulation, unsigned int channel)
{
	bool ret;

	switch (fe_type) {
	case FE_ATSC:
		ret = tune_atsc(modulation, channel);
		break;
	case FE_OFDM:
		ret = tune_dvbt(channel);
		break;
	default:
		fprintf(stderr, "unsupported FE TYPE\n");
		ret = false;
	}
	if (ret) {
		cur_chan = channel;
		time(&time_touched);
	}

	return ret;
}

bool linuxtv_tuner::tune_atsc(fe_modulation_t modulation, unsigned int channel)
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
		fprintf(stderr, "linuxtv_tuner: FE_SET_FRONTEND failed\n");
		return false;
	}
	else fprintf(stderr, "tuned to %d\n", fe_params.frequency);

	return true;
}

bool linuxtv_tuner::tune_dvbt(unsigned int channel)
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
		fprintf(stderr, "linuxtv_tuner: FE_SET_FRONTEND failed\n");
		return false;
	}
	else fprintf(stderr, "tuned to %d\n", fe_params.frequency);

	return true;
}

//static
void linuxtv_tuner::clear_filters(void *p_this)
{
	return static_cast<linuxtv_tuner*>(p_this)->clear_filters();
}

void linuxtv_tuner::clear_filters()
{
	dprintf("()");

	for (filtered_pid_map::const_iterator iter = filtered_pids.begin(); iter != filtered_pids.end(); ++iter) {
		if (ioctl(iter->second, DMX_STOP, NULL) < 0) {
			perror("DMX_STOP failed");
		}
		close(iter->second);
	}
	filtered_pids.clear();
}

//static
void linuxtv_tuner::add_filter(void *p_this, uint16_t pid)
{
	if (pid == 0xffff)
		return static_cast<linuxtv_tuner*>(p_this)->clear_filters();
	else
		return static_cast<linuxtv_tuner*>(p_this)->add_filter(pid);
}

void linuxtv_tuner::add_filter(uint16_t pid)
{
	dprintf("pid = %04x", pid);

	if (filtered_pids.count(pid))
		return;

	char filename[80]; // max path length??

	filtered_pids[pid] = -1;

	sprintf(filename, "/dev/dvb/adapter%i/demux%i", adap_id, demux_id);
	if ((filtered_pids[pid] = open(filename, O_RDWR)) < 0) {
		// try flat dvb dev structure if this fails
		sprintf(filename, "/dev/dvb%i.demux%i", adap_id, demux_id);
		if ((filtered_pids[pid] = open(filename, O_RDWR)) < 0) {
			fprintf(stderr,"%s: failed to open %s\n", __func__, filename);
			goto fail_demux;
		}
	}
	fprintf(stderr, "%s: using %s\n", __func__, filename);
#if 0
#define BUFSIZE_PID (188*((384*1024)/188))
	if (ioctl(filtered_pids[pid], DMX_SET_BUFFER_SIZE, BUFSIZE_PID) < 0)
		perror("DMX_SET_BUFFER_SIZE failed");
#endif
	struct dmx_pes_filter_params pesfilter;

	pesfilter.pid = pid;
	pesfilter.input = DMX_IN_FRONTEND;
	pesfilter.output = DMX_OUT_TS_TAP;
	pesfilter.pes_type = DMX_PES_OTHER;
	pesfilter.flags = DMX_IMMEDIATE_START;

	if (ioctl(filtered_pids[pid], DMX_SET_PES_FILTER, &pesfilter) < 0) {
		perror("DMX_SET_PES_FILTER failed");
	}
fail_demux:
	return;
}
