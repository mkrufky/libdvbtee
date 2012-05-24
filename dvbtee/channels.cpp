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
		if (atsc_vsb_chan_to_freq(channel) == frequency)
			return channel;
	}
	return 0;
}

unsigned int atsc_qam_freq_to_chan(const unsigned int frequency)
{
	for (int channel=2; channel <= 133; channel++) {
		if (atsc_qam_chan_to_freq(channel) == frequency)
			return channel;
	}
	return 0;
}
