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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  c7110-1301  USA
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "decoder.h"

#include "functions.h"

#include "tabl_c7.h"

#define TABLEID 0xc7

#define CLASS_MODULE "[MGT]"

#define dPrintf(fmt, arg...) __dPrintf(DBG_DECODE, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string TABLE_NAME = "MGT";

static std::string MGTTB = "MGTTB";

void mgt::store(const dvbpsi_atsc_mgt_t * const p_mgt)
#define MGT_DBG 1
{
//	if ((decoded_mgt.version == p_mgt->i_version) &&
//	    (!decoded_mgt.tables.empty())) {

//		dPrintf("v%d: ALREADY DECODED", p_mgt->i_version);
//		return false;
//	}
#if MGT_DBG
	fprintf(stderr, "%s MGT: v%d\n", __func__, p_mgt->i_version);
#endif
	decoded_mgt.version = p_mgt->i_version;
	decoded_mgt.tables.clear();

	set("version", p_mgt->i_version);

	Array tables;

	const dvbpsi_atsc_mgt_table_t *p_table = p_mgt->p_first_table;
#if MGT_DBG
	if (p_table)
		fprintf(stderr, "  table type |   pid  | ver | bytes\n");
#endif
	while (p_table) {

		mgtTb *table = new mgtTb(decoded_mgt, this, p_table);
		if (table->isValid()) tables.push((Object*)table);

		p_table = p_table->p_next;
	}

	descriptors.decode(p_mgt->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	set("tables", tables);

	setValid(true);

	dPrintf("%s", toJson().c_str());

	if ((/*changed*/true) && (m_watcher)) {
		m_watcher->updateTable(TABLEID, (Table*)this);
	}
}


bool mgt::ingest(TableStore *s, const dvbpsi_atsc_mgt_t * const t, TableWatcher *w)
{
#if 0
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = t->i_ts_id;
#else
	uint16_t __ts_id = t->i_extension;
#endif
	const std::vector<Table*> mgts = s->get(TABLEID);
	for (std::vector<Table*>::const_iterator it = mgts.begin(); it != mgts.end(); ++it) {
		mgt *thisMGT = (mgt*)*it;
		if (thisMGT->get<uint16_t>("tsId") == __ts_id) {
			if (thisMGT->get<uint16_t>("version") == t->i_version) {
				dPrintf("MGT v%d, ts_id %d: ALREADY DECODED", t->i_version, __ts_id);
				return false;
			}
			thisMGT->store(t);
			return true;
		}
	}
	return s->add<const dvbpsi_atsc_mgt_t>(TABLEID, t, w);
#else
	return s->setOnly<const dvbpsi_atsc_mgt_t, mgt>(TABLEID, t, w);
#endif
}


mgt::mgt(Decoder *parent, TableWatcher *watcher)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	//store table later (probably repeatedly)
}

mgt::mgt(Decoder *parent, TableWatcher *watcher, const dvbpsi_atsc_mgt_t * const p_mgt)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	store(p_mgt);
}

mgt::~mgt()
{
	//
}


mgtTb::mgtTb(decoded_mgt_t &decoded_mgt, Decoder *parent, const dvbpsi_atsc_mgt_table_t * const p_table)
: TableDataComponent(parent, MGTTB)
{
#if MGT_DBG
	fprintf(stderr, "    0x%04x   | 0x%04x | %3d | %d\n",
			p_table->i_table_type, p_table->i_table_type_pid,
			p_table->i_table_type_version, p_table->i_number_bytes);
#endif
	decoded_mgt_table_t &cur_table = decoded_mgt.tables[p_table->i_table_type];

	cur_table.type    = p_table->i_table_type;
	cur_table.pid     = p_table->i_table_type_pid;
	cur_table.version = p_table->i_table_type_version;
	cur_table.bytes   = p_table->i_number_bytes;

	set("type",    p_table->i_table_type);
	set("pid",     p_table->i_table_type_pid);
	set("version", p_table->i_table_type_version);
	set("bytes",   p_table->i_number_bytes);
#if 0
	switch (p_table->i_table_type) {
	default:
		break;
	}
#endif
	descriptors.decode(p_table->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	setValid(true);
}

mgtTb::~mgtTb()
{

}

REGISTER_TABLE_FACTORY(TABLEID, const dvbpsi_atsc_mgt_t, mgt);
