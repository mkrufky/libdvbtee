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

#include "desc_83.h"

#include "dvbpsi/dr_83.h" /* logical channel number */

#define CLASS_MODULE "[lcn]"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;

static std::string DESC_NAME = "DR[83]";

desc_83::desc_83(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(m_tag, 0x83)) return;

	dvbpsi_lcn_dr_t* dr = dvbpsi_DecodeLCNDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	for (int i = 0; i < dr->i_number_of_entries; i ++) {
#if 0
		lcn[dr->p_entries[i].i_service_id] = dr->p_entries[i].i_logical_channel_number;
#endif
		set(dr->p_entries[i].i_service_id, dr->p_entries[i].i_logical_channel_number);
	}

	dprintf("%s", toJson().c_str());

	setValid(true);
}

desc_83::~desc_83()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(0x83, desc_83);
