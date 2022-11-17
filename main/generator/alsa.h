#ifndef _ALSA_H_
#define _ALSA_H_

int alsa_start_streaming(const char* pcm_device, unsigned long period_size, unsigned int channels, unsigned int fs);
int alsa_add_block(void* block);
int alsa_close(void);

#endif //_ALSA_H_