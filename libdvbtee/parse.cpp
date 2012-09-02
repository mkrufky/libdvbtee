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
#if 1
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "functions.h"
#include "log.h"
#define CLASS_MODULE "parse"

#if USE_STATIC_DECODE_MAP
static map_decoder   decoders;
#endif

#define dprintf(fmt, arg...) __dprintf(DBG_PARSE, fmt, ##arg)

#define PID_PAT  0x00
#define PID_CAT  0x01
#define PID_TSDT 0x02
#define PID_NIT  0x10
#define PID_SDT  0x11
#define PID_BAT  0x11
#define PID_EIT  0x12
#define PID_RST  0x13
#define PID_TDT  0x14
#define PID_TOT  0x14
#define PID_DIT  0x1E
#define PID_SIT  0x1F
#define PID_ATSC 0x1FFB
#define PID_NULL 0x1FFF
#define PID_MAX  0x2000

#define TID_PAT  0x00
#define TID_CAT  0x01
#define TID_PMT  0x02
#define TID_TSDT 0x03
#define TID_NIT_ACTUAL  0x40
#define TID_NIT_OTHER   0x41
#define TID_SDT_ACTUAL  0x42
#define TID_SDT_OTHER   0x46
#define TID_BAT  0x4A
#define TID_EIT_ACTUAL  0x4E
#define TID_EIT_OTHER   0x4F
  //  0x50 - 0x5F
  //  0x60 - 0x6F
#define TID_TDT  0x70
#define TID_RST  0x71
#define TID_TOT  0x73
#define TID_DIT  0x7E
#define TID_SIT  0x7F

#define TID_ATSC_MGT  0xC7
#define TID_ATSC_TVCT 0xC8
#define TID_ATSC_CVCT 0xC9
#define TID_ATSC_RRT  0xCA
#define TID_ATSC_EIT  0xCB
#define TID_ATSC_ETT  0xCC
#define TID_ATSC_STT  0xCD

#if DBG
uint8_t pids[0x2000] = { 0 };
unsigned int pid_idx = 0;

void addpid(uint8_t pid)
{
	unsigned int i = 0;
	while (i < pid_idx) {
		if (pids[i] == pid)
			break;
		i++;
	}
	if (i >= pid_idx) {
		pids[i] = pid;
		pid_idx++;
	}
}

void printpids()
{
	unsigned int i = 0;
	fprintf(stderr, "%s: ", __func__);
	while (i < pid_idx) {
		fprintf(stderr, "%d(%04x) ", pids[i], pids[i]);
		i++;
	}
	fprintf(stderr, "\n");
}
#endif
/* --- */



/* -- TABLE HANDLERS -- */
bool parse::take_stt(dvbpsi_atsc_stt_t* p_stt, bool decoded)
{
	dprintf("(%s)", (decoded) ? "post" : "pre");

	if (decoded) return true;

	return true;
}

bool parse::take_tot(dvbpsi_tot_t* p_tot, bool decoded)
{
	dprintf("(%s)", (decoded) ? "post" : "pre");

	if (decoded) return true;

	return true;
}

void parse::rewrite_pat()
{
	if (0 == service_ids.size())
		return;

	dvbpsi_pat_t pat;
	dvbpsi_psi_section_t* p_section;
	const decoded_pat_t *decoded_pat = decoders[ts_id].get_decoded_pat();

	if (rewritten_pat_ver_offset == 0x1e)
		rewritten_pat_ver_offset = 0;

	dvbpsi_InitPAT(&pat, ts_id, 0x1f & (++rewritten_pat_ver_offset + decoded_pat->version), 1);

	for (map_eit_pids::const_iterator iter = service_ids.begin(); iter != service_ids.end(); ++iter)
		dvbpsi_PATAddProgram(&pat, iter->first, ((decoded_pat_t *) decoded_pat)->programs[iter->first]);

	p_section = dvbpsi_GenPATSections(&pat, 0);
	pat_pkt[0] = 0x47;
	pat_pkt[1] = pat_pkt[2] = pat_pkt[3] = 0x00;
	writePSI(pat_pkt, p_section);
	dvbpsi_DeletePSISections(p_section);
	dvbpsi_EmptyPAT(&pat);
}

