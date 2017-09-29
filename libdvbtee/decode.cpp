/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
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

#define DBG 0

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "decode.h"

#if !OLD_DECODER
#include "tabl_00.h"
#include "tabl_02.h"
#include "tabl_40.h"
#include "tabl_42.h"
#include "tabl_4e.h"
#if !USING_DVBPSI_VERSION_0
#include "tabl_c7.h"
#include "tabl_c8.h"
#include "tabl_cb.h"
#include "tabl_cc.h"
#endif
#endif

#include "log.h"
#define CLASS_MODULE "decode"

#include "parse.h"

bool fshowtime;

#define dPrintf(fmt, arg...)					\
do {								\
	__dPrintf(DBG_DECODE, fmt, ##arg);			\
	fshowtime = true;					\
} while (0)

#define dbg_time(fmt, arg...)					\
do {								\
	if ((fshowtime) | (dbg & DBG_TIME)) {			\
		__dPrintf((DBG_DECODE | DBG_TIME), fmt, ##arg);	\
		fshowtime = false;				\
	}							\
} while (0)

decode_report::decode_report()
{
#if DBG
	dPrintf("()");
#endif
}

decode_report::~decode_report()
{
#if DBG
	dPrintf("()");
#endif
}

decode_network_service::decode_network_service(
#if !OLD_DECODER
					       Decoder *parent, std::string &name
#endif
					       )
#if OLD_DECODER
  : services_w_eit_pf(0)
#else
  : LinkedDecoder(parent, name)
  , store(this)
  , subscribedTableWatcher(NULL)
  , services_w_eit_pf(0)
#endif
  , services_w_eit_sched(0)
  , m_eit_x(0)
{
	dPrintf("()");

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}
}

decode_network_service::~decode_network_service()
{
	//showChildren();

	dPrintf("(%05d|%05d)",
		decoded_sdt.network_id, decoded_sdt.ts_id);

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}
}

#if !OLD_DECODER
/* TableWatcher */
void decode_network_service::updateTable(uint8_t tId, dvbtee::decode::Table *table)
{
	dPrintf("0x%02x", tId);

	switch (tId) {
	case 0x42: /* SDT (actual) */
	case 0x46: /* SDT (other) */
		updateSDT(table);
		break;
	case 0x4e:          /* EIT | actual |  p/f  */
	case 0x4f:          /* EIT | other  |  p/f  */
	case 0x50 ... 0x5f: /* EIT | actual | sched */
	case 0x60 ... 0x6f: /* EIT | other  | sched */
		updateEIT(table);
		break;
	default:
		fprintf(stderr, "%s: UNHANDLED TABLE ID 0x%02x !!\n", __func__, tId);
		break;
	}
	if (subscribedTableWatcher)
		subscribedTableWatcher->updateTable(tId, table);
}

bool decode_network_service::updateEIT(dvbtee::decode::Table *table)
{
	if ((!table) || (!table->isValid()) ||
	    ((0x4e > table->getTableid()) ||
	     (0x6f < table->getTableid())))
		return false;

	dvbtee::decode::eit *eitTable = (dvbtee::decode::eit*)table;

	decoded_eit_t &cur_eit = decoded_eit[m_eit_x][eitTable->get<uint16_t>("serviceId")];

	cur_eit = eitTable->getDecodedEIT();

	return true;
}

bool decode_network_service::updateSDT(dvbtee::decode::Table *table)
{
	if ((!table) || (!table->isValid()) ||
	    ((0x42 != table->getTableid()) &&
	     (0x46 != table->getTableid())))
		return false;

	dvbtee::decode::sdt *sdtTable = (dvbtee::decode::sdt*)table;

	decoded_sdt = sdtTable->getDecodedSDT();

	services_w_eit_pf    = sdtTable->get_services_w_eit_pf();
	services_w_eit_sched = sdtTable->get_services_w_eit_sched();

	return true;
}
#endif

decode_network::decode_network(
#if !OLD_DECODER
			       Decoder *parent, std::string &name
#endif
			       )
#if OLD_DECODER
  : orig_network_id(0)
#else
  : LinkedDecoder(parent, name)
  , orig_network_id(0)
  , store(this)
  , subscribedTableWatcher(NULL)
#endif
{
	dPrintf("()");

	decoded_network_services.clear();
}

decode_network::~decode_network()
{
	//showChildren();

#if 0
	dPrintf("(%05d|%05d)",
		decoded_nit.network_id, decoded_sdt.network_id);
#else
	dPrintf("(%05d|%05d) %zu",
		decoded_nit.network_id, orig_network_id,
		decoded_network_services.size());
#endif

	for (map_decoded_network_services::const_iterator it = decoded_network_services.begin(); it != decoded_network_services.end(); ++it)
		delete it->second;
	decoded_network_services.clear();
}

decode_network_service *decode_network::fetch_network_service(uint16_t ts_id)
{
	static std::string name = "NETWORK SERVICE";
	decode_network_service *ret = NULL;
	map_decoded_network_services::const_iterator it = decoded_network_services.find(ts_id);

	if (it != decoded_network_services.end()) {
		ret = it->second;
	} else {
		ret = new decode_network_service(
#if !OLD_DECODER
						 this, name
#endif
						 );
		decoded_network_services[ts_id] = ret;
	}

	ret->subscribeTables(subscribedTableWatcher);

	return ret;
}

#if !OLD_DECODER
/* TableWatcher */
void decode_network::updateTable(uint8_t tId, dvbtee::decode::Table *table)
{
	dPrintf("0x%02x", tId);

	switch (tId) {
	case 0x40: /* NIT (actual) */
	case 0x41: /* NIT (other) */
		updateNIT(table);
		break;
	default:
		fprintf(stderr, "%s: UNHANDLED TABLE ID 0x%02x !!\n", __func__, tId);
		break;
	}
	if (subscribedTableWatcher)
		subscribedTableWatcher->updateTable(tId, table);
}

bool decode_network::updateNIT(dvbtee::decode::Table *table)
{
	if ((!table) || (!table->isValid()) ||
	    ((0x40 != table->getTableid()) &&
	     (0x41 != table->getTableid())))
		return false;

	dvbtee::decode::nit *nitTable = (dvbtee::decode::nit*)table;

	decoded_nit = nitTable->getDecodedNIT();

	return true;
}
#endif

#if !DVBTEE_HAS_CPLUSPLUS_11
static GlobalParse static_parser;

decode::decode()
#else
decode::decode(parse* p)
#endif
#if OLD_DECODER
  : orig_network_id(0)
#else
  : NullDecoder()
  , store(this)
  , subscribedTableWatcher(NULL)
  , orig_network_id(0)
#endif
  , network_id(0)
#if !DVBTEE_HAS_CPLUSPLUS_11
  , m_parser(&static_parser)
#else
  , m_parser(p)
#endif
#if !USE_OWN_NETWORK_DECODERS
  , networks(p->networks)
