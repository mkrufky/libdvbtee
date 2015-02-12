/*****************************************************************************
 * Copyright (C) 2011-2014 Michael Ira Krufky
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

#define DBG 0

#include <stdio.h>
#include <stdlib.h>

#include "functions.h"
#include "log.h"
#define CLASS_MODULE "desc"

//#define DVBPSI_SUPPORTS_DR_81_86_A0_A1 (DVBPSI_VERSION_INT > ((1<<16)+(1<<8)+1))
#define DVBPSI_SUPPORTS_DR_81_86_A0_A1 1

#include "dvbpsi/dr_0a.h" /* ISO639 language descriptor */
#include "dvbpsi/dr_48.h" /* service descriptor */
#include "dvbpsi/dr_4d.h" /* short event descriptor */
#include "dvbpsi/dr_62.h" /* frequency list descriptor */
#if DVBPSI_SUPPORTS_DR_81_86_A0_A1
#include "dvbpsi/dr_81.h" /* AC-3 Audio descriptor */
#endif
#include "dvbpsi/dr_83.h" /* LCN descriptor */
#if DVBPSI_SUPPORTS_DR_81_86_A0_A1
#include "dvbpsi/dr_86.h" /* caption service descriptor */
#include "dvbpsi/dr_a0.h" /* extended channel name descriptor */
#include "dvbpsi/dr_a1.h" /* service location descriptor */
#endif

#include "desc.h"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

#define DT_ISO639Language             0x0a
#define DT_Service                    0x48
#define DT_ShortEvent                 0x4d
#define DT_Teletext                   0x56
#define DT_FrequencyList              0x62
#define DT_Ac3Audio                   0x81
#define DT_LogicalChannelNumber       0x83
#define DT_CaptionService             0x86
#define DT_ExtendedChannelName        0xa0
#define DT_ServiceLocation            0xa1


desc::desc()
 : store(this)
{
	dprintf("()");
}

using namespace dvbtee::decode;

desc::~desc()
{
	dprintf("()");
}

void desc::decode(dvbpsi_descriptor_t* p_descriptor)
{
	while (p_descriptor) {
		bool ret = store.add(p_descriptor);
		if (!ret) switch (p_descriptor->i_tag) {
		case DT_ISO639Language:
		case DT_Service:
		case DT_ShortEvent:
		case DT_FrequencyList:
		case DT_Ac3Audio:
		case DT_LogicalChannelNumber:
		case DT_CaptionService:
		case DT_ExtendedChannelName:
		case DT_ServiceLocation:
			ret = store.add(p_descriptor);
			break;
		default:
			dprintf("unknown descriptor tag: %02x", p_descriptor->i_tag);
			ret = false;
			break;
		}
		if (!ret)
			dprintf("failed to decode descriptor! tag: %02x", p_descriptor->i_tag);

		p_descriptor = p_descriptor->p_next;
	}
}

Descriptor *desc::lastDesc(uint8_t tag)
{
	std::vector<Descriptor*> D = store.get(tag);
	ssize_t s = D.size();
	if (s > 1) dprintf("tag: %02x, %ld collected, returning last", tag, s);
	if (s) return D[s-1];
	return NULL;
}
