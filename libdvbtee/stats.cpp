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

#include "string.h"

#include "stats.h"
#include "log.h"
#define CLASS_MODULE "stats"

#define DBG_STATS DBG_PARSE

#define dprintf(fmt, arg...) __dprintf(DBG_STATS, fmt, ##arg)

stats::stats()
  : tei_count(0)
  , __timenow(0)
  , streamtime(NULL)
  , streamtime_priv(NULL)
{
	dprintf("()");
}

stats::~stats()
{
	dprintf("()");
	show(false);
}

#if 0
stats::stats(const stats&)
{
	dprintf("(copy)");
}

stats& stats::operator= (const stats& cSource)
{
	dprintf("(operator=)");

	if (this == &cSource)
		return *this;

	return *this;
}
#endif

static time_t walltime(void *p) { return time(NULL); }

static char *scale_unit(char *b, size_t n, uint64_t x)
{
	memset(b, 0, n);

	if (x >= 1000000) {
		if ((x % 1000000) < 100)
			snprintf(b, n, "%3lu.%lu m", x / 1000000, x % 1000000 * (((x % 1000000) < 10) ? 100 : 10));
		else
			snprintf(b, n, "%3lu.%lu m", x / 1000000, x % 1000000 / (((x % 1000000) >= 1000) ? 1000 : 1));
	} else if (x >= 1000) {
		if ((x % 1000) < 100)
			snprintf(b, n, "%3lu.%lu k", x / 1000, x % 1000 * (((x % 1000) < 10) ? 100 : 10));
		else
			snprintf(b, n, "%3lu.%lu k", x / 1000, x % 1000 / (((x % 1000) >= 1000) ? 1000 : 1));
	} else
		snprintf(b, n, "    %3lu  ", x);
	return b;
}

void stats::show(const uint16_t pid/*, time_t timenow*/)
{
	dprintf("%04x %lu.%lu kbps", pid, statistics[pid]/*[timenow]*/ / 1000, statistics[pid]/*[timenow]*/ % 1000);
}

void stats::show(bool per_sec)
{
	for (stats_map::const_iterator iter = statistics.begin(); iter != statistics.end(); ++iter) {
		char a[16];
		char b[16];
		dprintf("pid %04x %5lu p%s  %sb%s  %sbit",
			iter->first, iter->second / 188, (per_sec) ? "/s" : "",
			scale_unit(a, sizeof(a), iter->second), (per_sec) ? "/s" : "",
			scale_unit(b, sizeof(b), iter->second * 8));
	}
	for (stats_map::const_iterator iter = discontinuities.begin(); iter != discontinuities.end(); ++iter)
		dprintf("pid %04x\t%lu discontinuities (%lu%%)", iter->first, iter->second, (!statistics.count(iter->first)) ? 0 : 100 * iter->second / (statistics[iter->first] / 188));

	if (tei_count) dprintf("tei count: %lu (%lu%%)", tei_count, 18800 * tei_count / statistics[0x2000]);
}

void stats::push_pid(int c, const uint16_t pid)
{
	streamtime_callback cb = (streamtime) ? streamtime : &walltime;
	time_t timenow = cb(streamtime_priv);

#if 0
	if ((!statistics[pid][timenow]) && (statistics[pid][timenow-1]))
#else
	if (timenow > __timenow) {
#endif
		show();
		clear_stats();
		__timenow = timenow;
	}

	__push_pid(c, pid/*, timenow*/);
}