#endif
  , stream_time((time_t)0)
  , eit_x(0)
  , ett_x(0)
  , physical_channel(0)
{
	dPrintf("()");

	for (int i = 0; i < 128; i++) {
		for (map_decoded_atsc_eit::iterator iter =
			decoded_atsc_eit[i].begin();
		     iter != decoded_atsc_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_atsc_eit[i].clear();
	}

	for (map_decoded_pmt::iterator iter = decoded_pmt.begin();
	     iter != decoded_pmt.end(); ++iter)
		iter->second.es_streams.clear();

	decoded_pmt.clear();
	rcvd_pmt.clear();

	for (int i = 0; i < 128; i++) {
		decoded_ett[i].clear();
	}
}

decode::~decode()
{
	dPrintf("(%04x|%05d)",
		decoded_pat.ts_id, decoded_pat.ts_id);

	for (int i = 0; i < 128; i++) {
		for (map_decoded_atsc_eit::iterator iter =
			decoded_atsc_eit[i].begin();
		     iter != decoded_atsc_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_atsc_eit[i].clear();
	}

	for (map_decoded_pmt::iterator iter = decoded_pmt.begin();
	     iter != decoded_pmt.end(); ++iter)
		iter->second.es_streams.clear();

	rcvd_pmt.clear();
	decoded_pmt.clear();

	for (int i = 0; i < 128; i++) {
		decoded_ett[i].clear();
	}
}

decode_network *decode::fetch_network(uint16_t nw_id)
{
	static std::string name = "NETWORK";
	decode_network *ret = NULL;
	map_network_decoder::const_iterator it = networks.find(nw_id);

	if (it != networks.end()) {
		ret = it->second;
	} else {
		ret = new decode_network(
#if !OLD_DECODER
					 this, name
#endif
					 );
		networks[nw_id] = ret;
	}

	ret->subscribeTables(subscribedTableWatcher);

	return ret;
}

#if !OLD_DECODER
/* TableWatcher */
void decode::updateTable(uint8_t tId, dvbtee::decode::Table *table)
{
	dPrintf("0x%02x", tId);

	switch (tId) {
	case 0x00: /* PAT */
		updatePAT(table);
		break;
	case 0x02: /* PMT */
		updatePMT(table);
		break;
	case 0x70: /* TDT */
	case 0x73: /* TOT */
	case 0xcd: /* STT */
		stream_time = table->get<time_t>("time");
		dbg_time("%s", ctime(&stream_time));
		break;
#if !USING_DVBPSI_VERSION_0
	case 0xc7: /* MGT */
		updateMGT(table);
		break;
	case 0xc8: /* TVCT */
	case 0xc9: /* CVCT */
		updateVCT(table);
		break;
	case 0xcb: /* EIT */
		updateEIT(table);
		break;
	case 0xcc: /* ETT */
		updateETT(table);
		break;
#endif
	default:
		fprintf(stderr, "%s: UNHANDLED TABLE ID 0x%02x !!\n", __func__, tId);
		break;
	}
	if (subscribedTableWatcher)
		subscribedTableWatcher->updateTable(tId, table);
}

bool decode::updatePAT(dvbtee::decode::Table *table)
{
	if ((!table) || (!table->isValid()) || (0x00 != table->getTableid())) return false;

	dvbtee::decode::pat *patTable = (dvbtee::decode::pat*)table;
#if 0
	decoded_pat.ts_id    = patTable->getTsId();
	decoded_pat.version  = patTable->getVersion();
	decoded_pat.programs = patTable->getPrograms();
#else
	decoded_pat = patTable->getDecodedPAT();
#endif
	dPrintf("tsid %04x, ver %02x, %zu programs", decoded_pat.ts_id, decoded_pat.version, decoded_pat.programs.size());

	for (map_decoded_pat_programs::const_iterator iter = decoded_pat.programs.begin();
		iter != decoded_pat.programs.end(); ++iter)
	{
		rcvd_pmt[iter->first] = false;
	}

	return true;
}

bool decode::updatePMT(dvbtee::decode::Table *table)
{
#define PMT_DBG 1
	if ((!table) || (!table->isValid()) || (0x02 != table->getTableid())) return false;

	dvbtee::decode::pmt *pmtTable = (dvbtee::decode::pmt*)table;

	decoded_pmt_t &cur_decoded_pmt = decoded_pmt[pmtTable->get<uint16_t>("program")];

#if 1
	const decoded_pmt_t &new_decoded_pmt = pmtTable->getDecodedPMT();
	cur_decoded_pmt.program = new_decoded_pmt.program;
	cur_decoded_pmt.version = new_decoded_pmt.version;
	cur_decoded_pmt.pcr_pid = new_decoded_pmt.pcr_pid;
	cur_decoded_pmt.es_streams = new_decoded_pmt.es_streams;
#else
	cur_decoded_pmt.program = pmtTable->get<uint16_t>("program");
	cur_decoded_pmt.version = pmtTable->get<uint8_t>("version");
	cur_decoded_pmt.pcr_pid = pmtTable->get<uint16_t>("pcrPid");
	cur_decoded_pmt.es_streams.clear();

#if PMT_DBG
	__log_printf(stderr, "%s: v%d, service_id %d, pcr_pid %d\n", __func__,
		cur_decoded_pmt.version, cur_decoded_pmt.program, cur_decoded_pmt.pcr_pid);
	__log_printf(stderr, "  es_pid | type\n");
#endif
	const dvbtee::value::Array &streams = pmtTable->get<dvbtee::value::Array>("streams");

	for (size_t j = 0; j < streams.size(); ++j) {

		const dvbtee::value::Object &stream = streams.get<dvbtee::value::Object>(j);
		//if (!stream.isValid()) continue;

		ts_elementary_stream_t &cur_es = cur_decoded_pmt.es_streams[stream.get<uint16_t>("pid")];

		cur_es.type = stream.get<uint8_t>("streamType");
		cur_es.pid  = stream.get<uint16_t>("pid");

		const dvbtee::value::Array &desc = stream.get<dvbtee::value::Array>("descriptors");
		for (int k = desc.size()-1; k >= 0; --k)
		{
			const dvbtee::value::Object &d = desc.get<dvbtee::value::Object>(k);
			if (0x0a == d.get<uint8_t>("descriptorTag")) {
				const dvbtee::value::Array &a = d.get<dvbtee::value::Array>("ISO639Lang");
				for (unsigned int i = 0; i < a.size(); i++) {

					const Object &entry(a.get<Object>(i));
					const std::string &lang(entry.get<std::string>("language"));

					if (!lang.length())
						continue;

					memcpy(cur_es.iso_639_code,
					       lang.c_str(),
					       sizeof(cur_es.iso_639_code));
					cur_es.iso_639_code[3] = 0;
				}
				continue;
			}
		}
#if PMT_DBG
		__log_printf(stderr, "  %6x | 0x%02x (%s) | %s\n",
			cur_es.pid, cur_es.type,
			streamtype_name(cur_es.type),
			cur_es.iso_639_code);
#endif
	}
#endif
	return rcvd_pmt[new_decoded_pmt.program /*pmtTable->get<uint16_t>("program")*/] = true;
}

#if !USING_DVBPSI_VERSION_0
bool decode::updateVCT(dvbtee::decode::Table *table)
{
	if ((!table) || (!table->isValid()) ||
	    ((0xc8 != table->getTableid()) &&
	     (0xc9 != table->getTableid())))
		return false;

	dvbtee::decode::vct *vctTable = (dvbtee::decode::vct*)table;

	decoded_vct = vctTable->getDecodedVCT();

	return true;
}

bool decode::updateMGT(dvbtee::decode::Table *table)
{
	if ((!table) || (!table->isValid()) || (0xc7 != table->getTableid()))
		return false;

	dvbtee::decode::mgt *mgtTable = (dvbtee::decode::mgt*)table;

	decoded_mgt = mgtTable->getDecodedMGT();

	return true;
}

bool decode::updateEIT(dvbtee::decode::Table *table)
{
	if ((!table) || (!table->isValid()) || (0xcb != table->getTableid()))
		return false;

	dvbtee::decode::atsc_eit *eitTable = (dvbtee::decode::atsc_eit*)table;

	decoded_atsc_eit_t &cur_atsc_eit = decoded_atsc_eit[eit_x][eitTable->get<uint16_t>("sourceId")];

	cur_atsc_eit = eitTable->getDecodedEIT();

	return true;
}

bool decode::updateETT(dvbtee::decode::Table *table)
{
	if ((!table) || (!table->isValid()) || (0xcc != table->getTableid()))
		return false;

	dvbtee::decode::atsc_ett *ettTable = (dvbtee::decode::atsc_ett*)table;

	decoded_atsc_ett_t &cur_ett = decoded_ett[ett_x][ettTable->get<uint32_t>("etmId")];

	cur_ett = ettTable->getDecodedETT();

	return true;
}
#endif
#endif

/* -- STREAM TIME -- */
#if !USING_DVBPSI_VERSION_0
bool decode::take_stt(const dvbpsi_atsc_stt_t * const p_stt)
{
#if OLD_DECODER
	stream_time = atsc_datetime_utc(p_stt->i_system_time);

	dbg_time("%s", ctime(&stream_time));

	descriptors.decode(p_stt->p_first_descriptor);

	return true;
#else
	return store.ingest(p_stt, this);
#endif
}
#endif

bool decode::take_tot(const dvbpsi_tot_t * const p_tot)
{
#if OLD_DECODER
	stream_time = datetime_utc(p_tot->i_utc_time);

	dbg_time("%s", ctime(&stream_time));

	descriptors.decode(p_tot->p_first_descriptor);

	return true;
#else
	return store.ingest(p_tot, this);
#endif
}

/* -- TABLE HANDLERS -- */
bool decode::take_pat(const dvbpsi_pat_t * const p_pat)
#define PAT_DBG 1
{
	if ((decoded_pat.version == p_pat->i_version) &&
	    (decoded_pat.ts_id   == p_pat->i_ts_id)) {

		dPrintf("v%d, ts_id: %d: ALREADY DECODED",
			p_pat->i_version, p_pat->i_ts_id);
		return false;
	}

#if OLD_DECODER
#if PAT_DBG
	__log_printf(stderr, "%s: v%d, ts_id: %d\n", __func__,
		p_pat->i_version, p_pat->i_ts_id);
#endif
	decoded_pat.ts_id   = p_pat->i_ts_id;
	decoded_pat.version = p_pat->i_version;
	decoded_pat.programs.clear();

	const dvbpsi_pat_program_t *p_program = p_pat->p_first_program;
	while (p_program) {
//		if (p_program->i_number > 0)
		decoded_pat.programs[p_program->i_number] = p_program->i_pid;

		rcvd_pmt[p_program->i_number] = false;
#if PAT_DBG
		__log_printf(stderr, "  %10d | %x\n",
			p_program->i_number,
			decoded_pat.programs[p_program->i_number]);
#endif
		p_program = p_program->p_next;
	}
	return true;
#else
	return store.ingest(p_pat, this);
#endif
}

bool decode::take_pmt(const dvbpsi_pmt_t * const p_pmt)
{
	decoded_pmt_t &cur_decoded_pmt = decoded_pmt[p_pmt->i_program_number];

	if ((cur_decoded_pmt.version == p_pmt->i_version) &&
	    (cur_decoded_pmt.program == p_pmt->i_program_number)) {

		dPrintf("v%d, service_id %d, pcr_pid %d: ALREADY DECODED",
			p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
		return false;
	}
#if OLD_DECODER
#if PMT_DBG
	__log_printf(stderr, "%s: v%d, service_id %d, pcr_pid %d\n", __func__,
		p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
#endif
	cur_decoded_pmt.program = p_pmt->i_program_number;
	cur_decoded_pmt.version = p_pmt->i_version;
	cur_decoded_pmt.pcr_pid = p_pmt->i_pcr_pid;
	cur_decoded_pmt.es_streams.clear();
	//FIXME: descriptors
	descriptors.decode(p_pmt->p_first_descriptor);

	__log_printf(stderr, "  es_pid | type\n");

	const dvbpsi_pmt_es_t *p_es = p_pmt->p_first_es;
	while (p_es) {

		ts_elementary_stream_t &cur_es = cur_decoded_pmt.es_streams[p_es->i_pid];

		cur_es.type = p_es->i_type;
		cur_es.pid  = p_es->i_pid;

		//FIXME: descriptors
		descriptors.decode(p_es->p_first_descriptor);

#ifdef COPY_DRA1_FROM_VCT_TO_PMT // disabled, to be deleted
		if (descriptors._a1.count(p_es->i_pid)) {
			memcpy(cur_es.iso_639_code,
			       descriptors._a1[p_es->i_pid].iso_639_code,
			       sizeof(descriptors._a1[p_es->i_pid].iso_639_code));
		}
#endif
		desc local_descriptors;
		local_descriptors.decode(p_es->p_first_descriptor);

		std::string languages;

		for (map_dr0a::const_iterator iter_dr0a = local_descriptors._0a.begin(); iter_dr0a != local_descriptors._0a.end(); ++iter_dr0a) {
			if (!languages.empty()) languages.append(", ");
			if (iter_dr0a->second.iso_639_code[0]) {
				for (int i=0; i<3; i++) languages.push_back(iter_dr0a->second.iso_639_code[i]);

				memcpy(cur_es.iso_639_code,
				       iter_dr0a->second.iso_639_code,
				       sizeof(iter_dr0a->second.iso_639_code));
			}
		}
#if PMT_DBG
		__log_printf(stderr, "  %6x | 0x%02x (%s) | %s\n",
			cur_es.pid, cur_es.type,
			streamtype_name(cur_es.type),
#if 1 //def COPY_DRA1_FROM_VCT_TO_PMT // FIXME
			cur_es.iso_639_code);
#else
			languages.c_str());
#endif
#endif
		p_es = p_es->p_next;
	}
	rcvd_pmt[p_pmt->i_program_number] = true;

	return true;
#else
	return store.ingest(p_pmt, this);
#endif
}

#if !USING_DVBPSI_VERSION_0
bool decode::take_vct(const dvbpsi_atsc_vct_t * const p_vct)
#define VCT_DBG 1
{
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = p_vct->i_ts_id;
#else
	uint16_t __ts_id = p_vct->i_extension;
#endif
	if ((decoded_vct.version == p_vct->i_version) &&
	    (decoded_vct.ts_id   == __ts_id)) {

		dPrintf("v%d, ts_id %d, b_cable_vct %d: ALREADY DECODED",
			p_vct->i_version, __ts_id, p_vct->b_cable_vct);
		return false;
	}
#if OLD_DECODER
#if VCT_DBG
	__log_printf(stderr, "%s: v%d, ts_id %d, b_cable_vct %d\n", __func__,
		p_vct->i_version, __ts_id, p_vct->b_cable_vct);
#endif
	decoded_vct.version   = p_vct->i_version;
	decoded_vct.ts_id     = __ts_id;
	decoded_vct.cable_vct = p_vct->b_cable_vct;
	decoded_vct.channels.clear();

	const dvbpsi_atsc_vct_channel_t *p_channel = p_vct->p_first_channel;
#if VCT_DBG
	if (p_channel)
		__log_printf(stderr, "  channel | service_id | source_id | service_name\n");
#endif
	while (p_channel) {

		decoded_vct_channel_t &cur_channel = decoded_vct.channels[p_channel->i_program_number];

		memcpy(cur_channel.short_name,  p_channel->i_short_name,
		       sizeof(cur_channel.short_name));

		cur_channel.chan_major        = p_channel->i_major_number;
		cur_channel.chan_minor        = p_channel->i_minor_number;
		cur_channel.modulation        = p_channel->i_modulation;
		cur_channel.carrier_freq      = p_channel->i_carrier_freq;
		cur_channel.chan_ts_id        = p_channel->i_channel_tsid;
		cur_channel.program           = p_channel->i_program_number;
		cur_channel.etm_location      = p_channel->i_etm_location;
		cur_channel.access_controlled = p_channel->b_access_controlled;
		cur_channel.path_select       = p_channel->b_path_select;
		cur_channel.out_of_band       = p_channel->b_out_of_band;
		cur_channel.hidden            = p_channel->b_hidden;
		cur_channel.hide_guide        = p_channel->b_hide_guide;
		cur_channel.service_type      = p_channel->i_service_type;
		cur_channel.source_id         = p_channel->i_source_id;

		//FIXME: descriptors
		dPrintf("parsing channel descriptors for service: %d", p_channel->i_program_number);
		descriptors.decode(p_channel->p_first_descriptor);

#if OLD_DECODER
		desc local_descriptors;
#else
		dvbtee::decode::DescriptorStore local_descriptors;
#endif
		local_descriptors.decode(p_channel->p_first_descriptor);

		std::string languages;

#if OLD_DECODER
		for (map_dra1::const_iterator iter_dra1 = local_descriptors._a1.begin(); iter_dra1 != local_descriptors._a1.end(); ++iter_dra1) {
			if (!languages.empty()) languages.append(", ");
			if (iter_dra1->second.iso_639_code[0])
				for (int i=0; i<3; i++) languages.push_back(iter_dra1->second.iso_639_code[i]);
		}
#else
		const dvbtee::decode::Descriptor *d = local_descriptors.last(0xa1);
		if (d) {
			const dvbtee::value::Array& a  = d->get<dvbtee::value::Array>("serviceLocation");
			for (unsigned int i = 0; i < a.size(); i++) {
				const Object& o = a.get<Object>(i);

				const std::string lang = o.get<std::string>("lang");
				//o.get<uint16_t>("esPid");
				//o.get<uint8_t>("streamType");
				//o.get<std::string>("streamTypeString");

				if (lang.length()) {
					if (!languages.empty()) languages.append(", ");
					languages.append(lang.c_str());
				}
			}
		}
#endif
#if VCT_DBG
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = cur_channel.short_name[i*2+1];
		service_name[7] = 0;

		__log_printf(stderr, "  %5d.%d | %10d | %9d | %s | %s\n",
			cur_channel.chan_major,
			cur_channel.chan_minor,
			cur_channel.program,
			cur_channel.source_id,
			service_name, languages.c_str());
#endif
		p_channel = p_channel->p_next;
	}
	//FIXME: descriptors
	dPrintf("parsing channel descriptors for mux:");
	descriptors.decode(p_vct->p_first_descriptor);

	return true;
#else
	return store.ingest(p_vct, this);
#endif
}

bool decode::take_mgt(const dvbpsi_atsc_mgt_t * const p_mgt)
#define MGT_DBG 1
{
	if ((decoded_mgt.version == p_mgt->i_version) &&
	    (!decoded_mgt.tables.empty())) {

		dPrintf("v%d: ALREADY DECODED", p_mgt->i_version);
		return false;
	}
#if OLD_DECODER
#if MGT_DBG
	__log_printf(stderr, "%s: v%d\n", __func__, p_mgt->i_version);
#endif
	decoded_mgt.version = p_mgt->i_version;
	decoded_mgt.tables.clear();

	const dvbpsi_atsc_mgt_table_t *p_table = p_mgt->p_first_table;
#if MGT_DBG
	if (p_table)
		__log_printf(stderr, "  table type |   pid  | ver | bytes\n");
#endif
	while (p_table) {
#if MGT_DBG
		__log_printf(stderr, "    0x%04x   | 0x%04x | %3d | %d\n",
				p_table->i_table_type, p_table->i_table_type_pid,
				p_table->i_table_type_version, p_table->i_number_bytes);
#endif
		decoded_mgt_table_t &cur_table = decoded_mgt.tables[p_table->i_table_type];

		cur_table.type    = p_table->i_table_type;
		cur_table.pid     = p_table->i_table_type_pid;
		cur_table.version = p_table->i_table_type_version;
		cur_table.bytes   = p_table->i_number_bytes;
#if 0
		switch (p_table->i_table_type) {
		default:
			break;
		}
#endif
		//FIXME: descriptors
		descriptors.decode(p_table->p_first_descriptor);

		p_table = p_table->p_next;
	}
	//FIXME: descriptors
	descriptors.decode(p_mgt->p_first_descriptor);

	return true;
#else
	return store.ingest(p_mgt, this);
#endif
}
#endif

#if OLD_DECODER
static bool __take_nit(const dvbpsi_nit_t * const p_nit, decoded_nit_t* decoded_nit, /*dvbtee::decode::DescriptorStore*/ desc* descriptors)
#define NIT_DBG 1
{
	if ((decoded_nit->version    == p_nit->i_version) &&
	    (decoded_nit->network_id == p_nit->i_network_id)) {

		dPrintf("v%d, network_id %d: ALREADY DECODED",
			p_nit->i_version, p_nit->i_network_id);
		return false;
	}
#if NIT_DBG
	__log_printf(stderr, "%s: v%d, network_id %d\n", __func__,
		p_nit->i_version, p_nit->i_network_id);
#endif
	decoded_nit->version    = p_nit->i_version;
	decoded_nit->network_id = p_nit->i_network_id;

	const dvbpsi_nit_ts_t *p_ts = p_nit->p_first_ts;
#if NIT_DBG
	if (p_ts)
		__log_printf(stderr, "   ts_id | orig_network_id\n");
#endif
	while (p_ts) {

		decoded_nit_ts_t &cur_ts_list = decoded_nit->ts_list[p_ts->i_ts_id];

		cur_ts_list.ts_id           = p_ts->i_ts_id;
		cur_ts_list.orig_network_id = p_ts->i_orig_network_id;

#if NIT_DBG
		__log_printf(stderr, "   %05d | %d\n",
			cur_ts_list.ts_id,
			cur_ts_list.orig_network_id);
#endif
		/* descriptors contain frequency lists & LCNs */
		descriptors->decode(p_ts->p_first_descriptor);

		p_ts = p_ts->p_next;
	}

	descriptors->decode(p_nit->p_first_descriptor);

	return true;
}
#endif

bool decode::take_nit_actual(const dvbpsi_nit_t * const p_nit)
{
	network_id = p_nit->i_network_id;

	decode_network *nw = fetch_network(network_id);
#if 0
	return nw->take_nit(p_nit);
#else
	bool ret = nw->take_nit(p_nit);
	const decoded_nit_t *decoded_nit = get_decoded_nit();
	if ((decoded_nit) && (decoded_nit->ts_list.count(decoded_pat.ts_id))) {
		orig_network_id = ((decoded_nit_t*)decoded_nit)->ts_list[decoded_pat.ts_id].orig_network_id;

		nw->orig_network_id = orig_network_id;
#if 0
		return networks[orig_network_id].take_nit(p_nit);
#endif
	}
	return ret;
#endif
}

bool decode::take_nit_other(const dvbpsi_nit_t * const p_nit)
{
	decode_network *nw = fetch_network(p_nit->i_network_id);
#if 0
	return nw->take_nit(p_nit);
#else
	bool ret = nw->take_nit(p_nit);
	const decoded_nit_t *decoded_nit = get_decoded_nit();
	if ((decoded_nit) && (decoded_nit->ts_list.count(decoded_pat.ts_id))) {
		uint16_t other_orig_network_id = ((decoded_nit_t*)decoded_nit)->ts_list[decoded_pat.ts_id].orig_network_id;

		nw->orig_network_id = other_orig_network_id;
#if 0
		return networks[other_orig_network_id].take_nit(p_nit);
#endif
	}
	return ret;
#endif
}

bool decode_network::take_nit(const dvbpsi_nit_t * const p_nit)
{
#if OLD_DECODER
	return __take_nit(p_nit, &decoded_nit, &descriptors);
#else
	// XXX: FIXME: must refactor decode::get_lcn() & LCN descriptor 0x83
	descriptors.decode(p_nit->p_first_descriptor);
	dvbpsi_nit_ts_t *ts = p_nit->p_first_ts;
	while (ts) { descriptors.decode(ts->p_first_descriptor); ts = ts->p_next; }

	return store.ingest(p_nit, this);
#endif
}

const decoded_sdt_t *decode_network::get_decoded_sdt(uint16_t ts_id) const
{
	map_decoded_network_services::const_iterator it = decoded_network_services.find(ts_id);
	return (it == decoded_network_services.end()) ? NULL : it->second->get_decoded_sdt();
	//return decoded_network_services.count(ts_id) ? &decoded_network_services[ts_id].decoded_sdt : NULL;
}

const map_decoded_eit *decode_network::get_decoded_eit(uint16_t ts_id) const
{
	map_decoded_network_services::const_iterator it = decoded_network_services.find(ts_id);
	return (it == decoded_network_services.end()) ? NULL : it->second->get_decoded_eit();
	//return decoded_network_services.count(ts_id) ? decoded_network_services[ts_id].decoded_eit : NULL;
}

#if OLD_DECODER
static bool __take_sdt(const dvbpsi_sdt_t * const p_sdt, decoded_sdt_t* decoded_sdt, /*dvbtee::decode::DescriptorStore*/ desc* descriptors,
		       unsigned int* services_w_eit_pf, unsigned int* services_w_eit_sched)
#define SDT_DBG 1
{
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = p_sdt->i_ts_id;
#else
	uint16_t __ts_id = p_sdt->i_extension;
#endif
	if ((decoded_sdt->version    == p_sdt->i_version) &&
	    (decoded_sdt->network_id == p_sdt->i_network_id)) {

		dPrintf("v%d | ts_id %d | network_id %d: ALREADY DECODED",
			p_sdt->i_version,
			__ts_id,
			p_sdt->i_network_id);
		return false;
	}
	dPrintf("v%02d | ts_id %05d | network_id %05d\n"
		/*"------------------------------------"*/,
		p_sdt->i_version,
		__ts_id,
		p_sdt->i_network_id);

	decoded_sdt->ts_id      = __ts_id;
	decoded_sdt->version    = p_sdt->i_version;
	decoded_sdt->network_id = p_sdt->i_network_id;

	unsigned int _services_w_eit_pf    = 0;
	unsigned int _services_w_eit_sched = 0;
	//__log_printf(stderr, "  service_id | service_name");
	const dvbpsi_sdt_service_t *p_service = p_sdt->p_first_service;
	while (p_service) {

		decoded_sdt_service_t &cur_service = decoded_sdt->services[p_service->i_service_id];

		cur_service.service_id     = p_service->i_service_id; /* matches program_id / service_id from PAT */
		cur_service.f_eit_sched    = p_service->b_eit_schedule;
		cur_service.f_eit_present  = p_service->b_eit_present;
		cur_service.running_status = p_service->i_running_status;
		cur_service.f_free_ca      = p_service->b_free_ca;

		if (cur_service.f_eit_present)
			_services_w_eit_pf++;

		if (cur_service.f_eit_sched)
			_services_w_eit_sched++;

		/* service descriptors contain service provider name & service name */
		descriptors->decode(p_service->p_first_descriptor);

#if OLD_DECODER
		strncpy((char*)cur_service.provider_name,
			(const char*)descriptors->provider_name,
			sizeof(cur_service.provider_name)-1);
		cur_service.provider_name[sizeof(cur_service.provider_name)-1] = '\0';
		strncpy((char*)cur_service.service_name,
			(const char*)descriptors->service_name,
			sizeof(cur_service.service_name)-1);
		cur_service.service_name[sizeof(cur_service.service_name)-1] = '\0';
#else
		memset((void*)cur_service.provider_name, 0, sizeof(cur_service.provider_name));
		memset((void*)cur_service.service_name, 0, sizeof(cur_service.service_name));

		const dvbtee::decode::Descriptor *d = descriptors->last(0x48);
		if (d) {
			strncpy((char*)cur_service.provider_name,
				d->get<std::string>("providerName").c_str(),
				sizeof(cur_service.provider_name));
			strncpy((char*)cur_service.service_name,
				d->get<std::string>("serviceName").c_str(),
				sizeof(cur_service.service_name));
		}
#endif

		dPrintf("%05d | %s %s | %s - %s",
			cur_service.service_id,
			(cur_service.f_eit_present) ? "p/f" : "   ",
			(cur_service.f_eit_sched) ? "sched" : "     ",
			cur_service.provider_name,
			cur_service.service_name);

		p_service = p_service->p_next;
	}
	*services_w_eit_pf    = _services_w_eit_pf;
	*services_w_eit_sched = _services_w_eit_sched;
	return true;
}
#endif

bool decode::take_sdt_actual(const dvbpsi_sdt_t * const p_sdt)
{
	orig_network_id = p_sdt->i_network_id;

	decode_network *nw = fetch_network(orig_network_id);

	nw->orig_network_id = orig_network_id;

	return nw->take_sdt(p_sdt);
}

bool decode::take_sdt_other(const dvbpsi_sdt_t * const p_sdt)
{
	decode_network *nw = fetch_network(p_sdt->i_network_id);

	nw->orig_network_id = p_sdt->i_network_id;

	return nw->take_sdt(p_sdt);
}

bool decode_network_service::take_sdt(const dvbpsi_sdt_t * const p_sdt)
{
#if OLD_DECODER
	return __take_sdt(p_sdt, &decoded_sdt, &descriptors,
			  &services_w_eit_pf, &services_w_eit_sched);
#else
	return store.ingest(p_sdt, this);
#endif
}

#if OLD_DECODER
bool __take_eit(const dvbpsi_eit_t * const p_eit, map_decoded_eit *decoded_eit, /*dvbtee::decode::DescriptorStore*/ desc* descriptors, uint8_t eit_x)
{
#if USING_DVBPSI_VERSION_0
	uint16_t __service_id = p_eit->i_service_id;
#else
	uint16_t __service_id = p_eit->i_extension;
#endif

	decoded_eit_t &cur_eit = decoded_eit[eit_x][__service_id];

	if ((cur_eit.version == p_eit->i_version) &&
	    (cur_eit.service_id == __service_id)) {
#if DBG
		__log_printf(stderr, "%s-%d: v%d | ts_id %d | network_id %d service_id %d: ALREADY DECODED\n", __func__, eit_x,
			p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, __service_id);
#endif
		return false;
	}
#if DBG
	__log_printf(stderr, "%s-%d: v%d | ts_id %d | network_id %d service_id %d | table id: 0x%02x, last_table id: 0x%02x\n", __func__, eit_x,
		p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, __service_id, p_eit->i_table_id, p_eit->i_last_table_id);
#endif
	cur_eit.service_id    = __service_id;
	cur_eit.version       = p_eit->i_version;
	cur_eit.ts_id         = p_eit->i_ts_id;
	cur_eit.network_id    = p_eit->i_network_id;
	cur_eit.last_table_id = p_eit->i_last_table_id;

	const dvbpsi_eit_event_t *p_event = p_eit->p_first_event;
	while (p_event) {

		decoded_eit_event_t &cur_event = cur_eit.events[p_event->i_event_id];

		cur_event.event_id       = p_event->i_event_id;
		cur_event.start_time     = p_event->i_start_time;
		cur_event.length_sec     = p_event->i_duration;
		cur_event.running_status = p_event->i_running_status;
		cur_event.f_free_ca      = p_event->b_free_ca;

		descriptors->decode(p_event->p_first_descriptor);

#if OLD_DECODER
		cur_event.name.assign((const char *)descriptors->_4d.name);
		cur_event.text.assign((const char *)descriptors->_4d.text);
#else
		const dvbtee::decode::Descriptor *d = descriptors->last(0x4d);
		if (d) {
			cur_event.name.assign(d->get<std::string>("name").c_str());
			cur_event.text.assign(d->get<std::string>("text").c_str());
		}
#endif
#if DBG
		time_t start = datetime_utc(cur_event.start_time /*+ (60 * tz_offset)*/);
		time_t end   = datetime_utc(cur_event.start_time + cur_event.length_sec /*+ (60 * tz_offset)*/);

		struct tm tms = *localtime(&start);
		struct tm tme = *localtime(&end);

		__log_printf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, descriptors->_4d.name);
#endif
		p_event = p_event->p_next;
	}
	return true;
}
#endif

static inline bool table_id_to_eit_x(uint8_t table_id, uint8_t *eit_x)
{
#if 1
	switch (table_id) {
	case 0x4f: /* fall thru */
	case 0x4e:
		*eit_x = 0;
		break;
	case 0x60 ... 0x6f:
		*eit_x = table_id - 0x60 + 1;
		break;
	case 0x50 ... 0x5f:
		*eit_x = table_id - 0x50 + 1;
		break;
	}
	return true;
#else
	*actual = false;
	switch (table_id) {
	case 0x4e:
	case 0x50 ... 0x5f:
		*actual = true;
		/* fall-thru */
	case 0x4f:
	case 0x60 ... 0x6f:
		*eit_x = (table_id & 0xfe == 0x4e) ? 0 : table_id - (table_id & 0xf0) + 1;
		return true;
	default:
		return false;
	}
#endif
}

bool decode::take_eit(const dvbpsi_eit_t * const p_eit)
{
	/* we want our own eit_x here - we don't need to store this in our class, the stored eit_x is for ATSC */
	/* prevent warning: ‘eit_x’ may be used uninitialized in this function [-Wmaybe-uninitialized] */
	uint8_t eit_x = 0;
#if !USING_DVBPSI_VERSION_0 // this is totally a bug when this line is omitted.  instead of trying to fix this, just update your libdvbpsi
	table_id_to_eit_x(p_eit->i_table_id, &eit_x);
#endif

	decode_network *nw = fetch_network(p_eit->i_network_id);
	return nw->take_eit(p_eit, eit_x);
}

bool decode_network_service::take_eit(const dvbpsi_eit_t * const p_eit, uint8_t eit_x)
{
#if OLD_DECODER
	return __take_eit(p_eit, decoded_eit, &descriptors, eit_x);
#else
	m_eit_x = eit_x;
	return store.ingest(p_eit, this);
#endif
}

#if !USING_DVBPSI_VERSION_0
bool decode::take_eit(const dvbpsi_atsc_eit_t * const p_eit)
{
	decoded_atsc_eit_t &cur_atsc_eit = decoded_atsc_eit[eit_x][p_eit->i_source_id];

	if ((cur_atsc_eit.version   == p_eit->i_version) &&
	    (cur_atsc_eit.source_id == p_eit->i_source_id)) {
#if DBG
		__log_printf(stderr, "%s-%d: v%d, source_id %d: ALREADY DECODED\n", __func__,
			eit_x, p_eit->i_version, p_eit->i_source_id);
#endif
		return false;
	}
#if OLD_DECODER
#if DBG
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct)
		if (iter_vct->second.source_id == p_eit->i_source_id)
			break;

	if (iter_vct == decoded_vct.channels.end()) {
		__log_printf(stderr, "%s-%d: v%d, id:%d\n", __func__,
			eit_x, p_eit->i_version, p_eit->i_source_id);
	} else {
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		service_name[7] = 0;

		__log_printf(stderr, "%s-%d: v%d, id:%d - %d.%d: %s\n", __func__,
			eit_x, p_eit->i_version, p_eit->i_source_id,
			iter_vct->second.chan_major,
			iter_vct->second.chan_minor,
			service_name);
	}
#endif
	cur_atsc_eit.version   = p_eit->i_version;
	cur_atsc_eit.source_id = p_eit->i_source_id;

	const dvbpsi_atsc_eit_event_t *p_event = p_eit->p_first_event;
	while (p_event) {

		decoded_atsc_eit_event_t &cur_event = cur_atsc_eit.events[p_event->i_event_id];

		cur_event.event_id     = p_event->i_event_id;
		cur_event.start_time   = p_event->i_start_time;
		cur_event.etm_location = p_event->i_etm_location;
		cur_event.length_sec   = p_event->i_length_seconds;
		cur_event.title_bytes  = p_event->i_title_length;
		memcpy(cur_event.title, p_event->i_title, 256); // FIXME
#if DBG
		time_t start = atsc_datetime_utc(cur_event.start_time /*+ (60 * tz_offset)*/);
		time_t end   = atsc_datetime_utc(cur_event.start_time + cur_event.length_sec /*+ (60 * tz_offset)*/);

		unsigned char name[256];
		memset(name, 0, sizeof(char) * 256);
		decode_multiple_string(cur_event.title,
				       cur_event.title_bytes,
				       name, sizeof(name));
		//p_epg->text[0] = 0;

		struct tm tms = *localtime( &start );
		struct tm tme = *localtime( &end  );
		__log_printf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
		//FIXME: descriptors
		descriptors.decode(p_event->p_first_descriptor);

		p_event = p_event->p_next;
	}

	descriptors.decode(p_eit->p_first_descriptor);

	return true;
#else
	return store.ingest(p_eit, this);
#endif
}
#endif

static bool _get_epg_event(decoded_event_t *e,
			  const char * channel_name,
			  uint16_t chan_major,
			  uint16_t chan_minor,
			  uint16_t chan_physical,
			  uint16_t chan_svc_id,
			  //
			  uint16_t event_id,
			  time_t start_time,
			  uint32_t length_sec,
			  const char * name,
			  const char * text)
{
	if (!e) return false;

	e->channel_name.assign(channel_name);
	e->chan_major    = chan_major;
	e->chan_minor    = chan_minor;

	e->chan_physical = chan_physical;
	e->chan_svc_id   = chan_svc_id;

	e->event_id      = event_id;
	e->start_time    = start_time;
	e->length_sec    = length_sec;
	e->name.assign(name);
	e->text.assign(text);

	return true;
}

void decode_report::epg_event(const char * channel_name,
				   uint16_t chan_major,
				   uint16_t chan_minor,
				   uint16_t chan_physical,
				   uint16_t chan_svc_id,
				   //
				   uint16_t event_id,
				   time_t start_time,
				   uint32_t length_sec,
				   const char * name,
				   const char * text)
{
	decoded_event_t e;
	_get_epg_event(&e, channel_name,
		      chan_major, chan_minor,
		      chan_physical, chan_svc_id,
		      event_id, start_time, length_sec,
		      name, text);

	epg_event(e);
}


void decode::dump_epg_event(uint8_t current_eit_x, const decoded_vct_channel_t *channel, const decoded_atsc_eit_event_t *event, decode_report *reporter)
{
	unsigned char service_name[8] = { 0 };
	for ( int i = 0; i < 7; ++i ) service_name[i] = channel->short_name[i*2+1];
	service_name[7] = 0;

	__log_printf(stderr, "%s: id:%d - %d.%d: %s\t", __func__,
		channel->source_id,
		channel->chan_major,
		channel->chan_minor, service_name);

	time_t start = atsc_datetime_utc(event->start_time /*+ (60 * tz_offset)*/);
	time_t end   = atsc_datetime_utc(event->start_time + event->length_sec /*+ (60 * tz_offset)*/);

	unsigned char name[512];
	memset(name, 0, sizeof(name));
	decode_multiple_string(event->title, event->title_bytes, name, sizeof(name));

	//FIXME: descriptors

	struct tm tms = *localtime( &start );
	struct tm tme = *localtime( &end  );

	unsigned char message[ETM_MAX_LENGTH] = { 0 };

	const char* etm = (const char *)get_decoded_ett(current_eit_x, (channel->source_id << 16) | (event->event_id << 2) | 0x02, message, sizeof(message));

	__log_printf(stderr, "%04d-%02d-%02d %02d:%02d-%02d:%02d %s%s%s\n",
		     tms.tm_year+1900, tms.tm_mon+1, tms.tm_mday,
		     tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name, (message[0]) ? "\n" : "", (message[0]) ? (const char *)message : "");

	if (reporter) {
		reporter->epg_event((const char *)service_name,
					 channel->chan_major, channel->chan_minor,
					 physical_channel, channel->program,
					 event->event_id,
					 start,
					 (end - start),
					 (const char *)name,
					 (const char *)message);
	}
	return;
}

void decode::dump_epg_event(const decoded_sdt_service_t *service, const decoded_eit_event_t *event, decode_report *reporter)
{
	__log_printf(stderr, "%s: id:%d - %d: %s\t", __func__,
		service->service_id,
		get_lcn(service->service_id),
		service->service_name);

	time_t start = datetime_utc(event->start_time /*+ (60 * tz_offset)*/);
	time_t end   = datetime_utc(event->start_time + event->length_sec /*+ (60 * tz_offset)*/);

	//FIXME: descriptors

	struct tm tms = *localtime( &start );
	struct tm tme = *localtime( &end  );
	__log_printf(stderr, "%04d-%02d-%02d,%02d:%02d,%02d:%02d %s\n",
		     tms.tm_year+1900, tms.tm_mon+1, tms.tm_mday,
		     tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min,
		     event->name.c_str());

	if (reporter)
		reporter->epg_event((const char *)service->service_name,
					 get_lcn(service->service_id), 0,
					 physical_channel, service->service_id,
					 event->event_id,
					 start,
					 (end - start),
					 event->name.c_str(),
					 event->text.c_str());
	return;
}

void decode::get_epg_event(uint8_t current_eit_x, const decoded_vct_channel_t *channel, const decoded_atsc_eit_event_t *event, decoded_event_t *e)
{
#if 1//DBG
	__log_printf(stderr, "%s\n", __func__);
#endif
	unsigned char service_name[8] = { 0 };
	for ( int i = 0; i < 7; ++i ) service_name[i] = channel->short_name[i*2+1];
	service_name[7] = 0;

	time_t start = atsc_datetime_utc(event->start_time /*+ (60 * tz_offset)*/);
	time_t end   = atsc_datetime_utc(event->start_time + event->length_sec /*+ (60 * tz_offset)*/);

	unsigned char name[512];
	memset(name, 0, sizeof(name));
	decode_multiple_string(event->title, event->title_bytes, name, sizeof(name));
#if 1
	//FIXME: descriptors

	struct tm tms = *localtime( &start );
	struct tm tme = *localtime( &end  );
	__log_printf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
	unsigned char message[ETM_MAX_LENGTH];
	_get_epg_event(e, (const char *)service_name,
		      channel->chan_major, channel->chan_minor,
		      physical_channel, channel->program,
		      event->event_id,
		      start,
		      event->length_sec,
		      (const char *)name,
		      (const char *)get_decoded_ett(current_eit_x, (channel->source_id << 16) | (event->event_id << 2) | 0x02, message, sizeof(message)));
	return;
}

void decode::get_epg_event(const decoded_sdt_service_t *service, const decoded_eit_event_t *event, decoded_event_t *e)
{
#if 1//DBG
	__log_printf(stderr, "%s\n", __func__);
#endif
	time_t start = datetime_utc(event->start_time /*+ (60 * tz_offset)*/);
	time_t end   = datetime_utc(event->start_time + event->length_sec /*+ (60 * tz_offset)*/);
#if 1
	//FIXME: descriptors

	struct tm tms = *localtime( &start );
	struct tm tme = *localtime( &end  );
	__log_printf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, event->name.c_str()/*, iter_eit->second.text.c_str()*/ );
#endif

	_get_epg_event(e, (const char *)service->service_name,
		      get_lcn(service->service_id), 0,
		      physical_channel, service->service_id,
		      event->event_id,
		      start,
		      event->length_sec,
		      event->name.c_str(),
		      event->text.c_str());
	return;
}

