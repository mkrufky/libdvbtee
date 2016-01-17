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
#include <stdio.h>
#include <string.h>

#include "atsctext.h"

#include "functions.h"

#ifdef _WIN32
#define timegm _mkgmtime
#endif

/* taken from dvbstreamer-2.1.0/src/plugins/atsctoepg.c */
#define dvbpsi_atsc_unix_epoch_offset (315964800)
static uint8_t GPStoUTCSecondsOffset = 14;


static void dump_descriptor(const char *prefix, dvbpsi_descriptor_t *descriptor)
{
	int i;
	char line[(16 * 3) + 1];
	line[0] = 0;
	printf("%sTag : 0x%02x (Length %d)\n", prefix, descriptor->i_tag, descriptor->i_length);
	for (i = 0; i < descriptor->i_length; i ++) {
		if (i && ((i % 16) == 0)) {
			printf("%s%s\n", prefix, line);
			line[0] = 0;
		}
		sprintf(line + strlen(line), "%02x ", descriptor->p_data[i]);
	}
	if (line[0])
		printf("%s%s\n", prefix, line);
}

void dump_descriptors(const char* str, dvbpsi_descriptor_t* descriptors)
{
	while (descriptors) {
		dump_descriptor(str, descriptors);
		descriptors = descriptors->p_next;
	}
}

unsigned char* get_descriptor_text(unsigned char* desc, uint8_t len, unsigned char* text)
{
	unsigned char* p = text;
	for (int i = 0; i < len; i++, desc++) {
		if (*desc == ':')
			*text++ = ' ';
		else if (*desc >= 0x20 && (*desc < 0x80 || *desc > 0x9f))
			*text++ = *desc;
	}
	*text = 0;
	return p;
}

//-----------------------------------------------------------------------------
time_t datetime_utc(uint64_t time)
{
	/* 16 bits coded as Modified Julian Date (MJD) */
	uint32_t mjd = time >> 24;
	int Y = (int)((mjd - 15078.2) / 365.25);
	int M = (int)((mjd - 14956.1 - (int)(Y * 365.25)) / 30.6001);
	int D = mjd - 14956 - (int)(Y * 365.25) - (int)(M * 30.6001);

	M--;
	if ( M > 12 ) {
		Y++;
		M -= 12;
	}

	/* 24 bits coded as 6 digits in 4-bit Binary Coded Decimal (BCD) */
	int h = ((time >> 20) & 0xf) * 10 + ((time >> 16) & 0xf);
	int m = ((time >> 12) & 0xf) * 10 + ((time >>  8) & 0xf);
	int s = ((time >>  4) & 0xf) * 10 + ((time >>  0) & 0xf);

	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));

	tm.tm_year = Y;
	tm.tm_mon  = M - 1;
	tm.tm_mday = D;
	tm.tm_hour = h;
	tm.tm_min  = m;
	tm.tm_sec  = s;

	return timegm(&tm);
}

//-----------------------------------------------------------------------------
static inline void secs_to_tm(uint32_t seconds, struct tm *tm_time)
{
	struct tm *temp_time;
	time_t secs;

	secs = seconds + dvbpsi_atsc_unix_epoch_offset - GPStoUTCSecondsOffset;
	temp_time = gmtime(&secs);
	*tm_time = *temp_time;
}

time_t atsc_datetime_utc(uint32_t in_time)
{
	struct tm temp_time;
	secs_to_tm(in_time, &temp_time);
	return timegm(&temp_time);
}

//-----------------------------------------------------------------------------

int decode_multiple_string(const uint8_t* data, uint8_t len, unsigned char* text, size_t sizeof_text)
{
	ATSCMultipleStrings_t atsc_strings;
	ATSCMultipleStringsConvert(&atsc_strings, (uint8_t*)data, len);
	if (atsc_strings.number_of_strings && atsc_strings.strings && atsc_strings.strings[0].text) {

		if (sizeof_text)
			strncpy((char*)text, atsc_strings.strings[0].text, sizeof_text);
		else
			strcpy((char*)text, atsc_strings.strings[0].text);
	} else
		strcpy((char*)text, "");
	ATSCMultipleStringsDestructor(&atsc_strings);
	return 0;
}

/*****************************************************************************
 * writePSI - taken from libdvbpsi/misc/gen_pat.c
 *****************************************************************************/
void writePSI(uint8_t* p_packet, dvbpsi_psi_section_t* p_section)
{
	p_packet[0] = 0x47;

	while(p_section) {
		uint8_t* p_pos_in_ts;
		uint8_t* p_byte = p_section->p_data;
		uint8_t* p_end  = p_section->p_payload_end + (p_section->b_syntax_indicator ? 4 : 0);

		p_packet[1] |= 0x40;
		p_packet[3] = (p_packet[3] & 0x0f) | 0x10;

		p_packet[4] = 0x00; /* pointer_field */
		p_pos_in_ts = p_packet + 5;

		while((p_pos_in_ts < p_packet + 188) && (p_byte < p_end))
		  *(p_pos_in_ts++) = *(p_byte++);
		while(p_pos_in_ts < p_packet + 188)
		  *(p_pos_in_ts++) = 0xff;
#if 0
		fwrite(p_packet, 1, 188, stdout);
#endif
		p_packet[3] = (p_packet[3] + 1) & 0x0f;

		while(p_byte < p_end) {
			p_packet[1] &= 0xbf;
			p_packet[3] = (p_packet[3] & 0x0f) | 0x10;

			p_pos_in_ts = p_packet + 4;

			while((p_pos_in_ts < p_packet + 188) && (p_byte < p_end))
			  *(p_pos_in_ts++) = *(p_byte++);
			while(p_pos_in_ts < p_packet + 188)
			  *(p_pos_in_ts++) = 0xff;
#if 0
			fwrite(p_packet, 1, 188, stdout);
#endif
			p_packet[3] = (p_packet[3] + 1) & 0x0f;
		}

		p_section = p_section->p_next;
	}
}

/*****************************************************************************
 * URL Encoding/Decoding in C - taken from http://www.geekhideout.com/downloads/urlcode.c
 *****************************************************************************/

/* Converts a hex character to its integer value */
char from_hex(char ch) {
	return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
	static char hex[] = "0123456789abcdef";
	return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
	char *pstr = str, *buf = (char *)malloc(strlen(str) * 3 + 1), *pbuf = buf;
	while (*pstr) {
		if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
			*pbuf++ = *pstr;
		else if (*pstr == ' ')
#if 0
			*pbuf++ = '+';
#else
			*pbuf++ = ' ';
#endif
		else
			*pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_decode(char *str) {
	char *pstr = str, *buf = (char *)malloc(strlen(str) + 1), *pbuf = buf;
	while (*pstr) {
		if (*pstr == '%') {
			if (pstr[1] && pstr[2]) {
				*pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
				pstr += 2;
			}
		} else if (*pstr == '+') {
			*pbuf++ = ' ';
		} else {
			*pbuf++ = *pstr;
		}
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}
