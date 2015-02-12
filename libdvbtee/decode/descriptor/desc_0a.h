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

#ifndef __0A_H__
#define __0A_H__

#include <map>

#include "descriptor.h"

namespace dvbtee {

namespace decode {

/* ISO639 language descriptor */

class desc_0a: public Descriptor {
public:
	desc_0a(Decoder *, dvbpsi_descriptor_t*);
	virtual ~desc_0a();

	typedef struct {
		uint8_t       audio_type;
		unsigned char iso_639_code[3];
	} language_t;

	typedef std::map<uint8_t, language_t> lang_map_t;

	lang_map_t map_lang;
};

}

}

#endif /* __0A_H__ */
