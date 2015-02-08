/*****************************************************************************
 * Copyright (C) 2011-2014 Michael Ira Krufky
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

#define DBG 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "log.h"
#define CLASS_MODULE "desc"

//#define DVBPSI_SUPPORTS_DR_81_86_A0_A1 (DVBPSI_VERSION_INT > ((1<<16)+(1<<8)+1))
#define DVBPSI_SUPPORTS_DR_81_86_A0_A1 1

#include "dvbpsi/dr_0a.h" /* ISO639 language descriptor */
#include "dvbpsi/dr_48.h" /* service descriptor */
#include "dvbpsi/dr_4d.h" /* short event descriptor */
#include "dvbpsi/dr_62.h" /* frequency list descriptor */
#if DVBPSI_SUPPORTS_DR_81_86_A0_A1
#include "dvbpsi/dr_81.h" /* AC-3 Audio descriptor */
#endif
#include "dvbpsi/dr_83.h" /* LCN descriptor */
#if DVBPSI_SUPPORTS_DR_81_86_A0_A1
#include "dvbpsi/dr_86.h" /* caption service descriptor */
#include "dvbpsi/dr_a0.h" /* extended channel name descriptor */
#include "dvbpsi/dr_a1.h" /* service location descriptor */
#endif

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
 : _0a(NULL)
 , _48(NULL)
 , _4d(NULL)
//  : f_kill_thread(false)
{
	dprintf("()");
}

using namespace dvbtee::decode;

desc::~desc()
{
	dprintf("()");
	if (_0a) delete _0a;
	if (_48) delete _48;
	if (_4d) delete _4d;
}

bool desc::iso639language(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_ISO639Language)
		return false;

	if (_0a) delete _0a;
	_0a = (desc_0a*) DescriptorRegistry::instance().create(this, p_descriptor);

	return true;
}

bool desc::service(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_Service)
		return false;

	if (_48) delete _48;
	_48 = (desc_48*) DescriptorRegistry::instance().create(this, p_descriptor);

	return true;
}

bool desc::short_event(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_ShortEvent)
		return false;

	if (_4d) delete _4d;
	_4d = (desc_4d*) DescriptorRegistry::instance().create(this, p_descriptor);

	return true;
}


bool desc::freq_list(dvbpsi_descriptor_t* p_descriptor)
{
	if (p_descriptor->i_tag != DT_FrequencyList)
		return false;

	delete (desc_62*) DescriptorRegistry::instance().create(this, p_descriptor);

	return true;
}

bool desc::ac3_audio(dvbpsi_descriptor_t* p_descriptor)
{
#if DVBPSI_SUPPORTS_DR_81_86_A0_A1
	if (p_descriptor->i_tag != DT_Ac3Audio)
		return false;

	delete (desc_81*) DescriptorRegistry::instance().create(this, p_descriptor);
#endif
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
#if DVBPSI_SUPPORTS_DR_81_86_A0_A1
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
#endif
	return true;
}

bool desc::extended_channel_name(dvbpsi_descriptor_t* p_descriptor)
{
#if DVBPSI_SUPPORTS_DR_81_86_A0_A1
	if (p_descriptor->i_tag != DT_ExtendedChannelName)
		return false;

	delete (desc_a0*) DescriptorRegistry::instance().create(this, p_descriptor);
#endif
	return true;
}

bool desc::service_location(dvbpsi_descriptor_t* p_descriptor)
{
#if DVBPSI_SUPPORTS_DR_81_86_A0_A1
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
		dprintf("pid: 0x%04x, type %02x: %s, %c%c%c", element->i_elementary_pid,
			element->i_stream_type, streamtype_name(element->i_stream_type),
			element->i_iso_639_code[0],
			element->i_iso_639_code[1],
			element->i_iso_639_code[2]);
	}
#endif
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
