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

#ifndef __CHANNELS_H__
#define __CHANNELS_H__

unsigned int atsc_vsb_chan_to_freq(const unsigned int channel);
unsigned int atsc_qam_chan_to_freq(const unsigned int channel);
unsigned int atsc_vsb_freq_to_chan(const unsigned int frequency);
unsigned int atsc_qam_freq_to_chan(const unsigned int frequency);

unsigned int dvbt_chan_to_freq(const unsigned int channel);
unsigned int dvbt_freq_to_chan(const unsigned int frequency);

#endif /* __CHANNELS_H__ */
