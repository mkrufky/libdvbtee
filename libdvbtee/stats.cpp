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

	if (x >= 1000000)
		snprintf(b, n, "%lu.%lu m", x / 1000000, x % 1000000);
	else if (x >= 1000)
		snprintf(b, n, "%lu.%lu k", x / 1000, x % 1000);
	else
		snprintf(b, n, "%lu ", x);
	return b;
}

void stats::show(const uint16_t pid/*, time_t timenow*/)
{
	dprintf("%04x %lu.%lu kbps", pid, statistics[pid]/*[timenow]*/ / 1000, statistics[pid]/*[timenow]*/ % 1000);
}

void stats::show()
{
	for (stats_map::const_iterator iter = statistics.begin(); iter != statistics.end(); ++iter) {
		char a[16];
		char b[16];
		dprintf("pid %04x\t%lu p/s\t%sb/s\t%sbit",
			iter->first, iter->second / 188,
			scale_unit(a, sizeof(a), iter->second),
			scale_unit(b, sizeof(b), iter->second * 8));
	}
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

		pkt_stats->sync_loss = (p[0] != 0x47) ? true : false;
		if (!pkt_stats->sync_loss) {
			pkt_stats->tei = (p[1] & 0x80) ? true : false;
			pkt_stats->pid = ((uint16_t) (p[1] & 0x1f) << 8) + p[2];
		} else
			pkt_stats->pid = (uint16_t) - 1;
	}
	return pkt_stats;
}

void stats::clear_stats()
{
	statistics.clear();
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

	push_stats(pkt_stats);
}
