#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include "alsa.h"
static snd_pcm_t *handle;
static short *buffer;
static snd_pcm_uframes_t frames;
static int stop_capture;

void signal_callback_handler(int signum)
{
    // next signal should stop app.
    signal(SIGINT, SIG_DFL);
    stop_capture = 1;
    printf("\nTerminating capture...\n");
}

int alsa_start_capture(const char* pcm_device, unsigned long period_size, unsigned int channels, unsigned int fs)
{
    int rc;
    int size;
    
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    
    printf("Opening PCM device: %s\n", pcm_device);

    rc = snd_pcm_open(&handle, pcm_device,
                      SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0)
    {
        printf("unable to open pcm device: %s\n",
                snd_strerror(rc));
        return rc;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handle, params,
                                 SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params,
                                 SND_PCM_FORMAT_S16_LE);

    rc = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (rc < 0)
    {
        printf("snd_pcm_hw_params_set_channels failed: %s\n",
                snd_strerror(rc));
        return rc;
    }

    /* Set bits/second sampling rate */
    val = fs;
    rc = snd_pcm_hw_params_set_rate_near(handle, params,
                                    &val, &dir);
    if (rc < 0)
    {
        printf("snd_pcm_hw_params_set_rate_near failed: %s\n",
                snd_strerror(rc));
        return rc;
    }

    frames = period_size;
    rc = snd_pcm_hw_params_set_period_size_near(handle,
                                           params, &frames, &dir);
    if (rc < 0)
    {
        printf("snd_pcm_hw_params_set_period_size_near failed: %s\n",
                snd_strerror(rc));
        return rc;
    }

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0)
    {
        printf("unable to set hw parameters: %s\n",
                snd_strerror(rc));
        return rc;
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params,
                                      &frames, &dir);
    
    size = frames * sizeof(short) * channels; 
    buffer = (short *)malloc(size);

    rc = snd_pcm_prepare(handle);
    if (rc < 0)
    {
        printf("snd_pcm_prepare failed: %s\n", snd_strerror(rc));
        return rc;
    }

    return 0;
}

int alsa_close(void)
{
    if (handle)
    {
        //snd_pcm_drain(handle);
        snd_pcm_close(handle);
        handle = NULL;
    }

    if (buffer)
    {
        free(buffer);
        buffer = NULL;
    }

    stop_capture = 0;

    return 0;
}

int alsa_process(alsa_add_block_cb_t fn)
{
    int rc;

    signal(SIGINT, signal_callback_handler);
    printf("\nPress CTRL-C to stop audio capture\n\n");

    while (!stop_capture)
    {
        rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE)
        {
            /* EPIPE means overrun */
            printf("overrun occurred\n");
            snd_pcm_prepare(handle);
        }
        else if (rc < 0)
        {
            printf("error from read: %s\n",
                    snd_strerror(rc));
            return rc;
        }
        else if (rc != (int)frames)
        {
            printf("short read, read %d frames\n", rc);
        }
        else
        {
            fn(buffer);
        }
    }
    

    return 0;
}
