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

#ifndef __DESCRIPTOR_H__
#define __DESCRIPTOR_H__

#include <stdio.h>
#include <stdint.h>
#include "dvbpsi/dvbpsi.h"
#include "dvbpsi/descriptor.h"
#include "decoder.h"

namespace dvbtee {

namespace decode {

class Descriptor: public Decoder {
public:
	Descriptor(Decoder &, dvbpsi_descriptor_t*);
	virtual ~Descriptor();
protected:
	uint8_t m_tag;
};

}

}

#endif /* __DESCRIPTOR_H__ */
