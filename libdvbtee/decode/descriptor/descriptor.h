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

#include "decoder.h"
#include "dvbpsi/descriptor.h"

#include "log.h"

#define desc_check_tag(t, e) \
({ \
	bool __ret = (t == e); \
	if (!__ret) \
		__dprintf(DBG_DESC, "FAIL: 0x%02x != 0x%02x", t, e); \
	__ret; \
})

#define desc_dr_failed(dr) \
({ \
	bool __ret = !dr; \
	if (__ret) dprintf("decoder failed!"); \
	__ret; \
})


namespace dvbtee {

namespace decode {

class Descriptor: public Decoder {
public:
	bool isValid() { return m_valid; }

protected:
	Descriptor(Decoder &, dvbpsi_descriptor_t*);
	virtual ~Descriptor();

	uint8_t m_tag;
	bool m_valid;
};

}

}

#endif /* __DESCRIPTOR_H__ */
