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

#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdint.h>
#include <string.h>

#include "decode.h"
#include "demux.h"
#include "output.h"
#include "stats.h"

/* update version number by updating the LIBDVBTEE_VERSION_FOO fields below */
#define LIBDVBTEE_VERSION_A 0
#define LIBDVBTEE_VERSION_B 4
#define LIBDVBTEE_VERSION_C 9

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

/* Machine readible LIBDVBTEE version */
#define LIBDVBTEE_VERSION_INT  ((LIBDVBTEE_VERSION_A<<16)+(LIBDVBTEE_VERSION_B<<8)+LIBDVBTEE_VERSION_C)

/* Human readible LIBDVBTEE version */
#define LIBDVBTEE_VERSION EXPAND_AND_QUOTE(LIBDVBTEE_VERSION_A) "." EXPAND_AND_QUOTE(LIBDVBTEE_VERSION_B) "." EXPAND_AND_QUOTE(LIBDVBTEE_VERSION_C)

extern const char *parse_libdvbpsi_version;


#define USE_STATIC_DECODE_MAP 1

#include <map>

#if !USING_DVBPSI_VERSION_0
typedef void (*dvbpsi_detach_table_callback)(dvbpsi_t *, uint8_t, uint16_t);

typedef struct {
	dvbpsi_detach_table_callback detach_cb;
	uint8_t  table_id;
	uint16_t table_id_ext;
} detach_table_t;

typedef std::map<uint32_t, detach_table_t> detach_table_map;

#define attach_table_auto_detach(class, attach, detach, callback, id, ext) \
  if (attach(class->get_handle(), id, ext, callback, this)) class->set_detach(detach, id, ext)


class dvbpsi_class
{
public:
	dvbpsi_class();
	~dvbpsi_class();

	dvbpsi_class(const dvbpsi_class&);
	dvbpsi_class& operator= (const dvbpsi_class&);

	bool packet_push(uint8_t* p_data);
	dvbpsi_t* get_handle() { return handle; }
	void set_detach(dvbpsi_detach_table_callback cb, uint8_t id, uint16_t ext);
	void detach_demux();

	void purge();
private:
	dvbpsi_t* handle;
	detach_table_map tables;
	void detach_tables();
};

//typedef std::map<uint16_t, dvbpsi_handle> map_dvbpsi;
typedef std::map<uint16_t, dvbpsi_class> map_dvbpsi;
#else
typedef std::map<uint16_t, dvbpsi_handle> map_dvbpsi;
#endif
typedef std::map<uint16_t, decode> map_decoder;
#if 0 // moved to output.h
typedef std::map<uint16_t, uint16_t> map_pidtype;
#endif

typedef struct {
	unsigned int channel;
	uint32_t frequency;
	const char *modulation;
} channel_info_t;
typedef std::map<uint16_t, channel_info_t> map_channel_info;

class tsfilter_iface
{
public:
	virtual void addfilter(uint16_t) = 0;
};


typedef struct {
	uint16_t lcn;
	uint16_t major;
	uint16_t minor;
	uint16_t vpid;
	uint16_t apid;
	uint16_t program_number;
	uint16_t physical_channel;
	uint32_t freq;
	const char *modulation;
	unsigned char service_name[256];
} parsed_channel_info_t;

class parse_iface
{
public:
	virtual void chandump(parsed_channel_info_t *) = 0;
};


class parse
{
public:
	parse();
	~parse();

	unsigned int get_fed_pkt_count() const { return fed_pkt_count; }
	uint16_t get_ts_id() const { return ts_id; }
	uint16_t get_ts_id(unsigned int channel);

	bool get_stream_info(unsigned int channel, uint16_t service, parsed_channel_info_t *c, decoded_event_t *e0 = NULL, decoded_event_t *e1 = NULL);

	void add_service_pids(uint16_t service_id, map_pidtype &pids);
	void add_service_pids(char* service_ids, map_pidtype &pids);
	void add_service_pids(map_pidtype &pids);

	void reset_output_pids(int target_id = -1) { out.reset_pids(target_id); }

	void set_service_ids(char *ids);

	int feed(int, uint8_t*);
	void reset();
	void stop();
	void stop(int);

	int add_output(char*);
	int add_output(int, unsigned int);
	int add_output(void* priv, stream_callback);

	int add_output(char*, map_pidtype&);
	int add_output(int, unsigned int, map_pidtype&);
	int add_output(void* priv, stream_callback, map_pidtype&);

	int add_output(char*, uint16_t);
	int add_output(int, unsigned int, uint16_t);
	int add_output(void* priv, stream_callback, uint16_t);

	int add_output(char*, char*);
	int add_output(int, unsigned int, char*);
	int add_output(void* priv, stream_callback, char*);

	int add_stdout();
	int add_stdout(map_pidtype&);
	int add_stdout(uint16_t);
	int add_stdout(char*);

	unsigned int xine_dump(parse_iface *iface = NULL); /* full channel dump  */
	void epg_dump(decode_report *reporter = NULL); /* full channel dump  */

	void set_channel_info(unsigned int channel, uint32_t frequency, const char *modulation)
	{ new_channel_info.channel = channel; new_channel_info.frequency = frequency; new_channel_info.modulation = modulation; }

	void set_scan_mode(bool onoff) { scan_mode = onoff; }
	void set_epg_mode(bool onoff)  { epg_mode = onoff; }
	void enable(bool onoff)  { enabled = onoff; }

	void enable_ett_collection(bool onoff) { dont_collect_ett = !onoff; }

