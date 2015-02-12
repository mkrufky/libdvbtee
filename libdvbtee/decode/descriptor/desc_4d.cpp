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


desc_4d::desc_4d(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, p_descriptor)
{
	if (!desc_check_tag(m_tag, 0x4d)) return;

	dvbpsi_short_event_dr_t* dr = dvbpsi_DecodeShortEventDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	memcpy(lang, dr->i_iso_639_code, 3);
	get_descriptor_text(dr->i_event_name, dr->i_event_name_length, name);
	get_descriptor_text(dr->i_text, dr->i_text_length, text);

	dprintf("%s, %s, %s", lang, name, text);

	setValid(true);
}

desc_4d::~desc_4d()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(0x4d, desc_4d);
