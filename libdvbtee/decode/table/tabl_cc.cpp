/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
 *
 * Author: Michael Ira Krufky <mkrufky@linuxtv.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; atsc_etther
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  cc110-1301  USA
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "decoder.h"

#include "functions.h"

#include "tabl_cc.h"

#define TABLEID 0xcc

#define CLASS_MODULE "[ETT]"

#define dPrintf(fmt, arg...) __dPrintf(DBG_DECODE, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string TABLE_NAME = "ETT";

static std::string ETTEV = "ETTEV";

void atsc_ett::store(const dvbpsi_atsc_ett_t * const p_ett)
#define ETT_DBG 1
{
	//XXX: FIXME: decoded_atsc_ett_t &cur_ett = decoded_ett[p_ett->i_etm_id];
#if 1
//	if ((decoded_ett.version == p_ett->i_version) &&
//	    (decoded_ett.etm_id  == p_ett->i_etm_id)) {
//		__log_printf(stderr, "%s: v%d, ID %d: ALREADY DECODED\n", __func__,
//			p_ett->i_version, p_ett->i_etm_id);
//		return false;
//	}
#endif
	decoded_ett.version    = p_ett->i_version;
	decoded_ett.etm_id     = p_ett->i_etm_id;
	decoded_ett.etm_length = p_ett->i_etm_length;
	memcpy(decoded_ett.etm,
#if USING_DVBPSI_VERSION_0
	       p_ett->p_etm,
#else
	       p_ett->p_etm_data,
#endif
	       (sizeof(decoded_ett.etm) >= p_ett->i_etm_length) ?
		       p_ett->i_etm_length : sizeof(decoded_ett.etm));

	unsigned char message[ETM_MAX_LENGTH];
	memset(message, 0, sizeof(message));

	decode_multiple_string(decoded_ett.etm, decoded_ett.etm_length, message, sizeof(message));

	__log_printf(stderr, "%s ETT: v%d, ID: %d: %s\n", __func__,
		p_ett->i_version, p_ett->i_etm_id, message);

	set("version", p_ett->i_version);
	set("etmId", p_ett->i_etm_id);
	set("etm", (char*)message);

	descriptors.decode(p_ett->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	setValid(true);

	dPrintf("%s", toJson().c_str());

	if ((/*changed*/true) && (m_watcher)) {
		m_watcher->updateTable(TABLEID, (Table*)this);
	}
}


bool atsc_ett::ingest(TableStore *s, const dvbpsi_atsc_ett_t * const t, TableWatcher *w)
{
#if 1
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = t->i_ts_id;
#else
	uint16_t __ts_id = t->i_extension;
#endif
	const std::vector<Table*> atsc_etts = s->get(TABLEID);
	for (std::vector<Table*>::const_iterator it = atsc_etts.begin(); it != atsc_etts.end(); ++it) {
		atsc_ett *thisETT = (atsc_ett*)*it;
		if (thisETT->get<uint32_t>("etmId") == t->i_etm_id) {
			if (thisETT->get<uint8_t>("version") == t->i_version) {
				dPrintf("ETT v%d, ts_id %d: ALREADY DECODED", t->i_version, __ts_id);
				return false;
			}
			thisETT->store(t);
			return true;
		}
	}
	return s->add<const dvbpsi_atsc_ett_t>(TABLEID, t, w);
#else
	return s->setOnly<const dvbpsi_atsc_ett_t, atsc_ett>(TABLEID, t, w);
#endif
}


atsc_ett::atsc_ett(Decoder *parent, TableWatcher *watcher)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	//store table later (probably repeatedly)
}

atsc_ett::atsc_ett(Decoder *parent, TableWatcher *watcher, const dvbpsi_atsc_ett_t * const p_atsc_ett)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	store(p_atsc_ett);
}

atsc_ett::~atsc_ett()
{
	//
}

REGISTER_TABLE_FACTORY(TABLEID, const dvbpsi_atsc_ett_t, atsc_ett)
