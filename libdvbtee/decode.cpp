/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
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

#define DBG 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "decode.h"
#include "log.h"

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

static map_network_decoder   networks;

decode_network::decode_network()
//  : eit_x(0)
//  , services_w_eit_pf(0)
//  : services_w_eit_pf(0)
//  , services_w_eit_sched(0)
{
	dprintf("()");

	memset(&decoded_nit, 0, sizeof(decoded_nit_t));

	decoded_nit.ts_list.clear();

	for (map_decoded_network_services::iterator iter = decoded_network_services.begin(); iter != decoded_network_services.end(); ++iter)
		iter->second.decoded_sdt.services.clear();
	decoded_network_services.clear();

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}
}

decode_network::~decode_network()
{
#if 0
	dprintf("(%04x|%05d)/(%04x|%05d)",
		decoded_nit.network_id, decoded_nit.network_id,
		decoded_sdt.network_id, decoded_sdt.network_id);
#else
	dprintf("(%04x|%05d)",
		decoded_nit.network_id, decoded_nit.network_id);
#endif
	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}

	decoded_nit.ts_list.clear();

	for (map_decoded_network_services::iterator iter = decoded_network_services.begin(); iter != decoded_network_services.end(); ++iter)
		iter->second.decoded_sdt.services.clear();
	decoded_network_services.clear();
}

decode_network::decode_network(const decode_network&)
{
	dprintf("(copy)");

	memset(&decoded_nit, 0, sizeof(decoded_nit_t));

	decoded_nit.ts_list.clear();

	for (map_decoded_network_services::iterator iter = decoded_network_services.begin(); iter != decoded_network_services.end(); ++iter)
		iter->second.decoded_sdt.services.clear();
	decoded_network_services.clear();

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}
}

decode_network& decode_network::operator= (const decode_network& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	memset(&decoded_nit, 0, sizeof(decoded_nit_t));

	decoded_nit.ts_list.clear();

	for (map_decoded_network_services::iterator iter = decoded_network_services.begin(); iter != decoded_network_services.end(); ++iter)
		iter->second.decoded_sdt.services.clear();
	decoded_network_services.clear();

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
	}

	return *this;
}

decode::decode()
  : orig_network_id(0)
  , network_id(0)
  , eit_x(0)
  , services_w_eit_pf(0)
  , services_w_eit_sched(0)
{
	dprintf("()");

	memset(&decoded_pat, 0, sizeof(decoded_pat_t));
	memset(&decoded_vct, 0, sizeof(decoded_vct_t));
	memset(&decoded_mgt, 0, sizeof(decoded_mgt_t));
#if DECODE_LOCAL_NET
	memset(&decoded_nit, 0, sizeof(decoded_nit_t));
	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));
#endif
	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
#if DECODE_LOCAL_NET
	decoded_nit.ts_list.clear();
	decoded_sdt.services.clear();
#endif
	for (int i = 0; i < 128; i++) {
		for (map_decoded_atsc_eit::iterator iter =
			decoded_atsc_eit[i].begin();
		     iter != decoded_atsc_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_atsc_eit[i].clear();
	}

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
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

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
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
#if DECODE_LOCAL_NET
	decoded_nit.ts_list.clear();
	decoded_sdt.services.clear();
#endif
}

decode::decode(const decode&)
{
	dprintf("(copy)");

	memset(&decoded_pat, 0, sizeof(decoded_pat_t));
	memset(&decoded_vct, 0, sizeof(decoded_vct_t));
	memset(&decoded_mgt, 0, sizeof(decoded_mgt_t));
#if DECODE_LOCAL_NET
	memset(&decoded_nit, 0, sizeof(decoded_nit_t));
	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));
#endif
	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
#if DECODE_LOCAL_NET
	decoded_nit.ts_list.clear();
	decoded_sdt.services.clear();
