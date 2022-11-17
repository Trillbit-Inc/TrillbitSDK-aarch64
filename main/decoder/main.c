#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sndfile.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include "trill.h"
#include "trill_error.h"

#include "wave.h"
#include "alsa.h"

static trill_init_opts_t trill_init_opts;
static void* trill_handle;
static volatile int exit_trill_thread;

#define SAMPLES_PER_BLOCK           1024
#define SAMPLE_SIZE                 sizeof(int16_t)
#define RX_N_BLOCKS                 20
#define TX_N_BLOCKS                 2
#define WAVE_FILE_EXT               ".wav"
#define SAMPLE_RATE                 48000

static void log_print(const char* str)
{
    printf("%s", str);
}

static unsigned long timer_us(void)
{
    struct timeval tv;
    static unsigned long start_tv_sec;

    gettimeofday(&tv,NULL);
    
    if (start_tv_sec == 0)
    {
        start_tv_sec = tv.tv_sec;
    }

    unsigned long t = (tv.tv_sec - start_tv_sec) * 1000000;
    t += tv.tv_usec;

    return t;
}

static void data_link_evt_handler(const trill_data_link_event_params_t* params)
{
	static int count = 0;
    
	switch(params->event)
	{
		case TRILL_DATA_LINK_EVT_DATA_RCVD:
            printf("%d) Payload (%d):\n", ++count, params->payload_len);
            printf("%.*s\n\n", params->payload_len, params->payload);
            break;
		case TRILL_DATA_LINK_EVT_DATA_SENT:
			break;
		default:
			printf("Unknown data link event");
			break;
	}
}

static int add_audio_block(void* block)
{
    return trill_add_audio_block(trill_handle, block);
}

static int do_init_trill(int channels, int en_rx_add_blocking)
{
    int ret;

    FILE* fp = fopen("trillbit.lic", "rb");
    if (fp == NULL)
    {
        printf("license file not found\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char* lic_buf = malloc(length + 1);

    if (!lic_buf)
    {
        printf("failed to allocate license buffer. Its length %lu\n", length);
        ret = -1;
        goto err;
    }

    if ((ret = fread(lic_buf, length, 1, fp)) != 1)
    {
        printf("failed to read license file: %d %lu\n", ret, length);
        ret = -1;
        goto err;
    }

    lic_buf[length] = 0;

    printf("Loaded License: %.*s...%s\n", 10, lic_buf, &lic_buf[length-10]);

    trill_init_opts.n_rx_channels = channels;
    /*
        Bits     7 6 5 4 3 2 1 0
        Channels 7 6 5 4 3 2 1 0
        Enabled  0 0 0 0 0 0 0 1
    */
    trill_init_opts.rx_channels_en_bm = 1; 

    // enable blocking while adding blocks
    trill_init_opts.aud_buf_en_rx_add_block = en_rx_add_blocking; 

    trill_init_opts.aud_buf_rx_block_size_bytes = SAMPLES_PER_BLOCK * SAMPLE_SIZE;
    trill_init_opts.aud_buf_rx_n_blocks = RX_N_BLOCKS;
	trill_init_opts.aud_buf_rx_notify_cb = NULL;

	trill_init_opts.aud_buf_tx_block_size_bytes = SAMPLES_PER_BLOCK * SAMPLE_SIZE;
    trill_init_opts.aud_buf_tx_n_blocks = TX_N_BLOCKS;
	trill_init_opts.aud_buf_tx_notify_cb = NULL;

	trill_init_opts.audio_tx_enable_fn = NULL;
	trill_init_opts.data_link_cb = data_link_evt_handler;

	trill_init_opts.b64_ck = NULL; 
	trill_init_opts.b64_ck_nonce = NULL;
	trill_init_opts.b64_license = lic_buf;

	trill_init_opts.mem_alloc_fn = malloc;
    trill_init_opts.mem_aligned_alloc_fn = aligned_alloc;
	trill_init_opts.mem_free_fn = free;
	trill_init_opts.logger_fn = log_print;
	trill_init_opts.timer_get_fn = timer_us;

    
    ret = trill_init(&trill_init_opts, &trill_handle);
    printf("trill_init = %d\n", ret);
    
err:
    free(lic_buf);
    fclose(fp);
    return ret;
}

static void* trill_thread(void* arg)
{
    int ret;

    (void) arg;

    while (exit_trill_thread == 0)
    {
        ret = trill_process(trill_handle);
        if (ret < 0)
        {
            printf("trill_process = %d\n", ret);
        }
    }

    return NULL;
}

static void flush_rx_path(int n_channels)
{
    size_t size = n_channels * SAMPLES_PER_BLOCK * sizeof(int16_t);
    int ret;

    void* silent_buf = malloc(size);
    memset(silent_buf, 0, size);
    
    int n = 2 * RX_N_BLOCKS;
    for (int i = 0; i < n; i++)
    {
        //printf("%d) Flush rx path\n", i);

        ret = trill_add_audio_block(trill_handle, silent_buf);
        if (ret < 0)
        {
            printf("trill_add_audio_block = %d\n", ret);
            break;
        }
    }

    exit_trill_thread = 1;

    free(silent_buf);
}

int main(int argc, char* argv[])
{
    int ret;
    pthread_t th_process;
    int is_src_wavefile = 0;
    int n_channels = 2; // ideally query from source.
    int en_rx_add_blocking = 0;
    const char* device_id;

    printf("Wave Decoder\n");

    if (argc != 2)
    {
        printf("Args: <wave_file_path.wav | alsa PCM device name>\n");
        return -1;
    }

    ret = trill_get_device_id(&device_id);
    if (ret < 0)
    {
        printf("Failed to get device id\n");
        return -1;
    }

    printf("Device ID: %s\n", device_id);

    // Is input a wave file
    const char* token = strcasestr(argv[1], WAVE_FILE_EXT);
    if (token)
    {
        //extension will be at end.
        if (strlen(token) == strlen(WAVE_FILE_EXT))
        {
            is_src_wavefile = 1;
            en_rx_add_blocking = 1;
        }
    }

    if (is_src_wavefile)
    {
        ret = wave_start(argv[1], SAMPLES_PER_BLOCK, &n_channels);
        if (ret < 0)
        {
            goto err;
        }
    }
    else
    {
        ret = alsa_start_capture(argv[1], SAMPLES_PER_BLOCK, n_channels, SAMPLE_RATE);
        if (ret < 0)
        {
            goto err;
        }
    }
    
    ret = do_init_trill(n_channels, en_rx_add_blocking);
    if (ret < 0)
    {
        goto err;
    }

    ret = pthread_create(&th_process, NULL, trill_thread, NULL);
    if (ret != 0)
    {
        printf("pthread_create failed = %d\n", ret);
        goto err;
    }

    if (is_src_wavefile)
    {
        ret = wave_process(add_audio_block);
    }
    else
    {
        ret = alsa_process(add_audio_block);
    }

    if (ret < 0)
    {
        goto err;
    }

    /**
     * @brief Just setting the exit flag is not sufficient.
     * As the process call may have blocked for input blocks from audio buffer.
     * Flush dummy blocks into the audio buffer so that process call can return
     * and check for exit flag.
     */
    printf("Flushing rx path...\n");
    flush_rx_path(n_channels);

    pthread_join(th_process, NULL);
    printf("Cleaning up...\n");

err:
    if (trill_handle)
    {
        trill_deinit(trill_handle);
    }

    if (is_src_wavefile)
    {
        wave_close();
    }
    else
    {
        alsa_close();
    }

    return ret;
}
