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
#include "debug.h"

#define dprintf(fmt, arg...) __dprintf(DBG_DECODE, fmt, ##arg)

decode::decode()
  : eit_x(0)
{
	dprintf("()");

	memset(&decoded_pat, 0, sizeof(decoded_pat_t));
	memset(&decoded_vct, 0, sizeof(decoded_vct_t));
	memset(&decoded_mgt, 0, sizeof(decoded_mgt_t));
	memset(&decoded_nit, 0, sizeof(decoded_nit_t));
	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));

	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
	decoded_nit.ts_list.clear();
	decoded_sdt.services.clear();

	for (int i = 0; i < 0x80; i++) {
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
	decoded_ett.clear();
}

decode::~decode()
{
	dprintf("(%04x|%05d)",
		decoded_pat.ts_id, decoded_pat.ts_id);

	for (int i = 0; i < 0x80; i++) {
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
	decoded_ett.clear();
	decoded_nit.ts_list.clear();
	decoded_sdt.services.clear();
}

decode::decode(const decode&)
{
	dprintf("(copy)");

	memset(&decoded_pat, 0, sizeof(decoded_pat_t));
	memset(&decoded_vct, 0, sizeof(decoded_vct_t));
	memset(&decoded_mgt, 0, sizeof(decoded_mgt_t));
	memset(&decoded_nit, 0, sizeof(decoded_nit_t));
	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));

	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
	decoded_nit.ts_list.clear();
	decoded_sdt.services.clear();

	//decoded_atsc_eit.events.clear();

	for (int i = 0; i < 0x80; i++) {
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
	memset(&decoded_nit, 0, sizeof(decoded_nit_t));
	memset(&decoded_sdt, 0, sizeof(decoded_sdt_t));

	//memset(&decoded_atsc_eit, 0, sizeof(decoded_atsc_eit_t));

	decoded_pat.programs.clear();
	decoded_vct.channels.clear();
	decoded_mgt.tables.clear();
	decoded_nit.ts_list.clear();
	decoded_sdt.services.clear();

	//decoded_atsc_eit.events.clear();

	for (int i = 0; i < 0x80; i++) {
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
	decoded_ett.clear();

	return *this;
}

/* -- STREAM TIME -- */
bool decode::take_stt(dvbpsi_atsc_stt_t* p_stt)
{
	stream_time = atsc_datetime_utc(p_stt->i_system_time);

	dprintf("%s", ctime(&stream_time));

	return true;
}

bool decode::take_tot(dvbpsi_tot_t* p_tot)
{
	stream_time = datetime_utc(p_tot->i_utc_time);

	dprintf("%s", ctime(&stream_time));

	return true;
}

/* -- TABLE HANDLERS -- */
bool decode::take_pat(dvbpsi_pat_t* p_pat)
{
	if ((decoded_pat.version == p_pat->i_version) &&
	    (decoded_pat.ts_id   == p_pat->i_ts_id)) {

		dprintf("v%d, ts_id: %d: ALREADY DECODED",
			p_pat->i_version, p_pat->i_ts_id);
		return false;
	}
#if 1//DBG
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
#if 1//DBG
		fprintf(stderr, "  %10d | %d\n",
			p_program->i_number,
			decoded_pat.programs[p_program->i_number]);
#endif
		p_program = p_program->p_next;
	}
	return true;
}

bool decode::take_pmt(dvbpsi_pmt_t* p_pmt)
{
	if ((decoded_pmt[p_pmt->i_program_number].version == p_pmt->i_version) &&
	    (decoded_pmt[p_pmt->i_program_number].program == p_pmt->i_program_number)) {

		dprintf("v%d, service_id %d, pcr_pid %d: ALREADY DECODED",
			p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
		return false;
	}
#if 1//DBG
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

		fprintf(stderr, "  %6d | 0x%02x (%s)\n",
			decoded_pmt[p_pmt->i_program_number].es_streams[p_es->i_pid].pid,
			decoded_pmt[p_pmt->i_program_number].es_streams[p_es->i_pid].type,
			streamtype_name(decoded_pmt[p_pmt->i_program_number].es_streams[p_es->i_pid].type));

		rcvd_pmt[p_pmt->i_program_number] = true;
		p_es = p_es->p_next;
	}
	return true;
}