#endif
	//decoded_atsc_eit.events.clear();

	for (int i = 0; i < 128; i++) {
		for (map_decoded_atsc_eit::iterator iter =
			decoded_atsc_eit[i].begin();
		     iter != decoded_atsc_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_atsc_eit[i].clear();
	}

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
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

decode& decode::operator= (const decode& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	memset(&decoded_pat, 0, sizeof(decoded_pat_t));
	memset(&decoded_vct, 0, sizeof(decoded_vct_t));
	memset(&decoded_mgt, 0, sizeof(decoded_mgt_t));
#if DECODE_LOCAL_NET
	memset(&decoded_nit, 0, sizeof(decoded_nit_t));
	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));
#endif
	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
#if DECODE_LOCAL_NET
	decoded_nit.ts_list.clear();
	decoded_sdt.services.clear();
#endif
	//decoded_atsc_eit.events.clear();

	for (int i = 0; i < 128; i++) {
		for (map_decoded_atsc_eit::iterator iter =
			decoded_atsc_eit[i].begin();
		     iter != decoded_atsc_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_atsc_eit[i].clear();
	}

	for (int i = 0; i < NUM_EIT; i++) {
		for (map_decoded_eit::iterator iter =
			decoded_eit[i].begin();
		     iter != decoded_eit[i].end(); ++iter)
			iter->second.events.clear();

		decoded_eit[i].clear();
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

	return *this;
}

/* -- STREAM TIME -- */
bool decode::take_stt(dvbpsi_atsc_stt_t* p_stt)
{
	stream_time = atsc_datetime_utc(p_stt->i_system_time);

	dbg_time("%s", ctime(&stream_time));

	return true;
}

bool decode::take_tot(dvbpsi_tot_t* p_tot)
{
	stream_time = datetime_utc(p_tot->i_utc_time);

	dbg_time("%s", ctime(&stream_time));

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
		fprintf(stderr, "  %10d | %d\n",
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
	if ((decoded_pmt[p_pmt->i_program_number].version == p_pmt->i_version) &&
	    (decoded_pmt[p_pmt->i_program_number].program == p_pmt->i_program_number)) {

		dprintf("v%d, service_id %d, pcr_pid %d: ALREADY DECODED",
			p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
		return false;
	}
#if PMT_DBG
	fprintf(stderr, "%s: v%d, service_id %d, pcr_pid %d\n", __func__,
		p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
#endif
	decoded_pmt[p_pmt->i_program_number].program = p_pmt->i_program_number;
	decoded_pmt[p_pmt->i_program_number].version = p_pmt->i_version;
	decoded_pmt[p_pmt->i_program_number].pcr_pid = p_pmt->i_pcr_pid;
	decoded_pmt[p_pmt->i_program_number].es_streams.clear();
	//FIXME: descriptors

	fprintf(stderr, "  es_pid | type\n");

	dvbpsi_pmt_es_t* p_es = p_pmt->p_first_es;
	while (p_es) {

		decoded_pmt[p_pmt->i_program_number].es_streams[p_es->i_pid].type = p_es->i_type;
		decoded_pmt[p_pmt->i_program_number].es_streams[p_es->i_pid].pid  = p_es->i_pid;
#if PMT_DBG
		fprintf(stderr, "  %6d | 0x%02x (%s)\n",
			decoded_pmt[p_pmt->i_program_number].es_streams[p_es->i_pid].pid,
			decoded_pmt[p_pmt->i_program_number].es_streams[p_es->i_pid].type,
			streamtype_name(decoded_pmt[p_pmt->i_program_number].es_streams[p_es->i_pid].type));
#endif
		p_es = p_es->p_next;
	}
	rcvd_pmt[p_pmt->i_program_number] = true;

	return true;
}

bool decode::take_vct(dvbpsi_atsc_vct_t* p_vct)
#define VCT_DBG 1
{
	if ((decoded_vct.version == p_vct->i_version) &&
	    (decoded_vct.ts_id   == p_vct->i_ts_id)) {

		dprintf("v%d, ts_id %d, b_cable_vct %d: ALREADY DECODED",
			p_vct->i_version, p_vct->i_ts_id, p_vct->b_cable_vct);
		return false;
	}
#if VCT_DBG
	fprintf(stderr, "%s: v%d, ts_id %d, b_cable_vct %d\n", __func__,
		p_vct->i_version, p_vct->i_ts_id, p_vct->b_cable_vct);
#endif
	decoded_vct.version   = p_vct->i_version;
	decoded_vct.ts_id     = p_vct->i_ts_id;
	decoded_vct.cable_vct = p_vct->b_cable_vct;
	decoded_vct.channels.clear();

	dvbpsi_atsc_vct_channel_t* p_channel = p_vct->p_first_channel;
#if VCT_DBG
	if (p_channel)
		fprintf(stderr, "  channel | service_id | source_id | service_name\n");
#endif
	while (p_channel) {

		memcpy(decoded_vct.channels[p_channel->i_program_number].short_name,  p_channel->i_short_name,
		       sizeof(decoded_vct.channels[p_channel->i_program_number].short_name));
		decoded_vct.channels[p_channel->i_program_number].chan_major        = p_channel->i_major_number;
		decoded_vct.channels[p_channel->i_program_number].chan_minor        = p_channel->i_minor_number;
		decoded_vct.channels[p_channel->i_program_number].modulation        = p_channel->i_modulation;
		decoded_vct.channels[p_channel->i_program_number].carrier_freq      = p_channel->i_carrier_freq;
		decoded_vct.channels[p_channel->i_program_number].chan_ts_id        = p_channel->i_channel_tsid;
		decoded_vct.channels[p_channel->i_program_number].program           = p_channel->i_program_number;
		decoded_vct.channels[p_channel->i_program_number].etm_location      = p_channel->i_etm_location;
		decoded_vct.channels[p_channel->i_program_number].access_controlled = p_channel->b_access_controlled;
		decoded_vct.channels[p_channel->i_program_number].path_select       = p_channel->b_path_select;
		decoded_vct.channels[p_channel->i_program_number].out_of_band       = p_channel->b_out_of_band;
		decoded_vct.channels[p_channel->i_program_number].hidden            = p_channel->b_hidden;
		decoded_vct.channels[p_channel->i_program_number].hide_guide        = p_channel->b_hide_guide;
		decoded_vct.channels[p_channel->i_program_number].service_type      = p_channel->i_service_type;
		decoded_vct.channels[p_channel->i_program_number].source_id         = p_channel->i_source_id;
#if VCT_DBG
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = decoded_vct.channels[p_channel->i_program_number].short_name[i*2+1];
		service_name[7] = 0;

		fprintf(stderr, "  %5d.%d | %10d | %9d | %s\n",
			decoded_vct.channels[p_channel->i_program_number].chan_major,
			decoded_vct.channels[p_channel->i_program_number].chan_minor,
			decoded_vct.channels[p_channel->i_program_number].program,
			decoded_vct.channels[p_channel->i_program_number].source_id,
			service_name);
#endif
		//FIXME: descriptors

		p_channel = p_channel->p_next;
	}

	//FIXME: descriptors
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
		decoded_mgt.tables[p_table->i_table_type].type    = p_table->i_table_type;
		decoded_mgt.tables[p_table->i_table_type].pid     = p_table->i_table_type_pid;
		decoded_mgt.tables[p_table->i_table_type].version = p_table->i_table_type_version;
		decoded_mgt.tables[p_table->i_table_type].bytes   = p_table->i_number_bytes;
#if 0
		switch (p_table->i_table_type) {
		default:
			break;
		}
		//FIXME: descriptors
#endif
		p_table = p_table->p_next;
	}
	//FIXME: descriptors
	return true;
}

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

		decoded_nit->ts_list[p_ts->i_ts_id].ts_id           = p_ts->i_ts_id;
		decoded_nit->ts_list[p_ts->i_ts_id].orig_network_id = p_ts->i_orig_network_id;

#if NIT_DBG
		fprintf(stderr, "   %05d | %d\n",
			decoded_nit->ts_list[p_ts->i_ts_id].ts_id,
			decoded_nit->ts_list[p_ts->i_ts_id].orig_network_id);
#endif
		/* descriptors contain frequency lists & LCNs */
		descriptors->decode(p_ts->p_first_descriptor);

		p_ts = p_ts->p_next;
	}
	return true;
}

bool decode::take_nit_actual(dvbpsi_nit_t* p_nit)
{
	network_id = p_nit->i_network_id;
#if DECODE_LOCAL_NET
	//return
	__take_nit(p_nit, &decoded_nit, &descriptors);
	if (decoded_nit.ts_list.count(decoded_pat.ts_id)) {
		orig_network_id = decoded_nit.ts_list[decoded_pat.ts_id].orig_network_id;
		return networks[orig_network_id].take_nit(p_nit);
	}
#endif
	return networks[p_nit->i_network_id].take_nit(p_nit);
}

bool decode::take_nit_other(dvbpsi_nit_t* p_nit)
{
	return networks[p_nit->i_network_id].take_nit(p_nit);
}

bool decode_network::take_nit(dvbpsi_nit_t* p_nit)
{
	return __take_nit(p_nit, &decoded_nit, &descriptors);
}

static bool __take_sdt(dvbpsi_sdt_t* p_sdt, decoded_sdt_t* decoded_sdt, desc* descriptors,
		       unsigned int* services_w_eit_pf, unsigned int* services_w_eit_sched)
#define SDT_DBG 1
{
	if ((decoded_sdt->version    == p_sdt->i_version) &&
	    (decoded_sdt->network_id == p_sdt->i_network_id)) {

		dprintf("v%d | ts_id %d | network_id %d: ALREADY DECODED",
			p_sdt->i_version, p_sdt->i_ts_id, p_sdt->i_network_id);
		return false;
	}
#if SDT_DBG
	fprintf(stderr, "%s: v%d | ts_id %d | network_id %d\n", __func__,
		p_sdt->i_version, p_sdt->i_ts_id, p_sdt->i_network_id);
#endif
	decoded_sdt->ts_id      = p_sdt->i_ts_id;
	decoded_sdt->version    = p_sdt->i_version;
	decoded_sdt->network_id = p_sdt->i_network_id;

	unsigned int _services_w_eit_pf    = 0;
	unsigned int _services_w_eit_sched = 0;
	//fprintf(stderr, "  service_id | service_name");
	dvbpsi_sdt_service_t* p_service = p_sdt->p_first_service;
	while (p_service) {

		decoded_sdt->services[p_service->i_service_id].service_id     = p_service->i_service_id; /* matches program_id / service_id from PAT */
		decoded_sdt->services[p_service->i_service_id].f_eit_sched    = p_service->b_eit_schedule;
		decoded_sdt->services[p_service->i_service_id].f_eit_present  = p_service->b_eit_present;
		decoded_sdt->services[p_service->i_service_id].running_status = p_service->i_running_status;
		decoded_sdt->services[p_service->i_service_id].f_free_ca      = p_service->b_free_ca;

		if (decoded_sdt->services[p_service->i_service_id].f_eit_present)
			_services_w_eit_pf++;

		if (decoded_sdt->services[p_service->i_service_id].f_eit_sched)
			_services_w_eit_sched++;

		/* service descriptors contain service provider name & service name */
		descriptors->decode(p_service->p_first_descriptor);
		strcpy(decoded_sdt->services[p_service->i_service_id].provider_name, (const char *)descriptors->provider_name);
		strcpy(decoded_sdt->services[p_service->i_service_id].service_name, (const char *)descriptors->service_name);

#if SDT_DBG
		fprintf(stderr, "%s: %d | %s | %s | %s %s\n", __func__,
			decoded_sdt->services[p_service->i_service_id].service_id,
			decoded_sdt->services[p_service->i_service_id].provider_name,
			decoded_sdt->services[p_service->i_service_id].service_name,
			(decoded_sdt->services[p_service->i_service_id].f_eit_present) ? "p/f" : "   ",
			(decoded_sdt->services[p_service->i_service_id].f_eit_sched) ? "sched" : "     ");
#endif
		p_service = p_service->p_next;
	}
	*services_w_eit_pf    = _services_w_eit_pf;
	*services_w_eit_sched = _services_w_eit_sched;
	return true;
}

bool decode::take_sdt_actual(dvbpsi_sdt_t* p_sdt)
{
	orig_network_id = p_sdt->i_network_id;
#if DECODE_LOCAL_NET
	//return
	__take_sdt(p_sdt, &decoded_sdt, &descriptors, &services_w_eit_pf, &services_w_eit_sched);
#endif
	return networks[p_sdt->i_network_id].take_sdt(p_sdt);
}

bool decode::take_sdt_other(dvbpsi_sdt_t* p_sdt)
{
	return networks[p_sdt->i_network_id].take_sdt(p_sdt);
}

bool decode_network::take_sdt(dvbpsi_sdt_t* p_sdt)
{
	return __take_sdt(p_sdt, &decoded_network_services[p_sdt->i_ts_id].decoded_sdt,
			  &descriptors,
			  &decoded_network_services[p_sdt->i_ts_id].services_w_eit_pf,
			  &decoded_network_services[p_sdt->i_ts_id].services_w_eit_sched);
}

bool decode::take_eit(dvbpsi_eit_t* p_eit)
{
#if 1
	bool other = false;

	eit_x = p_eit->i_table_id;
	switch (p_eit->i_table_id) {
	case 0x4f:
		other = true;
		/* fall thru */
	case 0x4e:
		eit_x = 0;
		break;
	case 0x60 ... 0x6f:
		other = true;
		eit_x = p_eit->i_table_id - 0x60 + 1;
		break;
	case 0x50 ... 0x5f:
		eit_x = p_eit->i_table_id - 0x50 + 1;
		break;
	}
#endif
	if ((decoded_eit[eit_x][p_eit->i_service_id].version == p_eit->i_version) &&
	    (decoded_eit[eit_x][p_eit->i_service_id].service_id == p_eit->i_service_id)) {
#if DBG
		fprintf(stderr, "%s-%d: v%d | ts_id %d | network_id %d service_id %d: ALREADY DECODED\n", __func__, eit_x,
			p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, p_eit->i_service_id);
#endif
		return false;
	}
#if 1//DBG
	fprintf(stderr, "%s-%d: v%d | ts_id %d | network_id %d service_id %d | table id: 0x%02x, last_table id: 0x%02x\n", __func__, eit_x,
		p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, p_eit->i_service_id, p_eit->i_table_id, p_eit->i_last_table_id);
#endif
	if (other) return false;
	decoded_eit[eit_x][p_eit->i_service_id].service_id    = p_eit->i_service_id;
	decoded_eit[eit_x][p_eit->i_service_id].version       = p_eit->i_version;
	decoded_eit[eit_x][p_eit->i_service_id].ts_id         = p_eit->i_ts_id;
	decoded_eit[eit_x][p_eit->i_service_id].network_id    = p_eit->i_network_id;
	decoded_eit[eit_x][p_eit->i_service_id].last_table_id = p_eit->i_last_table_id;

	dvbpsi_eit_event_t* p_event = p_eit->p_first_event;
	while (p_event) {

		decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].event_id       = p_event->i_event_id;
		decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].start_time     = p_event->i_start_time;
		decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].length_sec     = p_event->i_duration;
		decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].running_status = p_event->i_running_status;
		decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].f_free_ca      = p_event->b_free_ca;

		descriptors.decode(p_event->p_first_descriptor);

		decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].name.assign((const char *)descriptors._4d.name);
		decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].text.assign((const char *)descriptors._4d.text);
