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

#include "dvbpsi/dr_0a.h" /* ISO639 language descriptor */
#include "dvbpsi/dr_48.h" /* service descriptor */
#include "dvbpsi/dr_4d.h" /* short event descriptor */
#include "dvbpsi/dr_62.h" /* frequency list descriptor */
#include "dvbpsi/dr_81.h" /* AC-3 Audio descriptor */
#include "dvbpsi/dr_83.h" /* LCN descriptor */
#include "dvbpsi/dr_86.h" /* caption service descriptor */
#include "dvbpsi/dr_a0.h" /* extended channel name descriptor */
#include "dvbpsi/dr_a1.h" /* service location descriptor */

#include "desc.h"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

#define DT_ISO639Language             0x0a
#define DT_Service                    0x48
#define DT_ShortEvent                 0x4d
#define DT_Teletext                   0x56
#define DT_FrequencyList              0x62
#define DT_Ac3Audio                   0x81
#define DT_LogicalChannelNumber       0x83
#define DT_CaptionService             0x86
#define DT_ExtendedChannelName        0xa0
#define DT_ServiceLocation            0xa1

#define desc_dr_failed(dr)			\
  ({						\
    bool __ret = !dr;				\
    if (__ret) dprintf("decoder failed!");	\
    __ret;					\
  })

desc::desc()
//  : f_kill_thread(false)
{
	dprintf("()");
}

desc::~desc()
{
	dprintf("()");
}

bool desc::iso639language(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_ISO639Language)
		return false;

	dvbpsi_iso639_dr_t* dr = dvbpsi_DecodeISO639Dr(p_descriptor);
	if (desc_dr_failed(dr)) return false;

	for (int i = 0; i < dr->i_code_count; ++i)
		dprintf("%c%c%c %x",
			dr->code[i].iso_639_code[0],
			dr->code[i].iso_639_code[1],
			dr->code[i].iso_639_code[2],
			dr->code[i].i_audio_type);

	return true;
}

bool desc::service(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_Service)
		return false;

	dvbpsi_service_dr_t* dr = dvbpsi_DecodeServiceDr(p_descriptor);
	if (desc_dr_failed(dr)) return false;

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
	if (desc_dr_failed(dr)) return false;

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
	if (desc_dr_failed(dr)) return false;

	for (int i = 0; i < dr->i_number_of_frequencies; ++i) {
#if 0
		= dr->p_center_frequencies[i]
#else
		dprintf("%d", dr->p_center_frequencies[i]);
#endif
	}
	return true;
}

static inline const char *sample_rate(uint8_t sample_rate_code)
{
	const char *ret = NULL;

	switch (sample_rate_code & 0x07) {
	case 0:
		ret = "48";
		break;
	case 1:
		ret = "44.1";
		break;
	case 2:
		ret = "32";
		break;
	case 3:
		ret = "Reserved";
		break;
	case 4:
		ret = "48 or 44.1";
		break;
	case 5:
		ret = "48 or 32";
		break;
	case 6:
		ret = "44.1 or 32";
		break;
	case 7:
		ret = "48 or 44.1 or 32";
		break;
	}
	return ret;
}

static inline const char *surround_mode(uint8_t surround_mode_code)
{
	const char *ret = NULL;

	switch (surround_mode_code & 0x03) {
	case 0:
		ret = "Not indicated";
		break;
	case 1:
		ret = "NOT Dolby surround encoded";
		break;
	case 2:
		ret = "Dolby surround encoded";
		break;
	case 3:
		ret = "Reserved";
		break;
	}
	return ret;
}

static inline const char *num_channels(uint8_t num_channels_code)
{
	const char *ret = NULL;

	switch (num_channels_code & 0x0f) {
	case 0x00:
		ret = "1 + 1";
		break;
	case 0x01:
		ret = "1/0";
		break;
	case 0x02:
		ret = "2/0";
		break;
	case 0x03:
		ret = "3/0";
		break;
	case 0x04:
		ret = "2/1";
		break;
	case 0x05:
		ret = "3/1";
		break;
	case 0x06:
		ret = "2/2";
		break;
	case 0x07:
		ret = "3/2";
		break;
	case 0x08:
		ret = "1";
		break;
	case 0x09:
		ret = "<= 2";
		break;
	case 0x0a:
		ret = "<= 3";
		break;
	case 0x0b:
		ret = "<= 4";
		break;
	case 0x0c:
		ret = "<= 5";
		break;
	case 0x0d:
		ret = "<= 6";
		break;
	case 0x0e:
		ret = "Reserved";
		break;
	case 0x0f:
		ret = "Reserved";
		break;
	}
	return ret;
}