bool decode::get_epg_event_atsc(uint16_t source_id, time_t showtime, decoded_event_t *e)
{
#if 1//DBG
	__log_printf(stderr, "%s\n", __func__);
#endif
	if (!source_id)
		return false;

	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct)
		if (source_id == iter_vct->second.source_id) {

	unsigned int eit_num = 0;

	while ((eit_num < 128) && (decoded_atsc_eit[eit_num].count(source_id))) {
#if 0
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		service_name[7] = 0;

		__log_printf(stdout, "%s-%d: id:%d - %d.%d: %s\n", __func__,
			eit_num, iter_vct->second.source_id,
			iter_vct->second.chan_major,
			iter_vct->second.chan_minor,
			service_name);
#endif
		decoded_atsc_eit_t &cur_atsc_eit = decoded_atsc_eit[eit_num][iter_vct->second.source_id];
		map_decoded_atsc_eit_events::const_iterator iter_eit;
		for (iter_eit = cur_atsc_eit.events.begin();
		     iter_eit != cur_atsc_eit.events.end();
		     ++iter_eit) {

			time_t start = atsc_datetime_utc(iter_eit->second.start_time /*+ (60 * tz_offset)*/);
			time_t end   = atsc_datetime_utc(iter_eit->second.start_time + iter_eit->second.length_sec /*+ (60 * tz_offset)*/);

			if ((start <= showtime) && (end > showtime)) {
#if 1
				unsigned char name[512];
				memset(name, 0, sizeof(name));
				decode_multiple_string(iter_eit->second.title, iter_eit->second.title_bytes, name, sizeof(name));

				//FIXME: descriptors

				struct tm tms = *localtime( &start );
				struct tm tme = *localtime( &end  );
				__log_printf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
				get_epg_event(eit_num, &iter_vct->second, &iter_eit->second, e);
				return true;
			}
		}
		eit_num++;
	}
	}
	return false;
}

