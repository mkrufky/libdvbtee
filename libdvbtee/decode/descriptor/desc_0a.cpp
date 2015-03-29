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

#include "desc_0a.h"

#include "dvbpsi/dr_0a.h" /* ISO639 language descriptor */

#define CLASS_MODULE "[iso639 lang]"
//#define CLASS_MODULE "desc_0a"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;

static std::string DESC_NAME = "DR[0A]";

#define DESC_TAG 0x0a

desc_0a::desc_0a(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(getTag(), DESC_TAG)) return;

	dvbpsi_iso639_dr_t* dr = dvbpsi_DecodeISO639Dr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	Array languages;

	for (int i = 0; i < dr->i_code_count; ++i) {
		Object entry;

		char lang[4] = { 0 };
		for (unsigned int j = 0; j < 3; j++) lang[j] = dr->code[i].iso_639_code[j];

		entry.set("language", std::string(lang));
		entry.set("audioType", dr->code[i].i_audio_type);

		languages.push(entry);
	}

	set("ISO639Lang", languages);

	dprintf("%s", toJson().c_str());

	setValid(true);
}

desc_0a::~desc_0a()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(DESC_TAG, desc_0a);