#if DBG
		time_t start = datetime_utc(decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].start_time /*+ (60 * tz_offset)*/);
		time_t end   = datetime_utc(decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].start_time + decoded_eit[eit_x][p_eit->i_service_id].events[p_event->i_event_id].length_sec /*+ (60 * tz_offset)*/);

		struct tm tms = *localtime(&start);
		struct tm tme = *localtime(&end);

		fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, descriptors._4d.name);
#endif
		p_event = p_event->p_next;
	}
	return true;
}

bool decode::take_eit(dvbpsi_atsc_eit_t* p_eit)
{
	if ((decoded_atsc_eit[eit_x][p_eit->i_source_id].version   == p_eit->i_version) &&
	    (decoded_atsc_eit[eit_x][p_eit->i_source_id].source_id == p_eit->i_source_id)) {
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
	decoded_atsc_eit[eit_x][p_eit->i_source_id].version   = p_eit->i_version;
	decoded_atsc_eit[eit_x][p_eit->i_source_id].source_id = p_eit->i_source_id;

	dvbpsi_atsc_eit_event_t* p_event = p_eit->p_first_event;
	while (p_event) {

			decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].event_id     = p_event->i_event_id;
			decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].start_time   = p_event->i_start_time;
			decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].etm_location = p_event->i_etm_location;
			decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].length_sec   = p_event->i_length_seconds;
			decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].title_bytes  = p_event->i_title_length;
			memcpy(decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].title, p_event->i_title, 256); // FIXME
