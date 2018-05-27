/*****************************************************************************
 * Copyright (C) 2011-2017 Michael Ira Krufky
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

#include <stdlib.h>

#include "desc_4e.h"

#include "dvbpsi/dr_4e.h" /* extended event descriptor */

#include "functions.h"
#include "parse.h"

#define CLASS_MODULE "[extended event]"

#define dPrintf(fmt, arg...) __dPrintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string DESC_NAME = "DR[4E]";

#define DESC_TAG 0x4e

desc_4e::desc_4e(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(getTag(), DESC_TAG)) return;

	bool b_translate_iso6937 = getParser()->iso6937_translation_enabled();

	dvbpsi_extended_event_dr_t* dr = dvbpsi_DecodeExtendedEventDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	unsigned char encoded_text[256];
	unsigned char lang[4] = { 0 };

	for (unsigned int i = 0; i < 3; i++) lang[i] = dr->i_iso_639_code[i];
	get_descriptor_text(dr->i_text, dr->i_text_length, encoded_text);

	set("descriptor_number", dr->i_descriptor_number);
	set("last_descriptor_number", dr->i_last_descriptor_number);
	set("lang", std::string((const char*)lang));

	size_t prefix;
	const char *encoding = detect_encoding((unsigned char *)encoded_text, &prefix);
	unsigned char *text;
	if (encoding) {
		set("original_encoding", std::string((const char *)encoding));

		text = (b_translate_iso6937) ? (unsigned char *)translate((char *)encoded_text, encoding) : encoded_text;
	} else {
		text = (b_translate_iso6937) ? (unsigned char *)translate_iso6937((char *)encoded_text) : encoded_text;
	}

	/* FIXME: we should escape these strings on output rather than on store */
	if (strchr((char*)text, '"')) {
		char* escaped = escape_quotes((const char*)text);
		set("text", std::string(escaped));
		free(escaped);
	} else {
		set("text", std::string((char*)text));
	}
	if (b_translate_iso6937) free(text);

	dPrintf("%s", toJson().c_str());

	setValid(true);
}

desc_4e::~desc_4e()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(DESC_TAG, desc_4e)