bool decode::get_epg_event_dvb(uint16_t service_id, time_t showtime, decoded_event_t *e)
{
#if 1//DBG
	__log_printf(stderr, "%s\n", __func__);
#endif
	if (!service_id)
		return false;

	const decoded_sdt_t *decoded_sdt = get_decoded_sdt();
	if (!decoded_sdt)
		return false;

	map_decoded_sdt_services::const_iterator iter_sdt = decoded_sdt->services.find(service_id);
	if (iter_sdt == decoded_sdt->services.end())
		return false;

	if (!iter_sdt->second.f_eit_present)
		return false;

	unsigned int eit_num = 0;

	const map_decoded_eit *decoded_eit = get_decoded_eit();
	// FIXME:  CHANGE TO CONST_ITERATOR -- THIS IS DANGEROUS!!

	if (decoded_eit) while ((eit_num < NUM_EIT) && (decoded_eit[eit_num].count(service_id))) {
#if 0
		__log_printf(stdout, "%s-%d: id:%d - %d: %s\n", __func__,
			eit_num, iter_sdt->second.service_id,
			get_lcn(iter_sdt->second.service_id),
			iter_sdt->second.service_name);
#endif

		map_decoded_eit_events::const_iterator iter_eit;
		//if (decoded_eit)
		for (iter_eit = ((map_decoded_eit*)decoded_eit)[eit_num][iter_sdt->second.service_id].events.begin();
		     iter_eit != ((map_decoded_eit*)decoded_eit)[eit_num][iter_sdt->second.service_id].events.end();
		     ++iter_eit) {

			time_t start = datetime_utc(iter_eit->second.start_time /*+ (60 * tz_offset)*/);
			time_t end   = datetime_utc(iter_eit->second.start_time + iter_eit->second.length_sec /*+ (60 * tz_offset)*/);

			if ((start <= showtime) && (end > showtime)) {
#if 1
				//FIXME: descriptors

				struct tm tms = *localtime( &start );
				struct tm tme = *localtime( &end  );
				__log_printf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, iter_eit->second.name.c_str()/*, iter_eit->second.text.c_str()*/ );
#endif
				get_epg_event(&iter_sdt->second, &iter_eit->second, e);
				return true;
			}
		}
		eit_num++;
	}
	return false;
}

