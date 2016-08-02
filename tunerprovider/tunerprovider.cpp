/*****************************************************************************
 * Copyright (C) 2013-2016 Michael Ira Krufky
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

#include "tunerprovider.h"

#include "dvbtee_config.h"
#ifdef USE_HDHOMERUN
#include "hdhr_tuner.h"
#endif
#ifdef USE_LINUXTV
#include "linuxtv_tuner.h"
#endif

TunerProvider::TunerProvider()
{
}

TunerProvider::~TunerProvider()
{
	for (map_tuners::const_iterator iter = tuners.begin(); iter != tuners.end(); ++iter) {
#if 0
		iter->second->feeder.stop_without_wait();
		iter->second->feeder.close_file();
#else
		iter->second->stop_feed();
#endif
		iter->second->close_fe();
		iter->second->feeder.parser.cleanup();
	}
	tuners.clear();
}

int TunerProvider::add_hdhr_tuner(const char *device_str)
{
#ifdef USE_HDHOMERUN
	int id = tuners.size();
	hdhr_tuner* hdhr  = new hdhr_tuner;
	tuners[id] = hdhr;
	//FIXME: if we don't call set_hdhr_id we get undefined effects :-/
	hdhr->set_hdhr_id(device_str ? device_str : "");
#else
	int id = -1;
#endif
	return id;
}

int TunerProvider::add_hdhr_tuner(uint32_t device_id, uint32_t device_ip, unsigned int tuner)
{
#ifdef USE_HDHOMERUN
	int id = tuners.size();
	hdhr_tuner* hdhr  = new hdhr_tuner;
	tuners[id] = hdhr;
	hdhr->set_hdhr_id(device_id, device_ip, tuner);
#else
	int id = -1;
#endif
	return id;
}

int TunerProvider::add_linuxtv_tuner()
{
#ifdef USE_LINUXTV
	int id = tuners.size();
	tuners[id] = new linuxtv_tuner;
#else
	int id = -1;
#endif
	return id;
}

bool TunerProvider::add_linuxtv_tuner(int adap, int fe, int demux, int dvr)
{
#ifdef USE_LINUXTV
	int id = tuners.size();
	linuxtv_tuner *linuxtv = new linuxtv_tuner;
	tuners[id] = linuxtv;
	linuxtv->set_device_ids(adap, fe, demux, dvr);
#else
	(void)adap;
	(void)fe;
	(void)demux;
	(void)dvr;

	int id = -1;
#endif
	return id;
}
