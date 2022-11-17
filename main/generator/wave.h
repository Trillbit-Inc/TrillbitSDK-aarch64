#ifndef _WAVE_H_
#define _WAVE_H_


int wave_start(const char* filename, unsigned int samples_per_block, int fs, int n_channels);
int wave_add_block(void* block);
int wave_close(void);

#endif //_WAVE_H_