bool decode::get_epg_event(uint16_t service_id, time_t showtime, decoded_event_t *e)
{
#if 1//DBG
	__log_printf(stderr, "%s\n", __func__);
#endif
	map_decoded_vct_channels::const_iterator iter_vct = decoded_vct.channels.find(service_id);
	if (iter_vct != decoded_vct.channels.end()) {
		get_epg_event_atsc(iter_vct->second.source_id, showtime, e);
		return true;
	} else {
		const decoded_sdt_t *decoded_sdt = get_decoded_sdt();
		if (decoded_sdt) {
			map_decoded_sdt_services::const_iterator iter_sdt = decoded_sdt->services.find(service_id);
			if ((iter_sdt != decoded_sdt->services.end()) && (iter_sdt->second.f_eit_present)) {
				get_epg_event_dvb(iter_sdt->second.service_id, showtime, e);
				return true;
			}
		}
	}

	return false;
}

void decode::dump_eit_x_atsc(decode_report *reporter, uint8_t eit_x, uint16_t source_id)
{
#if 1//DBG
	__log_printf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct) {

		if ((source_id) && (source_id != iter_vct->second.source_id))
			continue;
#if 0
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		service_name[7] = 0;

		__log_printf(stdout, "%s-%d: id:%d - %d.%d: %s\n", __func__,
			eit_x, iter_vct->second.source_id,
			iter_vct->second.chan_major,
			iter_vct->second.chan_minor,
			service_name);
#endif
		map_decoded_atsc_eit_events::const_iterator iter_eit;
		for (iter_eit = decoded_atsc_eit[eit_x][iter_vct->second.source_id].events.begin();
		     iter_eit != decoded_atsc_eit[eit_x][iter_vct->second.source_id].events.end();
		     ++iter_eit) {
#if 0
			time_t start = atsc_datetime_utc(iter_eit->second.start_time /*+ (60 * tz_offset)*/);
			time_t end   = atsc_datetime_utc(iter_eit->second.start_time + iter_eit->second.length_sec /*+ (60 * tz_offset)*/);

			unsigned char name[256];
			memset(name, 0, sizeof(char) * 256);
			decode_multiple_string(iter_eit->second.title, iter_eit->second.title_bytes, name, sizeof(name));

			//FIXME: descriptors

			struct tm tms = *localtime( &start );
			struct tm tme = *localtime( &end  );
			__log_printf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
			dump_epg_event(eit_x, &iter_vct->second, &iter_eit->second, reporter);
		}
	}
	return;
}