#if DBG
			time_t start = atsc_datetime_utc(decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].start_time /*+ (60 * tz_offset)*/);
			time_t end   = atsc_datetime_utc(decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].start_time + decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].length_sec /*+ (60 * tz_offset)*/);

			unsigned char name[256];
			memset(name, 0, sizeof(char) * 256);
			decode_multiple_string(decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].title, decoded_atsc_eit[eit_x][p_eit->i_source_id].events[p_event->i_event_id].title_bytes, name);
			//p_epg->text[0] = 0;

			//FIXME: descriptors

			struct tm tms = *localtime( &start );
			struct tm tme = *localtime( &end  );
			fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
		p_event = p_event->p_next;
	}
	return true;
}

void decode::dump_eit_x_atsc(uint8_t eit_x, uint16_t source_id)
{
#if DBG
	fprintf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct) {

		if ((source_id) && (source_id != iter_vct->second.source_id))
			continue;

		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		service_name[7] = 0;

		fprintf(stdout, "%s-%d: id:%d - %d.%d: %s\n", __func__,
			eit_x, iter_vct->second.source_id,
			iter_vct->second.chan_major,
			iter_vct->second.chan_minor,
			service_name);

		map_decoded_atsc_eit_events::const_iterator iter_eit;
		for (iter_eit = decoded_atsc_eit[eit_x][iter_vct->second.source_id].events.begin();
		     iter_eit != decoded_atsc_eit[eit_x][iter_vct->second.source_id].events.end();
		     ++iter_eit) {

			time_t start = atsc_datetime_utc(iter_eit->second.start_time /*+ (60 * tz_offset)*/);
			time_t end   = atsc_datetime_utc(iter_eit->second.start_time + iter_eit->second.length_sec /*+ (60 * tz_offset)*/);

			unsigned char name[256];
			memset(name, 0, sizeof(char) * 256);
			decode_multiple_string(iter_eit->second.title, iter_eit->second.title_bytes, name);

			//FIXME: descriptors

			struct tm tms = *localtime( &start );
			struct tm tme = *localtime( &end  );
			fprintf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
		}
	}
}

