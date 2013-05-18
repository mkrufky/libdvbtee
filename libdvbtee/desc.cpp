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

#define DBG 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "log.h"
#define CLASS_MODULE "desc"

#include "dvbpsi/dr_48.h" /* service descriptor */
#include "dvbpsi/dr_4d.h" /* short event descriptor */
#include "dvbpsi/dr_62.h" /* frequency list descriptor */
#include "dvbpsi/dr_83.h" /* LCN descriptor */
#include "dvbpsi/dr_86.h" /* caption service descriptor */
#include "dvbpsi/dr_a1.h" /* service location descriptor */

#include "desc.h"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

#define DT_Service                    0x48
#define DT_ShortEvent                 0x4d
#define DT_Teletext                   0x56
#define DT_FrequencyList              0x62
#define DT_LogicalChannelNumber       0x83
#define DT_CaptionService             0x86
#define DT_ServiceLocation            0xa1


desc::desc()
//  : f_kill_thread(false)
{
	dprintf("()");
}

desc::~desc()
{
	dprintf("()");
}

bool desc::service(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_Service)
		return false;

	dvbpsi_service_dr_t* dr = dvbpsi_DecodeServiceDr(p_descriptor);

	get_descriptor_text(dr->i_service_provider_name, dr->i_service_provider_name_length, provider_name);
	get_descriptor_text(dr->i_service_name,          dr->i_service_name_length,          service_name);

	dprintf("%s, %s", provider_name, service_name);

	return true;
}

bool desc::short_event(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_ShortEvent)
		return false;

	dvbpsi_short_event_dr_t* dr = dvbpsi_DecodeShortEventDr(p_descriptor);

	memcpy(_4d.lang, dr->i_iso_639_code, 3);
	get_descriptor_text(dr->i_event_name, dr->i_event_name_length, _4d.name);
	get_descriptor_text(dr->i_text, dr->i_text_length, _4d.text);

	dprintf("%s, %s, %s", _4d.lang, _4d.name, _4d.text);

	return true;
}


bool desc::freq_list(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_FrequencyList)
		return false;

	dvbpsi_frequency_list_dr_t* dr = dvbpsi_DecodeFrequencyListDr(p_descriptor);
	for (int i = 0; i < dr->i_number_of_frequencies; ++i) {
#if 0
		= dr->p_center_frequencies[i]
#else
		dprintf("%d", dr->p_center_frequencies[i]);
#endif
	}
	return true;
}

bool desc::_lcn(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_LogicalChannelNumber)
		return false;

	dvbpsi_lcn_dr_t* dr = dvbpsi_DecodeLCNDr(p_descriptor);
	for (int i = 0; i < dr->i_number_of_entries; i ++) {
#if 0
		= lcn->p_entries[i].i_service_id;
		= lcn->p_entries[i].i_logical_channel_number;
#else
		lcn[dr->p_entries[i].i_service_id] = dr->p_entries[i].i_logical_channel_number;
		dprintf("%d, %d", dr->p_entries[i].i_service_id, lcn[dr->p_entries[i].i_service_id]);
#endif
	}

	return true;
}

bool desc::caption_service(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_CaptionService)
		return false;

	dvbpsi_caption_service_dr_t* dr = dvbpsi_DecodeCaptionServiceDr(p_descriptor);
	dvbpsi_caption_service_t *service = dr->p_first_service;
	for (int i = 0; i < dr->i_number_of_services; i ++) {
		if (!service) {
			dprintf("error!");
			break;
		}
		dprintf("%d / %04x, %s line21 field: %d %d %s%s%c%c%c",
			service->i_caption_service_number,
			service->i_caption_service_number,
			(service->b_digital_cc) ? "708" : "608",
			service->b_line21_field,
			(service->b_digital_cc) ? service->i_caption_service_number : 0,
			(service->b_easy_reader) ? "easy reader " : "",
			(service->b_wide_aspect_ratio) ? "wide aspect ratio " : "",
			service->i_ISO_639_language_code[0],
			service->i_ISO_639_language_code[1],
			service->i_ISO_639_language_code[2]);
		service = service->p_next;
	}

	return true;
}

bool desc::service_location(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_ServiceLocation)
		return false;

	dvbpsi_service_location_dr_t* dr = dvbpsi_DecodeServiceLocationDr(p_descriptor);
	dvbpsi_service_location_element_t *element = dr->p_first_element;
	for (int i = 0; i < dr->i_number_elements; i ++) {
		if (!element) {
			dprintf("error!");
			break;
		}
		_a1[element->i_elementary_PID].elementary_PID = element->i_elementary_PID;
		_a1[element->i_elementary_PID].stream_type    = element->i_stream_type;
		memcpy(_a1[element->i_elementary_PID].ISO_639_language_code, element->i_ISO_639_language_code, 3*sizeof(unsigned char));
		dprintf("%d, %d, %c%c%c", element->i_elementary_PID, element->i_stream_type,
			element->i_ISO_639_language_code[0],
			element->i_ISO_639_language_code[1],
			element->i_ISO_639_language_code[2]);
		element = element->p_next;
	}

	return true;
}

void desc::decode(dvbpsi_descriptor_t* p_descriptor)
{
	while (p_descriptor) {
		switch (p_descriptor->i_tag) {
		case DT_Service:
			service(p_descriptor);
			break;
		case DT_ShortEvent:
			short_event(p_descriptor);
			break;
		case DT_FrequencyList:
			freq_list(p_descriptor);
			break;
		case DT_LogicalChannelNumber:
			_lcn(p_descriptor);
			break;
		case DT_CaptionService:
			caption_service(p_descriptor);
			break;
		case DT_ServiceLocation:
			service_location(p_descriptor);
			break;
		default:
			dprintf("unknown descriptor tag: %02x", p_descriptor->i_tag);
			break;
		}
		p_descriptor = p_descriptor->p_next;
	}
}