void decode::dump_eit_x_dvb(decode_report *reporter, uint8_t eit_x, uint16_t service_id)
{
#if 1//DBG
	__log_printf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	map_decoded_sdt_services::const_iterator iter_sdt;
	const decoded_sdt_t *decoded_sdt = get_decoded_sdt();
	if (decoded_sdt) for (iter_sdt = decoded_sdt->services.begin(); iter_sdt != decoded_sdt->services.end(); ++iter_sdt) {
		if ((!iter_sdt->second.f_eit_present) ||
		    ((service_id) && (service_id != iter_sdt->second.service_id)))
			continue;

#if 0
		__log_printf(stdout, "%s-%d: id:%d - %d: %s\n", __func__,
			eit_x, iter_sdt->second.service_id,
			get_lcn(iter_sdt->second.service_id),
			iter_sdt->second.service_name);
#endif

		map_decoded_eit_events::const_iterator iter_eit;
		const map_decoded_eit *decoded_eit = get_decoded_eit();
		// FIXME:  CHANGE TO CONST_ITERATOR -- THIS IS DANGEROUS!!
		if (decoded_eit)
		for (iter_eit = ((map_decoded_eit*)decoded_eit)[eit_x][iter_sdt->second.service_id].events.begin();
		     iter_eit != ((map_decoded_eit*)decoded_eit)[eit_x][iter_sdt->second.service_id].events.end();
		     ++iter_eit) {
#if 0
			time_t start = datetime_utc(iter_eit->second.start_time /*+ (60 * tz_offset)*/);
			time_t end   = datetime_utc(iter_eit->second.start_time + iter_eit->second.length_sec /*+ (60 * tz_offset)*/);

			//FIXME: descriptors

			struct tm tms = *localtime( &start );
			struct tm tme = *localtime( &end  );
			__log_printf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, iter_eit->second.name.c_str()/*, iter_eit->second.text.c_str()*/ );
#endif
			dump_epg_event(&iter_sdt->second, &iter_eit->second, reporter);
		}
	}
	return;
}

