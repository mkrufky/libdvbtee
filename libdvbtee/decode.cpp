/*****************************************************************************
 * Copyright (C) 2011-2014 Michael Ira Krufky
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
#include "log.h"
#define CLASS_MODULE "decode"

bool fshowtime;

#define dprintf(fmt, arg...)					\
do {								\
	__dprintf(DBG_DECODE, fmt, ##arg);			\
	fshowtime = true;					\
} while (0)

#define dbg_time(fmt, arg...)					\
do {								\
	if ((fshowtime) | (dbg & DBG_TIME)) {			\
		__dprintf((DBG_DECODE | DBG_TIME), fmt, ##arg);	\
		fshowtime = false;				\
	}							\
} while (0)

decode_report::decode_report()
{
#if DBG
	dprintf("()");
#endif
}

decode_report::~decode_report()
{
#if DBG
	dprintf("()");
#endif
}

static map_network_decoder   networks;

void clear_decoded_networks()
{
	networks.clear();
}

decode_network_service::decode_network_service()
  : services_w_eit_pf(0)
  , services_w_eit_sched(0)
{
	dprintf("()");

	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));
	decoded_sdt.services.clear();

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
	dprintf("(%05d|%05d)",
		decoded_sdt.network_id, decoded_sdt.ts_id);

	decoded_sdt.services.clear();

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}
}

decode_network_service::decode_network_service(const decode_network_service&)
{
	dprintf("(copy)");

	services_w_eit_pf = 0;
	services_w_eit_sched = 0;

	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));
	decoded_sdt.services.clear();

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}
}

decode_network_service& decode_network_service::operator= (const decode_network_service& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	services_w_eit_pf = 0;
	services_w_eit_sched = 0;

	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));
	decoded_sdt.services.clear();

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}

	return *this;
}

decode_network::decode_network()
  : orig_network_id(0)
{
	dprintf("()");

	memset(&decoded_nit, 0, sizeof(decoded_nit_t));

	decoded_nit.ts_list.clear();

	for (map_decoded_network_services::iterator iter = decoded_network_services.begin(); iter != decoded_network_services.end(); ++iter)
		iter->second.decoded_sdt.services.clear();
	decoded_network_services.clear();
}

decode_network::~decode_network()
{
#if 0
	dprintf("(%05d|%05d)",
		decoded_nit.network_id, decoded_sdt.network_id);
#else
	dprintf("(%05d|%05d) %zu",
		decoded_nit.network_id, orig_network_id,
		decoded_network_services.size());
#endif

	decoded_nit.ts_list.clear();

	for (map_decoded_network_services::iterator iter = decoded_network_services.begin(); iter != decoded_network_services.end(); ++iter)
		iter->second.decoded_sdt.services.clear();
	decoded_network_services.clear();
}

decode_network::decode_network(const decode_network&)
{
	dprintf("(copy)");

	orig_network_id = 0;

	memset(&decoded_nit, 0, sizeof(decoded_nit_t));

	decoded_nit.ts_list.clear();

	for (map_decoded_network_services::iterator iter = decoded_network_services.begin(); iter != decoded_network_services.end(); ++iter)
		iter->second.decoded_sdt.services.clear();
	decoded_network_services.clear();
}

decode_network& decode_network::operator= (const decode_network& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	orig_network_id = 0;

	memset(&decoded_nit, 0, sizeof(decoded_nit_t));

	decoded_nit.ts_list.clear();

	for (map_decoded_network_services::iterator iter = decoded_network_services.begin(); iter != decoded_network_services.end(); ++iter)
		iter->second.decoded_sdt.services.clear();
	decoded_network_services.clear();

	return *this;
}

decode::decode()
  : orig_network_id(0)
  , network_id(0)
  , stream_time((time_t)0)
  , eit_x(0)
  , physical_channel(0)
{
	dprintf("()");

	memset(&decoded_pat, 0, sizeof(decoded_pat_t));
	memset(&decoded_vct, 0, sizeof(decoded_vct_t));
	memset(&decoded_mgt, 0, sizeof(decoded_mgt_t));
	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();

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

	memset(&decoded_pmt, 0, sizeof(map_decoded_pmt));
	memset(&rcvd_pmt, 0, sizeof(map_rcvd));
	memset(&decoded_ett, 0, sizeof(map_decoded_atsc_ett));

	decoded_pmt.clear();
	rcvd_pmt.clear();
	decoded_ett.clear();
}

decode::~decode()
{
	dprintf("(%04x|%05d)",
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
	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
	//decoded_atsc_eit.events.clear();
	//decoded_eit.events.clear();
	decoded_ett.clear();
}

decode::decode(const decode&)
{
	dprintf("(copy)");

	memset(&decoded_pat, 0, sizeof(decoded_pat_t));
	memset(&decoded_vct, 0, sizeof(decoded_vct_t));
	memset(&decoded_mgt, 0, sizeof(decoded_mgt_t));
	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
	//decoded_atsc_eit.events.clear();

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

	memset(&decoded_pmt, 0, sizeof(map_decoded_pmt));
	memset(&rcvd_pmt, 0, sizeof(map_rcvd));
	memset(&decoded_ett, 0, sizeof(map_decoded_atsc_ett));

	decoded_pmt.clear();
	rcvd_pmt.clear();
	decoded_ett.clear();

	orig_network_id = 0;
	network_id = 0;
	stream_time = (time_t)0;
	eit_x = 0;
	physical_channel = 0;
}

decode& decode::operator= (const decode& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	memset(&decoded_pat, 0, sizeof(decoded_pat_t));
	memset(&decoded_vct, 0, sizeof(decoded_vct_t));
	memset(&decoded_mgt, 0, sizeof(decoded_mgt_t));
	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
	//decoded_atsc_eit.events.clear();

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

	memset(&decoded_pmt, 0, sizeof(map_decoded_pmt));
	memset(&rcvd_pmt, 0, sizeof(map_rcvd));
	memset(&decoded_ett, 0, sizeof(map_decoded_atsc_ett));

	decoded_pmt.clear();
	rcvd_pmt.clear();
	decoded_ett.clear();

	orig_network_id = 0;
	network_id = 0;
	stream_time = (time_t)0;
	eit_x = 0;
	physical_channel = 0;

	return *this;
}

/* -- STREAM TIME -- */
#if !USING_DVBPSI_VERSION_0
bool decode::take_stt(dvbpsi_atsc_stt_t* p_stt)
{
	stream_time = atsc_datetime_utc(p_stt->i_system_time);

	dbg_time("%s", ctime(&stream_time));

	descriptors.decode(p_stt->p_first_descriptor);

	return true;
}
#endif

