/*****************************************************************************
 * Copyright (C) 2011-2013 Michael Krufky
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

#ifndef __STATS_H__
#define __STATS_H__

#include <map>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#if 0
typedef std::map<time_t, uint64_t> pid_stats_map;
typedef std::map<uint16_t, pid_stats_map> stats_map;
#else
typedef std::map<uint16_t, uint64_t> stats_map;
typedef std::map<uint16_t, uint8_t> continuity_map; // 4bits
#endif

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

typedef time_t (*streamtime_callback)(void*);

class stats
{
public:
	stats();
	~stats();
#if 0
	stats(const stats&);
	stats& operator= (const stats&);
#endif
	void set_streamtime_callback(streamtime_callback cb, void *priv) { streamtime = cb; streamtime_priv = priv; };

	void push_pid(const uint16_t pid) { push_pid(188, pid); };

	void push(int c, const uint8_t *p, pkt_stats_t *pkt_stats = NULL) { for(int i = 0; i < c; i++) push(p+i*188, pkt_stats); };
	void push(const uint8_t *p, pkt_stats_t *pkt_stats = NULL);

	pkt_stats_t *parse(const uint8_t *p, pkt_stats_t *pkt_stats);
private:
	stats_map statistics;
	stats_map discontinuities;
	continuity_map continuity;
	uint64_t tei_count;
	time_t __timenow;

	pkt_hdr_t hdr;

	streamtime_callback streamtime;
	void *streamtime_priv;

	void __push_pid(int c, const uint16_t pid/*, time_t timenow*/) { statistics[pid]/*[timenow]*/ += c; statistics[0x2000] += c; };
	void push_pid(int c, const uint16_t pid);

	void __push(const uint8_t *p) { push_pid( (p[0] == 0x47) ? ((uint16_t) (p[1] & 0x1f) << 8) + p[2] : (uint16_t) - 1 ); };

	void show(const uint16_t pid/*, time_t timenow*/);
	void show();

	void push_stats(pkt_stats_t *pkt_stats);
	void push_discontinuity(const uint16_t pid) { discontinuities[pid]++; discontinuities[0x2000]++; };
	void clear_stats();
};

#endif /*__STATS_H__ */
