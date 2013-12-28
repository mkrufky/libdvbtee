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

#include <inttypes.h>
#include "string.h"

#include "stats.h"
#include "log.h"
#define CLASS_MODULE "stats"

#define DBG 0

#define dprintf(fmt, arg...) __dprintf(DBG_STATS, "(%s) "fmt, parent, ##arg)

stats::stats(const char *caller)
  : tei_count(0)
  , __timenow(0)
  , parent(caller)
  , streamtime_cb(NULL)
  , streamtime_priv(NULL)
  , statistics_cb(NULL)
  , statistics_priv(NULL)
{
	dprintf("(%s)", parent);
}

stats::~stats()
{
	dprintf("(%s)", parent);
	show(false);
}

#if 0
stats::stats(const stats&)
{
	dprintf("(%s, copy)", parent);
}

stats& stats::operator= (const stats& cSource)
{
	dprintf("(%s, operator=)", parent);

	if (this == &cSource)
		return *this;

	return *this;
}
#endif

static void parse_pcr(uint8_t *pcr, uint64_t *pcr_base, unsigned int *pcr_ext)
{
	*pcr_base = ((uint64_t)pcr[0] * 0x2000000) +
		    ((uint64_t)pcr[1] * 0x20000) +
		    ((uint64_t)pcr[2] * 0x200) +
		    ((uint64_t)pcr[3] * 0x02) +
		    ((uint64_t)(pcr[4] & 0x80) >> 7);

	*pcr_ext = ((pcr[4] & 0x01) * 0x100) + pcr[5];

	return;
}

static time_t walltime(void *p) { (void)p; return time(NULL); }

char *stats_scale_unit(char *b, size_t n, uint64_t x)
{
	memset(b, 0, n);

	if (x >= 1000000) {
		if ((x % 1000000) < 100)
			snprintf(b, n, "%3" PRIu64 ".%03" PRIu64 " m", x / 1000000, x % 1000000);
		else
			snprintf(b, n, "%3" PRIu64 ".%03" PRIu64 " m", x / 1000000, x % 1000000);
	} else if (x >= 1000) {
		if ((x % 1000) < 100)
			snprintf(b, n, "%3" PRIu64 ".%03" PRIu64 " k", x / 1000, x % 1000);
		else
			snprintf(b, n, "%3" PRIu64 ".%03" PRIu64 " k", x / 1000, x % 1000);
	} else
		snprintf(b, n, "    %3" PRIu64 "  ", x);
	return b;
}

void stats::show(bool per_sec)
{
	if (statistics_cb) {
		statistics_cb(statistics_priv, statistics, discontinuities, tei_count, per_sec);
		return;
	}
	for (stats_map::const_iterator iter = statistics.begin(); iter != statistics.end(); ++iter) {
		char a[16];
		char b[16];
		dprintf("pid %04x %5" PRIu64 " p%s  %sb%s  %sbit",
			iter->first, iter->second / 188, (per_sec) ? "/s" : "",
			stats_scale_unit(a, sizeof(a), iter->second), (per_sec) ? "/s" : "",
			stats_scale_unit(b, sizeof(b), iter->second * 8));
	}
	for (stats_map::const_iterator iter = discontinuities.begin(); iter != discontinuities.end(); ++iter)
		dprintf("pid %04x\t%" PRIu64 " continuity errors (%" PRIu64 "%%)", iter->first, iter->second, ((!iter->second) || (!statistics[iter->first])) ? 0 : (!statistics.count(iter->first)) ? 0 : (100 * iter->second / (statistics[iter->first] / 188)));

	if (tei_count) dprintf("tei count: %" PRIu64 " (%" PRIu64 "%%)", tei_count, (!statistics[0x2000]) ? 0 : (18800 * tei_count / statistics[0x2000]));
}

void stats::push_pid(int c, const uint16_t pid)
{
	streamtime_callback cb = (streamtime_cb) ? streamtime_cb : &walltime;
	time_t timenow = cb(streamtime_priv);

	if (timenow > __timenow) {
		show();
		clear_stats();
		__timenow = timenow;
	}

	__push_pid(c, pid);
}

