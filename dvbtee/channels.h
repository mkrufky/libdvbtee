#ifndef __CHANNELS_H__
#define __CHANNELS_H__

unsigned int atsc_vsb_chan_to_freq(const unsigned int channel);
unsigned int atsc_qam_chan_to_freq(const unsigned int channel);
unsigned int atsc_vsb_freq_to_chan(const unsigned int frequency);
unsigned int atsc_qam_freq_to_chan(const unsigned int frequency);

#endif /* __CHANNELS_H__ */
