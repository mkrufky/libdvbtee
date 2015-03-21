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

#include "desc_4d.h"

#include "dvbpsi/dr_4d.h" /* short event descriptor */

#include "functions.h"

#define CLASS_MODULE "[short event]"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;

static std::string DESC_NAME = "DR[4D]";

desc_4d::desc_4d(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(m_tag, 0x4d)) return;

	dvbpsi_short_event_dr_t* dr = dvbpsi_DecodeShortEventDr(p_descriptor);
	if (desc_dr_failed(dr)) return;
#if 0
	unsigned char lang[4] = { 0 };
	unsigned char name[256];
	unsigned char text[256];
#else
	lang[3] = 0;
#endif

	for (unsigned int i = 0; i < 3; i++) lang[i] = dr->i_iso_639_code[i];
	get_descriptor_text(dr->i_event_name, dr->i_event_name_length, name);
	get_descriptor_text(dr->i_text, dr->i_text_length, text);

	set("lang", std::string((const char*)lang));
	set("name", std::string((const char*)name));
	set("text", std::string((const char*)text));

	dprintf("%s", toJson().c_str());

	setValid(true);
}

desc_4d::~desc_4d()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(0x4d, desc_4d);
