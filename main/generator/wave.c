#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <sndfile.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "wave.h"

static SNDFILE* h_wav_file = NULL;
static SF_INFO sfinfo;
static unsigned int b_samples;

static const char* wavfmt_to_str(int fmt)
{
    switch(fmt)
    {
        case 1: return "PCM_S8";
        case 2: return "PCM_16";
        case 3: return "PCM_24";
        case 4: return "PCM_32";
        case 6: return "FLOAT";
        case 7: return "DOUBLE";
        default: return "Unknown";
    }
}

static SNDFILE* open_wave_file(const char* filepath, SF_INFO* sfinfo, int fs, int channels)
{
    printf("\nOpening wave file: %s\n", filepath);
    SNDFILE* file = NULL;

    sfinfo->format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    sfinfo->channels = channels;
    sfinfo->samplerate = fs;
    sfinfo->frames = 0; // not used
    sfinfo->sections = 0; // not used
    sfinfo->seekable = 0; // not used
    
    file = sf_open(filepath, SFM_WRITE, sfinfo);
    if (!file)
    {
        printf("error: %s\n", sf_strerror(NULL));
        return NULL;
    }

    printf("Wave file info:\n");
    printf("n_channels: %d\n", sfinfo->channels);
    printf("Fs: %u\n", sfinfo->samplerate);
    printf("Data type format (%u): %s\n", 
        sfinfo->format & SF_FORMAT_SUBMASK,
        wavfmt_to_str(sfinfo->format & SF_FORMAT_SUBMASK));

    printf("\n\n");

    return file;
}


int wave_add_block(void* block)
{
    int ret = 0;
    

    sf_count_t write_count = sf_writef_short(h_wav_file, block, b_samples);
                
    if (write_count != b_samples)
    {
        printf("sf_writef_short failed: %ld\n", write_count);
        ret = -1;
    }

    return ret;
}

int wave_start(const char* filename, unsigned int samples_per_block, int fs, int n_channels)
{
    b_samples = samples_per_block;
    
    h_wav_file = open_wave_file(filename, &sfinfo, fs, n_channels);
    if (!h_wav_file)
    {
        return -1;
    }

    return 0;
}

int wave_close(void)
{
    if (h_wav_file)
    {
        sf_close(h_wav_file);
        h_wav_file = NULL;
    }

    return 0;
}
