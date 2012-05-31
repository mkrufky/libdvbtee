/*****************************************************************************
 * Copyright (C) 2011-2012 Michael Krufky
 *
 * Author: Michael Krufky <mkrufky@linuxtv.org>
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

#ifndef __DESC_H__
#define __DESC_H__

#include <stdint.h>
//#include "dvbpsi/descriptor.h"

#include <map>

typedef std::map<uint16_t, uint16_t> map_lcn; /* service ID, lcn */
typedef std::map<uint16_t, uint16_t> map_lcn; /* service ID, lcn */

typedef struct
{
	unsigned char lang[4];
	unsigned char name[256];
	unsigned char text[256];
} dr4d_t;

class desc
{
public:
	desc();
	~desc();

	void decode(dvbpsi_descriptor_t*);

	//FIXME:	const map_lcn* get_lcn() { return &lcn; };
	map_lcn lcn;

	dr4d_t _4d;
private:
	bool service(dvbpsi_descriptor_t*);
	bool short_event(dvbpsi_descriptor_t*);
	bool freq_list(dvbpsi_descriptor_t*);
	bool _lcn(dvbpsi_descriptor_t*);
};

#endif /* __DECODE_H__ */