void decode::dump_eit_x_dvb(uint8_t eit_x, uint16_t service_id)
{
#if DBG
	fprintf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	map_decoded_sdt_services::const_iterator iter_sdt;
	const decoded_sdt_t *decoded_sdt = get_decoded_sdt();
	for (iter_sdt = decoded_sdt->services.begin(); iter_sdt != decoded_sdt->services.end(); ++iter_sdt) {
		if ((!iter_sdt->second.f_eit_present) ||
		    ((service_id) && (service_id != iter_sdt->second.service_id)))
			continue;

#if 0
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		service_name[7] = 0;
#endif
		fprintf(stdout, "%s-%d: id:%d - %d: %s\n", __func__,
			eit_x, iter_sdt->second.service_id,
			descriptors.lcn[iter_sdt->second.service_id],
			iter_sdt->second.service_name);

		map_decoded_eit_events::const_iterator iter_eit;
		for (iter_eit = decoded_eit[eit_x][iter_sdt->second.service_id].events.begin();
		     iter_eit != decoded_eit[eit_x][iter_sdt->second.service_id].events.end();
		     ++iter_eit) {

			time_t start = datetime_utc(iter_eit->second.start_time /*+ (60 * tz_offset)*/);
			time_t end   = datetime_utc(iter_eit->second.start_time + iter_eit->second.length_sec /*+ (60 * tz_offset)*/);

			//FIXME: descriptors

			struct tm tms = *localtime( &start );
			struct tm tme = *localtime( &end  );
			fprintf(stdout, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, iter_eit->second.name.c_str()/*, iter_eit->second.text.c_str()*/ );
		}
	}
}

