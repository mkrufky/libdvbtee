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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include "desc_86.h"

#include "dvbpsi/dr_86.h" /* caption service */

#define CLASS_MODULE "[caption service]"

#define dPrintf(fmt, arg...) __dPrintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string DESC_NAME = "DR[86]";

#define DESC_TAG 0x86

desc_86::desc_86(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(getTag(), DESC_TAG)) return;

	dvbpsi_caption_service_dr_t* dr = dvbpsi_DecodeCaptionServiceDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	Array captionService;

	for (int i = 0; i < dr->i_number_of_services; i ++) {
		dvbpsi_caption_service_t *service = &dr->services[0];
		if (!service) {
			dPrintf("error!");
			break;
		}
		Object obj;

		obj.set("serviceNumber", service->i_caption_service_number);
		obj.set("digital_cc", std::string((service->b_digital_cc) ? "708" : "608"));
		obj.set("line21field", service->b_line21_field ? true : false);
		obj.set("easyReader", service->b_easy_reader ? true : false);
		obj.set("wideAspectRatio", service->b_wide_aspect_ratio ? true : false);

		char lang[4] = { 0 };
		for (unsigned int i = 0; i < 3; i++) lang[i] = service->i_iso_639_code[i];
		obj.set("iso639lang", std::string(lang));

		captionService.push(obj);
	}

	set("CaptionService", captionService);

	dPrintf("%s", toJson().c_str());

	setValid(true);
}

desc_86::~desc_86()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(DESC_TAG, desc_86);
