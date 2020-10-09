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
#define PCM_DEVICE "default"

enum format {
    sample_8     = 1,
    sample_16    = 2,
    sample_24_3  = 3,
    sample_32    = 4,
    sample_24_4  = 5,
    sample_float = 6,
    sample_error
};

int main(int argc, char **argv) {
	unsigned int pcm, tmp, dir;
	int rate, channels;
	snd_pcm_t *pcm_handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames;
	snd_pcm_uframes_t write = 0;
        unsigned int shift = 0;
	char *buff;
	int buff_size, loops;
        snd_pcm_format_t pcm_format;
        unsigned buffer_time = 500000;
	unsigned period_time = 0;

        enum format sample_format;
        int sample_size = 0;
        char* format_str[] = {
            [0]            = "format          : not supported\n",
            [sample_8]     = "format          : SND_PCM_FORMAT_S8\n",
            [sample_16]    = "format          : SND_PCM_FORMAT_S16_LE\n",
            [sample_24_3]  = "format          : SND_PCM_FORMAT_S24_3LE\n",
            [sample_32]    = "format          : SND_PCM_FORMAT_S32_LE\n",
            [sample_24_4]  = "format          : SND_PCM_FORMAT_S24_LE\n",
            [sample_float] = "format          : SND_PCM_FORMAT_FLOAT_LE\n",
            [sample_error] = "format          : error!\n"
        };

	printf("Usage: %s <sample_rate> <format> <channels>\n", argv[0]);

        printf("\n************************\n");
        printf("available <formats> list:\n");
	printf("Sample_8     = 1\n");
	printf("Sample_16    = 2\n");
	printf("Sample_24_3  = 3\n");
	printf("Sample_32    = 4\n");
	printf("Sample_24_4  = 5\n");
	printf("sample_float = 6\n");
	printf("show all PCM formats = 7\n");
	printf("************************\n\n\n");

	if (argc < 4) {
        	return -1;
	}

	rate = atoi(argv[1]);
        sample_format = atoi(argv[2]);
        switch(sample_format) {
           case sample_8:
               pcm_format = SND_PCM_FORMAT_S8;
               break;
           case sample_16:
               pcm_format = SND_PCM_FORMAT_S16_LE;
               break;
           case sample_24_3:
               pcm_format = SND_PCM_FORMAT_S24_3LE;
               break;
           case sample_32:
               pcm_format = SND_PCM_FORMAT_S32_LE;
               break;
           case sample_24_4:
               pcm_format = SND_PCM_FORMAT_S24_LE;
               break;
           case sample_float:
               pcm_format = SND_PCM_FORMAT_FLOAT_LE;
               break;
          case sample_error:
               sample_size = 0;
               pcm_format = SND_PCM_FORMAT_LAST;
               break;
           default:
               exit(1);
        }
        channels = atoi(argv[3]);
        
	/* Open the PCM device in playback mode */
	if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
					SND_PCM_STREAM_PLAYBACK, 0) < 0) 
		printf("ERROR: Can't open \"%s\" PCM device. %s\n",
					PCM_DEVICE, snd_strerror(pcm));

	/* Allocate parameters object and fill it with default values*/
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle, params);

	/* Set parameters */
	if (pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
					SND_PCM_ACCESS_RW_INTERLEAVED) < 0) 
		printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

        if(SND_PCM_FORMAT_LAST == pcm_format) {
            for (snd_pcm_format_t i = 0; i <= SND_PCM_FORMAT_LAST; i++) {
                if (snd_pcm_hw_params_test_format(pcm_handle, params, i) == 0)
                    fprintf(stderr, "- %s\n", snd_pcm_format_name(i));
            }
            exit(1);
        }
	else if (pcm = snd_pcm_hw_params_set_format(pcm_handle, params, pcm_format) < 0) {
            for (snd_pcm_format_t i = 0; i <= SND_PCM_FORMAT_LAST; i++) {
                if (snd_pcm_hw_params_test_format(pcm_handle, params, i) == 0)
                    fprintf(stderr, "- %s\n", snd_pcm_format_name(i));
            } 
	    printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));
        }

	if (pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, channels) < 0) 
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

	if (pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, &dir) < 0) { 
		printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));
        } else {
                printf("rate            : %d bps\n", pcm);
        }

        if (buffer_time > 500000) {
            buffer_time = 500000;
        }
        period_time = buffer_time / 4;

        pcm  = snd_pcm_hw_params_set_buffer_time_near(pcm_handle, params, &buffer_time, 0);
        pcm |= snd_pcm_hw_params_set_period_time_near(pcm_handle, params, &period_time, 0);
        assert(pcm >= 0);

	sample_size = snd_pcm_format_physical_width(pcm_format) / 8;
	/* Write parameters */
	if (pcm = snd_pcm_hw_params(pcm_handle, params) < 0)
		printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

	/* Resume information */
        
	snd_pcm_hw_params_get_channels(params, &channels);
	snd_pcm_hw_params_get_rate(params, &rate, 0);
	

	/* Allocate buffer to hold single period */
	snd_pcm_hw_params_get_period_size(params, &frames, &dir);

	buff_size = frames * channels * sample_size;
	buff = (char *) malloc(buff_size);
        if(buff == 0) {
            printf("buffer is not allocated");
            return 1;
        }


	printf("PCM name        : '%s'\n", snd_pcm_name(pcm_handle));
	printf("PCM state       : %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));
        printf("%s", format_str[sample_format]);
	printf("channels        : %i ", channels);

	if (channels == 1)
		printf("(mono)\n");
	else if (channels == 2)
		printf("(stereo)\n");
        printf("rate            : %d bps\n", rate);
        printf("size of sample  : %d bytes %d msbits\n",sample_size, sample_size * 8);
        printf("period size     : %d \n", buff_size);
        printf("frames          : %ld\n", frames);
        //printf("buffer time     : %d \n", buffer_time);
        printf("period time     : %d \n", period_time);

        do {
            tmp = read(STDIN_FILENO, buff, buff_size);
            if (tmp) {
                do {
                    pcm = snd_pcm_writei(pcm_handle, buff + shift, frames - write);
                    if(pcm > 0) {
                        write += pcm;
                        shift = write * channels * sample_size;
                    }
                } while(pcm == -EAGAIN || (pcm >= 0 && write < frames));
                shift = 0;
                write = 0;

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
                    break;
                }
                //write(fd1, buffer_out, nbytes);
            }
	} while (tmp > 0);

	snd_pcm_drain(pcm_handle);
	snd_pcm_close(pcm_handle);
	free(buff);

	return 0;
}
