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

#ifndef __DECODE_H__
#define __DECODE_H__

#include <string>
#include <string.h>
#include <time.h>

#include "dvbpsi/dvbpsi.h"
#include "dvbpsi/pat.h"
#include "dvbpsi/descriptor.h"
#include "dvbpsi/pmt.h"
#include "dvbpsi/demux.h"
#include "dvbpsi/sdt.h"
#include "dvbpsi/eit.h"
#include "dvbpsi/nit.h"
#include "dvbpsi/tot.h"

#if (DVBPSI_VERSION_INT < ((1<<16)+(0<<8)+0))
#define USING_DVBPSI_VERSION_0 1
#else
#define USING_DVBPSI_VERSION_0 0
#endif

#if !USING_DVBPSI_VERSION_0
#include "dvbpsi/atsc_eit.h"
#include "dvbpsi/atsc_ett.h"
#include "dvbpsi/atsc_stt.h"
#include "dvbpsi/atsc_vct.h"
#include "dvbpsi/atsc_mgt.h"
#ifdef RRT
#include "dvbpsi/atsc_rrt.h"
#endif
#endif

#include "desc.h"

#include <map>

/* -- PAT -- */
typedef std::map<uint16_t, uint16_t> map_decoded_pat_programs; /* program number, pid */

typedef struct
{
	uint16_t			ts_id;
	uint8_t				version;
	map_decoded_pat_programs	programs;
} decoded_pat_t;

/* -- PMT -- */
typedef struct ts_elementary_stream_s
{
	uint8_t		type;
	uint16_t	pid;
	// FIXME: descriptors...
	// from ISO639 language descriptor 0A:
	unsigned char iso_639_code[4];

	ts_elementary_stream_s() : type(0xff), pid(0xffff) { memset(iso_639_code, 0, sizeof(iso_639_code)); }
} ts_elementary_stream_t; // FIXME: rename this later

typedef std::map<uint16_t, ts_elementary_stream_t> map_ts_elementary_streams; /* arbitrary idx(pid), ts_elementary_stream_t */

typedef struct
{
	uint16_t			program;
	uint8_t				version;
	uint16_t			pcr_pid;
	// FIXME: descriptors...
	map_ts_elementary_streams	es_streams;
} decoded_pmt_t;

typedef std::map<uint16_t, decoded_pmt_t> map_decoded_pmt; /* program num, decoded_pmt_t */

/* -- MGT -- */
typedef struct
{
	uint16_t			type;
	uint16_t			pid;
	uint8_t				version;
	uint32_t			bytes;
	// FIXME: descriptors...
} decoded_mgt_table_t;

typedef std::map<uint16_t, decoded_mgt_table_t> map_decoded_mgt_tables; /* type, decoded_mgt_table_t */

typedef struct
{
	uint8_t				version;
	uint16_t			table_id_ext;
	// FIXME: descriptors...
	map_decoded_mgt_tables		tables;
} decoded_mgt_t;

/* -- VCT -- */
typedef struct
{
	uint8_t				short_name[14];
	uint16_t			chan_major;
	uint16_t			chan_minor;
	uint8_t				modulation;
	uint32_t			carrier_freq;
	uint16_t			chan_ts_id;
	uint16_t			program;
	uint8_t				etm_location;
	int				access_controlled;
	int				path_select;
	int				out_of_band;
	int				hidden;
	int				hide_guide;
	uint8_t				service_type;
	uint16_t			source_id;
	// FIXME: descriptors...
} decoded_vct_channel_t;

typedef std::map<uint16_t, decoded_vct_channel_t> map_decoded_vct_channels; /* arbitrary idx((source_id) / program_id ???), decoded_vct_channel_t */

typedef struct
{
	uint8_t				version;
	uint16_t			ts_id;
	int				cable_vct;
	// FIXME: descriptors...
	map_decoded_vct_channels	channels;
} decoded_vct_t;

