/*****************************************************************************
 * Copyright (C) 2011-2016 Michael Ira Krufky
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

#ifndef __48_H__
#define __48_H__

#include <map>

#include "descriptor.h"

namespace dvbtee {

namespace decode {

/* service descriptor */

class desc_48: public Descriptor {
DESCRIPTOR_DECODER_TPL
public:
	desc_48(Decoder *, dvbpsi_descriptor_t*);
	virtual ~desc_48();

	/* std::string& providerName = o.get<std::string>("providerName");
	 * std::string&  serviceName = o.get<std::string>("serviceName");
	 */
};

}

}

#endif /* __48_H__ */
