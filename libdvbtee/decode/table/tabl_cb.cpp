/*****************************************************************************
 * Copyright (C) 2011-2015 Michael Ira Krufky
 *
 * Author: Michael Ira Krufky <mkrufky@linuxtv.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; atsc_either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  cb110-1301  USA
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "decoder.h"

#include "functions.h"

#include "tabl_cb.h"

#define TABLEID 0xcb

#define CLASS_MODULE "[EIT]"

#define dprintf(fmt, arg...) __dprintf(DBG_DECODE, fmt, ##arg)

using namespace dvbtee::decode;
using namespace dvbtee::value;

static std::string TABLE_NAME = "EIT";

static std::string EITEV = "EITEV";

void atsc_eit::store(dvbpsi_atsc_eit_t *p_atsc_eit)
#define EIT_DBG 1
{
	//XXX: FIXME: decoded_atsc_eit_t &cur_atsc_eit = decoded_atsc_eit[atsc_eit_x][p_atsc_eit->i_source_id];

//	if ((cur_atsc_eit.version   == p_atsc_eit->i_version) &&
//	    (cur_atsc_eit.source_id == p_atsc_eit->i_source_id)) {
//#if EIT_DBG
//		fprintf(stderr, "EIT %s-%d: v%d, source_id %d: ALREADY DECODED\n", __func__,
//			atsc_eit_x, p_atsc_eit->i_version, p_atsc_eit->i_source_id);
//#endif
//		return false;
//	}
#if EIT_DBG
	fprintf(stderr, "%s EIT-%d: v%d, source_id %d\n", __func__,
		/*XXX: FIXME: atsc_eit_x*/0xff,
		p_atsc_eit->i_version, p_atsc_eit->i_source_id);
#endif
#if 0
	map_decoded_vct_channels::const_iterator iter_vct;
	for (iter_vct = decoded_vct.channels.begin(); iter_vct != decoded_vct.channels.end(); ++iter_vct)
		if (iter_vct->second.source_id == p_atsc_eit->i_source_id)
			break;

	if (iter_vct == decoded_vct.channels.end()) {
		fprintf(stderr, "%s-%d: v%d, id:%d\n", __func__,
			atsc_eit_x, p_atsc_eit->i_version, p_atsc_eit->i_source_id);
	} else {
		unsigned char service_name[8] = { 0 };
		for ( int i = 0; i < 7; ++i ) service_name[i] = iter_vct->second.short_name[i*2+1];
		service_name[7] = 0;

		fprintf(stderr, "%s-%d: v%d, id:%d - %d.%d: %s\n", __func__,
			atsc_eit_x, p_atsc_eit->i_version, p_atsc_eit->i_source_id,
			iter_vct->second.chan_major,
			iter_vct->second.chan_minor,
			service_name);
	}
#endif
	decoded_eit.version   = p_atsc_eit->i_version;
	decoded_eit.source_id = p_atsc_eit->i_source_id;
	decoded_eit.events.clear();

	set("version", p_atsc_eit->i_version);
	set("sourceId", p_atsc_eit->i_source_id);
	set("tableId", p_atsc_eit->i_table_id);
	set("extension", p_atsc_eit->i_extension);

	Array events;

	dvbpsi_atsc_eit_event_t* p_event = p_atsc_eit->p_first_event;
	while (p_event) {

		atsc_eitEV *atsc_eitEv = new atsc_eitEV(decoded_eit, this, p_event);
		if (atsc_eitEv->isValid())
			events.push((Object*)atsc_eitEv);

		p_event = p_event->p_next;
	}

	set("events", events);

	descriptors.decode(p_atsc_eit->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	setValid(true);

	dprintf("%s", toJson().c_str());

	if ((/*changed*/true) && (m_watcher)) {
		m_watcher->updateTable(TABLEID, (Table*)this);
	}
}


atsc_eitEV::atsc_eitEV(decoded_atsc_eit_t &decoded_atsc_eit, Decoder *parent, dvbpsi_atsc_eit_event_t *p_event)
: TableDataComponent(parent, EITEV)
{
	decoded_atsc_eit_event_t &cur_event = decoded_atsc_eit.events[p_event->i_event_id];

	cur_event.event_id     = p_event->i_event_id;
	cur_event.start_time   = p_event->i_start_time;
	cur_event.etm_location = p_event->i_etm_location;
	cur_event.length_sec   = p_event->i_length_seconds;
	cur_event.title_bytes  = p_event->i_title_length;
	memcpy(cur_event.title, p_event->i_title, 256); // FIXME
#if EIT_DBG
	time_t start = atsc_datetime_utc(cur_event.start_time /*+ (60 * tz_offset)*/);
	time_t end   = atsc_datetime_utc(cur_event.start_time + cur_event.length_sec /*+ (60 * tz_offset)*/);

	unsigned char name[256];
	memset(name, 0, sizeof(char) * 256);
	decode_multiple_string(cur_event.title,
			       cur_event.title_bytes,
			       name, sizeof(name));
	//p_epg->text[0] = 0;

	struct tm tms = *localtime( &start );
	struct tm tme = *localtime( &end  );
	fprintf(stderr, "  %02d:%02d - %02d:%02d : %s\n", tms.tm_hour, tms.tm_min, tme.tm_hour, tme.tm_min, name );
#endif
	set("eventId",       p_event->i_event_id);
	set("startTime",     p_event->i_start_time);
	set("lengthSec",     p_event->i_length_seconds);
	set("etmLoc",        p_event->i_etm_location);
	set("title",         (char *)name);

	descriptors.decode(p_event->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	setValid(true);
}

atsc_eitEV::~atsc_eitEV()
{

}


bool atsc_eit::ingest(TableStore *s, dvbpsi_atsc_eit_t *t, TableWatcher *w)
{
#if 1
#if 0
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = t->i_ts_id;
#else
	uint16_t __ts_id = t->i_extension;
#endif
	const std::vector<Table*> atsc_eits = s->get(TABLEID);
	for (std::vector<Table*>::const_iterator it = atsc_eits.begin(); it != atsc_eits.end(); ++it) {
		atsc_eit *thisEIT = (atsc_eit*)*it;
		if (thisEIT->get<uint16_t>("sourceId") == t->i_source_id) {
			if (thisEIT->get<uint8_t>("version") == t->i_version) {
				if (thisEIT->get<uint8_t>("tableId") == t->i_table_id) {
					if (thisEIT->get<uint16_t>("extension") == t->i_extension) {
						dprintf("EIT v%d, ts_id %d: ALREADY DECODED", t->i_version, __ts_id);
						return false;
					}
				}
			}
			thisEIT->store(t);
			return true;
		}
	}
#endif
	return s->add<dvbpsi_atsc_eit_t>(TABLEID, t, w);
#else
	return s->setOnly<dvbpsi_atsc_eit_t, atsc_eit>(TABLEID, t, w);
#endif
}


atsc_eit::atsc_eit(Decoder *parent, TableWatcher *watcher)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	//store table later (probably repeatedly)
}

atsc_eit::atsc_eit(Decoder *parent, TableWatcher *watcher, dvbpsi_atsc_eit_t *p_atsc_eit)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	store(p_atsc_eit);
}

atsc_eit::~atsc_eit()
{
	//
}

REGISTER_TABLE_FACTORY(TABLEID, dvbpsi_atsc_eit_t, atsc_eit);
