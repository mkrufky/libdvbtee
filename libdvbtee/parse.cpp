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
#if 1
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "functions.h"
#include "log.h"
#define CLASS_MODULE "parse"

const char *parse_libdvbpsi_version = EXPAND_AND_QUOTE(DVBPSI_VERSION);

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
	(void)p_stt;
	dprintf("(%s)", (decoded) ? "post" : "pre");

	if (decoded) return true;

	return true;
}

bool parse::take_tot(dvbpsi_tot_t* p_tot, bool decoded)
{
	(void)p_tot;
	dprintf("(%s)", (decoded) ? "post" : "pre");

	if (decoded) return true;

	return true;
}

void parse::rewrite_pat()
{
	if (0 == service_ids.size())
		return;

#if !USING_DVBPSI_VERSION_0
	dvbpsi_class dvbpsi;
#else
#define dvbpsi_pat_init(a, b, c, d)     dvbpsi_InitPAT(a, b, c, d)
#define dvbpsi_pat_program_add(a, b, c) dvbpsi_PATAddProgram(a, b, c)
#define dvbpsi_pat_sections_generate(a, b, c) dvbpsi_GenPATSections(b, c)
#define dvbpsi_pat_empty(a) dvbpsi_EmptyPAT(a)
#endif
	dvbpsi_pat_t pat;
	dvbpsi_psi_section_t* p_section;
	const decoded_pat_t *decoded_pat = decoders[ts_id].get_decoded_pat();

	if (rewritten_pat_ver_offset == 0x1e)
		rewritten_pat_ver_offset = 0;

	dvbpsi_pat_init(&pat, ts_id, 0x1f & (++rewritten_pat_ver_offset + decoded_pat->version), 1);

	for (map_pidtype::const_iterator iter = service_ids.begin(); iter != service_ids.end(); ++iter)
		dvbpsi_pat_program_add(&pat, iter->first, ((decoded_pat_t *) decoded_pat)->programs[iter->first]);

	p_section = dvbpsi_pat_sections_generate(dvbpsi.get_handle(), &pat, 0);
	pat_pkt[0] = 0x47;
	pat_pkt[1] = pat_pkt[2] = pat_pkt[3] = 0x00;
	writePSI(pat_pkt, p_section);
	dvbpsi_DeletePSISections(p_section);
	dvbpsi_pat_empty(&pat);
}

void parse::process_pat(const decoded_pat_t *decoded_pat)
{
	dprintf("()");
	for (map_decoded_pat_programs::const_iterator iter = decoded_pat->programs.begin();
	     iter != decoded_pat->programs.end(); ++iter)
		if (iter->first > 0) {// FIXME: > 0 ???
			if ((!service_ids.size()) || (service_ids.count(iter->first)))  {
#if USING_DVBPSI_VERSION_0
				h_pmt[iter->second] = dvbpsi_AttachPMT(iter->first, take_pmt, this);
#else
				if (h_pmt.count(iter->second)) {
					if (dvbpsi_decoder_present(h_pmt[iter->second].get_handle()))
						dvbpsi_pmt_detach(h_pmt[iter->second].get_handle());
					h_pmt.erase(iter->second);
				}

				if (!dvbpsi_decoder_present(h_pmt[iter->second].get_handle()))
					dvbpsi_pmt_attach(h_pmt[iter->second].get_handle(), iter->first, take_pmt, this);
#endif
				add_filter(iter->second);
				rcvd_pmt[iter->first] = false;
			}
		}
}

bool parse::take_pat(dvbpsi_pat_t* p_pat, bool decoded)
{
	dprintf("(%s): v%d, ts_id: %d",
		(decoded) ? "post" : "pre",
		p_pat->i_version, p_pat->i_ts_id);

	if (!decoded) {
		set_ts_id(p_pat->i_ts_id);
#if USING_DVBPSI_VERSION_0
		h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
#else
		if (h_demux.count(PID_ATSC)) {
			h_demux[PID_ATSC].detach_demux();
			h_demux.erase(PID_ATSC);
		}

		if (!dvbpsi_decoder_present(h_demux[PID_ATSC].get_handle()))
			dvbpsi_AttachDemux(h_demux[PID_ATSC].get_handle(), attach_table, this);
#endif
		return true;
	}

	process_pat(decoders[p_pat->i_ts_id].get_decoded_pat());

	rewrite_pat();

	has_pat = true;

	return true;
}

void parse::process_pmt(const decoded_pmt_t *pmt)
{
	dprintf(": v%d, service_id %d, pcr_pid %d",
		pmt->version, pmt->program, pmt->pcr_pid);

	for (map_ts_elementary_streams::const_iterator iter_pmt_es = pmt->es_streams.begin();
	     iter_pmt_es != pmt->es_streams.end(); ++iter_pmt_es) {
			/* if we're going to stream a program, make sure its pid is coming thru */
			payload_pids[iter_pmt_es->second.pid] = iter_pmt_es->second.type;
			add_filter(iter_pmt_es->second.pid);
	}
}

bool parse::take_pmt(dvbpsi_pmt_t* p_pmt, bool decoded)
{
	dprintf("(%s): v%d, service_id %d, pcr_pid %d",
		(decoded) ? "post" : "pre",
		p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);

	if (!decoded) return true;

	const map_decoded_pmt* decoded_pmt = decoders[ts_id].get_decoded_pmt();

	map_decoded_pmt::const_iterator iter_pmt = decoded_pmt->find(p_pmt->i_program_number);
	if (iter_pmt != decoded_pmt->end())
		process_pmt(&iter_pmt->second);

	rcvd_pmt[p_pmt->i_program_number] = true;

	return true;
}

bool parse::take_vct(dvbpsi_atsc_vct_t* p_vct, bool decoded)
{
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = p_vct->i_ts_id;
#else
	uint16_t __ts_id = p_vct->i_extension;
#endif
	dprintf("(%s): v%d, ts_id %d, b_cable_vct %d",
		(decoded) ? "post" : "pre",
		p_vct->i_version, __ts_id, p_vct->b_cable_vct);

	if (!decoded) {
		set_ts_id(__ts_id);
		return true;
	}
	has_vct = true;

	return true;
}