/* -- EIT -- */
typedef struct
{
	uint16_t			event_id;
	uint64_t			start_time;
	uint32_t			length_sec;
	uint8_t				running_status;
	unsigned int                    f_free_ca:1;
	// FIXME: descriptors...
	std::string                     name;
	std::string                     text;
} decoded_eit_event_t;

typedef std::map<uint16_t, decoded_eit_event_t> map_decoded_eit_events; /* event_id, decoded_eit_event_t */

typedef struct
{
	uint16_t			service_id;
	uint8_t				version;
	uint16_t			ts_id;
	uint16_t			network_id;
	uint8_t				last_table_id;
	map_decoded_eit_events		events;
} decoded_eit_t;

typedef std::map<uint16_t, decoded_eit_t> map_decoded_eit; /* service_id, decoded_eit_t */

typedef struct
{
	uint16_t			event_id;
	uint32_t			start_time;
	uint8_t				etm_location;
	uint32_t			length_sec;
	uint8_t				title_bytes;
	uint8_t				title[256];
	// FIXME: descriptors...
} decoded_atsc_eit_event_t;

typedef std::map<uint16_t, decoded_atsc_eit_event_t> map_decoded_atsc_eit_events; /* event_id, decoded_atsc_eit_event_t */

typedef struct
{
	uint8_t				version;
	uint16_t			source_id;
	map_decoded_atsc_eit_events	events;
} decoded_atsc_eit_t;

typedef std::map<uint16_t, decoded_atsc_eit_t> map_decoded_atsc_eit; /* source_id, decoded_atsc_eit_t */
//typedef std::multimap<uint16_t, decoded_atsc_eit_t> map_decoded_atsc_eit; /* source id, decoded_atsc_eit_t */

#if 0
typedef void (* decoded_atsc_eit_callback)(void* p_cb_data, uint8_t eit_x);
#endif

/* -- ETT -- */
typedef struct
{
	uint8_t				version;
#if 1
	uint32_t			etm_id;
#else
	unsigned int			source_id:16;
	unsigned int			event_id:14;
	unsigned int			event_not_channel_id:2;
#endif
	uint16_t			etm_length;
	uint8_t				etm[256/*4096*/];
} decoded_atsc_ett_t;

typedef std::map<uint16_t, decoded_atsc_ett_t> map_decoded_atsc_ett; /* etm_id, decoded_atsc_eit_t */

/* -- NIT -- */
typedef struct
{
	uint16_t                        ts_id;
	uint16_t                        orig_network_id;
	// FIXME: descriptors...
} decoded_nit_ts_t;

typedef std::map<uint16_t, decoded_nit_ts_t> map_decoded_nit_ts_t; /* ts_id, decoded_nit_ts_t */

typedef struct
{
	uint16_t                        network_id;
	uint8_t                         version;
	// FIXME: descriptors...
	map_decoded_nit_ts_t            ts_list;
} decoded_nit_t;

/* -- SDT -- */
typedef struct
{
	uint16_t                        service_id;
	unsigned int                    f_eit_sched:1;
	unsigned int                    f_eit_present:1;
	uint8_t                         running_status;
	unsigned int                    f_free_ca:1;
	// FIXME: descriptors...
	unsigned char                   provider_name[256];
	unsigned char                   service_name[256];

} decoded_sdt_service_t;

typedef std::map<uint16_t, decoded_sdt_service_t> map_decoded_sdt_services; /* service_id, decoded_sdt_service */

typedef struct
{
	uint16_t                        ts_id;
	uint8_t                         version;
	uint16_t                        network_id;
	// FIXME: descriptors...
	map_decoded_sdt_services        services;
} decoded_sdt_t;


typedef std::map<uint16_t, bool> map_rcvd;


class decode_network_service
{
public:
	decode_network_service();
	~decode_network_service();

	decode_network_service(const decode_network_service&);
	decode_network_service& operator= (const decode_network_service&);

	bool take_eit(dvbpsi_eit_t*, uint8_t);
	bool take_sdt(dvbpsi_sdt_t*);

