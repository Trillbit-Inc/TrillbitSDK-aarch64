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
static int16_t* audio_input_buffer;
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

static SNDFILE* open_wave_file(const char* filepath, SF_INFO* sfinfo)
{
    printf("\nOpening wave file: %s\n", filepath);
    SNDFILE* file = NULL;
    
    sfinfo->format = 0;
    file = sf_open(filepath, SFM_READ, sfinfo);
    if (!file)
    {
        printf("error: %s\n", sf_strerror(NULL));
        return NULL;
    }

    printf("Wave file info:\n");
    printf("n_frames: %ld\n", sfinfo->frames);
    printf("n_channels: %d\n", sfinfo->channels);
    printf("Fs: %u\n", sfinfo->samplerate);
    printf("Data type format (%u): %s\n", 
        sfinfo->format & SF_FORMAT_SUBMASK,
        wavfmt_to_str(sfinfo->format & SF_FORMAT_SUBMASK));

    long int t_samples = sfinfo->frames * sfinfo->channels;
    printf("Total samples: %ld\n", t_samples);
    printf("Total Blocks (%d): %ld\n", 
        b_samples, 
        t_samples / b_samples);
    printf("\n\n");

    return file;
}

int wave_process(wave_add_block_cb_t fn)
{
    int ret;

    while (1)
    {
        sf_count_t read_count = sf_readf_short(h_wav_file, audio_input_buffer, b_samples);
        
        if (read_count == b_samples)
        {
            ret = fn(audio_input_buffer);
            if (ret < 0)
            {
                printf("add_audio_block = %d\n", ret);
                break;
            }
        }
        else
        {
            break;
        }
    }

    return 0;
}

int wave_start(const char* filename, unsigned int samples_per_block, int* n_channels)
{
    b_samples = samples_per_block;
    
    h_wav_file = open_wave_file(filename, &sfinfo);
    if (!h_wav_file)
    {
        return -1;
    }

    audio_input_buffer = malloc(sfinfo.channels * samples_per_block * sizeof(int16_t));
    if (!audio_input_buffer)
    {
        return -1;
    }

    *n_channels = sfinfo.channels;
    
    
    return 0;
}

int wave_close(void)
{
    if (h_wav_file)
    {
        sf_close(h_wav_file);
        h_wav_file = NULL;
    }

    if (audio_input_buffer)
    {
        free(audio_input_buffer);
        audio_input_buffer = NULL;
    }

    return 0;
}
