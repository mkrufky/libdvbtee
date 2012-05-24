#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include <stdint.h>
#include <time.h>

#if 0
#include "dvbpsi/descriptor.h"
#else
#include "decode.h"
#endif

#define ST_VideoMpeg1           0x01
#define ST_VideoMpeg2           0x02
#define ST_AudioMpeg1           0x03
#define ST_AudioMpeg2           0x04
#define ST_Private              0x06
#define ST_AudioAAC_ADTS        0x0F
#define ST_VideoMpeg4           0x10
#define ST_AudioAAC_LATM        0x11
#define ST_VideoH264            0x1B

#define ST_ATSC_VideoMpeg2      0x80
#define ST_ATSC_AudioAC3        0x81
#define ST_ATSC_AudioEAC3       0x87

inline const char* streamtype_name(uint8_t es_type)
{
	switch (es_type) {
	case ST_VideoMpeg2:
		return "Video MPEG-2";
	case ST_ATSC_VideoMpeg2:
		return "Video MPEG-2 (ATSC)";
	case ST_VideoH264:
		return "Video H.264";
	case ST_AudioMpeg1:
		return "Audio MPEG-1";
	case ST_AudioMpeg2:
		return "Audio MPEG-2";
	case ST_Private:
		return "Private (AC3/EAC3/TT/ST)";
	case ST_AudioAAC_ADTS:
		return "Audio AAC MPEG-2 (ADTS)";
	case ST_AudioAAC_LATM:
		return "Audio AAC MPEG-4 (LATM)";
	case ST_ATSC_AudioAC3:
		return "Audio AC3 (ATSC)";
	case ST_ATSC_AudioEAC3:
		return "Audio E-AC3 (ATSC)";
	}
	return "Unknown";
}


void dump_descriptors(const char* str, dvbpsi_descriptor_t* descriptors);
unsigned char* get_descriptor_text(unsigned char* desc, uint8_t len, unsigned char* text);

time_t      datetime_utc(uint64_t time);
time_t atsc_datetime_utc(uint32_t in_time);

int decode_multiple_string(const uint8_t* data, uint8_t len, unsigned char* text);

#endif /* __FUNCTIONS_H__ */
