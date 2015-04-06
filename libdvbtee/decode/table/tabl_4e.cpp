/*****************************************************************************
 * Copyright (C) 2011-2015 Michael Ira Krufky
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  4e110-1301  USA
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "decoder.h"

#include "functions.h"

#include "tabl_4e.h"

#define TABLEID 0x4e

#define CLASS_MODULE "[EIT]"

#define dprintf(fmt, arg...) __dprintf(DBG_DECODE, fmt, ##arg)

using namespace dvbtee::decode;
using namespace dvbtee::value;

static std::string TABLE_NAME = "EIT";

static std::string EITTS = "EITTS";

void eit::store(dvbpsi_eit_t *p_eit)
#define EIT_DBG 1
{
#if USING_DVBPSI_VERSION_0
	uint16_t __service_id = p_eit->i_service_id;
#else
	uint16_t __service_id = p_eit->i_extension;
#endif

	//XXX: FIXME:
	//decoded_eit_t &cur_eit = decoded_eit[eit_x][__service_id];

//	if ((cur_eit.version == p_eit->i_version) &&
//	    (cur_eit.service_id == __service_id)) {
//#if EIT_DBG
//		fprintf(stderr, "%s EIT-%d: v%d | ts_id %d | network_id %d service_id %d: ALREADY DECODED\n", __func__, eit_x,
//			p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, service_id);
//#endif
//		return false;
//	}
#if EIT_DBG
	fprintf(stderr, "%s EIT-%d: v%d | ts_id %d | network_id %d service_id %d | table id: 0x%02x, last_table id: 0x%02x\n", __func__, /*XXX: FIXME: eit_x*/0xff,
		p_eit->i_version, p_eit->i_ts_id, p_eit->i_network_id, __service_id, p_eit->i_table_id, p_eit->i_last_table_id);
#endif
	decoded_eit.service_id    = __service_id;
	decoded_eit.version       = p_eit->i_version;
	decoded_eit.ts_id         = p_eit->i_ts_id;
	decoded_eit.network_id    = p_eit->i_network_id;
	decoded_eit.last_table_id = p_eit->i_last_table_id;
	decoded_eit.events.clear();

	set("serviceId",   __service_id);
	set("version",     p_eit->i_version);
	set("tsId",        p_eit->i_ts_id);
	set("networkId",   p_eit->i_network_id);
	set("lastTableId", p_eit->i_last_table_id);

	Array events;

	dvbpsi_eit_event_t* p_event = p_eit->p_first_event;
	while (p_event) {

		eitEV *eitEv = new eitEV(decoded_eit, this, p_event);
		if (eitEv->isValid())
			events.push((Object*)eitEv);
		p_event = p_event->p_next;
	}

	set("events", events);

	setValid(true);

	dprintf("%s", toJson().c_str());

	if ((/*changed*/true) && (m_watcher)) {
		m_watcher->updateTable(TABLEID, (Table*)this);
	}
}

eitEV::eitEV(decoded_eit_t& decoded_eit, Decoder *parent, dvbpsi_eit_event_t *p_event)
: TableDataComponent(parent, EITTS)
{
	if (!p_event) return;

	decoded_eit_event_t &cur_event = decoded_eit.events[p_event->i_event_id];

	cur_event.event_id       = p_event->i_event_id;
	cur_event.start_time     = p_event->i_start_time;
	cur_event.length_sec     = p_event->i_duration;
	cur_event.running_status = p_event->i_running_status;
	cur_event.f_free_ca      = p_event->b_free_ca;

	set("eventId",       p_event->i_event_id);
	set("startTime",     p_event->i_start_time);
	set("lengthSec",     p_event->i_duration);
	set("runningStatus", p_event->i_running_status);
	set("f_free_ca",     p_event->b_free_ca);

	descriptors.decode(p_event->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	const dvbtee::decode::Descriptor *d = descriptors.last(0x4d);
	if (d) {
		cur_event.name.assign(d->get<std::string>("name").c_str());
		cur_event.text.assign(d->get<std::string>("text").c_str());
	}
#if EIT_DBG
	time_t start = datetime_utc(cur_event.start_time /*+ (60 * tz_offset)*/);
	time_t end   = datetime_utc(cur_event.start_time + cur_event.length_sec /*+ (60 * tz_offset)*/);

	struct tm tms = *localtime(&start);
	struct tm tme = *localtime(&end);

	fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, cur_event.name.c_str());
#endif
	setValid(true);
}

eitEV::~eitEV()
{

}


bool eit::ingest(TableStore *s, dvbpsi_eit_t *t, TableWatcher *w)
{
#if USING_DVBPSI_VERSION_0
	uint16_t __service_id = t->i_service_id;
#else
	uint16_t __service_id = t->i_extension;
#endif
	const std::vector<Table*> eits = s->get(TABLEID);
	for (std::vector<Table*>::const_iterator it = eits.begin(); it != eits.end(); ++it) {
		eit *thisEIT = (eit*)*it;
		if (thisEIT->get<uint16_t>("serviceId") == __service_id) {
			if (thisEIT->get<uint8_t>("version") == t->i_version) {
				dprintf("EIT v%d, service_id %d: ALREADY DECODED", t->i_version, __service_id);
				return false;
			}
			thisEIT->store(t);
			return true;
		}
	}
	return s->add<dvbpsi_eit_t>(TABLEID, t, w);
}


eit::eit(Decoder *parent, TableWatcher *watcher)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	//store table later (probably repeatedly)
}

eit::eit(Decoder *parent, TableWatcher *watcher, dvbpsi_eit_t *p_eit)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	store(p_eit);
}

eit::~eit()
{
	//
}

REGISTER_TABLE_FACTORY(TABLEID, dvbpsi_eit_t, eit);