void decode::dump_eit_x(decode_report *reporter, uint8_t eit_x, uint16_t source_id)
{
#if DBG
	__log_printf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	if (decoded_vct.channels.size()) {
		dump_eit_x_atsc(reporter, eit_x, source_id);
	} else {
		dump_eit_x_dvb(reporter, eit_x, source_id); /* service_id */
	}

#if 0
	__log_printf(stdout, "\n");
	fflush(stdout);
#endif
}

void decode::dump_epg_atsc(decode_report *reporter, uint16_t source_id)
{
	unsigned int eit_num = 0;

	while ((eit_num < 128) && (decoded_atsc_eit[eit_num].count(source_id))) {
		dump_eit_x_atsc(reporter, eit_num, source_id);
		eit_num++;
	}
	return;
}

void decode::dump_epg_dvb(decode_report *reporter, uint16_t service_id)
{
	unsigned int eit_num = 0;

	const map_decoded_eit *decoded_eit = get_decoded_eit();

	if (decoded_eit) while ((eit_num < NUM_EIT) && (decoded_eit[eit_num].count(service_id))) {
		dump_eit_x_dvb(reporter, eit_num, service_id);
		eit_num++;
	}
	return;
}

void decode::dump_epg(decode_report *reporter)
{
	if (reporter) reporter->epg_header_footer(true, false);

	if (decoded_vct.channels.size()) {
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct) {
		if (reporter) reporter->epg_header_footer(true, true);
		/*epg_str = */dump_epg_atsc(reporter, iter_vct->second.source_id);
		if (reporter) reporter->epg_header_footer(false, true);
	}} else {
	map_decoded_sdt_services::const_iterator iter_sdt;
	const decoded_sdt_t *decoded_sdt = get_decoded_sdt();
	if (decoded_sdt) for (iter_sdt = decoded_sdt->services.begin(); iter_sdt != decoded_sdt->services.end(); ++iter_sdt)
	if (iter_sdt->second.f_eit_present) {
		if (reporter) reporter->epg_header_footer(true, true);
		/*epg_str = */dump_epg_dvb(reporter, iter_sdt->second.service_id);
		if (reporter) reporter->epg_header_footer(false, true);
	}}

	if (reporter) reporter->epg_header_footer(false, false);

	return;
}

unsigned char * decode::get_decoded_ett(uint8_t current_eit_x, uint16_t etm_id, unsigned char *message, size_t sizeof_message)
{
	memset(message, 0, sizeof_message);

	if (decoded_ett[current_eit_x].count(etm_id)) {

		decoded_atsc_ett_t &cur_ett = decoded_ett[current_eit_x][etm_id];

		decode_multiple_string(cur_ett.etm,
				       cur_ett.etm_length,
				       message, sizeof_message);
	}
	return message;
}

#if !USING_DVBPSI_VERSION_0
bool decode::take_ett(const dvbpsi_atsc_ett_t * const p_ett)
{
	decoded_atsc_ett_t &cur_ett = decoded_ett[ett_x][p_ett->i_etm_id];

	if ((cur_ett.version == p_ett->i_version) &&
	    (cur_ett.etm_id  == p_ett->i_etm_id)) {
#if DBG
		__log_printf(stderr, "%s-%d: v%d, ID %d: ALREADY DECODED\n", __func__,
			ett_x, p_ett->i_version, p_ett->i_etm_id);
#endif
		return false;
	}

#if OLD_DECODER
	cur_ett.version    = p_ett->i_version;
	cur_ett.etm_id     = p_ett->i_etm_id;
	cur_ett.etm_length = p_ett->i_etm_length;
	memcpy(cur_ett.etm,
#if USING_DVBPSI_VERSION_0
	       p_ett->p_etm,
#else
	       p_ett->p_etm_data,
#endif
	       (sizeof(cur_ett.etm) >= p_ett->i_etm_length) ?
		       p_ett->i_etm_length : sizeof(cur_ett.etm));

	unsigned char message[512];
	memset(message, 0, sizeof(message));

	decode_multiple_string(cur_ett.etm, cur_ett.etm_length, message, sizeof(message));

	__log_printf(stderr, "%s: v%d, ID: %d: %s\n", __func__,
		p_ett->i_version, p_ett->i_etm_id, message);

	descriptors.decode(p_ett->p_first_descriptor);

	return true;
#else
	return store.ingest(p_ett, this);
#endif
}
#endif

/* -- -- -- */
bool decode::complete_pmt() const
{
#if 0
	return ((decoded_pat.programs.size()) && (decoded_pat.programs.size() == decoded_pmt.size()));
#else
	if (rcvd_pmt.size() == 0)
		return false;
	for (map_rcvd::const_iterator iter = rcvd_pmt.begin(); iter != rcvd_pmt.end(); ++iter)
		if ((iter->first) && (!iter->second)) {
#if 1//DBG
			fprintf(stderr, "%s: missing pmt for program %d\n", __func__, iter->first);
#endif
			return false;
		}
	return true;
#endif
}

bool decode::eit_x_complete_atsc(uint8_t current_eit_x)
{
#if 1
	return ((decoded_vct.channels.size()) &&
		((decoded_atsc_eit[current_eit_x].size()) &&
		 (decoded_atsc_eit[current_eit_x].size() == decoded_vct.channels.size())));
#else
	bool ret = ((decoded_vct.channels.size()) &&
		((decoded_atsc_eit[current_eit_x].size()) &&
		 (decoded_atsc_eit[current_eit_x].size() == decoded_vct.channels.size())));
	fprintf(stderr, "%s(%d):%s- decoded_vct.channels.size() = %d, decoded_atsc_eit[current_eit_x].size() = %d\n",
		__func__, current_eit_x, (ret) ? "true" : "false", decoded_vct.channels.size(), decoded_atsc_eit[current_eit_x].size());
	return ret;
#endif
}

bool decode::eit_x_complete_dvb_pf()
{
	return networks.count(orig_network_id) ? networks[orig_network_id]->eit_x_complete_dvb_pf(decoded_pat.ts_id) : false;
}

