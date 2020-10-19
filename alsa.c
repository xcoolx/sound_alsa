/*
 * Simple sound playback using ALSA API and libasound.
 *
 * Compile:
 * $ cc -o play sound_playback.c -lasound
 * 
 * Usage:
 * $ ./play <sample_rate> <channels> <seconds> < <file>
 * 
 * Examples:
 * $ ./play 44100 2 5 < /dev/urandom
 * $ ./play 22050 1 8 < /path/to/file.wav
 *
 * Copyright (C) 2009 Alessandro Ghedini <al3xbio@gmail.com>
 * --------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Alessandro Ghedini wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can
 * buy me a beer in return.
 * --------------------------------------------------------------
 */

#include <alsa/asoundlib.h>
#include <stdio.h>

//#define PCM_DEVICE "mix_playback"
static char PCM_DEVICE[64] = {'d','e','f','a','u','l','t'};

enum format {
    sample_8     = 1,
    sample_16    = 2,
    sample_24_3  = 3,
    sample_32    = 4,
    sample_24_4  = 5,
    sample_float = 6,
    sample_error
};

static char* format_str[] = {
    [0]            = "format         : not supported\n",
    [sample_8]     = "format         : SND_PCM_FORMAT_S8\n",
    [sample_16]    = "format         : SND_PCM_FORMAT_S16_LE\n",
    [sample_24_3]  = "format         : SND_PCM_FORMAT_S24_3LE\n",
    [sample_32]    = "format         : SND_PCM_FORMAT_S32_LE\n",
    [sample_24_4]  = "format         : SND_PCM_FORMAT_S24_LE\n",
    [sample_float] = "format         : SND_PCM_FORMAT_FLOAT_LE\n",
    [sample_error] = "format         : error!\n"
};

struct pcm_parameters {
	int rate;
        int channels;	
	snd_pcm_uframes_t period;
    snd_pcm_uframes_t buffer;
        snd_pcm_format_t pcm_format;
        unsigned buffer_time;
	unsigned period_time;
        unsigned sample_size;
        unsigned period_bytes;
};