bool decode::take_vct(dvbpsi_atsc_vct_t* p_vct)
{
	if ((decoded_vct.version == p_vct->i_version) &&
	    (decoded_vct.ts_id   == p_vct->i_ts_id)) {

		dprintf("v%d, ts_id %d, b_cable_vct %d: ALREADY DECODED\n",
			p_vct->i_version, p_vct->i_ts_id, p_vct->b_cable_vct);
		return false;
	}
#if 1//DBG
	fprintf(stderr, "%s: v%d, ts_id %d, b_cable_vct %d\n", __func__,
		p_vct->i_version, p_vct->i_ts_id, p_vct->b_cable_vct);
#endif
	decoded_vct.version   = p_vct->i_version;
	decoded_vct.ts_id     = p_vct->i_ts_id;
	decoded_vct.cable_vct = p_vct->b_cable_vct;
	decoded_vct.channels.clear();

	dvbpsi_atsc_vct_channel_t* p_channel = p_vct->p_first_channel;
	if (p_channel)
		fprintf(stderr, "  channel | service_id | source_id | service_name\n");
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

		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = p_channel->i_short_name[i*2+1];
		service_name[7] = 0;

		fprintf(stderr, "  %5d.%d | %10d | %9d | %s\n",
			decoded_vct.channels[p_channel->i_program_number].chan_major,
			decoded_vct.channels[p_channel->i_program_number].chan_minor,
			decoded_vct.channels[p_channel->i_program_number].program,
			decoded_vct.channels[p_channel->i_program_number].source_id,
			service_name);

		//FIXME: descriptors

		p_channel = p_channel->p_next;
	}

	//FIXME: descriptors
	return true;
}

bool decode::take_mgt(dvbpsi_atsc_mgt_t* p_mgt)
{
	if ((decoded_mgt.version == p_mgt->i_version) &&
	    (!decoded_mgt.tables.empty())) {

		dprintf("v%d: ALREADY DECODED", p_mgt->i_version);
		return false;
	}
	fprintf(stderr, "%s: v%d\n", __func__, p_mgt->i_version);

	decoded_mgt.version = p_mgt->i_version;
	decoded_mgt.tables.clear();

	dvbpsi_atsc_mgt_table_t* p_table = p_mgt->p_first_table;
	if (p_table)
		fprintf(stderr, "  table type |   pid  | ver | bytes\n" );
	while (p_table) {
		fprintf(stderr, "    0x%04x   | 0x%04x | %3d | %d\n",
				p_table->i_table_type, p_table->i_table_type_pid,
				p_table->i_table_type_version, p_table->i_number_bytes);

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

bool decode::take_nit(dvbpsi_nit_t* p_nit)
{
	if ((decoded_nit.version    == p_nit->i_version) &&
	    (decoded_nit.network_id == p_nit->i_network_id)) {

		dprintf("v%d, network_id %d: ALREADY DECODED\n",
			p_nit->i_version, p_nit->i_network_id );
		return false;
	}
	fprintf(stderr, "%s: v%d, network_id %d\n", __func__,
		p_nit->i_version, p_nit->i_network_id );

	decoded_nit.version = p_nit->i_version;
	decoded_nit.version = p_nit->i_version;
	//FIXME: descriptors

	dvbpsi_nit_ts_t* p_ts = p_nit->p_first_ts;
	while (p_ts) {

		decoded_nit.ts_list[p_ts->i_ts_id].ts_id           = p_ts->i_ts_id;
		decoded_nit.ts_list[p_ts->i_ts_id].orig_network_id = p_ts->i_orig_network_id;

		p_ts = p_ts->p_next;
	}
	return true;
}

bool decode::take_sdt(dvbpsi_sdt_t* p_sdt)
{
	if ((decoded_sdt.version    == p_sdt->i_version) &&
	    (decoded_sdt.network_id == p_sdt->i_network_id)){

		dprintf("v%d | ts_id %d | network_id %d: ALREADY DECODED",
			p_sdt->i_version, p_sdt->i_ts_id, p_sdt->i_network_id );
		return false;
	}
#if 1//DBG
	fprintf(stderr, "%s: v%d | ts_id %d | network_id %d", __func__,
		p_sdt->i_version, p_sdt->i_ts_id, p_sdt->i_network_id );
#endif
	decoded_sdt.ts_id      = p_sdt->i_ts_id;
	decoded_sdt.version    = p_sdt->i_version;
	decoded_sdt.network_id = p_sdt->i_network_id;

	//fprintf(stderr, "  service_id | service_name");
	dvbpsi_sdt_service_t* p_service = p_sdt->p_first_service;
	while (p_service) {

		decoded_sdt.services[p_service->i_service_id].service_id     = p_service->i_service_id; /* matches program_id / service_id from PAT */
		decoded_sdt.services[p_service->i_service_id].f_eit_sched    = p_service->b_eit_schedule;
		decoded_sdt.services[p_service->i_service_id].f_eit_present  = p_service->b_eit_present;
		decoded_sdt.services[p_service->i_service_id].running_status = p_service->i_running_status;
		decoded_sdt.services[p_service->i_service_id].f_free_ca      = p_service->b_free_ca;
		//FIXME: descriptors
		p_service = p_service->p_next;
	}
	return true;
}

bool decode::take_eit(dvbpsi_eit_t* p_eit)
{
	decoded_eit_t decoded_eit;
#if 1
	if (decoded_eit.version == p_eit->i_version) { // FIXME
		dprintf("v%d | ts_id %d | network_id %d service_id %d: ALREADY DECODED",
			p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, p_eit->i_service_id );
		return false;
	}
#endif
#if 1//DBG
	fprintf(stderr, "%s: v%d | ts_id %d | network_id %d service_id %d", __func__,
		p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, p_eit->i_service_id );
#endif
	decoded_eit.service_id    = p_eit->i_service_id;
	decoded_eit.version       = p_eit->i_version;
	decoded_eit.ts_id         = p_eit->i_ts_id;
	decoded_eit.network_id    = p_eit->i_network_id;
	decoded_eit.last_table_id = p_eit->i_last_table_id;

	dvbpsi_eit_event_t* p_event = p_eit->p_first_event;
	while (p_event) {

		decoded_eit.events[p_event->i_event_id].event_id       = p_event->i_event_id;
		decoded_eit.events[p_event->i_event_id].start_time     = p_event->i_start_time;
		decoded_eit.events[p_event->i_event_id].length_sec     = p_event->i_duration;
		decoded_eit.events[p_event->i_event_id].running_status = p_event->i_running_status;
		decoded_eit.events[p_event->i_event_id].f_free_ca      = p_event->b_free_ca;
#if 0
		time_t start = datetime_utc(decoded_eit.events[p_event->i_event_id].start_time /*+ (60 * tz_offset)*/);
		time_t end   = datetime_utc(decoded_eit.events[p_event->i_event_id].start_time + decoded_eit.events[p_event->i_event_id].length_sec /*+ (60 * tz_offset)*/);

		//FIXME: descriptors
		unsigned char name[256];
		unsigned char text[256];

		dvbpsi_descriptor_t* p_descriptor = p_event->p_first_descriptor;
		while (p_descriptor) {
			if (p_descriptor->i_tag == 0x4d /*DT_ShortEvent*/) {

				dvbpsi_short_event_dr_t* dr = dvbpsi_DecodeShortEventDr(p_descriptor);
				memcpy(lang, dr->i_iso_639_code, 3);
				get_descriptor_text(dr->i_event_name, dr->i_event_name_length, name);
				get_descriptor_text(dr->i_text, dr->i_text_length, text);
			}
			p_descriptor = p_descriptor->p_next;
		}

		struct tm tms = *localtime(&start);
		struct tm tme = *localtime(&end);
		fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name);
#endif
		p_event = p_event->p_next;
	}
	return true;
}

void decode::dump_eit_x(uint8_t eit_x, uint16_t source_id)
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
	fprintf(stdout, "\n");
}