bool decode::take_tot(dvbpsi_tot_t* p_tot)
{
	stream_time = datetime_utc(p_tot->i_utc_time);

	dbg_time("%s", ctime(&stream_time));

	descriptors.decode(p_tot->p_first_descriptor);

	return true;
}

/* -- TABLE HANDLERS -- */
bool decode::take_pat(dvbpsi_pat_t* p_pat)
#define PAT_DBG 1
{
	if ((decoded_pat.version == p_pat->i_version) &&
	    (decoded_pat.ts_id   == p_pat->i_ts_id)) {

		dprintf("v%d, ts_id: %d: ALREADY DECODED",
			p_pat->i_version, p_pat->i_ts_id);
		return false;
	}
#if PAT_DBG
	fprintf(stderr, "%s: v%d, ts_id: %d\n", __func__,
		p_pat->i_version, p_pat->i_ts_id);
#endif
	decoded_pat.ts_id   = p_pat->i_ts_id;
	decoded_pat.version = p_pat->i_version;
	decoded_pat.programs.clear();

	dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
	while (p_program) {
//		if (p_program->i_number > 0)
		decoded_pat.programs[p_program->i_number] = p_program->i_pid;

		rcvd_pmt[p_program->i_number] = false;
#if PAT_DBG
		fprintf(stderr, "  %10d | %x\n",
			p_program->i_number,
			decoded_pat.programs[p_program->i_number]);
#endif
		p_program = p_program->p_next;
	}
	return true;
}

