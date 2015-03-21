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

#include <string.h>

#include "desc_a1.h"

#include "dvbpsi/dr_a1.h" /* service location descriptor */

#include "functions.h"

#define CLASS_MODULE "[service location]"
//#define CLASS_MODULE "desc_a1"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;

static std::string DESC_NAME = "DR[A1]";

desc_a1::desc_a1(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(m_tag, 0xa1)) return;

	dvbpsi_service_location_dr_t* dr = dvbpsi_DecodeServiceLocationDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	for (int i = 0; i < dr->i_number_elements; i ++) {
		dvbpsi_service_location_element_t *element = &dr->elements[i];
		if (!element) {
			dprintf("error!");
			break;
		}
#if 1
		map_svcloc[element->i_elementary_pid].elementary_pid = element->i_elementary_pid;
		map_svcloc[element->i_elementary_pid].stream_type    = element->i_stream_type;
		memcpy(map_svcloc[element->i_elementary_pid].iso_639_code, element->i_iso_639_code, 3);
#endif
		Object svcloc;

		char __lang[4] = { 0 };
		for (unsigned int j = 0; j < 3; j++) __lang[j] = element->i_iso_639_code[j];
		std::string lang(__lang);

		if (lang.length()) svcloc.set("lang", lang);
		svcloc.set("esPid", element->i_elementary_pid);
		svcloc.set("streamType", element->i_stream_type);
		svcloc.set("streamTypeString", streamtype_name(element->i_stream_type));
		set(element->i_elementary_pid, svcloc);
	}

	dprintf("%s", toJson().c_str());

	setValid(true);
}

desc_a1::~desc_a1()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(0xa1, desc_a1);