void decode::dump_eit_x(uint8_t eit_x, uint16_t source_id)
{
#if DBG
	fprintf(stderr, "%s-%d\n", __func__, eit_x);
#endif
	if (decoded_vct.channels.size()) {
		dump_eit_x_atsc(eit_x, source_id);
	} else {
		dump_eit_x_dvb(eit_x, source_id); /* service_id */
	}

	fprintf(stdout, "\n");
	fflush(stdout);
}

void decode::dump_epg_atsc(uint16_t source_id)
{
	unsigned int eit_num = 0;

	while ((eit_num < 128) && (decoded_atsc_eit[eit_num].count(source_id))) {
		dump_eit_x_atsc(eit_num, source_id);
		eit_num++;
	}
}

void decode::dump_epg_dvb(uint16_t service_id)
{
	unsigned int eit_num = 0;

	while ((eit_num < NUM_EIT) && (decoded_eit[eit_num].count(service_id))) {
		dump_eit_x_dvb(eit_num, service_id);
		eit_num++;
	}
}

void decode::dump_epg()
{
	if (decoded_vct.channels.size()) {
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct)
		dump_epg_atsc(iter_vct->second.source_id);
	} else {
	map_decoded_sdt_services::const_iterator iter_sdt;
	const decoded_sdt_t *decoded_sdt = get_decoded_sdt();
	for (iter_sdt = decoded_sdt->services.begin(); iter_sdt != decoded_sdt->services.end(); ++iter_sdt) {
		if (iter_sdt->second.f_eit_present)
			dump_epg_dvb(iter_sdt->second.service_id);
	}}
}