bool parse::take_pat(dvbpsi_pat_t* p_pat, bool decoded)
{
	dprintf("(%s): v%d, ts_id: %d",
		(decoded) ? "post" : "pre",
		p_pat->i_version, p_pat->i_ts_id);

	if (!decoded) {
		set_ts_id(p_pat->i_ts_id);
		h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
		return true;
	}

#ifdef INLINE_PAT_REWRITE
	dvbpsi_pat_t pat;
	bool do_pat_rewrite = (service_ids.size()) ? true : false;

	if (do_pat_rewrite) {
		if (rewritten_pat_ver_offset == 0x1e)
			rewritten_pat_ver_offset = 0;
		dvbpsi_InitPAT(&pat, ts_id,
			       0x1f & (++rewritten_pat_ver_offset +
				       decoders[ts_id].get_decoded_pat()->version), 1);
	}
#endif

	for (map_decoded_pat_programs::const_iterator iter =
	       decoders[p_pat->i_ts_id].get_decoded_pat()->programs.begin();
	     iter != decoders[p_pat->i_ts_id].get_decoded_pat()->programs.end(); ++iter)
		if (iter->first > 0) {// FIXME: > 0 ???
			if ((!service_ids.size()) || (service_ids.count(iter->first)))  {
				h_pmt[iter->second] = dvbpsi_AttachPMT(iter->first, take_pmt, this);
				add_filter(iter->second);
#ifdef INLINE_PAT_REWRITE
				if (do_pat_rewrite)
					dvbpsi_PATAddProgram(&pat,
							     iter->first,
							     iter->second);
#endif
			}
		}

#ifdef INLINE_PAT_REWRITE
	if (do_pat_rewrite) {
		dvbpsi_psi_section_t* p_section = dvbpsi_GenPATSections(&pat, 0);
		pat_pkt[0] = 0x47;
		pat_pkt[1] = pat_pkt[2] = pat_pkt[3] = 0x00;
		writePSI(pat_pkt, p_section);
		dvbpsi_DeletePSISections(p_section);
		dvbpsi_EmptyPAT(&pat);
	}
#else
	rewrite_pat();
#endif

	has_pat = true;

	return true;
}

bool parse::process_pmt(const decoded_pmt_t *pmt)
{
	dprintf(": v%d, service_id %d, pcr_pid %d",
		pmt->version, pmt->program, pmt->pcr_pid);

	for (map_ts_elementary_streams::const_iterator iter_pmt_es = pmt->es_streams.begin();
	     iter_pmt_es != pmt->es_streams.end(); ++iter_pmt_es) {
			payload_pids[iter_pmt_es->second.pid] = iter_pmt_es->second.type;
			add_filter(iter_pmt_es->second.pid);
	}

	return true;
}

bool parse::take_pmt(dvbpsi_pmt_t* p_pmt, bool decoded)
{
	dprintf("(%s): v%d, service_id %d, pcr_pid %d",
		(decoded) ? "post" : "pre",
		p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);

	if (!decoded) return true;

	/* if we're going to stream a program, we should make sure its pid is coming thru after analyzing the decoded pmt's */
	map_decoded_pmt::const_iterator iter_pmt = decoders[ts_id].get_decoded_pmt()->find(p_pmt->i_program_number);
	if (iter_pmt != decoders[ts_id].get_decoded_pmt()->end()) {
		for (map_ts_elementary_streams::const_iterator iter_pmt_es = iter_pmt->second.es_streams.begin();
		     iter_pmt_es != iter_pmt->second.es_streams.end(); ++iter_pmt_es) {
				payload_pids[iter_pmt_es->second.pid] = iter_pmt_es->second.type;
				add_filter(iter_pmt_es->second.pid);
		}
	}

	return true;
}

bool parse::take_vct(dvbpsi_atsc_vct_t* p_vct, bool decoded)
{
	dprintf("(%s): v%d, ts_id %d, b_cable_vct %d",
		(decoded) ? "post" : "pre",
		p_vct->i_version, p_vct->i_ts_id, p_vct->b_cable_vct);

	if (!decoded) {
		set_ts_id(p_vct->i_ts_id);
		return true;
	}
	has_vct = true;

	return true;
}

