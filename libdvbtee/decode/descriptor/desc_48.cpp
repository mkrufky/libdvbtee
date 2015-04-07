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

#include "desc_48.h"

#include "dvbpsi/dr_48.h" /* service descriptor */

#include "functions.h"

#define CLASS_MODULE "[service]"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;
using namespace valueobj;

static std::string DESC_NAME = "DR[48]";

#define DESC_TAG 0x48

desc_48::desc_48(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(getTag(), DESC_TAG)) return;

	dvbpsi_service_dr_t* dr = dvbpsi_DecodeServiceDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	unsigned char provider_name[256] = { 0 };
	unsigned char service_name[256] = { 0 };

	get_descriptor_text(dr->i_service_provider_name, dr->i_service_provider_name_length, provider_name);
	get_descriptor_text(dr->i_service_name,          dr->i_service_name_length,          service_name);

	set("providerName", std::string((const char*)provider_name));
	set("serviceName", std::string((const char*)service_name));

	dprintf("%s", toJson().c_str());

	setValid(true);
}

desc_48::~desc_48()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(DESC_TAG, desc_48);