void decode::dump_epg(uint16_t source_id)
{
	unsigned int eit_num = 0;

	while ((eit_num < 128) && (decoded_atsc_eit[eit_num].count(source_id))) {
		dump_eit_x(eit_num, source_id);
		eit_num++;
	}
}

void decode::dump_epg()
{
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct)
		dump_epg(iter_vct->second.source_id);
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
	if (rcvd_pmt.size() == 0)
		return false;
	for (map_rcvd::const_iterator iter = rcvd_pmt.begin(); iter != rcvd_pmt.end(); ++iter)
		if (!iter->second) {
			//fprintf(stderr, "%s: missing pmt for program %d\n", __func__, iter->first);
			return false;
		}

	return true;
}

bool decode::eit_x_complete(uint8_t current_eit_x)
{
#if 0
	for (map_decoded_vct_channels::const_iterator iter =
	       decoders[ts_id].get_decoded_vct()->channels.begin();
	     iter != decoders[ts_id].get_decoded_vct()->channels.end(); ++iter)
		if (iter->first > 0) {// FIXME: > 0 ???
			if (0 == decoders[ts_id].get_decoded_atsc_eit()[current_eit_x].count(iter->second.source_id))
				return false;
		}
	return true;
#else
	return ((decoded_atsc_eit[current_eit_x].size()) &&
		(decoded_atsc_eit[current_eit_x].size() ==
		 decoded_vct.channels.size()));
#endif
}

bool decode::got_all_eit()
{
	for (map_decoded_mgt_tables::const_iterator iter = decoded_mgt.tables.begin(); iter != decoded_mgt.tables.end(); ++iter) {
		switch (iter->first) {
		case 0x0100 ... 0x017f: /* EIT-0 to EIT-127 */
			if (!eit_x_complete(0x0100 - iter->first))
				return false;
		}
	}
	return true;
}

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