void parse::process_mgt(bool attach)
{
	const decoded_mgt_t* decoded_mgt = decoders[ts_id].get_decoded_mgt();

	bool b_expecting_vct = false;

	eit_pids.clear();
	if (!decoded_mgt)
		fprintf(stderr, "%s: decoded_mgt is NULL!!!\n", __func__);
	else for (map_decoded_mgt_tables::const_iterator iter =
	       decoded_mgt->tables.begin();
	     iter != decoded_mgt->tables.end(); ++iter) {

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
			if (dont_collect_ett)
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

#if USING_DVBPSI_VERSION_0
			if (h_demux.count(iter->second.pid))
#if 0
				dvbpsi_DetachDemux(h_demux[iter->second.pid]);
#else
			{} else
#endif
#else
			if (h_demux.count(iter->second.pid)) {
				h_demux[iter->second.pid].detach_demux();
				h_demux.erase(iter->second.pid);
			}

			if ((attach) && (!dvbpsi_decoder_present(h_demux[iter->second.pid].get_handle())))
				dvbpsi_AttachDemux(h_demux[iter->second.pid].get_handle(), attach_table, this);
#endif
			add_filter(iter->second.pid);
#if USING_DVBPSI_VERSION_0
			h_demux[iter->second.pid] = dvbpsi_AttachDemux(attach_table, this);
#endif
		}
	}
	expect_vct = b_expecting_vct;
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
	process_mgt(true);

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
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = p_sdt->i_ts_id;
#else
	uint16_t __ts_id = p_sdt->i_extension;
#endif
	dprintf("(%s): v%d | ts_id %d | network_id %d",
		(decoded) ? "post" : "pre",
		p_sdt->i_version, __ts_id, p_sdt->i_network_id);

	if (!decoded) {
		set_ts_id(__ts_id);
		return true;
	}

	has_sdt = true;

	return true;
}

bool parse::take_sdt_other(dvbpsi_sdt_t* p_sdt, bool decoded)
{
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = p_sdt->i_ts_id;
#else
	uint16_t __ts_id = p_sdt->i_extension;
#endif
	dprintf("(%s): v%d | ts_id %d | network_id %d",
		(decoded) ? "post" : "pre",
		p_sdt->i_version, __ts_id, p_sdt->i_network_id);
#if 0
	if (!decoded) {
		set_ts_id(p_sdt->i_ts_id);
		return true;
	}
#endif
#if 0
	has_sdt = true;
#endif
	return true;
}

