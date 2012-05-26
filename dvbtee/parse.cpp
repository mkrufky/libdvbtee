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
#define TID_NIT  0x40
#define TID_NITx 0x41
#define TID_SDT  0x42
#define TID_SDTx 0x46
#define TID_BAT  0x4A
#define TID_EIT  0x4E
#define TID_EITx 0x4F
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
#if DBG
	fprintf(stderr, "%s(%s)\n", __func__, (decoded) ? "post" : "pre");
#endif
	if (decoded) return true;

	return true;
}

bool parse::take_tot(dvbpsi_tot_t* p_tot, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s)\n", __func__, (decoded) ? "post" : "pre");
#endif
	if (decoded) return true;

	return true;
}

bool parse::take_pat(dvbpsi_pat_t* p_pat, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): v%d, ts_id: %d\n",
		__func__, (decoded) ? "post" : "pre",
		p_pat->i_version, p_pat->i_ts_id);
#endif
	if (!decoded) {
		set_ts_id(p_pat->i_ts_id);
		h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
		return true;
	}
#if 0 // convert to read from decoded pat instead of decoding again
	dvbpsi_pat_program_t* p_program = p_pat->p_first_program;
	while (p_program) {

		if (p_program->i_number > 0) // FIXME: > 0 ???
			h_pmt[p_program->i_pid] =
				dvbpsi_AttachPMT(p_program->i_number, take_pmt, this);

		p_program = p_program->p_next;
	}
#else
	for (map_decoded_pat_programs::const_iterator iter =
	       decoders[p_pat->i_ts_id].get_decoded_pat()->programs.begin();
	     iter != decoders[p_pat->i_ts_id].get_decoded_pat()->programs.end(); ++iter)
		if (iter->first > 0) // FIXME: > 0 ???
			h_pmt[iter->second] = dvbpsi_AttachPMT(iter->first, take_pmt, this);
#endif
	has_pat = true;

	return true;
}

bool parse::take_pmt(dvbpsi_pmt_t* p_pmt, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): v%d, service_id %d, pcr_pid %d\n",
		__func__, (decoded) ? "post" : "pre",
		p_pmt->i_version, p_pmt->i_program_number, p_pmt->i_pcr_pid);
#endif
	if (!decoded) return true;

	/* if we're going to stream a program, we should make sure its pid is comign thru after analyzing the decoded pmt's */

	return true;
}

bool parse::take_vct(dvbpsi_atsc_vct_t* p_vct, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): v%d, ts_id %d, b_cable_vct %d\n",
		__func__, (decoded) ? "post" : "pre",
		p_vct->i_version, p_vct->i_ts_id, p_vct->b_cable_vct);
#endif
	if (!decoded) {
		set_ts_id(p_vct->i_ts_id);
		return true;
	}
	has_vct = true;

	return true;
}

bool parse::take_mgt(dvbpsi_atsc_mgt_t* p_mgt, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): v%d\n",
		__func__, (decoded) ? "post" : "pre",
		p_mgt->i_version);
#endif
	if (!decoded) {
		return true;
	}
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
#if 1
		case 0x0100 ... 0x017f: /* EIT-0 to EIT-127 */
			eit_pids[iter->second.pid] = iter->first - 0x0100;
#else
		case 0x0101 ... 0x017f: /* EIT-1 to EIT-127 */
			break;
		case 0x0100: /* EIT-0 */
#endif
			if ((scan_mode) && (!epg_mode))
				break;
#if 0
			if (0 == decoders[ts_id].get_current_eit_x())
#endif
				b_attach_demux  = true;
			break;
#if 0
		case 0x0200 ... 0x027f: /* ETT-0 to ETT-127 */
#else
		case 0x0201 ... 0x027f: /* ETT-1 to ETT-127 */
		case 0x0200: /* ETT-0 */
			break;
#endif
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
		if ((b_attach_demux) && (NULL == h_demux[iter->second.pid]))
			h_demux[iter->second.pid] = dvbpsi_AttachDemux(attach_table, this);
			/* else already attached */
	}
	expect_vct = b_expecting_vct;

	return true;
}

