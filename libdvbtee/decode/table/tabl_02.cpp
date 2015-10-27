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

#include "functions.h"

#include "tabl_02.h"

#define TABLEID 0x02

#define CLASS_MODULE "[PMT]"

#define dPrintf(fmt, arg...) __dPrintf(DBG_DECODE, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string TABLE_NAME = "PMT";

static std::string PMTES = "PMTES";

void pmt::store(const dvbpsi_pmt_t * const p_pmt)
#define PMT_DBG 1
{
#if PMT_DBG
	fprintf(stderr, "%s PMT: v%d, service_id %d, pcr_pid %d\n", __func__,
		p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
#endif
	set("program", p_pmt->i_program_number);
	set("version", p_pmt->i_version);
	set("pcrPid", p_pmt->i_pcr_pid);

	decoded_pmt.program = p_pmt->i_program_number;
	decoded_pmt.version = p_pmt->i_version;
	decoded_pmt.pcr_pid = p_pmt->i_pcr_pid;
	decoded_pmt.es_streams.clear();

	descriptors.decode(p_pmt->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	Array streams("pid");
#if PMT_DBG
	fprintf(stderr, "  es_pid | type\n");
#endif
	const dvbpsi_pmt_es_t * p_es = p_pmt->p_first_es;
	while (p_es) {
		pmtES * pmtes = new pmtES(decoded_pmt, this, p_es);
		if (pmtes->isValid())
			streams.push((Object*)pmtes);
		p_es = p_es->p_next;
	}

	set("streams", streams);

	setValid(true);

	dPrintf("%s", toJson().c_str());

	if ((/*changed*/true) && (m_watcher)) {
		m_watcher->updateTable(TABLEID, (Table*)this);
	}
}

pmtES::pmtES(decoded_pmt_t& decoded_pmt, Decoder *parent, const dvbpsi_pmt_es_t * const p_es)
: TableDataComponent(parent, PMTES)
{
	if (!p_es) return;

	ts_elementary_stream_t &cur_es = decoded_pmt.es_streams[p_es->i_pid];

	cur_es.type = p_es->i_type;
	cur_es.pid  = p_es->i_pid;

	std::string iso639lang;

	descriptors.decode(p_es->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

#if PMT_DBG
	const dvbtee::decode::Descriptor *d = descriptors.last(0x0a);
	if (d) {
		const Array &a = d->get<Array>("ISO639Lang");
		for (unsigned int i = 0; i < a.size(); i++) {

			const Object &entry(a.get<Object>(i));
			const std::string &lang(entry.get<std::string>("language"));

			if (!lang.length())
				continue;

			if (iso639lang.length()) iso639lang.append(", ");
			iso639lang.append(lang.c_str());

			memcpy(cur_es.iso_639_code, lang.c_str(), sizeof(cur_es.iso_639_code));
			cur_es.iso_639_code[3] = 0;
		}
	}

	fprintf(stderr, "  %6x | 0x%02x (%s) | %s\n",
		p_es->i_pid, p_es->i_type,
		streamtype_name(p_es->i_type),
		iso639lang.c_str());
#endif
	set("pid", p_es->i_pid);
	set("streamType", p_es->i_type);
	set("streamTypeString", streamtype_name(p_es->i_type));

	setValid(true);
}

pmtES::~pmtES()
{

}


bool pmt::ingest(TableStore *s, const dvbpsi_pmt_t * const t, TableWatcher *w)
{
	const std::vector<Table*> pmts = s->get(TABLEID);
	for (std::vector<Table*>::const_iterator it = pmts.begin(); it != pmts.end(); ++it) {
		pmt *thisPMT = (pmt*)*it;
		if (thisPMT->get<uint16_t>("program") == t->i_program_number) {
			if (thisPMT->get<uint16_t>("version") == t->i_version) {
				dPrintf("PMT v%d, service_id %d, pcr_pid %d: ALREADY DECODED", t->i_version, t->i_program_number, t->i_pcr_pid);
				return false;
			}
			thisPMT->store(t);
			return true;
		}
	}
	return s->add<const dvbpsi_pmt_t>(TABLEID, t, w);
}


pmt::pmt(Decoder *parent, TableWatcher *watcher)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	//store table later (probably repeatedly)
}

pmt::pmt(Decoder *parent, TableWatcher *watcher, const dvbpsi_pmt_t * const p_pmt)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	store(p_pmt);
}

pmt::~pmt()
{
	//
}

REGISTER_TABLE_FACTORY(TABLEID, const dvbpsi_pmt_t, pmt);