	bool eit_x_complete_dvb_sched(uint8_t current_eit_x);
	bool eit_x_complete_dvb_pf();

	decoded_sdt_t                   decoded_sdt;

#define NUM_EIT 17
	map_decoded_eit decoded_eit[NUM_EIT];
private:
	unsigned int                    services_w_eit_pf;
	unsigned int                    services_w_eit_sched;

	desc descriptors;
};

typedef std::map<uint16_t, decode_network_service> map_decoded_network_services;

class decode_network
{
public:
	decode_network();
	~decode_network();

	decode_network(const decode_network&);
	decode_network& operator= (const decode_network&);

	bool take_eit(dvbpsi_eit_t* p_eit, uint8_t eit_x) { return decoded_network_services[p_eit->i_ts_id].take_eit(p_eit, eit_x); }
	bool take_nit(dvbpsi_nit_t*);
#if USING_DVBPSI_VERSION_0
	bool take_sdt(dvbpsi_sdt_t* p_sdt) { return decoded_network_services[p_sdt->i_ts_id].take_sdt(p_sdt); }
#else
	bool take_sdt(dvbpsi_sdt_t* p_sdt) { return decoded_network_services[p_sdt->i_extension].take_sdt(p_sdt); }
#endif

	const decoded_sdt_t*   get_decoded_sdt(uint16_t ts_id) { return decoded_network_services.count(ts_id) ? &decoded_network_services[ts_id].decoded_sdt : NULL; }
	const decoded_nit_t*   get_decoded_nit() { return &decoded_nit; }
	const map_decoded_eit* get_decoded_eit(uint16_t ts_id) { return decoded_network_services.count(ts_id) ? decoded_network_services[ts_id].decoded_eit: NULL; }

	bool eit_x_complete_dvb_sched(uint16_t ts_id, uint8_t current_eit_x) { return decoded_network_services.count(ts_id) ? decoded_network_services[ts_id].eit_x_complete_dvb_sched(current_eit_x) : false; }
	bool eit_x_complete_dvb_pf(uint16_t ts_id) { return decoded_network_services.count(ts_id) ? decoded_network_services[ts_id].eit_x_complete_dvb_pf() : false; }

	desc descriptors;

	uint16_t orig_network_id;
private:
	map_decoded_network_services decoded_network_services;
	decoded_nit_t   decoded_nit;
};

typedef std::map<uint16_t, decode_network> map_network_decoder;

void clear_decoded_networks();

typedef struct
{
	std::string channel_name;
	uint16_t    chan_major;
	uint16_t    chan_minor;
	uint16_t    chan_physical;
	uint16_t    chan_svc_id;

	uint16_t    event_id;
	time_t      start_time;
	uint32_t    length_sec;
	std::string name;
	std::string text;
} decoded_event_t;


class decode_report
{
public:
	decode_report();
	~decode_report();

	void epg_event(const char * channel_name,
			    uint16_t chan_major,
			    uint16_t chan_minor,
			    uint16_t chan_physical,
			    uint16_t chan_svc_id,
			    //
			    uint16_t event_id,
			    time_t start_time,
			    uint32_t length_sec,
			    const char * name,
			    const char * text);

	virtual void epg_header_footer(bool, bool) = 0; // {}
	virtual void epg_event(decoded_event_t&) = 0;
	virtual void print(const char *, ...) = 0;
};

class decode
{
public:
	decode();
	~decode();

	decode(const decode&);
	decode& operator= (const decode&);

	bool take_pat(dvbpsi_pat_t*);
	bool take_pmt(dvbpsi_pmt_t*);
	bool take_eit(dvbpsi_eit_t*);
	bool take_nit_actual(dvbpsi_nit_t*);
	bool take_nit_other(dvbpsi_nit_t*);
	bool take_sdt_actual(dvbpsi_sdt_t*);
	bool take_sdt_other(dvbpsi_sdt_t*);
	bool take_tot(dvbpsi_tot_t*);
#if !USING_DVBPSI_VERSION_0
	bool take_vct(dvbpsi_atsc_vct_t*);
	bool take_eit(dvbpsi_atsc_eit_t*);
	bool take_ett(dvbpsi_atsc_ett_t*);
	bool take_stt(dvbpsi_atsc_stt_t*);
	bool take_mgt(dvbpsi_atsc_mgt_t*);
#ifdef RRT
	bool take_rrt(dvbpsi_atsc_mgt_t*);
#endif
#endif
	bool complete_pmt();
#if 0
	bool complete_psip();
#endif