bool parse::take_nit(dvbpsi_nit_t* p_nit, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): TODO\n", __func__, (decoded) ? "post" : "pre");
#endif
	if (decoded) return true;

	return true;
}

bool parse::take_sdt(dvbpsi_sdt_t* p_sdt, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): TODO\n", __func__, (decoded) ? "post" : "pre");
#endif
	if (decoded) return true;

	return true;
}

bool parse::take_eit(dvbpsi_eit_t* p_eit, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): TODO\n", __func__, (decoded) ? "post" : "pre");
#endif
	if (decoded) return true;

	return true;
}

bool parse::take_eit(dvbpsi_atsc_eit_t* p_eit, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): v%d, source_id %d\n", __func__,
		(decoded) ? "post" : "pre",
		p_eit->i_version, p_eit->i_source_id);
#endif
	if (decoded) {
#if 0
#if 0
		uint8_t new_eit_x = grab_next_eit(decoders[ts_id].get_current_eit_x());
		if (new_eit_x == 0x80)
			decoders[ts_id].set_current_eit_x(0);
		else
			decoders[ts_id].set_current_eit_x(new_eit_x);
#else
		uint8_t current_eit_x = decoders[ts_id].get_current_eit_x();
		if (decoders[ts_id].eit_x_complete(current_eit_x)) {
			for (map_eit_pids::const_iterator iter = eit_pids.begin(); iter != eit_pids.end(); ++iter)
				if (iter->second == current_eit_x) {
					eit_pids.erase(iter->first);
					if (h_demux.count(iter->first)) {
						//dvbpsi_DetachDemux(h_demux[iter->first]);
						h_demux.erase(iter->first);
					}
				}
		}
		epg_complete = (eit_pids.size() == 0);
#endif
#endif
		return true;
	}
	return true;
}

bool parse::take_ett(dvbpsi_atsc_ett_t* p_ett, bool decoded)
{
#if DBG
	fprintf(stderr, "%s(%s): v%d, ID: %d\n", __func__,
		(decoded) ? "post" : "pre",
		p_ett->i_version, p_ett->i_etm_id);
#endif
	if (decoded) return true;

	return true;
}

