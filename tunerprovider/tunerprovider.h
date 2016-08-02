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

#ifndef TUNERPROVIDER_H
#define TUNERPROVIDER_H

#include "tune.h"

typedef std::map<uint8_t, tune*> map_tuners;

class TunerProvider
{
public:
	TunerProvider();
	~TunerProvider();

	//the following will return the new tuner id:
	int add_hdhr_tuner(uint32_t device_id, uint32_t device_ip, unsigned int tuner = 0);
	int add_hdhr_tuner(unsigned int tuner) { return add_hdhr_tuner(0, 0, tuner); }
	int add_hdhr_tuner(const char *device_str = NULL);

	int add_linuxtv_tuner();
	bool add_linuxtv_tuner(int adap, int fe, int demux, int dvr);

	tune *get_tuner(int id) { return tuners.count(id) ? tuners[id] : NULL; }

protected:
	map_tuners tuners;
};

#endif // TUNERPROVIDER_H