pkt_stats_t *stats::parse(const uint8_t *p, pkt_stats_t *pkt_stats)
{
	adaptation_field_t adapt;
	pkt_hdr_t hdr;

	return parse(p, pkt_stats, hdr, adapt);
}

pkt_stats_t *stats::parse(const uint8_t *p, pkt_stats_t *pkt_stats, pkt_hdr_t &hdr, adaptation_field_t &adapt)
{
	if (pkt_stats) {
		memset(pkt_stats, 0, sizeof(pkt_stats_t));

		const uint8_t *q = p;

		memset(&adapt, 0, sizeof(adapt));
		memset(&hdr, 0, sizeof(hdr));

		hdr.sync_byte          = q[0];
		hdr.tei                = (q[1] & 0x80) >> 7;
		hdr.payload_unit_start = (q[1] & 0x40) >> 6;
		hdr.transport_priority = (q[1] & 0x20) >> 5;
		hdr.pid                = ((q[1] & 0x1f) << 8) | q[2];
		hdr.scrambling         = (q[3] & 0xc0) >> 6;
		hdr.adaptation_flags   = (q[3] & 0x30) >> 4;
		hdr.continuity_ctr     = (q[3] & 0x0f);

		if (hdr.adaptation_flags & 0x02) {
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
				memcpy(adapt.PCR, &q[2], 6);
				q += 6;
			}
			if (adapt.opcr) {
				memcpy(adapt.OPCR, &q[2], 6);
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
	last_pcr_base.clear();
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
	adaptation_field_t adapt;
	pkt_hdr_t hdr;

	if (!pkt_stats) {
		__push(p);
		return;
	}

	memset(pkt_stats, 0, sizeof(pkt_stats_t));

	parse(p, pkt_stats, hdr, adapt);

	if (hdr.adaptation_flags & 0x01) {// payload present
		if (continuity.count(hdr.pid)) {
			uint8_t next = (continuity[hdr.pid] + 1) & 0x0f;
			if ((next != (hdr.continuity_ctr & 0x0f)) && (hdr.continuity_ctr + continuity[hdr.pid] > 0)) {
				if (!(hdr.adaptation_flags & 0x02) && (adapt.discontinuity)) {
					push_discontinuity(hdr.pid);
#if DBG
					dprintf("CONTINUITY ERROR pid: %04x cur: 0x%x prev 0x%x", hdr.pid, hdr.continuity_ctr, continuity[hdr.pid]);
#endif
				}
			}
		}
		continuity[hdr.pid] = hdr.continuity_ctr;
	}

	if (hdr.adaptation_flags & 0x02) {
		if (adapt.pcr) {
			uint64_t pcr_base;
			unsigned int pcr_ext;

			parse_pcr(adapt.PCR, &pcr_base, &pcr_ext);
			dprintf("PID: 0x%04x, PCR base: %" PRIu64 ", ext: %d", hdr.pid, pcr_base, pcr_ext);

			stats_map::const_iterator iter = last_pcr_base.find(hdr.pid);
#if DBG
			if ((iter != last_pcr_base.end()) && (pcr_base < iter->second))
				fprintf(stderr, "%s: PID: 0x%04x, %" PRIu64 " < %" PRIu64 " !!!\n",
					__func__, hdr.pid, pcr_base, iter->second);
#endif
			last_pcr_base[hdr.pid] = pcr_base;
		}
		if (adapt.opcr) {
			uint64_t pcr_base;
			unsigned int pcr_ext;

			parse_pcr(adapt.OPCR, &pcr_base, &pcr_ext);
			dprintf("PID: 0x%04x, PCR base: %" PRIu64 ", ext: %d", hdr.pid, pcr_base, pcr_ext);
		}
		if (adapt.splicing_point) {
			dprintf("PID: 0x%04x, splicing countdown: %d", hdr.pid, adapt.splicing_countdown);
		}
	}

	push_stats(pkt_stats);
}