bool decode::take_ett(dvbpsi_atsc_ett_t* p_ett)
{
#if 1
	if ((decoded_ett[p_ett->i_etm_id].version == p_ett->i_version) &&
	    (decoded_ett[p_ett->i_etm_id].etm_id  == p_ett->i_etm_id)) {
		fprintf(stderr, "%s: v%d, ID %d: ALREADY DECODED\n", __func__,
			p_ett->i_version, p_ett->i_etm_id);
		return false;
	}
#endif
	decoded_ett[p_ett->i_etm_id].version    = p_ett->i_version;
	decoded_ett[p_ett->i_etm_id].etm_id     = p_ett->i_etm_id;
	decoded_ett[p_ett->i_etm_id].etm_length = p_ett->i_etm_length;
	memcpy(&decoded_ett[p_ett->i_etm_id].etm, p_ett->p_etm, p_ett->i_etm_length);

	unsigned char message[256];
	memset(message, 0, sizeof(char) * 256);

	decode_multiple_string(p_ett->p_etm, p_ett->i_etm_length, message);

	fprintf(stderr, "%s: v%d, ID: %d: %s\n", __func__,
		p_ett->i_version, p_ett->i_etm_id, message);
	return true;
}

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
	uint8_t current_eit_x = 0;
	return ((get_decoded_sdt()->services.size()) &&
		(((decoded_eit[current_eit_x].size()) || (!services_w_eit_pf)) &&
		 (decoded_eit[current_eit_x].size() == services_w_eit_pf)));
}

bool decode::eit_x_complete_dvb_sched(uint8_t current_eit_x)
{
	return ((get_decoded_sdt()->services.size()) &&
		(((decoded_eit[current_eit_x].size()) || (!services_w_eit_sched)) &&
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
		if ((get_decoded_sdt()->services.size()) && (get_decoded_nit()->ts_list.size())) {
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

const decoded_sdt_t* decode::get_decoded_sdt()
{
	return networks[orig_network_id].get_decoded_sdt(decoded_pat.ts_id);
};

const decoded_nit_t* decode::get_decoded_nit()
{
	return networks[network_id].get_decoded_nit();
};

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
