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

#ifndef __A1_H__
#define __A1_H__

#include <map>

#include "descriptor.h"

namespace dvbtee {

namespace decode {

/* service location descriptor */

class desc_a1: public Descriptor {
public:
	desc_a1(Decoder *, dvbpsi_descriptor_t*);
	virtual ~desc_a1();
#if 1
	typedef struct {
		uint8_t       stream_type;
		uint16_t      elementary_pid;
		unsigned char iso_639_code[3];
	} svcloc_t;

	typedef std::map<uint16_t, svcloc_t> svcloc_map_t;

	svcloc_map_t map_svcloc;
#endif
};

}

}

#endif /* __A1_H__ */