	const decoded_pat_t*   get_decoded_pat() { return &decoded_pat; }
	const map_decoded_pmt* get_decoded_pmt() { return &decoded_pmt; }
	const decoded_vct_t*   get_decoded_vct() { return &decoded_vct; }
	const decoded_mgt_t*   get_decoded_mgt() { return &decoded_mgt; }
	const decoded_sdt_t*   get_decoded_sdt();
	const decoded_nit_t*   get_decoded_nit();

	const desc*            get_descriptors() { return &descriptors; }

	const map_decoded_atsc_eit* get_decoded_atsc_eit() { return decoded_atsc_eit; }
	const map_decoded_eit*      get_decoded_eit();

	unsigned char* get_decoded_ett(uint16_t etm_id, unsigned char *message, size_t sizeof_message); /* message must be an array of 256 unsigned char's */

	uint8_t get_current_eit_x() { return eit_x; }
	uint8_t set_current_eit_x(uint8_t new_eit_x) { eit_x = new_eit_x; return eit_x; }

	uint16_t get_lcn(uint16_t);

	const decode_network*  get_decoded_network();

	void dump_eit_x(decode_report *reporter, uint8_t eit_x, uint16_t source_id = 0);
	bool eit_x_complete(uint8_t current_eit_x);
	bool got_all_eit(int limit = -1);

	void dump_epg(decode_report *reporter);

	void dump_epg_event(const decoded_vct_channel_t*, const decoded_atsc_eit_event_t*, decode_report *reporter);
	void dump_epg_event(const decoded_sdt_service_t*, const decoded_eit_event_t*, decode_report *reporter);

	void set_physical_channel(unsigned int chan) { physical_channel = chan; }

	bool get_epg_event(uint16_t service_id, time_t showtime, decoded_event_t *e);
private:
	uint16_t orig_network_id;
	uint16_t      network_id;

	time_t stream_time;

	decoded_pat_t   decoded_pat;
	map_decoded_pmt decoded_pmt;
	decoded_vct_t   decoded_vct;
	decoded_mgt_t   decoded_mgt;

	map_rcvd rcvd_pmt;

	uint8_t eit_x;

	map_decoded_atsc_eit decoded_atsc_eit[128];
#if 0
	decoded_atsc_eit_callback atsc_eit_callback;
#endif
	//map_rcvd rcvd_eit;
	map_decoded_atsc_ett decoded_ett;

	desc descriptors;

	void dump_eit_x_atsc(decode_report *reporter, uint8_t eit_x, uint16_t source_id = 0);
	void dump_eit_x_dvb(decode_report *reporter, uint8_t eit_x, uint16_t source_id = 0);

	void dump_epg_atsc(decode_report *reporter, uint16_t source_id);
	void dump_epg_dvb(decode_report *reporter, uint16_t source_id);

	bool eit_x_complete_atsc(uint8_t current_eit_x);
	bool eit_x_complete_dvb_sched(uint8_t current_eit_x);
	bool eit_x_complete_dvb_pf();


	void get_epg_event(const decoded_vct_channel_t*, const decoded_atsc_eit_event_t*, decoded_event_t *);
	void get_epg_event(const decoded_sdt_service_t*, const decoded_eit_event_t*, decoded_event_t *);

	bool get_epg_event_atsc(uint16_t source_id, time_t showtime, decoded_event_t *e);
	bool get_epg_event_dvb(uint16_t service_id, time_t showtime, decoded_event_t *e);

	unsigned int physical_channel;
};

#endif /* __DECODE_H__ */
