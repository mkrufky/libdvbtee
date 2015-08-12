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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  c8110-1301  USA
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include "decoder.h"

#include "functions.h"

#include "tabl_c8.h"

#define TABLEID 0xc8

#define CLASS_MODULE "[VCT]"

#define dprintf(fmt, arg...) __dprintf(DBG_DECODE, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string TABLE_NAME = "VCT";

static std::string VCTCH = "VCTCH";

void vct::store(const dvbpsi_atsc_vct_t * const p_vct)
#define VCT_DBG 1
{
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = p_vct->i_ts_id;
#else
	uint16_t __ts_id = p_vct->i_extension;
#endif
#if VCT_DBG
	fprintf(stderr, "%s VCT: v%d, ts_id %d, b_cable_vct %d\n", __func__,
		p_vct->i_version, __ts_id, p_vct->b_cable_vct);
#endif
	set("version", p_vct->i_version);
	set("tsId",  __ts_id);
	set("isCableVCT", p_vct->b_cable_vct);
	decoded_vct.version   = p_vct->i_version;
	decoded_vct.ts_id     = __ts_id;
	decoded_vct.cable_vct = p_vct->b_cable_vct;
	decoded_vct.channels.clear();

	dprintf("parsing channel descriptors for mux:");
	descriptors.decode(p_vct->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	Array channels;

	const dvbpsi_atsc_vct_channel_t *p_channel = p_vct->p_first_channel;
#if VCT_DBG
	if (p_channel)
		fprintf(stderr, "  channel | service_id | source_id | service_name\n");
#endif
	while (p_channel) {
		vctCh *channel = new vctCh(decoded_vct, this, p_channel);
		if (channel->isValid())
			channels.push((Object*)channel);
		p_channel = p_channel->p_next;
	}

	set("channels", channels);

	setValid(true);

	dprintf("%s", toJson().c_str());

	if ((/*changed*/true) && (m_watcher)) {
		m_watcher->updateTable(TABLEID, (Table*)this);
	}
}


bool vct::ingest(TableStore *s, const dvbpsi_atsc_vct_t * const t, TableWatcher *w)
{
#if 0
#if USING_DVBPSI_VERSION_0
	uint16_t __ts_id = t->i_ts_id;
#else
	uint16_t __ts_id = t->i_extension;
#endif
	const std::vector<Table*> vcts = s->get(TABLEID);
	for (std::vector<Table*>::const_iterator it = vcts.begin(); it != vcts.end(); ++it) {
		vct *thisVCT = (vct*)*it;
		if (thisVCT->get<uint16_t>("tsId") == __ts_id) {
			if (thisVCT->get<uint16_t>("version") == t->i_version) {
				dprintf("VCT v%d, ts_id %d: ALREADY DECODED", t->i_version, __ts_id);
				return false;
			}
			thisVCT->store(t);
			return true;
		}
	}
	return s->add<const dvbpsi_atsc_vct_t>(TABLEID, t, w);
#else
	return s->setOnly<const dvbpsi_atsc_vct_t, vct>(TABLEID, t, w);
#endif
}


vct::vct(Decoder *parent, TableWatcher *watcher)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	//store table later (probably repeatedly)
}

vct::vct(Decoder *parent, TableWatcher *watcher, const dvbpsi_atsc_vct_t * const p_vct)
 : Table(parent, TABLE_NAME, TABLEID, watcher)
{
	store(p_vct);
}

vct::~vct()
{
	//
}


vctCh::vctCh(decoded_vct_t &decoded_vct, Decoder *parent, const dvbpsi_atsc_vct_channel_t * const p_channel)
: TableDataComponent(parent, VCTCH)
{
	decoded_vct_channel_t &cur_channel = decoded_vct.channels[p_channel->i_program_number];

	memcpy(cur_channel.short_name,  p_channel->i_short_name,
	       sizeof(cur_channel.short_name));

	cur_channel.chan_major        = p_channel->i_major_number;
	cur_channel.chan_minor        = p_channel->i_minor_number;
	cur_channel.modulation        = p_channel->i_modulation;
	cur_channel.carrier_freq      = p_channel->i_carrier_freq;
	cur_channel.chan_ts_id        = p_channel->i_channel_tsid;
	cur_channel.program           = p_channel->i_program_number;
	cur_channel.etm_location      = p_channel->i_etm_location;
	cur_channel.access_controlled = p_channel->b_access_controlled;
	cur_channel.path_select       = p_channel->b_path_select;
	cur_channel.out_of_band       = p_channel->b_out_of_band;
	cur_channel.hidden            = p_channel->b_hidden;
	cur_channel.hide_guide        = p_channel->b_hide_guide;
	cur_channel.service_type      = p_channel->i_service_type;
	cur_channel.source_id         = p_channel->i_source_id;

	set("major",            p_channel->i_major_number);
	set("minor",            p_channel->i_minor_number);

	set("modulation",       p_channel->i_modulation);
	set("carrierFreq",      p_channel->i_carrier_freq);
	set("tsId",             p_channel->i_channel_tsid);

	set("program",          p_channel->i_program_number);

	set("etmLocation",      p_channel->i_etm_location);
	set("accessControlled", p_channel->b_access_controlled);
	set("pathSelect",       p_channel->b_path_select);
	set("outOfBand",        p_channel->b_out_of_band);
	set("hidden",           p_channel->b_hidden);
	set("hideGuide",        p_channel->b_hide_guide);
	set("serviceType",      p_channel->i_service_type);

	set("sourceId",         p_channel->i_source_id);

	dprintf("parsing channel descriptors for service: %d", p_channel->i_program_number);
	descriptors.decode(p_channel->p_first_descriptor);
	if (descriptors.size()) set<Array>("descriptors", descriptors);

	std::string languages;

	const dvbtee::decode::Descriptor *d = descriptors.last(0xa1);
	if (d) {
		const valueobj::Array& a  = d->get<valueobj::Array>("serviceLocation");
		for (unsigned int i = 0; i < a.size(); i++) {
			const Object& o = a.get<Object>(i);

			const std::string lang = o.get<std::string>("lang");
			//o.get<uint16_t>("esPid");
			//o.get<uint8_t>("streamType");
			//o.get<std::string>("streamTypeString");

			if (lang.length()) {
				if (!languages.empty()) languages.append(", ");
				languages.append(lang.c_str());
			}
		}
	}
	unsigned char service_name[8] = { 0 };
	for ( int i = 0; i < 7; ++i ) service_name[i] = cur_channel.short_name[i*2+1];
	service_name[7] = 0;

	set("serviceName", (char *)service_name);
#if VCT_DBG
	fprintf(stderr, "  %5d.%d | %10d | %9d | %s | %s\n",
		cur_channel.chan_major,
		cur_channel.chan_minor,
		cur_channel.program,
		cur_channel.source_id,
		service_name, languages.c_str());
#endif
	setValid(true);
}

vctCh::~vctCh()
{

}

REGISTER_TABLE_FACTORY(TABLEID, const dvbpsi_atsc_vct_t, vct);