pkt_stats_t *stats::parse(const uint8_t *p, pkt_stats_t *pkt_stats)
{
	if (pkt_stats) {
		memset(pkt_stats, 0, sizeof(pkt_stats));

		const uint8_t *q = p;

		hdr.sync_byte          = q[0];
		hdr.tei                = (q[1] & 0x80) >> 7;
		hdr.payload_unit_start = (q[1] & 0x40) >> 6;
		hdr.transport_priority = (q[1] & 0x20) >> 5;
		hdr.pid                = ((q[1] & 0x1f) << 8) | q[2];
		hdr.scrambling         = (q[3] & 0xc0) >> 6;
		hdr.adaptation_flags   = (q[3] & 0x30) >> 4;
		hdr.continuity_ctr     = (q[3] & 0x0f);

		if (hdr.adaptation_flags & 0x10) {
			q += 4;
			adapt.field_length   = q[0];
			adapt.discontinuity  = (q[1] & 0x80) >> 7;
			adapt.random_access  = (q[1] & 0x40) >> 6; /* set to 1 if the PES pkt in this TS pkt starts an a/v sequence */
			adapt.es_priority    = (q[1] & 0x20) >> 5;
			adapt.pcr            = (q[1] & 0x10) >> 4;
			adapt.opcr           = (q[1] & 0x08) >> 3;
			adapt.splicing_point = (q[1] & 0x04) >> 2;
			adapt.tp_priv_data   = (q[1] & 0x02) >> 1;
			adapt.field_ext      = (q[1] & 0x01) >> 0;

			if (adapt.pcr) {
				adapt.PCR =
					((unsigned long long) (0xff & q[2]) << 40) |
					((unsigned long long) (0xff & q[3]) << 32) |
					((unsigned long long) (0xff & q[4]) << 24) |
					((unsigned long long) (0xff & q[5]) << 16) |
					((unsigned long long) (0xff & q[6]) << 8)  |
					((unsigned long long) (0xff & q[7]) << 0);
				q += 6;
			}
			if (adapt.opcr) {
				adapt.OPCR =
					((unsigned long long) (0xff & q[2]) << 40) |
					((unsigned long long) (0xff & q[3]) << 32) |
					((unsigned long long) (0xff & q[4]) << 24) |
					((unsigned long long) (0xff & q[5]) << 16) |
					((unsigned long long) (0xff & q[6]) << 8)  |
					((unsigned long long) (0xff & q[7]) << 0);
				q += 6;
			}
			if (adapt.splicing_point) {
				adapt.splicing_countdown = q[2];
				q ++;
			}
		}

		pkt_stats->sync_loss = (hdr.sync_byte != 0x47) ? true : false;
		if (!pkt_stats->sync_loss) {
			pkt_stats->tei = (hdr.tei) ? true : false;
			pkt_stats->pid = hdr.pid;
		} else
			pkt_stats->pid = (uint16_t) - 1;
	}
	return pkt_stats;
}

void stats::clear_stats()
{
	statistics.clear();
	discontinuities.clear();
	tei_count = 0;
}

void stats::push_stats(pkt_stats_t *pkt_stats)
{
	if (pkt_stats->tei) {
		tei_count++;
		push_pid((uint16_t) - 1);
	} else
		push_pid(pkt_stats->pid);
}

void stats::push(const uint8_t *p, pkt_stats_t *pkt_stats)
{
	if (!pkt_stats) {
		__push(p);
		return;
	}

	memset(pkt_stats, 0, sizeof(pkt_stats));

	parse(p, pkt_stats);

	if (hdr.adaptation_flags & 0x01) {// payload present
		if (continuity.count(hdr.pid)) {
			uint8_t next = (continuity[hdr.pid] + 1) & 0x0f;
			if ((next != (hdr.continuity_ctr & 0x0f)) && (hdr.continuity_ctr + continuity[hdr.pid] > 0)) {
				if (!(hdr.adaptation_flags & 0x10) && (adapt.discontinuity)) {
#if 1
					push_discontinuity(hdr.pid);
#else
					dprintf("DISCONTINUITY %d cur: 0x%x prev 0x%x", hdr.pid, hdr.continuity_ctr, continuity[hdr.pid]);
#endif
				}
			}
		}
		continuity[hdr.pid] = hdr.continuity_ctr;
	}
	if (hdr.adaptation_flags & 0x10) {
		dprintf("ADAPTATION FIELD: %02x %02x", p[4], p[5]);
		if (adapt.pcr)
			dprintf("PCR: 0x%12llx", adapt.PCR);
		if (adapt.opcr)
			dprintf("OPCR: 0x%12llx", adapt.OPCR);
		if (adapt.splicing_point)
			dprintf("splicing countdown: %d", adapt.splicing_countdown);
	}

	push_stats(pkt_stats);
}