bool parse::take_mgt(dvbpsi_atsc_mgt_t* p_mgt, bool decoded)
{
	dprintf("(%s): v%d",
		(decoded) ? "post" : "pre",
		p_mgt->i_version);

	if (!decoded) return true;
#if 0
	has_mgt = false;
#endif
	bool b_expecting_vct = false;

	eit_pids.clear();
	for (map_decoded_mgt_tables::const_iterator iter =
	       decoders[ts_id].get_decoded_mgt()->tables.begin();
	     iter != decoders[ts_id].get_decoded_mgt()->tables.end(); ++iter) {

		bool b_attach_demux = false;

		switch (iter->first) {
		case 0x0000 ... 0x0003: /* TVCT / CVCT */
			b_expecting_vct = true;
			b_attach_demux  = true;
			break;
		case 0x0100 ... 0x017f:	/* EIT-0 to EIT-127 */
			if ((scan_mode) && (!epg_mode))
				break;
			if ((eit_collection_limit == -1) || (eit_collection_limit >= iter->first - 0x0100)) {
				eit_pids[iter->second.pid] = iter->first - 0x0100;
				b_attach_demux  = true;
			}
			break;
		case 0x0200 ... 0x027f: /* ETT-0 to ETT-127 */
			break;

			if (scan_mode)
				break;
			/* FALL THRU */
		case 0x0004:            /* Channel ETT */
			b_attach_demux  = true;
			break;
		case 0x0301 ... 0x03ff: /* RRT w/ rating region 1-255 */
#if RRT
			b_attach_demux  = true;
#endif
			break;
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
		if ((b_attach_demux) && (iter->second.pid != PID_ATSC)) {
			if (h_demux.count(iter->second.pid))
#if 0
				dvbpsi_DetachDemux(h_demux[iter->second.pid]);
#else
			{} else
#endif
			add_filter(iter->second.pid);
			h_demux[iter->second.pid] = dvbpsi_AttachDemux(attach_table, this);
		}
	}
	expect_vct = b_expecting_vct;
	has_mgt = true;

	return true;
}

bool parse::take_nit_actual(dvbpsi_nit_t* p_nit, bool decoded)
{
	dprintf("(%s): v%d, network_id %d",
		(decoded) ? "post" : "pre",
		p_nit->i_version, p_nit->i_network_id);

	if (decoded) return true;

	has_nit = true;

	return true;
}

bool parse::take_nit_other(dvbpsi_nit_t* p_nit, bool decoded)
{
	dprintf("(%s): v%d, network_id %d",
		(decoded) ? "post" : "pre",
		p_nit->i_version, p_nit->i_network_id);

	if (decoded) return true;
#if 0
	has_nit = true;
#endif
	return true;
}

bool parse::take_sdt_actual(dvbpsi_sdt_t* p_sdt, bool decoded)
{
	dprintf("(%s): v%d | ts_id %d | network_id %d",
		(decoded) ? "post" : "pre",
		p_sdt->i_version, p_sdt->i_ts_id, p_sdt->i_network_id);

	if (!decoded) {
		set_ts_id(p_sdt->i_ts_id);
		return true;
	}

	has_sdt = true;

	return true;
}

bool parse::take_sdt_other(dvbpsi_sdt_t* p_sdt, bool decoded)
{
	dprintf("(%s): v%d | ts_id %d | network_id %d",
		(decoded) ? "post" : "pre",
		p_sdt->i_version, p_sdt->i_ts_id, p_sdt->i_network_id);

	if (!decoded) {
#if 0
		set_ts_id(p_sdt->i_ts_id);
#endif
		return true;
	}
#if 0
	has_sdt = true;
#endif
	return true;
}

bool parse::take_eit(dvbpsi_eit_t* p_eit, bool decoded)
{
	dprintf("(%s): v%d, service_id %d",
		(decoded) ? "post" : "pre",
		p_eit->i_version, p_eit->i_service_id);

	if (decoded) return true;

	return true;
}

bool parse::take_eit(dvbpsi_atsc_eit_t* p_eit, bool decoded)
{
	dprintf("(%s): v%d, source_id %d",
		(decoded) ? "post" : "pre",
		p_eit->i_version, p_eit->i_source_id);

	if (decoded) return true;

	return true;
}

bool parse::take_ett(dvbpsi_atsc_ett_t* p_ett, bool decoded)
{
	dprintf("(%s): v%d, ID: %d",
		(decoded) ? "post" : "pre",
		p_ett->i_version, p_ett->i_etm_id);

	if (decoded) return true;

	return true;
}

void parse::attach_table(dvbpsi_handle h_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
	if ((scan_mode) && (!epg_mode)) switch (i_table_id) {
	default:
		return;
	case TID_NIT_ACTUAL:
	case TID_NIT_OTHER:
	case TID_SDT_ACTUAL:
	case TID_SDT_OTHER:
	case TID_ATSC_TVCT:
	case TID_ATSC_CVCT:
	case TID_ATSC_MGT:
		break;
	}
	switch (i_table_id) {
	case 0x60 ... 0x6f: /* eit | other  | sched */
	case TID_EIT_OTHER:          /* eit | other  | p/f   */
	case 0x50 ... 0x5f: /* eit | actual | sched */
	case TID_EIT_ACTUAL:     /* eit | actual | p/f */
		dvbpsi_AttachEIT(h_dvbpsi, i_table_id, i_extension, take_eit, this);
		break;
	case TID_NIT_ACTUAL:
		dvbpsi_AttachNIT(h_dvbpsi, i_table_id, i_extension, take_nit_actual, this);
		break;
	case TID_NIT_OTHER:
		dvbpsi_AttachNIT(h_dvbpsi, i_table_id, i_extension, take_nit_other, this);
		break;
	case TID_SDT_ACTUAL:
		dvbpsi_AttachSDT(h_dvbpsi, i_table_id, i_extension, take_sdt_actual, this);
		break;
	case TID_SDT_OTHER:
		dvbpsi_AttachSDT(h_dvbpsi, i_table_id, i_extension, take_sdt_other, this);
		break;
	case TID_TDT:
	case TID_TOT:
		dvbpsi_AttachTOT(h_dvbpsi, i_table_id, i_extension, take_tot, this);
		break;
	case TID_ATSC_TVCT:
	case TID_ATSC_CVCT:
		dvbpsi_atsc_AttachVCT(h_dvbpsi, i_table_id, i_extension, take_vct, this);
		break;
	case TID_ATSC_EIT:
		dvbpsi_atsc_AttachEIT(h_dvbpsi, i_table_id, i_extension, take_eit, this);
		break;
	case TID_ATSC_ETT:
		dvbpsi_atsc_AttachETT(h_dvbpsi, i_table_id, i_extension, take_ett, this);
		break;
	case TID_ATSC_STT:
		dvbpsi_atsc_AttachSTT(h_dvbpsi, i_table_id, take_stt, this);
		break;
#ifdef RRT
	case TID_ATSC_RRT:
		dvbpsi_atsc_AttachRRT(h_dvbpsi, i_table_id, i_extension, take_rrt, this);
		break;
#endif
	case TID_ATSC_MGT:
		dvbpsi_atsc_AttachMGT(h_dvbpsi, i_table_id, i_extension, take_mgt, this);
		break;
	}
}

#if USE_STATIC_DECODE_MAP
#define define_table_wrapper(a, b, c)					\
void parse::a(void* p_this, b* p_table)					\
{									\
	parse* parser = (parse*)p_this;					\
	if ((parser) &&							\
	    (((parser->a(p_table, false)) && (parser->get_ts_id())) &&	\
	     (decoders[parser->get_ts_id()].a(p_table))))		\
		parser->a(p_table, true);				\
	c(p_table);							\
}
#else
#define define_table_wrapper(a, b, c)					\
void parse::a(void* p_this, b* p_table)					\
{									\
	parse* parser = (parse*)p_this;					\
	if ((parser) &&							\
	    (((parser->a(p_table, false)) && (parser->get_ts_id())) &&	\
	     (parser->decoders[parser->get_ts_id()].a(p_table))))	\
		parser->a(p_table, true);				\
	c(p_table);							\
}
#endif /* USE_STATIC_DECODE_MAP */

define_table_wrapper(take_pat, dvbpsi_pat_t, dvbpsi_DeletePAT);
define_table_wrapper(take_pmt, dvbpsi_pmt_t, dvbpsi_DeletePMT);
define_table_wrapper(take_eit, dvbpsi_eit_t, dvbpsi_DeleteEIT);
define_table_wrapper(take_nit_actual, dvbpsi_nit_t, dvbpsi_DeleteNIT);
define_table_wrapper(take_nit_other,  dvbpsi_nit_t, dvbpsi_DeleteNIT);
define_table_wrapper(take_sdt_actual, dvbpsi_sdt_t, dvbpsi_DeleteSDT);
define_table_wrapper(take_sdt_other,  dvbpsi_sdt_t, dvbpsi_DeleteSDT);
define_table_wrapper(take_tot, dvbpsi_tot_t, dvbpsi_DeleteTOT);
define_table_wrapper(take_vct, dvbpsi_atsc_vct_t, dvbpsi_atsc_DeleteVCT);
define_table_wrapper(take_eit, dvbpsi_atsc_eit_t, dvbpsi_atsc_DeleteEIT);
define_table_wrapper(take_ett, dvbpsi_atsc_ett_t, dvbpsi_atsc_DeleteETT);
define_table_wrapper(take_stt, dvbpsi_atsc_stt_t, dvbpsi_atsc_DeleteSTT);
define_table_wrapper(take_mgt, dvbpsi_atsc_mgt_t, dvbpsi_atsc_DeleteMGT);

void parse::attach_table(void* p_this, dvbpsi_handle h_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
	parse* parser = (parse*)p_this;
	if (parser)
		parser->attach_table(h_dvbpsi, i_table_id, i_extension);
}

#if 0
uint8_t parse::grab_next_eit(uint8_t current_eit_x)
{
	if (!eit_x_complete(current_eit_x))
		return current_eit_x;

	map_decoded_mgt_tables::const_iterator iter = decoders[ts_id].get_decoded_mgt()->tables.find(0x0100 + current_eit_x);

	if (iter != decoders[ts_id].get_decoded_mgt()->tables.end()) {
		map_dvbpsi::const_iterator iter_demux = h_demux.find(iter->second.pid);
		if (iter_demux != h_demux.end()) {
			//dvbpsi_DetachDemux(h_demux[iter->second.pid]);
			h_demux.erase(iter->second.pid);
		}
	}//else???


	if (current_eit_x == 0x7f)
		goto eit_complete;

	//map_decoded_mgt_tables::const_iterator
	iter = decoders[ts_id].get_decoded_mgt()->tables.find(0x0100 + current_eit_x + 1);

	if (iter == decoders[ts_id].get_decoded_mgt()->tables.end())
		goto eit_complete;

	h_demux[iter->second.pid] = dvbpsi_AttachDemux(attach_table, this);

	/* the range is 0x00 thru 0x7f, so 0x80 indicates error */
	return current_eit_x + 1;
eit_complete:
	fprintf(stderr, "%s: EIT COMPLETE: %d\n", __func__, current_eit_x);
	return 0x80; /* throw an error to signify last EIT */
}
#endif

parse::parse()
  : fed_pkt_count(0)
  , ts_id(0)
  , epg_mode(false)
  , scan_mode(false)
  , has_pat(false)
  , has_mgt(false)
  , has_vct(false)
  , has_sdt(false)
  , has_nit(false)
  , expect_vct(true)
  , dumped_eit(0)
  , eit_collection_limit(-1)
  , process_err_pkts(false)
  , addfilter_cb(NULL)
  , addfilter_context(NULL)
  , enabled(true)
  , rewritten_pat_ver_offset(0)
  , rewritten_pat_cont_ctr(0)
{
	dprintf("()");

	memset(&new_channel_info, 0, sizeof(channel_info_t));

	h_pat = dvbpsi_AttachPAT(take_pat, this);
#if 0
	h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
#endif
	h_demux[PID_EIT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
	h_demux[PID_NIT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_SDT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_TOT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode

	memset(&service_ids, 0, sizeof(service_ids));
	service_ids.clear();
}

parse::~parse()
{
	dprintf("()");
#if DBG
	xine_dump();
#endif
	cleanup();
#if DBG
	printpids();
#endif
#if 1//DBG
	if (fed_pkt_count)
		fprintf(stderr, "%d packets read in total\n", fed_pkt_count);
#endif
	service_ids.clear();
}

void parse::detach_demux()
{
	dprintf("()");

	for (map_dvbpsi::const_iterator iter = h_demux.begin(); iter != h_demux.end(); ++iter)
		dvbpsi_DetachDemux(iter->second);
	h_demux.clear();

	for (map_dvbpsi::const_iterator iter = h_pmt.begin(); iter != h_pmt.end(); ++iter)
		dvbpsi_DetachPMT(iter->second);
	h_pmt.clear();

	dvbpsi_DetachPAT(h_pat);

	clear_filters();
	service_ids.clear();
	payload_pids.clear();
}

void parse::stop()
{
	dprintf("()");

	out.stop();
}

void parse::cleanup()
{
	dprintf("()");

	detach_demux();
	clear_decoded_networks();
	decoders.clear();
	channel_info.clear();
}

void parse::reset_filters()
{
	add_filter(PID_ATSC);
	add_filter(PID_PAT);
	add_filter(PID_NIT);
	add_filter(PID_SDT);
	add_filter(PID_TOT);
	add_filter(PID_EIT);
}

void parse::reset()
{
	dprintf("()");

	detach_demux();

	memset(&new_channel_info, 0, sizeof(channel_info_t));

	ts_id = 0;
	dumped_eit = 0;
	has_pat = false;
	has_mgt = false;
	has_vct = false;
	has_sdt = false;
	has_nit = false;
	expect_vct = true;

	h_pat = dvbpsi_AttachPAT(take_pat, this);
#if 0
	h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
#endif
	h_demux[PID_EIT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
	h_demux[PID_NIT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_SDT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_TOT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
	reset_filters();
}

inline uint16_t tp_pkt_pid(uint8_t* pkt)
{
	return (pkt[0] == 0x47) ? ((uint16_t) (pkt[1] & 0x1f) << 8) + pkt[2] : (uint16_t) - 1;
}

static void xine_chandump(void *context,
			  uint16_t lcn, uint16_t major, uint16_t minor,
			  uint16_t physical_channel, uint32_t freq, const char *modulation,
			  unsigned char *service_name, uint16_t vpid, uint16_t apid, uint16_t program_number)
{
	char channelno[7]; /* XXX.XXX */
	if (major + minor > 1)
		sprintf(channelno, "%d.%d", major, minor);
	else if (lcn)
		sprintf(channelno, "%d", lcn);
	else
		sprintf(channelno, "%d", physical_channel);

	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		service_name,
		freq,//iter_vct->second.carrier_freq,
		modulation,
		vpid, apid, program_number);
}

unsigned int parse::xine_dump(uint16_t ts_id, channel_info_t* channel_info)
{
	uint32_t freq          = channel_info->frequency;
	uint16_t channel       = channel_info->channel;
	const char* modulation = channel_info->modulation;;

	int count = 0;

	fprintf(stdout, "\n# channel %d, %d, %s %s\n", channel, freq, "", "");

	if (decoders.count(ts_id))
	for (map_decoded_pat_programs::const_iterator iter_pat = decoders[ts_id].get_decoded_pat()->programs.begin();
	     iter_pat != decoders[ts_id].get_decoded_pat()->programs.end(); ++iter_pat) {
		int program_number = iter_pat->first;
		//int pmt_pid        = iter_pat->second;

		int apid = 0;
		int vpid = 0;

		map_decoded_pmt::const_iterator iter_pmt = decoders[ts_id].get_decoded_pmt()->find(program_number);
		if (iter_pmt == decoders[ts_id].get_decoded_pmt()->end())
			continue;
		//map_ts_elementary_streams::iterator iter_pmt_es = iter_pmt->second.es_streams.find(program_number);
		for (map_ts_elementary_streams::const_iterator iter_pmt_es = iter_pmt->second.es_streams.begin();
		     iter_pmt_es != iter_pmt->second.es_streams.end(); ++iter_pmt_es)
				switch (iter_pmt_es->second.type) {
#if 1
				case ST_VideoMpeg1:
				case ST_VideoMpeg4:
				case ST_VideoH264:
				case ST_ATSC_VideoMpeg2:
#endif
				case ST_VideoMpeg2:
					if (!vpid) vpid = iter_pmt_es->second.pid;
					break;
#if 1
				case ST_AudioMpeg1:
				case ST_AudioMpeg2:
				case ST_AudioAAC_ADTS:
				case ST_AudioAAC_LATM:
				case ST_ATSC_AudioEAC3:
#endif
				case ST_ATSC_AudioAC3:
					if (!apid) apid = iter_pmt_es->second.pid;
					break;
				}

		unsigned char service_name[256] = { 0 };
		service_name[7] = 0;
		uint16_t lcn = 0;
		uint16_t major = 0;
		uint16_t minor = 0;
		map_decoded_vct_channels::const_iterator iter_vct = decoders[ts_id].get_decoded_vct()->channels.find(program_number);
		if (iter_vct != decoders[ts_id].get_decoded_vct()->channels.end()) {
			major = iter_vct->second.chan_major;
			minor = iter_vct->second.chan_minor;
			for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		} else { // FIXME: use SDT info
			lcn = decoders[ts_id].get_lcn(program_number);

			decoded_sdt_t *decoded_sdt = (decoded_sdt_t*)decoders[ts_id].get_decoded_sdt();
			if ((decoded_sdt) && (decoded_sdt->services.count(program_number)))
				strcpy((char*)service_name, (const char *)decoded_sdt->services[program_number].service_name);
			else
				sprintf((char*)service_name, "%04d_UNKNOWN", program_number);
		}
		if (!chandump_cb)
			set_chandump_callback(xine_chandump);

		chandump_cb(chandump_context, lcn, major, minor, channel, freq, modulation, service_name, vpid, apid, program_number);
		count++;
	}
	return count;
}

typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;

unsigned int parse::xine_dump()
{
	map_chan_to_ts_id channels;
	int count = 0;
#if 0
	for (map_decoder::iterator iter = decoders.begin(); iter != decoders.end(); ++iter)
		count += iter->second.xine_dump(count, count, ""); // FIXME
#else
#if 0
	for (map_channel_info::iterator iter = channel_info.begin(); iter != channel_info.end(); ++iter)
		count += xine_dump(iter->first, &iter->second);
#else
	for (map_channel_info::iterator iter = channel_info.begin(); iter != channel_info.end(); ++iter)
		channels[iter->second.channel] = iter->first;

	for (map_chan_to_ts_id::iterator iter = channels.begin(); iter != channels.end(); ++iter)
		count += xine_dump(iter->second, &channel_info[iter->second]);

	channels.clear();
#endif
#endif
	return count;
}

void parse::epg_dump()
{
	map_chan_to_ts_id channels;
	//fprintf(stderr, "%s(%d, %d)\n", __func__, channel_info.size(), channels.size());

	for (map_channel_info::iterator iter = channel_info.begin(); iter != channel_info.end(); ++iter)
		channels[iter->second.channel] = iter->first;

	for (map_chan_to_ts_id::iterator iter = channels.begin(); iter != channels.end(); ++iter)
		if (decoders.count(iter->second)) decoders[iter->second].dump_epg();

	channels.clear();
}

bool parse::is_psip_ready()
{
	return ((has_pat) &&
		(((has_mgt) && ((has_vct) || (!expect_vct))) || ((has_sdt) && (has_nit))) &&
		((decoders.count(get_ts_id())) && (decoders[get_ts_id()].complete_pmt())));
};

bool parse::is_epg_ready()
{
	return ((is_psip_ready()) && ((decoders.count(get_ts_id()) && (decoders[get_ts_id()].got_all_eit(eit_collection_limit)))));
};

int parse::add_output(char* target)
{
	int ret = out.add(target);
	if (ret < 0)
		return ret;
	else
		return out.start();
}

void parse::set_service_ids(char *ids)
{
	char *save, *id = strtok_r(ids, ",", &save);

#if 1
	service_ids.clear();
#endif
	if (id) while (id) {
		if (!id)
			id = ids;
		set_service_id(strtoul(id, NULL, 0));
		id = strtok_r(NULL, ",", &save);
	} else
		set_service_id(strtoul(ids, NULL, 0));
#if 1
	if (has_pat) {
		rewrite_pat();
		payload_pids.clear();

		for (map_decoded_pat_programs::const_iterator iter =
		       decoders[ts_id].get_decoded_pat()->programs.begin();
		     iter != decoders[ts_id].get_decoded_pat()->programs.end(); ++iter)
			if (iter->first > 0) {// FIXME: > 0 ???
				if ((!service_ids.size()) || (service_ids.count(iter->first)))  {
					h_pmt[iter->second] = dvbpsi_AttachPMT(iter->first, take_pmt, this);
					add_filter(iter->second);
					//
					map_decoded_pmt::const_iterator iter_pmt =
						decoders[ts_id].get_decoded_pmt()->find(iter->first);
					if (iter_pmt != decoders[ts_id].get_decoded_pmt()->end())
						process_pmt(&iter_pmt->second);
				}
			}
	}
#endif
}

int parse::feed(int count, uint8_t* p_data)
{
	if (count <= 0) {
		/* no data! */
		return count;
	}
	if (!p_data) {
		/* no data pointer! */
		return -1;
	}

        uint8_t* p = p_data;
	if (!enabled)
		out.push(p, count);
	else
	/* one TS packet at a time */
        for (int i = count / 188; i > 0; --i) {
		uint16_t pid = tp_pkt_pid(p);
		bool send_pkt = false;
		unsigned int sync_offset = 0;
		output_options out_type = OUTPUT_NONE;

		while (((i > 0) && (pid == (uint16_t) - 1)) || ((i > 1) && (tp_pkt_pid(p+188) == (uint16_t) - 1))) {
			p++;
			sync_offset++;
			if (sync_offset == 188) {
				sync_offset = 0;
				i--;
			}
			pid = tp_pkt_pid(p);
			fprintf(stderr, ".\t");
		}

		if (p[1] & 0x80) {
			fprintf(stderr, "\tTEI\t");//"%s: TEI detected, dropping packet\n", __func__);
			if (!process_err_pkts) continue;
		}

		switch (pid) {
		case PID_PAT:
			dvbpsi_PushPacket(h_pat, p);
			send_pkt = (service_ids.size()) ? false : true;
			out_type = OUTPUT_PATPMT;
			if (!send_pkt) {
				pat_pkt[3] = (0x0f & ++rewritten_pat_cont_ctr) | 0x10;
				out.push(pat_pkt, out_type);
			}
			break;
		case PID_ATSC:
		case PID_NIT:
		case PID_SDT:
		case PID_TOT:
		case PID_EIT:
			send_pkt = true;
			out_type = OUTPUT_PSIP;
			/* fall-thru */
		default:
			map_dvbpsi::const_iterator iter;

			iter = h_pmt.find(pid);
			if (iter != h_pmt.end()) {
				dvbpsi_PushPacket(iter->second, p);
				send_pkt = true;
				out_type = OUTPUT_PATPMT;
				break;
			}

			map_eit_pids::const_iterator iter_eit;
			iter_eit = eit_pids.find(pid);
			if (iter_eit != eit_pids.end()) {

				if (decoders[ts_id].eit_x_complete(iter_eit->second)) {
					if (h_demux.count(iter_eit->first)) {
						dvbpsi_DetachDemux(h_demux[iter_eit->first]);
						h_demux.erase(iter_eit->first);
					}
					eit_pids.erase(iter_eit->first);
					//epg_complete = (eit_pids.size() == 0);
					continue;
				}
				decoders[ts_id].set_current_eit_x(iter_eit->second);
				out_type = OUTPUT_PSIP;
			}

			iter = h_demux.find(pid);
			if (iter != h_demux.end()) {
				dvbpsi_PushPacket(iter->second, p);
				send_pkt = true;
				//if (!out_type) out_type = OUTPUT_PSIP;
				break;
			}

#if 0
			map_pidtype::const_iterator iter_payload;
			iter_payload = payload_pids.find(pid);
			if (iter_payload != payload_pids.end()) {
#else
			if (payload_pids.count(pid)) {
#endif
				send_pkt = true;
				out_type = OUTPUT_PES;
				break;
			}
#if 0
			break;
#endif
		}
		if (send_pkt) {
			out.push(p, out_type);
		}
#if DBG
		addpid(pid);
#endif
		p += 188;
		fed_pkt_count++;
	}
#if 1//DBG
	while (((decoders.count(ts_id)) && (decoders[ts_id].eit_x_complete(dumped_eit)))) {
		decoders[ts_id].dump_eit_x(dumped_eit);
		dumped_eit++;
	}
#endif
	return 0;
}
