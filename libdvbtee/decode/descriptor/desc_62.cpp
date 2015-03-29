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

#include "desc_62.h"

#include "dvbpsi/dr_62.h" /* Frequency List descriptor */

#define CLASS_MODULE "[freq list]"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;

static std::string DESC_NAME = "DR[62]";

desc_62::desc_62(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(getTag(), 0x62)) return;

	dvbpsi_frequency_list_dr_t* dr = dvbpsi_DecodeFrequencyListDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

//	enum CodingType {
//		CODINGTYPE_SATELLITE   = 1,
//		CODINGTYPE_CABLE       = 2,
//		CODINGTYPE_TERRESTRIAL = 3,
//	};

	set("codingType", dr->i_coding_type);

	Array freqs;

	for (int i = 0; i < dr->i_number_of_frequencies; ++i) {
		freqs.push(dr->p_center_frequencies[i]);
	}

	set("centerFrequencies", freqs);

	dprintf("%s", toJson().c_str());

	setValid(true);
}

desc_62::~desc_62()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(0x62, desc_62);
