#ifndef _ALSA_H_
#define _ALSA_H_

typedef int (*alsa_add_block_cb_t)(void* block);

int alsa_start_capture(const char* pcm_device, unsigned long period_size, unsigned int channels, unsigned int fs);
int alsa_process(alsa_add_block_cb_t fn);
int alsa_close(void);

#endif //_ALSA_H_