bool desc::ac3_audio(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_Ac3Audio)
		return false;

	dvbpsi_ac3_audio_dr_t* dr = dvbpsi_DecodeAc3AudioDr(p_descriptor);
	if (desc_dr_failed(dr)) return false;

	dprintf("sample rate: %s", sample_rate(dr->i_sample_rate_code));
	dprintf("bsid: %02x", dr->i_bsid);
	dprintf("bit rate code: %02x", dr->i_bit_rate_code);
	dprintf("surround mode: %s", surround_mode(dr->i_surround_mode));
	dprintf("bsmod: %02x", dr->i_bsmod);
	dprintf("num channels: %s", num_channels(dr->i_num_channels));
	dprintf("full svc: %s", (dr->b_full_svc) ? "true" : "false");
	dprintf("description: %s", dr->text);
	if (dr->b_language_flag)
		dprintf("language: %c%c%c",
			dr->language[0],
			dr->language[1],
			dr->language[2]);
	if (dr->b_language_flag_2)
		dprintf("language_2: %c%c%c",
			dr->language_2[0],
			dr->language_2[1],
			dr->language_2[2]);

	return true;
}

bool desc::_lcn(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_LogicalChannelNumber)
		return false;

	dvbpsi_lcn_dr_t* dr = dvbpsi_DecodeLCNDr(p_descriptor);
	if (desc_dr_failed(dr)) return false;

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
	if (desc_dr_failed(dr)) return false;

	for (int i = 0; i < dr->i_number_of_services; i ++) {
		dvbpsi_caption_service_t *service = &dr->services[0];
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
			service->i_iso_639_code[0],
			service->i_iso_639_code[1],
			service->i_iso_639_code[2]);
	}

	return true;
}

bool desc::extended_channel_name(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_ExtendedChannelName)
		return false;

	dvbpsi_extended_channel_name_dr_t *dr = dvbpsi_ExtendedChannelNameDr(p_descriptor);
	if (desc_dr_failed(dr)) return false;

	unsigned char name[256];
	memset(name, 0, sizeof(name));
	decode_multiple_string(dr->i_long_channel_name, dr->i_long_channel_name_length, name, sizeof(name));

	dprintf("%s", name);

	return true;
}

bool desc::service_location(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_ServiceLocation)
		return false;

	dvbpsi_service_location_dr_t* dr = dvbpsi_DecodeServiceLocationDr(p_descriptor);
	if (desc_dr_failed(dr)) return false;

	for (int i = 0; i < dr->i_number_elements; i ++) {
		dvbpsi_service_location_element_t *element = &dr->elements[i];
		if (!element) {
			dprintf("error!");
			break;
		}
		_a1[element->i_elementary_pid].elementary_pid = element->i_elementary_pid;
		_a1[element->i_elementary_pid].stream_type    = element->i_stream_type;
		memcpy(_a1[element->i_elementary_pid].iso_639_code, element->i_iso_639_code, 3);
		dprintf("%d, %02x, %c%c%c", element->i_elementary_pid, element->i_stream_type,
			element->i_iso_639_code[0],
			element->i_iso_639_code[1],
			element->i_iso_639_code[2]);
	}

	return true;
}

void desc::decode(dvbpsi_descriptor_t* p_descriptor)
{
	while (p_descriptor) {
		switch (p_descriptor->i_tag) {
		case DT_ISO639Language:
			iso639language(p_descriptor);
			break;
		case DT_Service:
			service(p_descriptor);
			break;
		case DT_ShortEvent:
			short_event(p_descriptor);
			break;
		case DT_FrequencyList:
			freq_list(p_descriptor);
			break;
		case DT_Ac3Audio:
			ac3_audio(p_descriptor);
			break;
		case DT_LogicalChannelNumber:
			_lcn(p_descriptor);
			break;
		case DT_CaptionService:
			caption_service(p_descriptor);
			break;
		case DT_ExtendedChannelName:
			extended_channel_name(p_descriptor);
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
