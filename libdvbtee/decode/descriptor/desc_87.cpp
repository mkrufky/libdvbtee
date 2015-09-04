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

#include "desc_87.h"

#include "dvbpsi/multi_string.h"

#include "dvbpsi/dr_87.h" /* content advisory */

#define CLASS_MODULE "[content advisory]"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string DESC_NAME = "DR[87]";

#define DESC_TAG 0x87

struct desc_87_rating_dimension : valueobj::Object {
	desc_87_rating_dimension(dvbpsi_rating_dimension_t *p)
	{
		set("dimension", p->i_rating_dimension);
		set("value", p->i_rating_value);
	}
};

struct desc_87_rating_region : valueobj::Object {
	desc_87_rating_region(dvbpsi_rating_region_t *p)
	{
		set("region", p->i_rating_region);
		set("description", p->rating_description->ISO_639_language_code);
		//FIXME get description from p->rating_description (multi_string)

		valueobj::Array dimensions;

		dvbpsi_rating_dimension_t *pd = p->p_first_dimension;
		while (pd) {
			desc_87_rating_dimension RatingDimension(pd);
			dimensions.push((Object*)&RatingDimension);
			pd = pd->p_next;
		}
		set("Dimensions", dimensions);
	}
};

desc_87::desc_87(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(getTag(), DESC_TAG)) return;

	dvbpsi_content_advisory_dr_t* dr = dvbpsi_DecodeContentAdvisoryDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	valueobj::Array regions;

	dvbpsi_rating_region_t *pr = dr->p_first_region;
	while (pr) {
		desc_87_rating_region RatingRegion(pr);
		regions.push((Object*)&RatingRegion);
		pr = pr->p_next;
	}
	set("ContentAdvisory", regions);

	dprintf("%s", toJson().c_str());

	setValid(true);
}

desc_87::~desc_87()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(DESC_TAG, desc_87);