bool decode::eit_x_complete_dvb_sched(uint8_t current_eit_x)
{
	return networks.count(orig_network_id) ? networks[orig_network_id]->eit_x_complete_dvb_sched(decoded_pat.ts_id, current_eit_x) : false;
}

bool decode_network_service::eit_x_complete_dvb_pf()
{
	uint8_t current_eit_x = 0;

	return ((decoded_sdt.services.size()) &&
		(((decoded_eit[current_eit_x].size())
#if 0
		  || (!services_w_eit_pf)
#endif
		  ) &&
		 (decoded_eit[current_eit_x].size() == services_w_eit_pf)));
}

bool decode_network_service::eit_x_complete_dvb_sched(uint8_t current_eit_x)
{
	return ((decoded_sdt.services.size()) &&
		(((decoded_eit[current_eit_x].size())
#if 0
		  || (!services_w_eit_sched)
#endif
		  ) &&
		 (decoded_eit[current_eit_x].size() == services_w_eit_sched)));
}


bool decode::eit_x_complete(uint8_t current_eit_x)
{
#if 1
	return (eit_x_complete_atsc(current_eit_x) ||
		((current_eit_x == 0) ? eit_x_complete_dvb_pf() : eit_x_complete_dvb_sched(current_eit_x)));
#else
	bool ret = (eit_x_complete_atsc(current_eit_x) ||
		    ((current_eit_x == 0) ? eit_x_complete_dvb_pf() : eit_x_complete_dvb_sched(current_eit_x)));
	fprintf(stderr, "%s(%d):%s- eit_x_complete_atsc(current_eit_x)= %s\n",
		__func__, current_eit_x, (ret) ? "true" : "false", (eit_x_complete_atsc(current_eit_x)) ? "true" : "false");
	return ret;
#endif
}

bool decode::ett_x_complete(uint8_t current_ett_x)
{
	if (!eit_x_complete_atsc(current_ett_x))
		return false;

	int etm_missing = 0;

	for (map_decoded_atsc_eit::const_iterator iter =
		decoded_atsc_eit[current_ett_x].begin();
	     iter != decoded_atsc_eit[current_ett_x].end(); ++iter) {
		for (map_decoded_atsc_eit_events::const_iterator event_iter =
			iter->second.events.begin();
		     event_iter != iter->second.events.end(); ++event_iter) {

		     if (event_iter->second.etm_location == 1) {

				uint32_t event_id = event_iter->second.event_id;
				uint32_t etm_id = (iter->second.source_id << 16) | (event_id << 2) | 0x02;
				if (!decoded_ett[current_ett_x].count(etm_id)) {
				        etm_missing++;
				}
			}
		}
	}

	return !etm_missing;
}

bool decode::got_all_eit(int limit)
{
	if (decoded_mgt.tables.size() == 0) {
		const decoded_nit_t* decoded_nit = get_decoded_nit();
		const decoded_sdt_t* decoded_sdt = get_decoded_sdt();
		if (((decoded_nit) && (decoded_sdt)) && ((decoded_sdt->services.size()) && (decoded_nit->ts_list.size()))) {
			return ((eit_x_complete_dvb_pf()) && (eit_x_complete_dvb_sched(1)));
		} else // FIXME
			return false;
	}
	for (map_decoded_mgt_tables::const_iterator iter = decoded_mgt.tables.begin(); iter != decoded_mgt.tables.end(); ++iter) {
		switch (iter->first) {
		case 0x0100 ... 0x017f: /* EIT-0 to EIT-127 */
			if (((limit == -1) || (limit >= iter->first - 0x0100)) && (!eit_x_complete(iter->first - 0x0100))) {
#if DBG
				fprintf(stderr, "%s: eit #%d MISSING\n", __func__,
					iter->first - 0x0100);
#endif
				return false;
			}
		}
	}
	return true;
}

bool decode::got_all_ett(int limit)
{
	// returning true for now if DVBT
	if (decoded_mgt.tables.size() == 0) {
		return true;
	}
	for (map_decoded_mgt_tables::const_iterator iter = decoded_mgt.tables.begin(); iter != decoded_mgt.tables.end(); ++iter) {
		switch (iter->first) {
#if 0
		case            0x0004: /* FIXME: Channel ETT */
#endif
		case 0x0200 ... 0x027f: /* ETT-0 to ETT-127 */
			if (((limit == -1) || (limit >= iter->first - 0x0200)) && (!ett_x_complete(iter->first - 0x0200))) {
#if DBG
				fprintf(stderr, "%s: ett #%d MISSING\n", __func__,
					iter->first - 0x0200);
#endif
				return false;
			}
		}
	}
	return true;
}

const decode_network* decode::get_decoded_network() const
{
	map_network_decoder::const_iterator it = networks.find(orig_network_id);
	return (it == networks.end()) ? NULL : it->second;
	//return networks.count(orig_network_id) ? &networks[orig_network_id] : NULL;
}

uint16_t decode::get_lcn(uint16_t service_id) const
{
	uint16_t lcn = 0;

	// XXX: FIXME: must refactor decode::get_lcn() & LCN descriptor 0x83
	map_network_decoder::const_iterator it = networks.find(network_id);
	if (it != networks.end()) {
#if OLD_DECODER
		lcn = it->second->descriptors.lcn[service_id];
#else
		const dvbtee::decode::Descriptor *d = it->second->descriptors.last(0x83);
		if (d) lcn = d->get<uint16_t>(service_id);
#endif
	}
	return lcn;
}

const map_decoded_eit* decode::get_decoded_eit() const
{
	map_network_decoder::const_iterator it = networks.find(orig_network_id);
	return (it == networks.end()) ? NULL : it->second->get_decoded_eit(decoded_pat.ts_id);
	//return networks.count(orig_network_id) ? networks[orig_network_id].get_decoded_eit(decoded_pat.ts_id) : NULL;
}

const decoded_sdt_t* decode::get_decoded_sdt() const
{
	map_network_decoder::const_iterator it = networks.find(orig_network_id);
	return (it == networks.end()) ? NULL : it->second->get_decoded_sdt(decoded_pat.ts_id);
	//return networks.count(orig_network_id) ? networks[orig_network_id].get_decoded_sdt(decoded_pat.ts_id) : NULL;
}

const decoded_nit_t* decode::get_decoded_nit() const
{
	map_network_decoder::const_iterator it = networks.find(network_id);
	return (it == networks.end()) ? NULL : it->second->get_decoded_nit();
	//return networks.count(network_id) ? networks[network_id].get_decoded_nit() : NULL;
}

void decode_network::dumpJsonServices()
{
	for (map_decoded_network_services::const_iterator it = decoded_network_services.begin(); it != decoded_network_services.end(); ++it) {
		__log_printf(stderr, "\nNET_SVC_ID#%04x: ", it->first);
#if !OLD_DECODER
		it->second->showChildren();
#endif
	}
	__log_printf(stderr, "\n");
}

void decode_network::dumpJson(map_network_decoder &networks)
{
	for (map_network_decoder::const_iterator it = networks.begin(); it != networks.end(); ++it) {
		__log_printf(stderr, "\nNET_ID#%04x: ", it->first);
#if !OLD_DECODER
		it->second->showChildren();
#endif
		it->second->dumpJsonServices();
	}
	__log_printf(stderr, "\n");
}

#if 0
bool decode::complete_psip()
{
//	bool complete = true;
	for (map_decoded_mgt_tables::const_iterator iter_mgt = decoded_mgt.tables.begin(); iter_mgt != decoded_mgt.tables.end(); ++iter_mgt) {
#if 0
		bool b_attach_demux = false;

		switch (iter->first) {
		case 0x0000 ... 0x0003: /* TVCT / CVCT */
			if decoded_vct.channels.empty()
				return false;
		case 0x0100 ... 0x017f: /* EIT-0 to EIT-127 */
			if (((!decoded_atsc_eit.count(iter->first - 0x0100)) || (!decoded_atsc_eit[iter->first - 0x0100)].size())) || (decoded_atsc_eit[iter->first - 0x0100].size() != decoded_vct.channels.size()))
				return false;
#if 0
		case 0x0200 ... 0x027f: /* ETT-0 to ETT-127 */
#endif
		case 0x0004:            /* Channel ETT */
		case 0x0301 ... 0x03ff: /* RRT w/ rating region 1-255 */
		case 0x0005:            /* DCCSCT */
		case 0x0006 ... 0x00ff: /* Reserved for future ATSC use */
		case 0x0180 ... 0x01ff: /* Reserved for future ATSC use */
		case 0x0280 ... 0x0300: /* Reserved for future ATSC use */
		case 0x0400 ... 0x0fff: /* private */
		case 0x1000 ... 0x13ff: /* Reserved for future ATSC use */
		case 0x1400 ... 0x14ff: /* DCCT dcc id 0x00 - 0xff */
		case 0x1500 ... 0xffff: /* Reserved for future ATSC use */
			break;
		default:
			if (scan_mode)
				break;
			b_attach_demux  = true;
			break;
		}
		if ((b_attach_demux) && (NULL == h_demux[iter->second.pid]))
			h_demux[iter->second.pid] = const dvbpsi_AttachDemux(attach_table, this);
			/* else already attached */
#endif
	}
	return false;
}
#endif
