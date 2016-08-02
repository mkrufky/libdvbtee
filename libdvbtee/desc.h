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

#ifndef __DESC_H__
#define __DESC_H__

#include <stdint.h>
//#include "dvbpsi/descriptor.h"

#include <map>

/* THIS CODE IS OBSOLETE AND ONLY PROVIDED FOR BACKWARDS-COMPAT
 * (when the old decoder is enabled, for debugging purposes)
 * Instead, build the code under decode/descriptor/
 */

typedef std::map<uint16_t, uint16_t> map_lcn; /* service ID, lcn */

typedef struct
{
	unsigned char lang[4];
	unsigned char name[256];
	unsigned char text[256];
} dr4d_t;

typedef struct
{
	uint8_t       stream_type;
	uint16_t      elementary_pid;
	unsigned char iso_639_code[3];
} dra1_t;

typedef struct
{
	uint8_t       audio_type;
	unsigned char iso_639_code[3];
} dr0a_t;

typedef std::map<uint8_t, dr0a_t> map_dr0a;
typedef std::map<uint16_t, dra1_t> map_dra1;

class desc
{
public:
	desc();
	~desc();

	void decode(dvbpsi_descriptor_t*);

	//FIXME:	const map_lcn* get_lcn() { return &lcn; };
	map_lcn lcn;

	map_dr0a _0a;

	dr4d_t _4d;

	map_dra1 _a1;

	unsigned char provider_name[256];
	unsigned char service_name[256];
private:
	bool iso639language(dvbpsi_descriptor_t*);
	bool service(dvbpsi_descriptor_t*);
	bool short_event(dvbpsi_descriptor_t*);
	bool freq_list(dvbpsi_descriptor_t*);
	bool ac3_audio(dvbpsi_descriptor_t*);
	bool _lcn(dvbpsi_descriptor_t*);
	bool caption_service(dvbpsi_descriptor_t*);
	bool extended_channel_name(dvbpsi_descriptor_t*);
	bool service_location(dvbpsi_descriptor_t*);
};

#endif /* __DECODE_H__ */
