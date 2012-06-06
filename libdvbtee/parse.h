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

#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdint.h>
#include <string.h>

#include "decode.h"
#include "output.h"

#define USE_STATIC_DECODE_MAP 1

#include <map>
typedef std::map<uint16_t, dvbpsi_handle> map_dvbpsi;
typedef std::map<uint16_t, decode> map_decoder;

typedef struct {
	unsigned int channel;
	uint32_t frequency;
	const char *modulation;
} channel_info_t;
typedef std::map<uint16_t, channel_info_t> map_channel_info;

typedef std::map<uint16_t, uint16_t> map_eit_pids; /* pid, eit-x */

class parse
{
public:
	parse();
	~parse();

	unsigned int get_fed_pkt_count() const { return fed_pkt_count; };
	uint16_t get_ts_id() const { return ts_id; };

	int feed(int, uint8_t*);
	void reset();

	unsigned int xine_dump(uint16_t ts_id) { return xine_dump(ts_id, &channel_info[ts_id]); };
	unsigned int xine_dump(); /* full channel dump  */
	void epg_dump(); /* full channel dump  */

	void set_channel_info(unsigned int channel, uint32_t frequency, const char *modulation)
	{ new_channel_info.channel = channel; new_channel_info.frequency = frequency; new_channel_info.modulation = modulation; };

	void set_scan_mode(bool onoff) { scan_mode = onoff; };
	void set_epg_mode(bool onoff)  { epg_mode = onoff; };
//got_all_eit()
	bool is_psip_ready();
	bool is_epg_ready();

	void cleanup();

	void limit_eit(int limit) { eit_collection_limit = limit; }

	void process_error_packets(bool yesno) { process_err_pkts = yesno; }
private:
#if !USE_STATIC_DECODE_MAP
	map_decoder   decoders;
#endif
	static void take_pat(void*, dvbpsi_pat_t*);
	static void take_pmt(void*, dvbpsi_pmt_t*);
	static void take_eit(void*, dvbpsi_eit_t*);
	static void take_nit(void*, dvbpsi_nit_t*);
	static void take_sdt(void*, dvbpsi_sdt_t*);
	static void take_tot(void*, dvbpsi_tot_t*);
	static void take_vct(void*, dvbpsi_atsc_vct_t*);
	static void take_eit(void*, dvbpsi_atsc_eit_t*);
	static void take_ett(void*, dvbpsi_atsc_ett_t*);
	static void take_stt(void*, dvbpsi_atsc_stt_t*);
	static void take_mgt(void*, dvbpsi_atsc_mgt_t*);

	static void attach_table(void*, dvbpsi_handle, uint8_t, uint16_t);

	bool take_pat(dvbpsi_pat_t*, bool);
	bool take_pmt(dvbpsi_pmt_t*, bool);
	bool take_eit(dvbpsi_eit_t*, bool);
	bool take_nit(dvbpsi_nit_t*, bool);
	bool take_sdt(dvbpsi_sdt_t*, bool);
	bool take_tot(dvbpsi_tot_t*, bool);
	bool take_vct(dvbpsi_atsc_vct_t*, bool);
	bool take_eit(dvbpsi_atsc_eit_t*, bool);
	bool take_ett(dvbpsi_atsc_ett_t*, bool);
	bool take_stt(dvbpsi_atsc_stt_t*, bool);
	bool take_mgt(dvbpsi_atsc_mgt_t*, bool);

	void attach_table(dvbpsi_handle, uint8_t, uint16_t);

	unsigned int xine_dump(uint16_t, channel_info_t*);

	void set_ts_id(uint16_t new_ts_id) { fprintf(stderr, "%s(%04x|%d)\n", __func__, new_ts_id, new_ts_id); ts_id = new_ts_id; memcpy(&channel_info[ts_id], &new_channel_info, sizeof(channel_info_t)); };
	void detach_demux();

	channel_info_t new_channel_info;
	map_channel_info channel_info;

	unsigned int fed_pkt_count;

	dvbpsi_handle h_pat;
	map_dvbpsi    h_pmt;
	map_dvbpsi    h_demux;

	time_t stream_time;
	uint16_t ts_id;

	bool epg_mode;
	bool scan_mode;
	bool has_pat;
	bool has_mgt;
	bool has_vct;
	bool has_sdt;
	bool has_nit;
	bool expect_vct;

//	uint8_t grab_next_eit(uint8_t current_eit_x);
	map_eit_pids eit_pids;

	int dumped_eit;
	int eit_collection_limit;

	output out;

	bool process_err_pkts;
};

#endif//__PARSE_H__
