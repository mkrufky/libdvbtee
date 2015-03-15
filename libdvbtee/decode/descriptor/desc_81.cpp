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

#include "desc_81.h"

#include "dvbpsi/dr_81.h" /* AC-3 Audio descriptor */

#define CLASS_MODULE "[ac-3 audio]"

#define dprintf(fmt, arg...) __dprintf(DBG_DESC, fmt, ##arg)

using namespace dvbtee::decode;

static std::string DESC_NAME = "DR[81]";

static const char *num_channels(uint8_t num_channels_code);
static const char *surround_mode(uint8_t surround_mode_code);
static const char *sample_rate(uint8_t sample_rate_code);

desc_81::desc_81(Decoder *parent, dvbpsi_descriptor_t *p_descriptor)
 : Descriptor(parent, DESC_NAME, p_descriptor)
{
	if (!desc_check_tag(m_tag, 0x81)) return;

	dvbpsi_ac3_audio_dr_t* dr = dvbpsi_DecodeAc3AudioDr(p_descriptor);
	if (desc_dr_failed(dr)) return;
#if 0
	dprintf("sample rate: %s", sample_rate(dr->i_sample_rate_code));
	dprintf("bsid: %02x", dr->i_bsid);
	dprintf("bit rate code: %02x", dr->i_bit_rate_code);
	dprintf("surround mode: %s", surround_mode(dr->i_surround_mode));
	dprintf("bsmod: %02x", dr->i_bsmod);
	dprintf("num channels: %s", num_channels(dr->i_num_channels));
	dprintf("full svc: %s", (dr->b_full_svc) ? "true" : "false");
	dprintf("description: %s", dr->text);
	if (dr->b_language_flag)
		dprintf("language: %c%c%c",
			dr->language[0],
			dr->language[1],
			dr->language[2]);
	if (dr->b_language_flag_2)
		dprintf("language_2: %c%c%c",
			dr->language_2[0],
			dr->language_2[1],
			dr->language_2[2]);
#endif
	set("sampleRate", std::string(sample_rate(dr->i_sample_rate_code)));
	set("bsid", dr->i_bsid);
	set("bitRateCode", dr->i_bit_rate_code);
	set("surroundMode", std::string(surround_mode(dr->i_surround_mode)));
	set("bsmod", dr->i_bsmod);
	set("numChannels", std::string(num_channels(dr->i_num_channels)));
	set("fullSvc", (dr->b_full_svc) ? true : false);
	set("description", std::string((const char*)dr->text));
	if (dr->b_language_flag) {
		char lang[4] = { 0 };
		for (unsigned int i = 0; i < 3; i++) lang[i] = dr->language[i];
		set("language", std::string(lang));
	}
	if (dr->b_language_flag_2) {
		char lang[4] = { 0 };
		for (unsigned int i = 0; i < 3; i++) lang[i] = dr->language_2[i];
		set("language2", std::string(lang));
	}

	dprintf("%s", toJson().c_str());

	setValid(true);
}

desc_81::~desc_81()
{
	//
}

static const char *sample_rate(uint8_t sample_rate_code)
{
	const char *ret = NULL;

	switch (sample_rate_code & 0x07) {
	case 0:
		ret = "48";
		break;
	case 1:
		ret = "44.1";
		break;
	case 2:
		ret = "32";
		break;
	case 3:
		ret = "Reserved";
		break;
	case 4:
		ret = "48 or 44.1";
		break;
	case 5:
		ret = "48 or 32";
		break;
	case 6:
		ret = "44.1 or 32";
		break;
	case 7:
		ret = "48 or 44.1 or 32";
		break;
	}
	return ret;
}

static const char *surround_mode(uint8_t surround_mode_code)
{
	const char *ret = NULL;

	switch (surround_mode_code & 0x03) {
	case 0:
		ret = "Not indicated";
		break;
	case 1:
		ret = "NOT Dolby surround encoded";
		break;
	case 2:
		ret = "Dolby surround encoded";
		break;
	case 3:
		ret = "Reserved";
		break;
	}
	return ret;
}

static const char *num_channels(uint8_t num_channels_code)
{
	const char *ret = NULL;

	switch (num_channels_code & 0x0f) {
	case 0x00:
		ret = "1 + 1";
		break;
	case 0x01:
		ret = "1/0";
		break;
	case 0x02:
		ret = "2/0";
		break;
	case 0x03:
		ret = "3/0";
		break;
	case 0x04:
		ret = "2/1";
		break;
	case 0x05:
		ret = "3/1";
		break;
	case 0x06:
		ret = "2/2";
		break;
	case 0x07:
		ret = "3/2";
		break;
	case 0x08:
		ret = "1";
		break;
	case 0x09:
		ret = "<= 2";
		break;
	case 0x0a:
		ret = "<= 3";
		break;
	case 0x0b:
		ret = "<= 4";
		break;
	case 0x0c:
		ret = "<= 5";
		break;
	case 0x0d:
		ret = "<= 6";
		break;
	case 0x0e:
		ret = "Reserved";
		break;
	case 0x0f:
		ret = "Reserved";
		break;
	}
	return ret;
}

REGISTER_DESCRIPTOR_FACTORY(0x81, desc_81);