bool parse::take_eit(dvbpsi_eit_t* p_eit, bool decoded)
{
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = p_eit->i_ts_id;
#else
	uint16_t __ts_id = p_eit->i_extension;
#endif
	dprintf("(%s): v%d, service_id %d",
		(decoded) ? "post" : "pre",
		p_eit->i_version, __ts_id);

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

#if USING_DVBPSI_VERSION_0
#define attach_table_auto_detach_wrapper(container, attach, detach, take, table_id, extension) \
	attach(h_dvbpsi, table_id, extension, take, this)
#define attach_table_auto_detach_wrapper_noext(container, attach, detach, take, table_id, extension) \
	attach(h_dvbpsi, table_id, take, this)
#define dvbpsi_eit_attach dvbpsi_AttachEIT
#define dvbpsi_nit_attach dvbpsi_AttachNIT
#define dvbpsi_sdt_attach dvbpsi_AttachSDT
#define dvbpsi_tot_attach dvbpsi_AttachTOT
#else
#define attach_table_auto_detach_wrapper(container, attach, detach, take, table_id, extension) \
	if (container) { \
		attach_table_auto_detach(container, attach, detach, take, table_id, extension); \
	} else { \
		attach(p_dvbpsi, table_id, extension, take, this); \
	}
#define attach_table_auto_detach_wrapper_noext attach_table_auto_detach_wrapper
#endif

#if USING_DVBPSI_VERSION_0
void parse::attach_table(dvbpsi_handle h_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
#else
void parse::attach_table(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
#endif
{
#if !USING_DVBPSI_VERSION_0
	dvbpsi_class *container = NULL;
	for (map_dvbpsi::iterator iter = h_demux.begin(); iter != h_demux.end(); ++iter)
		if (iter->second.get_handle() == p_dvbpsi)
			container = &iter->second;
	if (!container) fprintf(stdout, "%s: CONTAINER NOT FOUND FOR %02x|%04x!\n", __func__, i_table_id, i_extension);
	//uint32_t idx = (((i_table_id << 16) & 0x00ff0000) | (i_extension & 0x0000ffff));
#endif
	if ((scan_mode) && (!epg_mode)) switch (i_table_id) {
#if 0 /* already handled */
	case 0x00: // PAT
	case 0x01: // CAT
	case 0x02: // program_map_section
	case 0x03: // transport_stream_description_section
	case 0x04: /* ISO_IEC_14496_scene_description_section */
	case 0x05: /* ISO_IEC_14496_object_descriptor_section */
	case 0x06: /* Metadata_section */
	case 0x07: /* IPMP_Control_Information_section (defined in ISO/IEC 13818-11) */
	/* 0x08-0x3F: ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved */
#endif
	default:
		return;
	case TID_NIT_ACTUAL:	// 0x40: // NIT network_information_section - actual_network
	case TID_NIT_OTHER:	// 0x41: // NIT network_information_section - other_network
	case TID_SDT_ACTUAL:	// 0x42: // SDT service_description_section - actual_transport_stream
	case TID_SDT_OTHER:	// 0x46: // SDT service_description_section - other_transport_stream
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
		attach_table_auto_detach_wrapper(container, dvbpsi_eit_attach, dvbpsi_eit_detach, take_eit, i_table_id, i_extension);
		break;
	case TID_NIT_ACTUAL:
		attach_table_auto_detach_wrapper(container, dvbpsi_nit_attach, dvbpsi_nit_detach, take_nit_actual, i_table_id, i_extension);
		break;
	case TID_NIT_OTHER:
		attach_table_auto_detach_wrapper(container, dvbpsi_nit_attach, dvbpsi_nit_detach, take_nit_other, i_table_id, i_extension);
		break;
	case TID_SDT_ACTUAL:
		attach_table_auto_detach_wrapper(container, dvbpsi_sdt_attach, dvbpsi_sdt_detach, take_sdt_actual, i_table_id, i_extension);
		break;
	case TID_SDT_OTHER:
		attach_table_auto_detach_wrapper(container, dvbpsi_sdt_attach, dvbpsi_sdt_detach, take_sdt_other, i_table_id, i_extension);
		break;
	case TID_TDT:
	case TID_TOT:
		attach_table_auto_detach_wrapper(container, dvbpsi_tot_attach, dvbpsi_tot_detach, take_tot, i_table_id, i_extension);
		break;
	case TID_ATSC_TVCT:
	case TID_ATSC_CVCT:
		attach_table_auto_detach_wrapper(container, dvbpsi_atsc_AttachVCT, dvbpsi_atsc_DetachVCT, take_vct, i_table_id, i_extension);
		break;
	case TID_ATSC_EIT:
		attach_table_auto_detach_wrapper(container, dvbpsi_atsc_AttachEIT, dvbpsi_atsc_DetachEIT, take_eit, i_table_id, i_extension);
		break;
#ifdef ETT
	case TID_ATSC_ETT:
		attach_table_auto_detach_wrapper(container, dvbpsi_atsc_AttachETT, dvbpsi_atsc_DetachETT, take_ett, i_table_id, i_extension);
		break;
#endif
	case TID_ATSC_STT:
		attach_table_auto_detach_wrapper_noext(container, dvbpsi_atsc_AttachSTT, dvbpsi_atsc_DetachSTT, take_stt, i_table_id, i_extension);
		break;
#ifdef RRT
	case TID_ATSC_RRT:
		attach_table_auto_detach_wrapper(container, dvbpsi_atsc_AttachRRT, dvbpsi_atsc_DetachRRT, take_rrt, i_table_id, i_extension);
		break;
#endif
	case TID_ATSC_MGT:
		attach_table_auto_detach_wrapper(container, dvbpsi_atsc_AttachMGT, dvbpsi_atsc_DetachMGT, take_mgt, i_table_id, i_extension);
		break;
	}
}

#if USE_STATIC_DECODE_MAP
#define define_table_wrapper(a, b, c, d)				\
void parse::a(void* p_this, b* p_table)					\
{									\
	parse* parser = (parse*)p_this;					\
	if ((parser) &&							\
	    (((parser->a(p_table, false)) && (parser->get_ts_id())) &&	\
	     ((decoders[parser->get_ts_id()].a(p_table)) ||		\
	      (!parser->d))))						\
		parser->a(p_table, true);				\
	c(p_table);							\
}
#else
#define define_table_wrapper(a, b, c, d)				\
void parse::a(void* p_this, b* p_table)					\
{									\
	parse* parser = (parse*)p_this;					\
	if ((parser) &&							\
	    (((parser->a(p_table, false)) && (parser->get_ts_id())) &&	\
	     ((parser->decoders[parser->get_ts_id()].a(p_table)) ||	\
	      (!parser->d))))						\
		parser->a(p_table, true);				\
	c(p_table);							\
}
#endif /* USE_STATIC_DECODE_MAP */

#if USING_DVBPSI_VERSION_0
#define dvbpsi_pat_delete dvbpsi_DeletePAT
#define dvbpsi_pmt_delete dvbpsi_DeletePMT
#define dvbpsi_eit_delete dvbpsi_DeleteEIT
#define dvbpsi_nit_delete dvbpsi_DeleteNIT
#define dvbpsi_sdt_delete dvbpsi_DeleteSDT
#define dvbpsi_tot_delete dvbpsi_DeleteTOT
#endif

define_table_wrapper(take_pat, dvbpsi_pat_t, dvbpsi_pat_delete, has_pat)
define_table_wrapper(take_pmt, dvbpsi_pmt_t, dvbpsi_pmt_delete, is_pmt_ready(p_table->i_program_number))
define_table_wrapper(take_eit, dvbpsi_eit_t, dvbpsi_eit_delete, enabled)
define_table_wrapper(take_nit_actual, dvbpsi_nit_t, dvbpsi_nit_delete, has_nit)
define_table_wrapper(take_nit_other,  dvbpsi_nit_t, dvbpsi_nit_delete, enabled)
define_table_wrapper(take_sdt_actual, dvbpsi_sdt_t, dvbpsi_sdt_delete, has_sdt)
define_table_wrapper(take_sdt_other,  dvbpsi_sdt_t, dvbpsi_sdt_delete, enabled)
define_table_wrapper(take_tot, dvbpsi_tot_t, dvbpsi_tot_delete, enabled)
define_table_wrapper(take_vct, dvbpsi_atsc_vct_t, dvbpsi_atsc_DeleteVCT, has_vct)
define_table_wrapper(take_eit, dvbpsi_atsc_eit_t, dvbpsi_atsc_DeleteEIT, enabled)
define_table_wrapper(take_ett, dvbpsi_atsc_ett_t, dvbpsi_atsc_DeleteETT, enabled)
define_table_wrapper(take_stt, dvbpsi_atsc_stt_t, dvbpsi_atsc_DeleteSTT, enabled)
define_table_wrapper(take_mgt, dvbpsi_atsc_mgt_t, dvbpsi_atsc_DeleteMGT, has_mgt)

#if USING_DVBPSI_VERSION_0
void parse::attach_table(void* p_this, dvbpsi_handle h_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
	parse* parser = (parse*)p_this;
	if (parser)
		parser->attach_table(h_dvbpsi, i_table_id, i_extension);
}
#else
void parse::attach_table(dvbpsi_t *p_dvbpsi, uint8_t i_table_id, uint16_t i_extension, void *p_data)
{
	parse* parser = (parse*)p_data;
	if (parser)
		parser->attach_table(p_dvbpsi, i_table_id, i_extension);
}
#endif

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

static bool hello = false;

parse::parse()
  : statistics(CLASS_MODULE)
  , fed_pkt_count(0)
  , ts_id(0)
  , epg_mode(false)
  , scan_mode(false)
  , dont_collect_ett(true)
  , has_pat(false)
  , has_mgt(false)
  , has_vct(false)
  , has_sdt(false)
  , has_nit(false)
  , expect_vct(true)
  , dumped_eit(0)
  , eit_collection_limit(-1)
  , process_err_pkts(false)
  , tei_count(0)
  , m_tsfilter_iface(NULL)
  , enabled(true)
  , rewritten_pat_ver_offset(0)
  , rewritten_pat_cont_ctr(0)
{
	if (!hello)
		fprintf(stdout, "# dvbtee v" LIBDVBTEE_VERSION
#if 0
			", built " __DATE__ " " __TIME__
#endif
			" - http://github.com/mkrufky/libdvbtee\n\n");
	hello = true;
	dprintf("()");

	memset(&new_channel_info, 0, sizeof(channel_info_t));

#if USING_DVBPSI_VERSION_0
	h_pat = dvbpsi_AttachPAT(take_pat, this);
#if 0
	h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
#endif
	h_demux[PID_EIT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
	h_demux[PID_NIT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_SDT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_TOT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
#else
	dvbpsi_pat_attach(h_pat.get_handle(), take_pat, this);
#if 0
	dvbpsi_AttachDemux(h_demux[PID_ATSC].get_handle(), attach_table, this);
#endif
	dvbpsi_AttachDemux(h_demux[PID_EIT].get_handle(), attach_table, this);//if !scan_mode
	dvbpsi_AttachDemux(h_demux[PID_NIT].get_handle(), attach_table, this);
	dvbpsi_AttachDemux(h_demux[PID_SDT].get_handle(), attach_table, this);
	dvbpsi_AttachDemux(h_demux[PID_TOT].get_handle(), attach_table, this);//if !scan_mode
#endif

	service_ids.clear();
	rcvd_pmt.clear();
	out_pids.clear();
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
	rcvd_pmt.clear();
	out_pids.clear();
}

void parse::detach_demux()
{
	dprintf("()");

#if USING_DVBPSI_VERSION_0
	for (map_dvbpsi::const_iterator iter = h_demux.begin(); iter != h_demux.end(); ++iter)
		dvbpsi_DetachDemux(iter->second);
#else
	for (map_dvbpsi::iterator iter = h_demux.begin(); iter != h_demux.end(); ++iter)
		iter->second.detach_demux();
#endif
	h_demux.clear();

#if USING_DVBPSI_VERSION_0
	for (map_dvbpsi::const_iterator iter = h_pmt.begin(); iter != h_pmt.end(); ++iter)
	dvbpsi_DetachPMT(iter->second);
#else
	for (map_dvbpsi::iterator iter = h_pmt.begin(); iter != h_pmt.end(); ++iter) {
		if (dvbpsi_decoder_present(iter->second.get_handle()))
			dvbpsi_pmt_detach(iter->second.get_handle());
	}
#endif
	h_pmt.clear();

#if USING_DVBPSI_VERSION_0
	dvbpsi_DetachPAT(h_pat);
#else
	if (dvbpsi_decoder_present(h_pat.get_handle()))
		dvbpsi_pat_detach(h_pat.get_handle());
	h_pat.purge();
#endif

	clear_filters();
	service_ids.clear();
	rcvd_pmt.clear();
	payload_pids.clear();
	out_pids.clear();
}

void parse::stop()
{
	dprintf("()");

	out.stop();
}

void parse::stop(int id)
{
	dprintf("(%d)", id);

	out.stop(id);
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
	tei_count = 0;
	has_pat = false;
	has_mgt = false;
	has_vct = false;
	has_sdt = false;
	has_nit = false;
	expect_vct = true;

#if USING_DVBPSI_VERSION_0
	h_pat = dvbpsi_AttachPAT(take_pat, this);
	h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_EIT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
	h_demux[PID_NIT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_SDT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_TOT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
#else
	if (!dvbpsi_decoder_present(h_pat.get_handle()))
		dvbpsi_pat_attach(h_pat.get_handle(), take_pat, this);
#if 0
	if (!dvbpsi_decoder_present(h_demux[PID_ATSC].get_handle()))
		dvbpsi_AttachDemux(h_demux[PID_ATSC].get_handle(), attach_table, this);
#endif
	if (!dvbpsi_decoder_present(h_demux[PID_EIT].get_handle()))
		dvbpsi_AttachDemux(h_demux[PID_EIT].get_handle(), attach_table, this);//if !scan_mode
	if (!dvbpsi_decoder_present(h_demux[PID_NIT].get_handle()))
		dvbpsi_AttachDemux(h_demux[PID_NIT].get_handle(), attach_table, this);
	if (!dvbpsi_decoder_present(h_demux[PID_SDT].get_handle()))
		dvbpsi_AttachDemux(h_demux[PID_SDT].get_handle(), attach_table, this);
	if (!dvbpsi_decoder_present(h_demux[PID_TOT].get_handle()))
		dvbpsi_AttachDemux(h_demux[PID_TOT].get_handle(), attach_table, this);//if !scan_mode
#endif

	reset_filters();
}

inline uint16_t tp_pkt_pid(uint8_t* pkt)
{
	return (pkt[0] == 0x47) ? ((uint16_t) (pkt[1] & 0x1f) << 8) + pkt[2] : (uint16_t) - 1;
}

static const char * xine_chandump(parsed_channel_info_t *c)
{
	char channelno[7]; /* XXX.XXX */
	if (c->major + c->minor > 1)
		sprintf(channelno, "%d.%d", c->major, c->minor);
	else if (c->lcn)
		sprintf(channelno, "%d", c->lcn);
	else
		sprintf(channelno, "%d", c->physical_channel);

	fprintf(stdout, "%s-%s:%d:%s:%d:%d:%d\n",
		channelno,
		c->service_name,
		c->freq,//iter_vct->second.carrier_freq,
		c->modulation,
		c->vpid, c->apid, c->program_number);

	return NULL;
}

void parse::parse_channel_info(const uint16_t ts_id, const decoded_pmt_t* decoded_pmt, const decoded_vct_t* decoded_vct, parsed_channel_info_t& c)
{
	//map_ts_elementary_streams::iterator iter_pmt_es = decoded_pmt->es_streams.find(program_number);
	for (map_ts_elementary_streams::const_iterator iter_pmt_es = decoded_pmt->es_streams.begin();
	     iter_pmt_es != decoded_pmt->es_streams.end(); ++iter_pmt_es)
			switch (iter_pmt_es->second.type) {
#if 1
				case ST_VideoMpeg1:
				case ST_VideoMpeg4:
				case ST_VideoH264:
				case ST_ATSC_VideoMpeg2:
#endif
				case ST_VideoMpeg2:
					if (!c.vpid) c.vpid = iter_pmt_es->second.pid;
					break;
#if 1
				case ST_AudioMpeg1:
				case ST_AudioMpeg2:
				case ST_AudioAAC_ADTS:
				case ST_AudioAAC_LATM:
				case ST_ATSC_AudioEAC3:
#endif
				case ST_ATSC_AudioAC3:
					if (!c.apid) c.apid = iter_pmt_es->second.pid;
					break;
			}

	c.lcn = 0;
	c.major = 0;
	c.minor = 0;
	map_decoded_vct_channels::const_iterator iter_vct = decoded_vct->channels.find(c.program_number);
	if (iter_vct != decoded_vct->channels.end()) {
		c.major = iter_vct->second.chan_major;
		c.minor = iter_vct->second.chan_minor;
		for ( int i = 0; i < 7; ++i ) c.service_name[i] = iter_vct->second.short_name[i*2+1];
		c.service_name[7] = 0;
	} else { // FIXME: use SDT info
		c.lcn = decoders[ts_id].get_lcn(c.program_number);

		decoded_sdt_t *decoded_sdt = (decoded_sdt_t*)decoders[ts_id].get_decoded_sdt();
		if ((decoded_sdt) && (decoded_sdt->services.count(c.program_number)))
			snprintf((char*)c.service_name, sizeof(c.service_name), "%s", decoded_sdt->services[c.program_number].service_name);
		else {
			snprintf((char*)c.service_name, sizeof(c.service_name), "%04d_UNKNOWN", c.program_number);
		}
	}
}

unsigned int parse::xine_dump(uint16_t ts_id, channel_info_t* channel_info, parse_iface *iface)
{
	parsed_channel_info_t c;
	c.freq             = channel_info->frequency;
	c.physical_channel = channel_info->channel;
	c.modulation       = channel_info->modulation;

	int count = 0;

	const decoded_pat_t* decoded_pat = decoders[ts_id].get_decoded_pat();
	const map_decoded_pmt* decoded_pmt = decoders[ts_id].get_decoded_pmt();
	const decoded_vct_t* decoded_vct = decoders[ts_id].get_decoded_vct();

	fprintf(stdout, "\n# channel %d, %d, %s %s\n", c.physical_channel, c.freq, "", "");

	if (decoders.count(ts_id))
	for (map_decoded_pat_programs::const_iterator iter_pat = decoded_pat->programs.begin();
	     iter_pat != decoded_pat->programs.end(); ++iter_pat) {
		c.program_number = iter_pat->first;
		//int pmt_pid        = iter_pat->second;

		c.apid = 0;
		c.vpid = 0;

		map_decoded_pmt::const_iterator iter_pmt = decoded_pmt->find(c.program_number);
		if (iter_pmt == decoded_pmt->end())
			continue;

		parse_channel_info(ts_id, &iter_pmt->second, decoded_vct, c);

		if (iface)
			iface->chandump(&c);
		else
			xine_chandump(&c);
		count++;
	}
	return count;
}

typedef std::map<unsigned int, uint16_t> map_chan_to_ts_id;

unsigned int parse::xine_dump(parse_iface *iface)
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
		count += xine_dump(iter->second, &channel_info[iter->second], iface);

	channels.clear();
#endif
#endif
	return count;
}

void parse::epg_dump(decode_report *reporter)
{
	map_chan_to_ts_id channels;
	//fprintf(stderr, "%s(%d, %d)\n", __func__, channel_info.size(), channels.size());

	for (map_channel_info::iterator iter = channel_info.begin(); iter != channel_info.end(); ++iter)
		channels[iter->second.channel] = iter->first;

	for (map_chan_to_ts_id::iterator iter = channels.begin(); iter != channels.end(); ++iter)
		if (decoders.count(iter->second)) decoders[iter->second].dump_epg(reporter);

	channels.clear();

	//fprintf(stderr, "%s", str.c_str());

	return;
}

bool parse::get_stream_info(unsigned int channel, uint16_t service, parsed_channel_info_t *c, decoded_event_t *e0, decoded_event_t *e1)
{
	if (!service)
		return false;

	uint16_t requested_ts_id = get_ts_id(channel);

	if (!channel_info.count(requested_ts_id))
		return false;

	const map_decoded_pmt* decoded_pmt = decoders[requested_ts_id].get_decoded_pmt();
	map_decoded_pmt::const_iterator iter_pmt = decoded_pmt->find(service);
	if (iter_pmt == decoded_pmt->end())
		return false;

	const decoded_vct_t* decoded_vct = decoders[requested_ts_id].get_decoded_vct();

	channel_info_t *info = &channel_info[requested_ts_id];

	if (c) {
		c->physical_channel = info->channel;
		c->freq             = info->frequency;
		c->modulation       = info->modulation;
		//
		c->program_number   = service;
		c->apid = 0;
		c->vpid = 0;
		//
		parse_channel_info(requested_ts_id, &iter_pmt->second, decoded_vct, *c);
	}

	time_t last;

	time(&last);

	if (e0) {
		decoders[requested_ts_id].get_epg_event(service, last, e0);
		last = e0->start_time + e0->length_sec + 1;
	}
	if (e1)
		decoders[requested_ts_id].get_epg_event(service, last, e1);

	return true;
}

bool parse::is_pmt_ready(uint16_t id)
{
#if 0
	return (has_pat && decoders[get_ts_id()].complete_pmt());
#endif
	if ((!has_pat) || (!rcvd_pmt.size()))
		return false;

	if (id) return (rcvd_pmt.count(id) && rcvd_pmt[id]);

	for (map_rcvd::const_iterator iter = rcvd_pmt.begin(); iter != rcvd_pmt.end(); ++iter)
		if ((iter->first) && (!iter->second)) {
#if DBG
			fprintf(stderr, "%s: missing pmt for program %d\n", __func__, iter->first);
#endif
			return false;
		}
	return true;

}

bool parse::is_psip_ready()
{
	return ((is_basic_psip_ready()) && ((decoders.count(get_ts_id())) && (is_pmt_ready())));
}

bool parse::is_epg_ready()
{
	return ((is_psip_ready()) && ((decoders.count(get_ts_id()) && (decoders[get_ts_id()].got_all_eit(eit_collection_limit)))));
}

int parse::add_output(void* priv, stream_callback callback)
{
	map_pidtype pids;
	add_service_pids(pids);
	return add_output(priv, callback, pids);
}

int parse::add_output(int socket, unsigned int method)
{
	map_pidtype pids;
	add_service_pids(pids);
	return add_output(socket, method, pids);
}

int parse::add_output(char* target)
{
	map_pidtype pids;
	add_service_pids(pids);
	return add_output(target, pids);
}

int parse::add_output(void* priv, stream_callback callback, uint16_t service)
{
	map_pidtype pids;
	if (service)
		add_service_pids(service, pids);

	return add_output(priv, callback, pids);
}

int parse::add_output(int socket, unsigned int method, uint16_t service)
{
	map_pidtype pids;
	if (service)
		add_service_pids(service, pids);

	return add_output(socket, method, pids);
}

int parse::add_output(char* target, uint16_t service)
{
	map_pidtype pids;
	if (service)
		add_service_pids(service, pids);

	return add_output(target, pids);
}

int parse::add_output(void* priv, stream_callback callback, char* services)
{
	map_pidtype pids;
	if (services)
		add_service_pids(services, pids);

	return add_output(priv, callback, pids);
}

int parse::add_output(int socket, unsigned int method, char* services)
{
	map_pidtype pids;
	if (services)
		add_service_pids(services, pids);

	return add_output(socket, method, pids);
}

int parse::add_output(char* target, char* services)
{
	map_pidtype pids;
	if (services)
		add_service_pids(services, pids);

	return add_output(target, pids);
}

int parse::add_output(void* priv, stream_callback callback, map_pidtype &pids)
{
	int ret, target_id = out.add(priv, callback, pids);
	if (target_id < 0)
		goto fail;

	ret = out.start();
	if (ret < 0)
		return ret;

	out.get_pids(out_pids);
	dprintf("success adding callback target id:%4d", target_id);
fail:
	return target_id;
}

int parse::add_output(int socket, unsigned int method, map_pidtype &pids)
{
	int ret, target_id = out.add(socket, method, pids);
	if (target_id < 0)
		goto fail;

	ret = out.start();
	if (ret < 0)
		return ret;

	out.get_pids(out_pids);
	dprintf("success adding socket target id:%4d", target_id);
fail:
	return target_id;
}

int parse::add_output(char* target, map_pidtype &pids)
{
	int ret, target_id = out.add(target, pids);
	if (target_id < 0)
		goto fail;

	ret = out.start();
	if (ret < 0)
		return ret;

	out.get_pids(out_pids);
	dprintf("success adding url target id:%4d", target_id);
fail:
	return target_id;
}

int parse::add_stdout()
{
	map_pidtype pids;
	add_service_pids(pids);
	return add_stdout(pids);
}

int parse::add_stdout(uint16_t service)
{
	map_pidtype pids;
	if (service)
		add_service_pids(service, pids);

	return add_stdout(pids);
}

int parse::add_stdout(char* services)
{
	map_pidtype pids;
	if (services)
		add_service_pids(services, pids);

	return add_stdout(pids);
}

int parse::add_stdout(map_pidtype &pids)
{
	int ret, target_id = out.add_stdout(pids);
	if (target_id < 0)
		goto fail;

	ret = out.start();
	if (ret < 0)
		return ret;

	out.get_pids(out_pids);
	dprintf("success adding stdout target id:%4d", target_id);
fail:
	return target_id;
}

#define CHAR_CMD_COMMA ","

void parse::add_service_pids(uint16_t service_id, map_pidtype &pids)
{
	const decoded_pat_t* decoded_pat = decoders[ts_id].get_decoded_pat();
	map_decoded_pat_programs::const_iterator iter_pat = decoded_pat->programs.find(service_id);
	if (iter_pat != decoded_pat->programs.end())
		pids[iter_pat->second] = 0;//FIXME

	const map_decoded_pmt* decoded_pmt = decoders[ts_id].get_decoded_pmt();
	map_decoded_pmt::const_iterator iter_pmt = decoded_pmt->find(service_id);
	if (iter_pmt != decoded_pmt->end()) {

		for (map_ts_elementary_streams::const_iterator iter_pmt_es = iter_pmt->second.es_streams.begin();
		     iter_pmt_es != iter_pmt->second.es_streams.end(); ++iter_pmt_es) {
				pids[iter_pmt_es->second.pid] = iter_pmt_es->second.type;
		}
	}
}

void parse::add_service_pids(char* service_ids, map_pidtype &pids)
{
	char *save, *id = (service_ids) ? strtok_r(service_ids, CHAR_CMD_COMMA, &save) : NULL;

	if (id) while (id) {
		add_service_pids(strtoul(id, NULL, 0), pids);
		id = strtok_r(NULL, CHAR_CMD_COMMA, &save);
	} else
		if (service_ids) add_service_pids(strtoul(service_ids, NULL, 0), pids);
}

void parse::add_service_pids(map_pidtype &pids)
{
	if (!service_ids.size()) return;
	for (map_pidtype::const_iterator iter = service_ids.begin(); iter != service_ids.end(); ++iter)
		add_service_pids(iter->first, pids);
}

void parse::set_service_ids(char *ids)
{
	char *save, *id = (ids) ? strtok_r(ids, CHAR_CMD_COMMA, &save) : NULL;

	service_ids.clear();
	payload_pids.clear();

	if (id) while (id) {
		if (id) set_service_id(strtoul(id, NULL, 0));
		id = strtok_r(NULL, CHAR_CMD_COMMA, &save);
	} else
		if (ids) set_service_id(strtoul(ids, NULL, 0));

	if (has_pat) {
		rewrite_pat();

		const decoded_pat_t* decoded_pat = decoders[ts_id].get_decoded_pat();
		const map_decoded_pmt* decoded_pmt = decoders[ts_id].get_decoded_pmt();

		process_pat(decoded_pat);

		for (map_pidtype::const_iterator iter = service_ids.begin(); iter != service_ids.end(); ++iter) {
			map_decoded_pmt::const_iterator iter_pmt = decoded_pmt->find(iter->first);
			if (iter_pmt != decoded_pmt->end())
				process_pmt(&iter_pmt->second);
		}
	}
}

bool parse::check()
{
	dprintf("(%s) "
		"fed packets: %d, "
		"ts id: %d, "
		"mode:%s%s%s, "
		"has: %s%s%s%s%s%s, "
		"dumped eit: %d, limit %d, "
		"%sPAT version offset: %d, PAT continuity counter: %d"
		//"\t "
		,
		//	);
		enabled ? "enabled" : "disabled",
		fed_pkt_count,
		ts_id,
		epg_mode ? " epg" : "",
		scan_mode ? " scan" : "",
		((!epg_mode) && (!scan_mode)) ? " none" : "",
		has_pat ? "pat " : "",
		has_mgt ? "mgt " : "",
		has_vct ? "vct " : "",
		has_sdt ? "sdt " : "",
		has_nit ? "nit " : "",
		expect_vct ? "*expects vct " : "",
		dumped_eit,
		eit_collection_limit,
		process_err_pkts ? "processing error packets, " : "",
		rewritten_pat_ver_offset,
		rewritten_pat_cont_ctr);

	return out.check();
}

void parse::set_ts_id(uint16_t new_ts_id)
{
	dprintf("(%04x|%d)\n", new_ts_id, new_ts_id);
	ts_id = new_ts_id;
	memcpy(&channel_info[ts_id], &new_channel_info, sizeof(channel_info_t));
	decoders[ts_id].set_physical_channel(channel_info[ts_id].channel);
}

uint16_t parse::get_ts_id(unsigned int channel)
{
	if (!channel)
		return get_ts_id();
	for (map_channel_info::const_iterator iter = channel_info.begin(); iter != channel_info.end(); ++iter)
		if (channel == iter->second.channel)
			return iter->first;
	return 0;
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
		bool send_pkt = false;
		unsigned int sync_offset = 0;
		output_options out_type = OUTPUT_NONE;
		pkt_stats_t pkt_stats;

		statistics.parse(p, &pkt_stats);

		while (((i > 0) && (pkt_stats.pid == (uint16_t) - 1)) || ((i > 1) && (tp_pkt_pid(p+188) == (uint16_t) - 1))) {
			p++;
			sync_offset++;
			if (sync_offset == 188) {
				sync_offset = 0;
				i--;
				fprintf(stderr, "\nSYNC LOSS\n\n");
			}
			statistics.parse(p, &pkt_stats);
			fprintf(stderr, ".\t");
		}

		if (sync_offset) fprintf(stderr, "\nSYNC LOSS\n\n");
#if 0
		/* demux & statistics for entire read TS */
		statistics.push(p, &pkt_stats);
		demuxer.push(pkt_stats.pid, p);
#endif
		if (pkt_stats.tei) {
			if (!tei_count)
				fprintf(stderr, "\tTEI");//"%s: TEI detected, dropping packet\n", __func__);
			else if (tei_count % 100 == 0)
				fprintf(stderr, ".");
			tei_count++;
			if (!process_err_pkts) continue;
		}

		switch (pkt_stats.pid) {
		case PID_PAT:
#if USING_DVBPSI_VERSION_0
			dvbpsi_PushPacket(h_pat, p);
#else
			h_pat.packet_push(p);
#endif
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
#if USING_DVBPSI_VERSION_0
			map_dvbpsi::const_iterator iter;
#else
			map_dvbpsi::iterator iter;
#endif

			iter = h_pmt.find(pkt_stats.pid);
			if (iter != h_pmt.end()) {
#if USING_DVBPSI_VERSION_0
				dvbpsi_PushPacket(iter->second, p);
#else
				iter->second.packet_push(p);
#endif

				send_pkt = true;
				out_type = OUTPUT_PATPMT;
				break;
			}

			map_pidtype::const_iterator iter_eit;
			iter_eit = eit_pids.find(pkt_stats.pid);
			if (iter_eit != eit_pids.end()) {

				if (decoders[ts_id].eit_x_complete(iter_eit->second)) {
					if (h_demux.count(iter_eit->first)) {
#if USING_DVBPSI_VERSION_0
						dvbpsi_DetachDemux(h_demux[iter_eit->first]);
#else
						h_demux[iter_eit->first].detach_demux();
#endif
						h_demux.erase(iter_eit->first);
					}
					eit_pids.erase(iter_eit->first);
					//epg_complete = (eit_pids.size() == 0);
					continue;
				}
				decoders[ts_id].set_current_eit_x(iter_eit->second);
				out_type = OUTPUT_PSIP;
			}

			iter = h_demux.find(pkt_stats.pid);
			if (iter != h_demux.end()) {
#if USING_DVBPSI_VERSION_0
				dvbpsi_PushPacket(iter->second, p);
#else
				iter->second.packet_push(p);
#endif

				send_pkt = true;
				//if (!out_type) out_type = OUTPUT_PSIP;
				break;
			}

			if ((payload_pids.count(pkt_stats.pid)) ||
			    (out_pids.count(pkt_stats.pid))) {
				send_pkt = true;
				out_type = OUTPUT_PES;
				break;
			}
		}
		if (send_pkt) {
			out.push(p, out_type);
#if 1
			/* demux & statistics for selected PIDs */
			statistics.push(p, &pkt_stats);
#ifdef DVBTEE_DEMUXER
			demuxer.push(pkt_stats.pid, p);
#endif
#endif
		}
#if DBG
		addpid(pid);
#endif
		p += 188;
		fed_pkt_count++;
	}
#if 1//DBG
	while (((decoders.count(ts_id)) && (decoders[ts_id].eit_x_complete(dumped_eit)))) {
		decoders[ts_id].dump_eit_x(NULL, dumped_eit);
		dumped_eit++;
	}
#endif
	return 0;
}

#if !USING_DVBPSI_VERSION_0
static void dvbpsi_message(dvbpsi_t *handle, const dvbpsi_msg_level_t level, const char* msg)
{
	(void)handle;

	const char *status;
	switch(level)
	{
		case DVBPSI_MSG_ERROR: status = "Error: "; break;
		case DVBPSI_MSG_WARN:  status = "Warning: "; break;
		case DVBPSI_MSG_DEBUG: status = "Debug: "; if (dbg & DBG_DVBPSI) break;
		default: /* do nothing */
			return;
	}
	fprintf(stderr, "%s%s\n", status, msg);
}

dvbpsi_class::dvbpsi_class()
  : handle(dvbpsi_new(&dvbpsi_message, DVBPSI_MSG_DEBUG))
{
#if DBG
	dprintf("()");
#endif
	if (!handle) fprintf(stderr, "\n!!! !!! !!! !!! MK- DVBPSI NOT INITIALIZED!!! !!! !!! !!!\n\n");
}

dvbpsi_class::~dvbpsi_class()
{
	if (dvbpsi_decoder_present(handle)) {
		fprintf(stderr, "\n!!! !!! !!! !!! MK- DVBPSI NOT DETACHED!!! !!! !!! !!!\n\n");
		detach_demux();
	}

	if (handle) dvbpsi_delete(handle);
#if DBG
	dprintf("()");
#endif
}

dvbpsi_class::dvbpsi_class(const dvbpsi_class&)
{
#if DBG
	dprintf("(copy)");
#endif
	handle = dvbpsi_new(&dvbpsi_message, DVBPSI_MSG_DEBUG);
	if (!handle) fprintf(stderr, "\n!!! !!! !!! !!! MK- DVBPSI NOT INITIALIZED!!! !!! !!! !!!\n\n");
}

dvbpsi_class& dvbpsi_class::operator= (const dvbpsi_class& cSource)
{
#if DBG
	dprintf("(operator=)");
#endif
	if (this == &cSource)
		return *this;

	handle = dvbpsi_new(&dvbpsi_message, DVBPSI_MSG_DEBUG);
	if (!handle) fprintf(stderr, "\n!!! !!! !!! !!! MK- DVBPSI NOT INITIALIZED!!! !!! !!! !!!\n\n");

	return *this;
}

void dvbpsi_class::purge()
{
#if DBG
	dprintf("()");
#endif
	detach_demux();
	if (handle) dvbpsi_delete(handle);
	handle = dvbpsi_new(&dvbpsi_message, DVBPSI_MSG_DEBUG);
	if (!handle) fprintf(stderr, "\n!!! !!! !!! !!! MK- DVBPSI NOT INITIALIZED!!! !!! !!! !!!\n\n");
}

void dvbpsi_class::detach_tables()
{
#if DBG
	dprintf("()");
#endif
	if ((handle) && (tables.size()))
		for (detach_table_map::iterator iter = tables.begin(); iter != tables.end(); ++iter)
			if (iter->second.detach_cb) {
#if DBG
				dprintf("detaching table: %02x|%04x...", iter->second.table_id, iter->second.table_id_ext);
#endif
				iter->second.detach_cb(handle, iter->second.table_id, iter->second.table_id_ext);
			}
}

void dvbpsi_class::detach_demux()
{
#if DBG
	dprintf("()");
#endif
	if ((handle) && (dvbpsi_decoder_present(handle))) {
		detach_tables();
		dvbpsi_DetachDemux(handle);
#if DBG
		dprintf("(done)");
#endif
	}
}

bool dvbpsi_class::packet_push(uint8_t* p_data)
{
	return ((handle) && (dvbpsi_decoder_present(handle))) ? dvbpsi_packet_push(handle, p_data) : false;
}

void dvbpsi_class::set_detach(dvbpsi_detach_table_callback cb, uint8_t id, uint16_t ext)
{
	uint32_t idx = (((id << 16) & 0x00ff0000) | (ext & 0x0000ffff));
	dprintf("attaching table %02x|%04x...", id, ext);
	tables[idx].detach_cb = cb;
	tables[idx].table_id = id;
	tables[idx].table_id_ext = ext;
}
#endif
