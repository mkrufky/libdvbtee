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

#ifndef __STATS_H__
#define __STATS_H__

#include <map>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

typedef std::map<uint16_t, uint64_t> stats_map;
typedef std::map<uint16_t, uint8_t> continuity_map; // 4bits

char *stats_scale_unit(char *b, size_t n, uint64_t x);

typedef struct
{
	bool tei;
	bool sync_loss;
	uint16_t pid;
} pkt_stats_t;

typedef struct
{
	unsigned int sync_byte:8;
	unsigned int tei:1;
	unsigned int payload_unit_start:1;
	unsigned int transport_priority:1;
	unsigned int pid:13;
	unsigned int scrambling:2;
	unsigned int adaptation_flags:2;
	unsigned int continuity_ctr:4;
} pkt_hdr_t;

typedef struct
{
	unsigned int field_length:8;
	unsigned int discontinuity:1;
	unsigned int random_access:1; /* set to 1 if the PES pkt in this TS pkt starts an a/v sequence */
	unsigned int es_priority:1;
	unsigned int pcr:1;
	unsigned int opcr:1;
	unsigned int splicing_point:1;
	unsigned int tp_priv_data:1;
	unsigned int field_ext:1;
	uint8_t PCR[6];
	uint8_t OPCR[6];
	signed int splicing_countdown:8;
} adaptation_field_t;

typedef time_t (*streamtime_callback)(void*);

typedef void (*statistics_callback)(void *priv, stats_map &bitrates, stats_map &discontinuities, uint64_t tei_count, bool per_sec);

class stats_iface
{
public:
	virtual void stats(stats_map &bitrates, stats_map &discontinuities, uint64_t tei_count, bool per_sec) = 0;
};

class stats
{
public:
	stats(const char*);
	~stats();
#if 0
	stats(const stats&);
	stats& operator= (const stats&);
#endif
	void set_streamtime_callback(streamtime_callback cb, void *priv) { streamtime_cb = cb; streamtime_priv = priv; }
	void set_statistics_callback(statistics_callback cb, void *priv) { statistics_cb = cb; statistics_priv = priv; }
	void set_statistics_iface(stats_iface *iface) { statistics_iface = iface; }

	void push_pid(const uint16_t pid) { push_pid(188, pid); }

	void push(int c, const uint8_t *p, pkt_stats_t *pkt_stats = NULL) { for(int i = 0; i < c; i++) push(p+i*188, pkt_stats); }
	void push(const uint8_t *p, pkt_stats_t *pkt_stats = NULL);

	pkt_stats_t *parse(const uint8_t *p, pkt_stats_t *pkt_stats);
private:
	stats_map statistics;
	stats_map discontinuities;
	continuity_map continuity;
	uint64_t tei_count;
	time_t __timenow;

	stats_map last_pcr_base;

	const char *parent;

	streamtime_callback streamtime_cb;
	void *streamtime_priv;

	statistics_callback statistics_cb;
	void *statistics_priv;

	stats_iface *statistics_iface;

	pkt_stats_t *parse(const uint8_t *p, pkt_stats_t *pkt_stats, pkt_hdr_t &hdr, adaptation_field_t &adapt);

	void __push_pid(int c, const uint16_t pid) { statistics[pid] += c; statistics[0x2000] += c; }
	void push_pid(int c, const uint16_t pid);

	void __push(const uint8_t *p) { push_pid( (p[0] == 0x47) ? ((uint16_t) (p[1] & 0x1f) << 8) + p[2] : (uint16_t) - 1 ); }

	void show(bool per_sec = true);

	void push_stats(pkt_stats_t *pkt_stats);
	void push_discontinuity(const uint16_t pid) { discontinuities[pid]++; discontinuities[0x2000]++; }
	void clear_stats();
};

#endif /*__STATS_H__ */