void parse::attach_table(dvbpsi_handle h_dvbpsi, uint8_t i_table_id, uint16_t i_extension)
{
	if ((scan_mode) && (!epg_mode)) switch (i_table_id) {
	default:
		return;
	case TID_NIT:
	case TID_SDT:
	case TID_ATSC_TVCT:
	case TID_ATSC_CVCT:
	case TID_ATSC_MGT:
		break;
	}
	switch (i_table_id) {
	case TID_EIT:
		dvbpsi_AttachEIT(h_dvbpsi, i_table_id, i_extension, take_eit, this);
		break;
	case TID_NIT:
		dvbpsi_AttachNIT(h_dvbpsi, i_table_id, i_extension, take_nit, this);
		break;
	case TID_SDT:
		dvbpsi_AttachSDT(h_dvbpsi, i_table_id, i_extension, take_sdt, this);
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
define_table_wrapper(take_nit, dvbpsi_nit_t, dvbpsi_DeleteNIT);
define_table_wrapper(take_sdt, dvbpsi_sdt_t, dvbpsi_DeleteSDT);
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
  , has_vct(false)
  , expect_vct(true)
  , dumped_eit(0)
{
	fprintf(stderr, "%s()\n", __func__);

	memset(&new_channel_info, 0, sizeof(channel_info_t));

	h_pat = dvbpsi_AttachPAT(take_pat, this);
#if 0
	h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
#endif
	h_demux[PID_EIT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
	h_demux[PID_NIT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_SDT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_TOT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
}

parse::~parse()
{
	fprintf(stderr, "%s()\n", __func__);
#if DBG
	xine_dump();
#endif
	cleanup();
#if 1
#if DBG
	printpids();
#endif
	fprintf(stderr, "%d packets read in total\n", fed_pkt_count);
#endif
}

void parse::detach_demux()
{
	fprintf(stderr, "%s()\n", __func__);

	for (map_dvbpsi::const_iterator iter = h_demux.begin(); iter != h_demux.end(); ++iter)
		dvbpsi_DetachDemux(iter->second);
	h_demux.clear();

	for (map_dvbpsi::const_iterator iter = h_pmt.begin(); iter != h_pmt.end(); ++iter)
		dvbpsi_DetachPMT(iter->second);
	h_pmt.clear();

	dvbpsi_DetachPAT(h_pat);
}

void parse::cleanup()
{
	fprintf(stderr, "%s()\n", __func__);

	detach_demux();
	decoders.clear();
	channel_info.clear();
}

void parse::reset()
{
	fprintf(stderr, "%s()\n", __func__);

	detach_demux();

	memset(&new_channel_info, 0, sizeof(channel_info_t));

	ts_id = 0;
	dumped_eit = 0;
	has_pat = false;
	has_vct = false;
	expect_vct = true;

	h_pat = dvbpsi_AttachPAT(take_pat, this);
#if 0
	h_demux[PID_ATSC] = dvbpsi_AttachDemux(attach_table, this);
#endif
	h_demux[PID_EIT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
	h_demux[PID_NIT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_SDT]  = dvbpsi_AttachDemux(attach_table, this);
	h_demux[PID_TOT]  = dvbpsi_AttachDemux(attach_table, this);//if !scan_mode
}

inline uint16_t tp_pkt_pid(uint8_t* pkt)
{
	return (pkt[0] == 0x47) ? ((uint16_t) (pkt[1] & 0x1f) << 8) + pkt[2] : (uint16_t) - 1;
}

#if 1
unsigned int parse::xine_dump(uint16_t ts_id, channel_info_t* channel_info)
{
	uint32_t freq          = channel_info->frequency;
	unsigned int channel   = channel_info->channel;
	const char* modulation = channel_info->modulation;;

	int count = 0;

	fprintf(stdout, "\n# channel %d, %d, %s %s\n", channel, freq, "", "");

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
		map_decoded_vct_channels::const_iterator iter_vct = decoders[ts_id].get_decoded_vct()->channels.find(program_number);
		if (iter_vct == decoders[ts_id].get_decoded_vct()->channels.end())
			continue;

		fprintf(stdout, "%d.%d:%d:%s:%d:%d:%d\n",
			iter_vct->second.chan_major,
			iter_vct->second.chan_minor,
			freq,//iter_vct->second.carrier_freq,
			modulation,
			vpid, apid, program_number);
		count++;
	}
	return count;
}
#else
unsigned int parse::xine_dump(uint32_t freq, unsigned int channel, const char* modulation)
{
	return decoders[get_ts_id()].xine_dump(freq, channel, modulation);
}
#endif


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
		decoders[iter->second].dump_epg();

	channels.clear();

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

	/* one TS packet at a time */
        uint8_t* p = p_data;
        for (int i = count / 188; i > 0; --i) {
		uint16_t pid = tp_pkt_pid(p);

		if (p[1] & 0x80) {
			fprintf(stderr, "\tTEI\t");//"%s: TEI detected, dropping packet\n", __func__);
			continue;
		}

		if (PID_PAT == pid)
			dvbpsi_PushPacket(h_pat, p);
		else {
			map_dvbpsi::const_iterator iter;

			iter = h_pmt.find(pid);
			if (iter != h_pmt.end())
				dvbpsi_PushPacket(iter->second, p);

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
			}

			iter = h_demux.find(pid);
			if (iter != h_demux.end())
				dvbpsi_PushPacket(iter->second, p);
		}
#if DBG
		addpid(pid);
#endif
		p += 188;
		fed_pkt_count++;
	}
#if 1//DBG
	while(decoders[ts_id].eit_x_complete(dumped_eit)) {
		decoders[ts_id].dump_eit_x(dumped_eit);
		dumped_eit++;
	}
#endif
	return 0;
}