int custom_print_boundary_info(snd_pcm_t *pcm_handle, snd_pcm_hw_params_t *hw_params) {
    printf("**********************************DEBUG \n");
    int pcm;
    unsigned int buffer_time_temp = 0;
    unsigned int period_time_temp = 0;
    unsigned int channels_temp = 0;
    unsigned int rate_temp = 0;
    snd_pcm_uframes_t period_temp = 0;
    snd_pcm_uframes_t buffer_temp = 0;
    snd_pcm_format_t format_temp;

    /*time period*/
    pcm = snd_pcm_hw_params_get_period_time_min(hw_params, &period_time_temp, 0);
    if (pcm < 0) {
        printf("ERROR: can't get period time: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("period time min    : %d\n", period_time_temp);
    pcm = snd_pcm_hw_params_get_period_time_max(hw_params, &period_time_temp, 0);
    if (pcm < 0) {
        printf("ERROR: can't get period time: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("period time max    : %d\n", period_time_temp);
    /*time buffer*/
    pcm = snd_pcm_hw_params_get_buffer_time_min(hw_params, &buffer_time_temp, 0);
    if (pcm < 0) {
        printf("ERROR: can't get buffer time: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("buffer time min    : %d\n", buffer_time_temp);
    pcm = snd_pcm_hw_params_get_buffer_time_max(hw_params, &buffer_time_temp, 0);
    if (pcm < 0) {
        printf("ERROR: can't get buffer time: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("buffer time max    : %d\n", buffer_time_temp);
    /*channels*/
    pcm = snd_pcm_hw_params_get_channels_min(hw_params, &channels_temp);
    if (pcm < 0) {
        printf("ERROR: can't get channels number: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("channels min       : %d\n", channels_temp);
    pcm = snd_pcm_hw_params_get_channels_max(hw_params, &channels_temp);
    if (pcm < 0) {
        printf("ERROR: can't get channels number: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("channels max       : %d\n", channels_temp);
    /*rate*/
    pcm = snd_pcm_hw_params_get_rate_min(hw_params, &rate_temp, 0);
    if (pcm < 0) {
        printf("ERROR: can't get rate: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("rate  min          : %d\n", rate_temp);
    pcm = snd_pcm_hw_params_get_rate_max(hw_params, &rate_temp, 0);
    if (pcm < 0) {
        printf("ERROR: can't get rate: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("rate  max          : %d\n", rate_temp);
    /*period*/
    pcm = snd_pcm_hw_params_get_period_size_min(hw_params, &period_temp, 0);
    if (pcm < 0) {
        printf("ERROR: can't get period size: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("period size min    : %d\n", (int)period_temp);
    pcm = snd_pcm_hw_params_get_period_size_max(hw_params, &period_temp, 0);
    if (pcm < 0) {
        printf("ERROR: can't get period size: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("period size max    : %d\n", (int)period_temp);
    /*buffer*/
    pcm = snd_pcm_hw_params_get_buffer_size_min(hw_params, &buffer_temp);
    if (pcm < 0) {
        printf("ERROR: can't get buffer size: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("buffer size min    : %d\n", (int)buffer_temp);
    pcm = snd_pcm_hw_params_get_buffer_size_max(hw_params, &buffer_temp);
    if (pcm < 0) {
        printf("ERROR: can't get buffer size: %s\n", snd_strerror(pcm));
        goto error_handling;
    }
    printf("buffer size max    : %d\n", (int)buffer_temp);


    printf("Available formats:\n");
    for (format_temp = 0; format_temp < SND_PCM_FORMAT_LAST; format_temp++) {
        if (snd_pcm_hw_params_test_format(pcm_handle, hw_params, format_temp) == 0)
            printf("- %s\n", snd_pcm_format_name(format_temp));
    }
    printf("**********************************DEBUG_END\n");
    
    return 0;
    error_handling:
        return pcm;
}

static int hw_params_set(snd_pcm_t *pcm_handle, snd_output_t *log, struct pcm_parameters* expected) {
        snd_pcm_hw_params_t *params;
        int pcm;
        int dir;

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&params);

 	pcm = snd_pcm_hw_params_any(pcm_handle, params);
        if (pcm < 0) {
                printf("Broken configuration for this PCM: no configurations available\n");
                goto error_handling;
        }

        printf("hw boundary params  ***********************\n");
        snd_pcm_hw_params_dump(params, log);
        printf("*******************************************\n");

	/* Set parameters */
	pcm = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
        if (pcm < 0) {
                printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));
                goto error_handling;
        }

        pcm = snd_pcm_hw_params_set_format(pcm_handle, params, expected->pcm_format);
        if (pcm < 0) {
	    printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));
            goto error_handling;
        }

	pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, expected->channels);
        if (pcm < 0) {
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));
                goto error_handling;
        }

	pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &expected->rate, &dir);
    if(pcm < 0) {
        printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));
        goto error_handling;
    }

    pcm = snd_pcm_hw_params_get_buffer_time_max(params, &expected->buffer_time, 0);
    if(pcm < 0) {
        printf("ERROR: Can't get buffer time max. %s\n", snd_strerror(pcm));
        goto error_handling;
    }

    if (expected->buffer_time > 500000) expected->buffer_time = 500000;
    expected->period_time = expected->buffer_time / 4;
    
    pcm = snd_pcm_hw_params_set_period_time_near(pcm_handle, params, &expected->period_time, 0);
    if(pcm < 0) {
        printf("ERROR: Can't set period_time. %s\n", snd_strerror(pcm));
        goto error_handling;
    }

    pcm  = snd_pcm_hw_params_set_buffer_time_near(pcm_handle, params, &expected->buffer_time, 0);
    if(pcm < 0) {
            printf("ERROR: Can't set buffer time. %s\n", snd_strerror(pcm));
            goto error_handling;
    }

    /* Write parameters */
    pcm = snd_pcm_hw_params(pcm_handle, params);
    if(pcm < 0) {
            printf("ERROR: Can't set hardware parameters. %s\n", snd_strerror(pcm));
            goto error_handling;
    }

    printf( "format       :%d\n", (int)expected->pcm_format);
	printf( "channels     :%d\n", (int)expected->channels);
	printf( "rate         :%d\n", (int)expected->rate);
	printf( "p time       :%d\n", (int)expected->period_time);
	printf( "b time       :%d\n", (int)expected->buffer_time);
	printf( "p size       :%d\n", (int)0);
	printf( "b size       :%d\n", (int)0);
	/* Resume information */
    
	pcm = snd_pcm_hw_params_get_channels(params, &expected->channels);
	if (pcm < 0) {
		printf("ERROR: can't get channels number: %s\n", snd_strerror(pcm));
		goto error_handling;
	}
	pcm = snd_pcm_hw_params_get_rate(params, &expected->rate, 0);
	if (pcm < 0) {
		printf("ERROR: can't get rate: %s\n", snd_strerror(pcm));
		goto error_handling;
	}
	pcm = snd_pcm_hw_params_get_period_size(params, &expected->period, 0);
	if (pcm < 0) {
		printf("ERROR: can't get period size: %s\n", snd_strerror(pcm));
		goto error_handling;
	}
    pcm = snd_pcm_hw_params_get_buffer_size(params, &expected->buffer);
	if (pcm < 0) {
		printf("ERROR: can't get buffer size: %s\n", snd_strerror(pcm));
		goto error_handling;
	}
	pcm = snd_pcm_hw_params_get_format(params, &expected->pcm_format);
	if (pcm < 0) {
		printf("ERROR: can't get format: %s\n", snd_strerror(pcm));
		goto error_handling;
	}
	pcm = snd_pcm_hw_params_get_period_time(params, &expected->period_time, 0);
	if (pcm < 0) {
		printf("ERROR: can't get period time: %s\n", snd_strerror(pcm));
		goto error_handling;
	}
	pcm = snd_pcm_hw_params_get_buffer_time(params, &expected->buffer_time, 0);
	if (pcm < 0) {
		printf("ERROR: can't get buffer time: %s\n", snd_strerror(pcm));
		goto error_handling;
	}

        expected->sample_size = snd_pcm_format_physical_width(expected->pcm_format) / 8;
	expected->period_bytes = expected->period * expected->channels * expected->sample_size;
        return 0;
        error_handling:
            return pcm;
}

static int sw_params_set(snd_pcm_t *pcm_handle, snd_output_t *log, struct pcm_parameters* expected) {
        snd_pcm_sw_params_t *params;
        int pcm;

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_sw_params_alloca(&params);

	snd_pcm_sw_params_current(pcm_handle, params);
	pcm = snd_pcm_sw_params_set_avail_min(pcm_handle, params, expected->period);
        if(pcm < 0) {
                printf("ERROR: Can't set sw avail min. %s\n", snd_strerror(pcm));
                goto error_handling;
        }

	pcm = snd_pcm_sw_params_set_start_threshold(pcm_handle, params, expected->buffer);
        if(pcm < 0) {
                printf("ERROR: Can't set sw start threshold. %s\n", snd_strerror(pcm));
                goto error_handling;
        }

	pcm = snd_pcm_sw_params_set_stop_threshold(pcm_handle, params, expected->buffer);
        if(pcm < 0) {
                printf("ERROR: Can't set sw stop threshold. %s\n", snd_strerror(pcm));
                goto error_handling;
        }

	pcm = snd_pcm_sw_params(pcm_handle, params);
        if(pcm < 0) {
		printf("unable to install sw params:\n");
		snd_pcm_sw_params_dump(params, log);
		goto error_handling;
	}
        return 0;
        error_handling:
            return pcm;
}

int main(int argc, char **argv) {
        snd_output_t *log = NULL;
        snd_pcm_t *pcm_handle = NULL;
        snd_pcm_uframes_t write_frames = 0;
        unsigned int shift = 0;
	char *buff = NULL;
        int pcm_format;
        int tmp;
        int pcm = 0;

        struct pcm_parameters actual = {0};
        struct pcm_parameters expected = {0};

	printf("Usage: %s <pcm device name> <sample_rate> <format> <channels>\n", argv[0]);

    printf("\n************************\n");
    printf("available <format> list:\n");
	printf("Sample_8     = 1\n");
	printf("Sample_16    = 2\n");
	printf("Sample_24_3  = 3\n");
	printf("Sample_32    = 4\n");
	printf("Sample_24_4  = 5\n");
	printf("sample_float = 6\n");
	printf("************************\n\n\n");

	if (argc < 5) {
                printf("Arguments number error\n");
        	goto error_handling;
	}

    memset(PCM_DEVICE, 0, sizeof(PCM_DEVICE));
    memcpy(PCM_DEVICE, argv[1], strlen(argv[1]));

	expected.rate = atoi(argv[2]);
    pcm_format = atoi(argv[3]);
        switch(pcm_format) {
           case sample_8:
               expected.pcm_format = SND_PCM_FORMAT_S8;
               break;
           case sample_16:
               expected.pcm_format = SND_PCM_FORMAT_S16_LE;
               break;
           case sample_24_3:
               expected.pcm_format = SND_PCM_FORMAT_S24_3LE;
               break;
           case sample_32:
               expected.pcm_format = SND_PCM_FORMAT_S32_LE;
               break;
           case sample_24_4:
               expected.pcm_format = SND_PCM_FORMAT_S24_LE;
               break;
           case sample_float:
               expected.pcm_format = SND_PCM_FORMAT_FLOAT_LE;
               break;
           case sample_error:
               expected.pcm_format = SND_PCM_FORMAT_LAST;
               break;
           default:
               goto error_handling;
        }
        expected.channels = atoi(argv[4]);
        
        if(SND_PCM_FORMAT_LAST == pcm_format) {
            goto error_handling;
        }

        memcpy(&actual, &expected, sizeof(struct pcm_parameters));
        pcm = snd_output_stdio_attach(&log, stdout, 0);
        if (pcm < 0) {
                printf("ERROR: Can't attach log %s\n", snd_strerror(pcm));
                goto error_handling;
        }
	/* Open the PCM device in playback mode */
	pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
					SND_PCM_STREAM_PLAYBACK, 0);
        if (pcm < 0) {
		printf("ERROR: Can't open \"%s\" PCM device. %s\n",
					PCM_DEVICE, snd_strerror(pcm));
                goto error_handling;
        }

        pcm = hw_params_set(pcm_handle, log, &actual);
        if (pcm < 0) {
                printf("HW parameters set fail\n");
                goto error_handling;
        }

        pcm = sw_params_set(pcm_handle, log, &actual);
        if (pcm < 0) {
                printf("SW parameters set fail\n");
                goto error_handling;
        }

        buff = (char *) malloc(actual.period_bytes);
        if(buff == 0) {
            printf("buffer is not allocated");
            return 1;
        }

        printf("print expected params *********************\n");
        printf("rate           : %d\n", expected.rate);
        printf("channels       : %d\n", expected.channels);
        printf("%s", format_str[pcm_format]);
        printf("buffer time    : %d\n", expected.buffer_time);
        printf("period time    : %d\n", expected.period_time);
        printf("print actual params ***********************\n");
        printf("PCM name       : '%s'\n", snd_pcm_name(pcm_handle));
        printf("PCM state      : %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));
        printf("sample sz      : %d\n", actual.sample_size);
        printf("period bytes   : %d\n", actual.period_bytes);
        snd_pcm_dump(pcm_handle, log);
        printf("*******************************************\n");

        int fd = 0;
        fd = open("test.txt",  O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

        tmp = read(STDIN_FILENO, buff, 2);
        do {
            tmp = read(STDIN_FILENO, buff, actual.period_bytes);
            write(fd,buff,actual.period_bytes);
            
            if(tmp != actual.period_bytes) {
                 printf("read bytes %d expected %d\n", tmp, actual.period_bytes);
            }
            if (tmp) {
                do {
                    pcm = snd_pcm_writei(pcm_handle, buff + shift, actual.period - write_frames);

                    if(pcm > 0) {
                        write_frames += pcm;
                        shift = write_frames * actual.sample_size;
                    }
                } while(pcm == -EAGAIN || (pcm >= 0 && write_frames < actual.period));
                shift = 0;
                write_frames = 0;

                if(pcm < 0) {
                    if (pcm == -ESTRPIPE) {
                        while ((pcm = snd_pcm_resume(pcm_handle)) == -EAGAIN) {
                            sleep(1);
                        }
                    }
                    //try to recovery
                    pcm = snd_pcm_prepare(pcm_handle);
                }
                if(pcm < 0) {
                    printf("playback. Can't recover device to SND_PCM_STATE_PREPARED. %s\n", snd_strerror(pcm));
                    goto error_handling;
                }
            }
	} while (tmp > 0);

error_handling:
        if(pcm_handle) {
	    snd_pcm_drain(pcm_handle);
	    snd_pcm_close(pcm_handle); 
        }
        if(log) snd_output_close(log);
	if(buff) free(buff);
        if(fd) close(fd);

	return pcm;
}
