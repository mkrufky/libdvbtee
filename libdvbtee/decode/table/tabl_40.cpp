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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  40110-1301  USA
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "decoder.h"

#include "functions.h"

#include "tabl_40.h"

#define TABLEID 0x40

#define CLASS_MODULE "[NIT]"

#define dPrintf(fmt, arg...) __dPrintf(DBG_DECODE, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string TABLE_NAME = "NIT";

static std::string NITTS = "NITTS";

void nit::store(const dvbpsi_nit_t * const p_nit)
#define NIT_DBG 1
{
//	if ((decoded_nit->version    == p_nit->i_version) &&
//	    (decoded_nit->network_id == p_nit->i_network_id)) {

//		dPrintf("v%d, network_id %d: ALREADY DECODED",
//			p_nit->i_version, p_nit->i_network_id);
//		return false;
//	}
#if NIT_DBG
	fprintf(stderr, "%s NIT: v%d, network_id %d\n", __func__,
		p_nit->i_version, p_nit->i_network_id);
#endif
	decoded_nit.version    = p_nit->i_version;
	decoded_nit.network_id = p_nit->i_network_id;
	decoded_nit.ts_list.clear();

	set("version", p_nit->i_version);
	set("networkId", p_nit->i_network_id);

	descriptors.decode(p_nit->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	Array ts_list;

	const dvbpsi_nit_ts_t * p_ts = p_nit->p_first_ts;
#if NIT_DBG
	if (p_ts)
		fprintf(stderr, "   ts_id | orig_network_id\n");
#endif
	while (p_ts) {
		nitTS *nitTs = new nitTS(decoded_nit, this, p_ts);
		if (nitTs->isValid())
			ts_list.push((Object*)nitTs);
		p_ts = p_ts->p_next;
	}

	set("tsList", ts_list);

	setValid(true);

	dPrintf("%s", toJson().c_str());

	if ((/*changed*/true) && (m_watcher)) {
		m_watcher->updateTable(TABLEID, (Table*)this);
	}
}

nitTS::nitTS(decoded_nit_t& decoded_nit, Decoder *parent, const dvbpsi_nit_ts_t *p_ts)
: TableDataComponent(parent, NITTS)
{
	if (!p_ts) return;

	decoded_nit_ts_t &cur_ts_list = decoded_nit.ts_list[p_ts->i_ts_id];

	cur_ts_list.ts_id           = p_ts->i_ts_id;
	cur_ts_list.orig_network_id = p_ts->i_orig_network_id;

#if NIT_DBG
	fprintf(stderr, "   %05d | %d\n",
		cur_ts_list.ts_id,
		cur_ts_list.orig_network_id);
#endif
	/* descriptors contain frequency lists & LCNs */
	descriptors.decode(p_ts->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	set("tsId", p_ts->i_ts_id);
	set("origNetworkId", p_ts->i_orig_network_id);

	setValid(true);
}

nitTS::~nitTS()
{

}


bool nit::ingest(TableStore *s, const dvbpsi_nit_t * const t, TableWatcher *w)
{
	const std::vector<Table*> nits = s->get(TABLEID);
	for (std::vector<Table*>::const_iterator it = nits.begin(); it != nits.end(); ++it) {
		nit *thisNIT = (nit*)*it;
		if (thisNIT->get<uint16_t>("networkId") == t->i_network_id) {
			if (thisNIT->get<uint16_t>("version") == t->i_version) {
				dPrintf("NIT v%d, network_id %d: ALREADY DECODED", t->i_version, t->i_network_id);
				return false;
			}
			thisNIT->store(t);
			return true;
		}
	}
	return s->add<const dvbpsi_nit_t>(TABLEID, t, w);
}


nit::nit(Decoder *parent, TableWatcher *watcher)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	//store table later (probably repeatedly)
}

nit::nit(Decoder *parent, TableWatcher *watcher, const dvbpsi_nit_t * const p_nit)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	store(p_nit);
}

nit::~nit()
{
	//
}

REGISTER_TABLE_FACTORY(TABLEID, const dvbpsi_nit_t, nit)
