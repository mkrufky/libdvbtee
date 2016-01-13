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

#include <stdlib.h>
#include "channels.h"

static int atsc_vsb_base_offset(const unsigned int channel)
{
	int base_offset;

	if (channel < 5)
		base_offset = 45000000;
	else if (channel < 7)
		base_offset = 49000000;
	else if (channel < 14)
		base_offset = 135000000;
	else
		base_offset = 389000000;

	return base_offset;
}

static int atsc_qam_base_offset(const unsigned int channel)
{
	int base_offset;

	if (channel < 5)
		base_offset = 45000000;
	else if (channel < 7)
		base_offset = 49000000;
	else if (channel < 14)
		base_offset = 135000000;
	else if (channel < 17)
		base_offset = 39012500;
	else if (channel < 23)
		base_offset = 39000000;
	else if (channel < 25)
		base_offset = 81000000;
	else if (channel < 54)
		base_offset = 81012500;
	else if (channel < 95)
		base_offset = 81000000;
	else if (channel < 98)
		base_offset = -477000000;
	else if (channel < 100)
		base_offset = -476987500;
	else
		base_offset = 51000000;

	return base_offset;
}

static int dvbt_base_offset(const unsigned int channel)
{
	int base_offset;

	if (channel < 13)
		base_offset = 142500000;
	else if (channel < 70)
		base_offset = 306000000;
	else
		base_offset = 0; /* FIXME */

	return base_offset;
}

unsigned int atsc_vsb_chan_to_freq(const unsigned int channel)
{
	return (unsigned int)((int)channel*6000000 + atsc_vsb_base_offset(channel));
}

unsigned int atsc_qam_chan_to_freq(const unsigned int channel)
{
	return (unsigned int)((int)channel*6000000 + atsc_qam_base_offset(channel));
}

unsigned int atsc_vsb_freq_to_chan(const unsigned int frequency)
{
	for (int channel=2; channel <= 69; channel++) {
		if (abs((int)atsc_vsb_chan_to_freq(channel) - (int)frequency) < 1000000)
			return channel;
	}
	return 0;
}

unsigned int atsc_qam_freq_to_chan(const unsigned int frequency)
{
	for (int channel=2; channel <= 133; channel++) {
		if (abs((int)atsc_qam_chan_to_freq(channel) - (int)frequency) < 1000000)
			return channel;
	}
	return 0;
}

#if 0
#define VHF_LOWER_FREQUENCY 177500 /* channel 5 */
#define VHF_UPPER_FREQUENCY 226500 /* channel 12 */
#define UHF_LOWER_FREQUENCY 474000 /* channel 21 */
#define UHF_UPPER_FREQUENCY 858000 /* channel 69 */
#endif
unsigned int dvbt_chan_to_freq(const unsigned int channel)
{
	return (unsigned int)((int)channel* ((channel <= 12) ? 7000000 : 8000000) + dvbt_base_offset(channel));
}

unsigned int dvbt_freq_to_chan(const unsigned int frequency)
{
	for (int channel=5; channel <= 69; channel++) {
		if (abs((int)dvbt_chan_to_freq(channel) - (int)frequency) < 1000000)
			return channel;
	}
	return 0;
}