	bool is_pmt_ready(uint16_t id = 0);
	inline bool is_basic_psip_ready() { return ((has_pat) && (((has_mgt) && ((has_vct) || (!expect_vct))) || ((has_sdt) && (has_nit)))); }
	bool is_psip_ready();
	bool is_epg_ready();

	void cleanup();

	void limit_eit(int limit) { eit_collection_limit = limit; }

	void process_error_packets(bool yesno) { process_err_pkts = yesno; }

	void set_tsfilter_iface(tsfilter_iface &iface) { m_tsfilter_iface = &iface; }

	static void dumpJson();

	output out;

	bool check();
	bool is_enabled() { return enabled; }

	stats statistics;
	static int count_decoder_factories();
private:
#if !USE_STATIC_DECODE_MAP
	map_decoder   decoders;
#endif
	static void take_pat(void*, dvbpsi_pat_t*);
	static void take_pmt(void*, dvbpsi_pmt_t*);
	static void take_eit(void*, dvbpsi_eit_t*);
	static void take_nit_actual(void*, dvbpsi_nit_t*);
	static void take_nit_other(void*,  dvbpsi_nit_t*);
	static void take_sdt_actual(void*, dvbpsi_sdt_t*);
	static void take_sdt_other(void*,  dvbpsi_sdt_t*);
	static void take_tot(void*, dvbpsi_tot_t*);
#if !USING_DVBPSI_VERSION_0
	static void take_vct(void*, dvbpsi_atsc_vct_t*);
	static void take_eit(void*, dvbpsi_atsc_eit_t*);
	static void take_ett(void*, dvbpsi_atsc_ett_t*);
	static void take_stt(void*, dvbpsi_atsc_stt_t*);
	static void take_mgt(void*, dvbpsi_atsc_mgt_t*);
#endif

#if USING_DVBPSI_VERSION_0
	static void attach_table(void*, dvbpsi_handle, uint8_t, uint16_t);
#else
	static void attach_table(dvbpsi_t*, uint8_t, uint16_t, void *);
	static void attach_table(dvbpsi_class* a, uint8_t b, uint16_t c, void *d) { attach_table(a->get_handle(), b, c, d); }
#endif

	bool take_pat(dvbpsi_pat_t*, bool);
	bool take_pmt(dvbpsi_pmt_t*, bool);
	bool take_eit(dvbpsi_eit_t*, bool);
	bool take_nit_actual(dvbpsi_nit_t*, bool);
	bool take_nit_other(dvbpsi_nit_t*,  bool);
	bool take_sdt_actual(dvbpsi_sdt_t*, bool);
	bool take_sdt_other(dvbpsi_sdt_t*,  bool);
	bool take_tot(dvbpsi_tot_t*, bool);
#if !USING_DVBPSI_VERSION_0
	bool take_vct(dvbpsi_atsc_vct_t*, bool);
	bool take_eit(dvbpsi_atsc_eit_t*, bool);
	bool take_ett(dvbpsi_atsc_ett_t*, bool);
	bool take_stt(dvbpsi_atsc_stt_t*, bool);
	bool take_mgt(dvbpsi_atsc_mgt_t*, bool);
#endif

#if USING_DVBPSI_VERSION_0
	void attach_table(dvbpsi_handle, uint8_t, uint16_t);
#else
	void attach_table(dvbpsi_t*, uint8_t, uint16_t);
	void attach_table(dvbpsi_class* a, uint8_t b, uint16_t c) { attach_table(a->get_handle(), b, c); }
#endif

	unsigned int xine_dump(uint16_t ts_id, parse_iface *iface) { return xine_dump(ts_id, &channel_info[ts_id], iface); }
	unsigned int xine_dump(uint16_t, channel_info_t*, parse_iface *);

	void set_ts_id(uint16_t);
	void set_service_id(uint16_t id) { service_ids[id] = 0; }
	void detach_demux();

	channel_info_t new_channel_info;
	map_channel_info channel_info;

	unsigned int fed_pkt_count;

#if USING_DVBPSI_VERSION_0
	dvbpsi_handle h_pat;
#else
	dvbpsi_class  h_pat;
#endif
	map_dvbpsi    h_pmt;
	map_dvbpsi    h_demux;

#if 0
	time_t stream_time;
#endif
	uint16_t ts_id;
	map_pidtype service_ids; // ignore the type name used here

	bool epg_mode;
	bool scan_mode;
	bool dont_collect_ett;
	bool has_pat;
	bool has_mgt;
	bool has_vct;
	bool has_sdt;
	bool has_nit;
	bool expect_vct;
	map_rcvd rcvd_pmt;

//	uint8_t grab_next_eit(uint8_t current_eit_x);
	map_pidtype eit_pids; /* pid, eit-x */

	int dumped_eit;
	int eit_collection_limit;

	bool process_err_pkts;
	unsigned int tei_count;
	map_pidtype payload_pids;

	tsfilter_iface *m_tsfilter_iface;
	void add_filter(uint16_t pid) { if (m_tsfilter_iface) m_tsfilter_iface->addfilter(pid); }
	void clear_filters() { if (m_tsfilter_iface) m_tsfilter_iface->addfilter(0xffff); }
	void reset_filters();

	bool enabled;

	uint8_t pat_pkt[188];

	uint8_t rewritten_pat_ver_offset, rewritten_pat_cont_ctr;
	void rewrite_pat();
	void process_pat(const decoded_pat_t *);
	void process_pmt(const decoded_pmt_t *);
	void process_mgt(bool attach);
#ifdef DVBTEE_DEMUXER
	demux demuxer;
#endif
	map_pidtype out_pids;

	void parse_channel_info(const uint16_t, const decoded_pmt_t*, const decoded_vct_t*, parsed_channel_info_t&);
};

#endif //__PARSE_H__
