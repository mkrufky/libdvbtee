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


desc_48::desc_48(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, p_descriptor)
{
	if (!desc_check_tag(m_tag, 0x48)) return;

	dvbpsi_service_dr_t* dr = dvbpsi_DecodeServiceDr(p_descriptor);
	if (desc_dr_failed(dr)) return;

	get_descriptor_text(dr->i_service_provider_name, dr->i_service_provider_name_length, provider_name);
	get_descriptor_text(dr->i_service_name,          dr->i_service_name_length,          service_name);

	dprintf("%s, %s", provider_name, service_name);

	setValid(true);
}

desc_48::~desc_48()
{
	//
}

REGISTER_DESCRIPTOR_FACTORY(0x48, desc_48);