bool decode::take_pmt(dvbpsi_pmt_t* p_pmt)
#define PMT_DBG 1
{
	decoded_pmt_t &cur_decoded_pmt = decoded_pmt[p_pmt->i_program_number];

	if ((cur_decoded_pmt.version == p_pmt->i_version) &&
	    (cur_decoded_pmt.program == p_pmt->i_program_number)) {

		dprintf("v%d, service_id %d, pcr_pid %d: ALREADY DECODED",
			p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
		return false;
	}
#if PMT_DBG
	fprintf(stderr, "%s: v%d, service_id %d, pcr_pid %d\n", __func__,
		p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
#endif
	cur_decoded_pmt.program = p_pmt->i_program_number;
	cur_decoded_pmt.version = p_pmt->i_version;
	cur_decoded_pmt.pcr_pid = p_pmt->i_pcr_pid;
	cur_decoded_pmt.es_streams.clear();
	//FIXME: descriptors
	descriptors.decode(p_pmt->p_first_descriptor);

	fprintf(stderr, "  es_pid | type\n");

	dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;
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
		fprintf(stderr, "  %6x | 0x%02x (%s) | %s\n",
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
}

#if !USING_DVBPSI_VERSION_0
bool decode::take_vct(dvbpsi_atsc_vct_t* p_vct)
#define VCT_DBG 1
{
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = p_vct->i_ts_id;
#else
	uint16_t __ts_id = p_vct->i_extension;
#endif
	if ((decoded_vct.version == p_vct->i_version) &&
	    (decoded_vct.ts_id   == __ts_id)) {

		dprintf("v%d, ts_id %d, b_cable_vct %d: ALREADY DECODED",
			p_vct->i_version, __ts_id, p_vct->b_cable_vct);
		return false;
	}
#if VCT_DBG
	fprintf(stderr, "%s: v%d, ts_id %d, b_cable_vct %d\n", __func__,
		p_vct->i_version, __ts_id, p_vct->b_cable_vct);
#endif
	decoded_vct.version   = p_vct->i_version;
	decoded_vct.ts_id     = __ts_id;
	decoded_vct.cable_vct = p_vct->b_cable_vct;
	decoded_vct.channels.clear();

	dvbpsi_atsc_vct_channel_t* p_channel = p_vct->p_first_channel;
#if VCT_DBG
	if (p_channel)
		fprintf(stderr, "  channel | service_id | source_id | service_name\n");
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
		dprintf("parsing channel descriptors for service: %d", p_channel->i_program_number);
		descriptors.decode(p_channel->p_first_descriptor);

		desc local_descriptors;
		local_descriptors.decode(p_channel->p_first_descriptor);

		std::string languages;

		for (map_dra1::const_iterator iter_dra1 = local_descriptors._a1.begin(); iter_dra1 != local_descriptors._a1.end(); ++iter_dra1) {
#ifdef COPY_DRA1_FROM_VCT_TO_PMT // disabled, to be deleted
			//stuff descriptor 0xa1 lang codes into PMT table if PMT has been decoded
			if (decoded_pmt.count(p_channel->i_program_number)) {
				memcpy(decoded_pmt[p_channel->i_program_number].es_streams[iter_dra1->second.elementary_pid].iso_639_code,
				       iter_dra1->second.iso_639_code, sizeof(iter_dra1->second.iso_639_code));
				dprintf("copied service location descriptor from VCT into PMT");
			}
#endif
			if (!languages.empty()) languages.append(", ");
			if (iter_dra1->second.iso_639_code[0])
				for (int i=0; i<3; i++) languages.push_back(iter_dra1->second.iso_639_code[i]);
		}
#if VCT_DBG
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = cur_channel.short_name[i*2+1];
		service_name[7] = 0;

		fprintf(stderr, "  %5d.%d | %10d | %9d | %s | %s\n",
			cur_channel.chan_major,
			cur_channel.chan_minor,
			cur_channel.program,
			cur_channel.source_id,
			service_name, languages.c_str());
#endif
		p_channel = p_channel->p_next;
	}
	//FIXME: descriptors
	dprintf("parsing channel descriptors for mux:");
	descriptors.decode(p_vct->p_first_descriptor);

	return true;
}

bool decode::take_mgt(dvbpsi_atsc_mgt_t* p_mgt)
#define MGT_DBG 1
{
	if ((decoded_mgt.version == p_mgt->i_version) &&
	    (!decoded_mgt.tables.empty())) {

		dprintf("v%d: ALREADY DECODED", p_mgt->i_version);
		return false;
	}
#if MGT_DBG
	fprintf(stderr, "%s: v%d\n", __func__, p_mgt->i_version);
#endif
	decoded_mgt.version = p_mgt->i_version;
	decoded_mgt.tables.clear();

	dvbpsi_atsc_mgt_table_t* p_table = p_mgt->p_first_table;
#if MGT_DBG
	if (p_table)
		fprintf(stderr, "  table type |   pid  | ver | bytes\n");
#endif
	while (p_table) {
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
}
#endif

static bool __take_nit(dvbpsi_nit_t* p_nit, decoded_nit_t* decoded_nit, desc* descriptors)
#define NIT_DBG 1
{
	if ((decoded_nit->version    == p_nit->i_version) &&
	    (decoded_nit->network_id == p_nit->i_network_id)) {

		dprintf("v%d, network_id %d: ALREADY DECODED",
			p_nit->i_version, p_nit->i_network_id);
		return false;
	}
#if NIT_DBG
	fprintf(stderr, "%s: v%d, network_id %d\n", __func__,
		p_nit->i_version, p_nit->i_network_id);
#endif
	decoded_nit->version    = p_nit->i_version;
	decoded_nit->network_id = p_nit->i_network_id;

	dvbpsi_nit_ts_t* p_ts = p_nit->p_first_ts;
#if NIT_DBG
	if (p_ts)
		fprintf(stderr, "   ts_id | orig_network_id\n");
#endif
	while (p_ts) {

		decoded_nit_ts_t &cur_ts_list = decoded_nit->ts_list[p_ts->i_ts_id];

		cur_ts_list.ts_id           = p_ts->i_ts_id;
		cur_ts_list.orig_network_id = p_ts->i_orig_network_id;

#if NIT_DBG
		fprintf(stderr, "   %05d | %d\n",
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

bool decode::take_nit_actual(dvbpsi_nit_t* p_nit)
{
	network_id = p_nit->i_network_id;
#if 0
	return networks[network_id].take_nit(p_nit);
#else
	bool ret = networks[network_id].take_nit(p_nit);
	const decoded_nit_t *decoded_nit = get_decoded_nit();
	if ((decoded_nit) && (decoded_nit->ts_list.count(decoded_pat.ts_id))) {
		orig_network_id = ((decoded_nit_t*)decoded_nit)->ts_list[decoded_pat.ts_id].orig_network_id;

		networks[network_id].orig_network_id = orig_network_id;
#if 0
		return networks[orig_network_id].take_nit(p_nit);
#endif
	}
	return ret;
#endif
}

bool decode::take_nit_other(dvbpsi_nit_t* p_nit)
{
#if 0
	return networks[p_nit->i_network_id].take_nit(p_nit);
#else
	bool ret = networks[p_nit->i_network_id].take_nit(p_nit);
	const decoded_nit_t *decoded_nit = get_decoded_nit();
	if ((decoded_nit) && (decoded_nit->ts_list.count(decoded_pat.ts_id))) {
		uint16_t other_orig_network_id = ((decoded_nit_t*)decoded_nit)->ts_list[decoded_pat.ts_id].orig_network_id;

		networks[p_nit->i_network_id].orig_network_id = other_orig_network_id;
#if 0
		return networks[other_orig_network_id].take_nit(p_nit);
#endif
	}
	return ret;
#endif
}

bool decode_network::take_nit(dvbpsi_nit_t* p_nit)
{
	return __take_nit(p_nit, &decoded_nit, &descriptors);
}

static bool __take_sdt(dvbpsi_sdt_t* p_sdt, decoded_sdt_t* decoded_sdt, desc* descriptors,
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

		dprintf("v%d | ts_id %d | network_id %d: ALREADY DECODED",
			p_sdt->i_version,
			__ts_id,
			p_sdt->i_network_id);
		return false;
	}
	dprintf("v%02d | ts_id %05d | network_id %05d\n"
		/*"------------------------------------"*/,
		p_sdt->i_version,
		__ts_id,
		p_sdt->i_network_id);

	decoded_sdt->ts_id      = __ts_id;
	decoded_sdt->version    = p_sdt->i_version;
	decoded_sdt->network_id = p_sdt->i_network_id;

	unsigned int _services_w_eit_pf    = 0;
	unsigned int _services_w_eit_sched = 0;
	//fprintf(stderr, "  service_id | service_name");
	dvbpsi_sdt_service_t* p_service = p_sdt->p_first_service;
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
		strncpy((char*)cur_service.provider_name,
			(const char*)descriptors->provider_name,
			sizeof(cur_service.provider_name)-1);
		cur_service.provider_name[sizeof(cur_service.provider_name)-1] = '\0';
		strncpy((char*)cur_service.service_name,
			(const char*)descriptors->service_name,
			sizeof(cur_service.service_name)-1);
		cur_service.service_name[sizeof(cur_service.service_name)-1] = '\0';

		dprintf("%05d | %s %s | %s - %s",
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

bool decode::take_sdt_actual(dvbpsi_sdt_t* p_sdt)
{
	orig_network_id = p_sdt->i_network_id;

	decode_network &nw = networks[orig_network_id];

	nw.orig_network_id = orig_network_id;

	return nw.take_sdt(p_sdt);
}

bool decode::take_sdt_other(dvbpsi_sdt_t* p_sdt)
{
	decode_network &nw = networks[p_sdt->i_network_id];

	nw.orig_network_id = p_sdt->i_network_id;

	return nw.take_sdt(p_sdt);
}

bool decode_network_service::take_sdt(dvbpsi_sdt_t* p_sdt)
{
	return __take_sdt(p_sdt, &decoded_sdt, &descriptors,
			  &services_w_eit_pf, &services_w_eit_sched);
}

bool __take_eit(dvbpsi_eit_t* p_eit, map_decoded_eit *decoded_eit, desc* descriptors, uint8_t eit_x)
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
		fprintf(stderr, "%s-%d: v%d | ts_id %d | network_id %d service_id %d: ALREADY DECODED\n", __func__, eit_x,
			p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, __service_id);
#endif
		return false;
	}
#if DBG
	fprintf(stderr, "%s-%d: v%d | ts_id %d | network_id %d service_id %d | table id: 0x%02x, last_table id: 0x%02x\n", __func__, eit_x,
		p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, __service_id, p_eit->i_table_id, p_eit->i_last_table_id);
#endif
	cur_eit.service_id    = __service_id;
	cur_eit.version       = p_eit->i_version;
	cur_eit.ts_id         = p_eit->i_ts_id;
	cur_eit.network_id    = p_eit->i_network_id;
	cur_eit.last_table_id = p_eit->i_last_table_id;

	dvbpsi_eit_event_t* p_event = p_eit->p_first_event;
	while (p_event) {

		decoded_eit_event_t &cur_event = cur_eit.events[p_event->i_event_id];

		cur_event.event_id       = p_event->i_event_id;
		cur_event.start_time     = p_event->i_start_time;
		cur_event.length_sec     = p_event->i_duration;
		cur_event.running_status = p_event->i_running_status;
		cur_event.f_free_ca      = p_event->b_free_ca;

		descriptors->decode(p_event->p_first_descriptor);

		cur_event.name.assign((const char *)descriptors->_4d.name);
		cur_event.text.assign((const char *)descriptors->_4d.text);
#if DBG
		time_t start = datetime_utc(cur_event.start_time /*+ (60 * tz_offset)*/);
		time_t end   = datetime_utc(cur_event.start_time + cur_event.length_sec /*+ (60 * tz_offset)*/);

		struct tm tms = *localtime(&start);
		struct tm tme = *localtime(&end);

		fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, descriptors->_4d.name);
#endif
		p_event = p_event->p_next;
	}
	return true;
}

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

bool decode::take_eit(dvbpsi_eit_t* p_eit)
{
	/* we want our own eit_x here - we don't need to store this in our class, the stored eit_x is for ATSC */
	/* prevent warning: ‘eit_x’ may be used uninitialized in this function [-Wmaybe-uninitialized] */
	uint8_t eit_x = 0;
#if !USING_DVBPSI_VERSION_0 // this is totally a bug when this line is omitted.  instead of trying to fix this, just update your libdvbpsi
	table_id_to_eit_x(p_eit->i_table_id, &eit_x);
#endif

	return networks[p_eit->i_network_id].take_eit(p_eit, eit_x);
}

bool decode_network_service::take_eit(dvbpsi_eit_t* p_eit, uint8_t eit_x)
{
	return __take_eit(p_eit, decoded_eit, &descriptors, eit_x);
}

#if !USING_DVBPSI_VERSION_0
bool decode::take_eit(dvbpsi_atsc_eit_t* p_eit)
{
	decoded_atsc_eit_t &cur_atsc_eit = decoded_atsc_eit[eit_x][p_eit->i_source_id];

	if ((cur_atsc_eit.version   == p_eit->i_version) &&
	    (cur_atsc_eit.source_id == p_eit->i_source_id)) {
#if DBG
		fprintf(stderr, "%s-%d: v%d, source_id %d: ALREADY DECODED\n", __func__,
			eit_x, p_eit->i_version, p_eit->i_source_id);
#endif
		return false;
	}
#if DBG
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct)
		if (iter_vct->second.source_id == p_eit->i_source_id)
			break;

	if (iter_vct == decoded_vct.channels.end()) {
		fprintf(stderr, "%s-%d: v%d, id:%d\n", __func__,
			eit_x, p_eit->i_version, p_eit->i_source_id);
	} else {
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		service_name[7] = 0;

		fprintf(stderr, "%s-%d: v%d, id:%d - %d.%d: %s\n", __func__,
			eit_x, p_eit->i_version, p_eit->i_source_id,
			iter_vct->second.chan_major,
			iter_vct->second.chan_minor,
			service_name);
	}
#endif
	cur_atsc_eit.version   = p_eit->i_version;
	cur_atsc_eit.source_id = p_eit->i_source_id;

	dvbpsi_atsc_eit_event_t* p_event = p_eit->p_first_event;
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
		fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
		//FIXME: descriptors
		descriptors.decode(p_event->p_first_descriptor);

		p_event = p_event->p_next;
	}

	descriptors.decode(p_eit->p_first_descriptor);

	return true;
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


void decode::dump_epg_event(const decoded_vct_channel_t *channel, const decoded_atsc_eit_event_t *event, decode_report *reporter)
{
	unsigned char service_name[8] = { 0 };
	for ( int i = 0; i < 7; ++i ) service_name[i] = channel->short_name[i*2+1];
	service_name[7] = 0;

	fprintf(stderr, "%s: id:%d - %d.%d: %s\t", __func__,
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
	fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );

	if (reporter) {
		unsigned char message[512];
		reporter->epg_event((const char *)service_name,
					 channel->chan_major, channel->chan_minor,
					 physical_channel, channel->program,
					 event->event_id,
					 start,
					 (end - start),
					 (const char *)name,
					 (const char *)get_decoded_ett((channel->source_id << 16) | (event->event_id << 2) | 0x02, message, sizeof(message)));
	}
	return;
}

void decode::dump_epg_event(const decoded_sdt_service_t *service, const decoded_eit_event_t *event, decode_report *reporter)
{
	fprintf(stderr, "%s: id:%d - %d: %s\t", __func__,
		service->service_id,
		get_lcn(service->service_id),
		service->service_name);

	time_t start = datetime_utc(event->start_time /*+ (60 * tz_offset)*/);
	time_t end   = datetime_utc(event->start_time + event->length_sec /*+ (60 * tz_offset)*/);

	//FIXME: descriptors

	struct tm tms = *localtime( &start );
	struct tm tme = *localtime( &end  );
	fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, event->name.c_str()/*, iter_eit->second.text.c_str()*/ );

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

void decode::get_epg_event(const decoded_vct_channel_t *channel, const decoded_atsc_eit_event_t *event, decoded_event_t *e)
{
#if 1//DBG
	fprintf(stderr, "%s\n", __func__);
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
	fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
	unsigned char message[512];
	_get_epg_event(e, (const char *)service_name,
		      channel->chan_major, channel->chan_minor,
		      physical_channel, channel->program,
		      event->event_id,
		      start,
		      event->length_sec,
		      (const char *)name,
		      (const char *)get_decoded_ett((channel->source_id << 16) | (event->event_id << 2) | 0x02, message, sizeof(message)));
	return;
}

void decode::get_epg_event(const decoded_sdt_service_t *service, const decoded_eit_event_t *event, decoded_event_t *e)
{
#if 1//DBG
	fprintf(stderr, "%s\n", __func__);
#endif
	time_t start = datetime_utc(event->start_time /*+ (60 * tz_offset)*/);
	time_t end   = datetime_utc(event->start_time + event->length_sec /*+ (60 * tz_offset)*/);
#if 1
	//FIXME: descriptors

	struct tm tms = *localtime( &start );
	struct tm tme = *localtime( &end  );
	fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, event->name.c_str()/*, iter_eit->second.text.c_str()*/ );
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
	fprintf(stderr, "%s\n", __func__);
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

		fprintf(stdout, "%s-%d: id:%d - %d.%d: %s\n", __func__,
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
				fprintf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
				get_epg_event(&iter_vct->second, &iter_eit->second, e);
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
	fprintf(stderr, "%s\n", __func__);
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
		fprintf(stdout, "%s-%d: id:%d - %d: %s\n", __func__,
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
				fprintf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, iter_eit->second.name.c_str()/*, iter_eit->second.text.c_str()*/ );
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
	fprintf(stderr, "%s\n", __func__);
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
	fprintf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct) {

		if ((source_id) && (source_id != iter_vct->second.source_id))
			continue;
#if 0
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		service_name[7] = 0;

		fprintf(stdout, "%s-%d: id:%d - %d.%d: %s\n", __func__,
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
			fprintf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
			dump_epg_event(&iter_vct->second, &iter_eit->second, reporter);
		}
	}
	return;
}

void decode::dump_eit_x_dvb(decode_report *reporter, uint8_t eit_x, uint16_t service_id)
{
#if 1//DBG
	fprintf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	map_decoded_sdt_services::const_iterator iter_sdt;
	const decoded_sdt_t *decoded_sdt = get_decoded_sdt();
	if (decoded_sdt) for (iter_sdt = decoded_sdt->services.begin(); iter_sdt != decoded_sdt->services.end(); ++iter_sdt) {
		if ((!iter_sdt->second.f_eit_present) ||
		    ((service_id) && (service_id != iter_sdt->second.service_id)))
			continue;

#if 0
		fprintf(stdout, "%s-%d: id:%d - %d: %s\n", __func__,
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
			fprintf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, iter_eit->second.name.c_str()/*, iter_eit->second.text.c_str()*/ );
#endif
			dump_epg_event(&iter_sdt->second, &iter_eit->second, reporter);
		}
	}
	return;
}

void decode::dump_eit_x(decode_report *reporter, uint8_t eit_x, uint16_t source_id)
{
#if DBG
	fprintf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	if (decoded_vct.channels.size()) {
		dump_eit_x_atsc(reporter, eit_x, source_id);
	} else {
		dump_eit_x_dvb(reporter, eit_x, source_id); /* service_id */
	}

#if 0
	fprintf(stdout, "\n");
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

unsigned char * decode::get_decoded_ett(uint16_t etm_id, unsigned char *message, size_t sizeof_message)
{
	memset(message, 0, sizeof_message);

	if (decoded_ett.count(etm_id)) {

		decoded_atsc_ett_t &cur_ett = decoded_ett[etm_id];

		decode_multiple_string(cur_ett.etm,
				       cur_ett.etm_length,
				       message, sizeof_message);
	}
	return message;
}

#if !USING_DVBPSI_VERSION_0
bool decode::take_ett(dvbpsi_atsc_ett_t* p_ett)
{
	decoded_atsc_ett_t &cur_ett = decoded_ett[p_ett->i_etm_id];
#if 1
	if ((cur_ett.version == p_ett->i_version) &&
	    (cur_ett.etm_id  == p_ett->i_etm_id)) {
		fprintf(stderr, "%s: v%d, ID %d: ALREADY DECODED\n", __func__,
			p_ett->i_version, p_ett->i_etm_id);
		return false;
	}
#endif
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

	fprintf(stderr, "%s: v%d, ID: %d: %s\n", __func__,
		p_ett->i_version, p_ett->i_etm_id, message);

	descriptors.decode(p_ett->p_first_descriptor);

	return true;
}
#endif

/* -- -- -- */
bool decode::complete_pmt()
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
	return networks.count(orig_network_id) ? networks[orig_network_id].eit_x_complete_dvb_pf(decoded_pat.ts_id) : false;
}

bool decode::eit_x_complete_dvb_sched(uint8_t current_eit_x)
{
	return networks.count(orig_network_id) ? networks[orig_network_id].eit_x_complete_dvb_sched(decoded_pat.ts_id, current_eit_x) : false;
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

const decode_network* decode::get_decoded_network()
{
	return networks.count(orig_network_id) ? &networks[orig_network_id] : NULL;
}

uint16_t decode::get_lcn(uint16_t service_id)
{
	return networks.count(network_id) ? networks[network_id].descriptors.lcn[service_id]: 0;
}

const map_decoded_eit* decode::get_decoded_eit()
{
	return networks.count(orig_network_id) ? networks[orig_network_id].get_decoded_eit(decoded_pat.ts_id) : NULL;
}

const decoded_sdt_t* decode::get_decoded_sdt()
{
	return networks.count(orig_network_id) ? networks[orig_network_id].get_decoded_sdt(decoded_pat.ts_id) : NULL;
}

const decoded_nit_t* decode::get_decoded_nit()
{
	return networks.count(network_id) ? networks[network_id].get_decoded_nit() : NULL;
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
			h_demux[iter->second.pid] = dvbpsi_AttachDemux(attach_table, this);
			/* else already attached */
#endif
	}
	return false;
}
#endif
