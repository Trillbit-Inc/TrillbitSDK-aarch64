#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <sndfile.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "trill.h"
#include "trill_error.h"

#include "wave.h"
#include "alsa.h"

typedef int (*add_block_fn_t)(void*);

static trill_init_opts_t trill_init_opts;
static void* trill_handle;
static volatile int exit_trill_thread;
static int audio_tx_on;
static unsigned char tx_payload_buf[TRILL_MAX_DATA_PAYLOAD_LEN];

#define SAMPLES_PER_BLOCK           1024
#define SAMPLE_SIZE                 sizeof(int16_t)
#define RX_N_BLOCKS                 20
#define WAVE_FILE_EXT               ".wav"
#define SAMPLE_RATE                 48000

enum {
    PLAY_ST_WAIT_START = 0,
    PLAY_ST_INPROGRESS,
    PLAY_ST_WAIT_DRAIN
};

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

static void audio_tx_enable_cb(int enable)
{
    audio_tx_on = enable ? 1: 0;
}

static int do_init_trill(void)
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

    trill_init_opts.n_rx_channels = 0;
    trill_init_opts.rx_channels_en_bm = 0; 

    // enable blocking while adding blocks
    trill_init_opts.aud_buf_en_rx_add_block = 0; 

    trill_init_opts.aud_buf_rx_block_size_bytes = SAMPLES_PER_BLOCK * SAMPLE_SIZE;
    trill_init_opts.aud_buf_rx_n_blocks = 2;
	trill_init_opts.aud_buf_rx_notify_cb = NULL;

	trill_init_opts.aud_buf_tx_block_size_bytes = SAMPLES_PER_BLOCK * SAMPLE_SIZE;
    trill_init_opts.aud_buf_tx_n_blocks = 2;
	trill_init_opts.aud_buf_tx_notify_cb = NULL;

	trill_init_opts.audio_tx_enable_fn = audio_tx_enable_cb;
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

static int fetch_audio(add_block_fn_t add_block_fn)
{
    int ret;
    int state = PLAY_ST_WAIT_START;
    int16_t* block_addr = NULL;
    
    while (1)
    {
        //printf("state = %d\n", state);
        switch(state)
        {
            case PLAY_ST_WAIT_START:
                if (audio_tx_on)
                {
                    state = PLAY_ST_INPROGRESS;
                }
                break;
            case PLAY_ST_INPROGRESS:
                if (!audio_tx_on)
                {
                    state = PLAY_ST_WAIT_DRAIN;
                    break;
                }

                ret = trill_acquire_audio_block(trill_handle, 
                            &block_addr, 
                            1);
                if (ret < 0)
                {
                    printf("trill_acquire_audio_block failed = %d\n", ret);
                    return ret;
                }
                
                ret = add_block_fn(block_addr);
                if (ret < 0)
                {
                    return ret;
                }

                ret = trill_release_audio_block(trill_handle);
                if (ret < 0)
                    return ret;
                break;
            case PLAY_ST_WAIT_DRAIN:
                ret = trill_acquire_audio_block(trill_handle, 
                            &block_addr, 
                            0);
                if (ret < 0)
                {
                    if (ret == TRILL_ERR_AUDIO_TX_BLOCK_NOT_AVAILABLE)
                    {
                        state = PLAY_ST_WAIT_START;
                        ret = 0;
                    }
                    return ret;
                }
                
                ret = add_block_fn(block_addr);
                if (ret < 0)
                {
                    return ret;
                }

                ret = trill_release_audio_block(trill_handle);
                if (ret < 0)
                    return ret;
                break;
            default:
                printf("Unknown play state = %d\n", state);
                state = PLAY_ST_WAIT_START;
                break;
        }
    }
}

static int read_payload_file(const char* filepath)
{
    int ret;
    FILE* fp = fopen(filepath, "rb");
    if (fp == NULL)
    {
        printf("payload file not found\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    // cap to max length;
    if (length > TRILL_MAX_DATA_PAYLOAD_LEN)
        length = TRILL_MAX_DATA_PAYLOAD_LEN;
    
    if ((ret = fread(tx_payload_buf, length, 1, fp)) != 1)
    {
        printf("failed to read payload file: %d, %lu\n", ret, length);
        ret = -1;
    }
    else
    {
        ret = (int) length;
    }

    fclose(fp);

    return ret;
}

int main(int argc, char* argv[])
{
    int ret;
    pthread_t th_process = 0;
    trill_tx_params_t tx_params;
    trill_data_config_range_t tx_cfg_range = TRILL_DATA_CFG_RANGE_NEAR;
    int tx_payload_len;
    int is_src_wavefile = 0;
    int n_channels = 1; // ideally query from source.
    const char* device_id;

    printf("Wave Generator\n");
    
    if (argc < 3)
    {
        printf("Args: <payload_filepath> <output_wave_filepath.wav | alsa PCM device name> [data_cfg_range]\n");
        return -1;
    }

    if (argc > 3)
    {
        tx_cfg_range = atoi(argv[3]);
        if ((tx_cfg_range < TRILL_DATA_CFG_RANGE_NEAR) ||
            (tx_cfg_range >= TRILL_DATA_CFG_UNKNOWN))
        {
            printf("Unknown config range: %d\n", tx_cfg_range);
            return -1;
        }
    }

    ret = trill_get_device_id(&device_id);
    if (ret < 0)
    {
        printf("Failed to get device id\n");
        return -1;
    }

    printf("Device ID: %s\n", device_id);
    
    // Is input a wave file
    const char* token = strcasestr(argv[2], WAVE_FILE_EXT);
    if (token)
    {
        //extension will be at end.
        if (strlen(token) == strlen(WAVE_FILE_EXT))
        {
            is_src_wavefile = 1;
        }
    }
    
    ret = read_payload_file(argv[1]);
    if (ret < 0)
    {
        return -1;
    }
    tx_payload_len = ret;

    if (is_src_wavefile)
    {
        ret = wave_start(argv[2], SAMPLES_PER_BLOCK, SAMPLE_RATE, n_channels);
    }
    else
    {
        ret = alsa_start_streaming(argv[2], SAMPLES_PER_BLOCK, n_channels, SAMPLE_RATE);
    }

    if (ret < 0)
    {
        goto err;
    }


    ret = do_init_trill();
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

    tx_params.ck_nonce = NULL;
    tx_params.data_cfg_range = tx_cfg_range;
    tx_params.ssi = 0;
    
    printf("\nTx parameters:\n");
    printf("data_cfg_range: %d\n", tx_params.data_cfg_range);
    printf("ssi: %d\n", tx_params.ssi);
    printf("payload length: %d\n", tx_payload_len);
    printf("\n");


    ret = trill_tx_data(trill_handle, &tx_params, 
	        tx_payload_buf, tx_payload_len);
    if (ret < 0)
    {
        printf("trill_tx_data failed = %d\n", ret);
        goto err;
    }
    
    if (is_src_wavefile)
    {
        ret = fetch_audio(wave_add_block);
    }
    else
    {
        ret = fetch_audio(alsa_add_block);
    }

err:
    printf("Cleaning up...\n");

    if (th_process)
    {
        exit_trill_thread = 1;
        pthread_join(th_process, NULL);
    }

    if (trill_handle)
    {
        trill_deinit(trill_handle);
    }

    if (is_src_wavefile)
    {
        ret = wave_close();
    }
    else
    {
        ret = alsa_close();
    }

    return ret;
}
