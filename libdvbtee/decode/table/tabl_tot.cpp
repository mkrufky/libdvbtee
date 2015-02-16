/*****************************************************************************
 * Copyright (C) 2011-2015 Michael Ira Krufky
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

#include <stdio.h>
#include <stdint.h>
#include "decoder.h"
//#include "dvbpsi/descriptor.h"

#include "functions.h"

#include "tabl_tot.h"

#define TABLEID 0x14

#define CLASS_MODULE "[TDT (Time and Date Table)/TOT (Time Offset Table)]"

#define dprintf(fmt, arg...) __dprintf(DBG_DECODE, fmt, ##arg)

using namespace dvbtee::decode;

void tot::store(dvbpsi_tot_t *p_tot)
{
	time_t o_time = m_time;

	m_time = datetime_utc(p_tot->i_utc_time);

	//dbg_time("%s", ctime(&stream_time));

	descriptors.decode(p_tot->p_first_descriptor);

	if ((o_time != m_time) && (m_watcher)) {
		TOT_Watcher *watcher = (TOT_Watcher*)m_watcher;
		watcher->updateTOT(*this);
		watcher->updateTable(TABLEID);
	}
}

tot::tot(Decoder *parent)
 : Table(parent)
 , m_time(-1)
{
	//store table later (probably repeatedly)
}

tot::tot(Decoder *parent, TOT_Watcher *watcher)
 : Table(parent, watcher)
 , m_time(-1)
{
	//store table later (probably repeatedly)
}

tot::tot(Decoder *parent, TOT_Watcher *watcher, dvbpsi_tot_t *p_tot)
 : Table(parent, watcher)
 , m_time(-1)
{
	store(p_tot);
}

tot::tot(Decoder *parent, dvbpsi_tot_t *p_tot)
 : Table(parent/*, p_tot*/)
 , m_time(-1)
{
	store(p_tot);
}

tot::~tot()
{
	//
}

//REGISTER_TABLE_FACTORY("tot", tot);
