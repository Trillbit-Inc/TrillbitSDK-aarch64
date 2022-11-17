#ifndef _WAVE_H_
#define _WAVE_H_

typedef int (*wave_add_block_cb_t)(void* block);

int wave_start(const char* filename, unsigned int samples_per_block, int* n_channels);
int wave_process(wave_add_block_cb_t fn);
int wave_close(void);

#endif //_WAVE_H_