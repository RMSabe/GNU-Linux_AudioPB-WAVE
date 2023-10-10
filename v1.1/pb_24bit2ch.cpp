/*
 * WAVE file audio playback
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include <fstream>
#include <iostream>
#include <string>
#include <pthread.h>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE_SAMPLES 8192U
#define BUFFER_SIZE_BYTES (4U*BUFFER_SIZE_SAMPLES)
#define BUFFER_SIZE_FRAMES (BUFFER_SIZE_SAMPLES/2U)
#define BYTE_BUFFER_SIZE (3U*BUFFER_SIZE_BYTES/4U)

pthread_t loadthread;
pthread_t playthread;

char *audio_file_dir = NULL;
std::fstream audio_file;
unsigned int audio_data_begin = 0u;
unsigned int audio_data_end = 0u;
unsigned int audio_file_pos = 0u;
unsigned int sample_rate = 0u;

char *audio_dev_desc = NULL;
snd_pcm_t *audio_dev = NULL;
snd_pcm_uframes_t n_frames;
unsigned int audio_buffer_size_samples = 0u;
unsigned int buffer_n_div = 1u;
int **pp_startpoints = NULL;

unsigned char *byte_buffer = NULL;
int *buffer_0 = NULL;
int *buffer_1 = NULL;

bool curr_buf_cycle = false;

int *load_out = NULL;
int *play_out = NULL;

unsigned int n_sample = 0u;

bool stop = false;

bool audio_hw_init(void);
void buffer_malloc(void);
void buffer_free(void);
void playback(void);
void buffer_preload(void);
void *p_loadthread_proc(void *args);
void *p_playthread_proc(void *args);
void buffer_load(void);
void buffer_play(void);
void buffer_transfer(void);
void load_startpoints(void);
void update_buf_cycle(void);
void buffer_remap(void);

int main(int argc, char **argv)
{
	if(argc < 6)
	{
		std::cout << "Error: invalid runtime parameters\n";
		return 0;
	}

	audio_file_dir = argv[1];
	audio_dev_desc = argv[2];
	audio_data_begin = std::stoi(argv[3]);
	audio_data_end = std::stoi(argv[4]);
	sample_rate = std::stoi(argv[5]);

	audio_file.open(audio_file_dir, std::ios_base::in);
	if(!audio_file.is_open())
	{
		std::cout << "Error opening audio file\nError code: " << errno << std::endl;
		return 0;
	}

	if(!audio_hw_init())
	{
		std::cout << "Error code: " << errno << std::endl;
		return 0;
	}

	audio_file_pos = audio_data_begin;
	buffer_malloc();

	std::cout << "Playback started\n";
	playback();
	std::cout << "Playback finished\n";

	audio_file.close();
	snd_pcm_drain(audio_dev);
	snd_pcm_close(audio_dev);
	buffer_free();

	return 0;
}

bool audio_hw_init(void)
{
	int n_return = 0;
	snd_pcm_hw_params_t *hw_params = NULL;

	n_return = snd_pcm_open(&audio_dev, audio_dev_desc, SND_PCM_STREAM_PLAYBACK, 0);
	if(n_return < 0)
	{
		std::cout << "Audio Hardware: error opening audio device\n";
		return false;
	}

	snd_pcm_hw_params_malloc(&hw_params);
	snd_pcm_hw_params_any(audio_dev, hw_params);

	n_return = snd_pcm_hw_params_set_access(audio_dev, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(n_return < 0)
	{
		std::cout << "Audio Hardware: error setting access\n";
		return false;
	}

	n_return = snd_pcm_hw_params_set_format(audio_dev, hw_params, SND_PCM_FORMAT_S24_LE);
	if(n_return < 0)
	{
		std::cout << "Audio Hardware: error setting format\n";
		return false;
	}

	n_return = snd_pcm_hw_params_set_channels(audio_dev, hw_params, 2);
	if(n_return < 0)
	{
		std::cout << "Audio Hardware: error setting channels\n";
		return false;
	}

	unsigned int rate = sample_rate;
	n_return = snd_pcm_hw_params_set_rate_near(audio_dev, hw_params, &rate, 0);
	if(n_return < 0 || rate < sample_rate)
	{
		std::cout << "Audio Hardware: error setting sample rate\n";
		return false;
	}

	n_return = snd_pcm_hw_params(audio_dev, hw_params);
	if(n_return < 0)
	{
		std::cout << "Audio Hardware: error setting hardware parameters\n";
		return false;
	}

	snd_pcm_hw_params_get_period_size(hw_params, &n_frames, 0);
	snd_pcm_hw_params_free(hw_params);
	return true;
}

void buffer_malloc(void)
{
	audio_buffer_size_samples = 2u*n_frames;
	if(audio_buffer_size_samples < BUFFER_SIZE_SAMPLES) buffer_n_div = BUFFER_SIZE_SAMPLES/audio_buffer_size_samples;
	else buffer_n_div = 1u;

	pp_startpoints = (int**) malloc(buffer_n_div*sizeof(int*));

	byte_buffer = (unsigned char*) malloc(BYTE_BUFFER_SIZE);
	buffer_0 = (int*) malloc(BUFFER_SIZE_BYTES);
	buffer_1 = (int*) malloc(BUFFER_SIZE_BYTES);

	memset(byte_buffer, 0, BYTE_BUFFER_SIZE);
	memset(buffer_0, 0, BUFFER_SIZE_BYTES);
	memset(buffer_1, 0, BUFFER_SIZE_BYTES);
	return;
}

void buffer_free(void)
{
	free(pp_startpoints);
	free(byte_buffer);
	free(buffer_0);
	free(buffer_1);

	return;
}

void playback(void)
{
	buffer_preload();
	while(!stop)
	{
		pthread_create(&playthread, NULL, p_playthread_proc, NULL);
		pthread_create(&loadthread, NULL, p_loadthread_proc, NULL);
		pthread_join(loadthread, NULL);
		pthread_join(playthread, NULL);
		buffer_remap();
	}

	return;
}

void buffer_preload(void)
{
	curr_buf_cycle = false;
	buffer_remap();

	buffer_load();
	buffer_transfer();
	update_buf_cycle();
	buffer_remap();

	return;
}

void *p_loadthread_proc(void *args)
{
	buffer_load();
	buffer_transfer();
	update_buf_cycle();
	return NULL;
}

void *p_playthread_proc(void *args)
{
	load_startpoints();
	buffer_play();
	return NULL;
}

void buffer_load(void)
{
	if(audio_file_pos >= audio_data_end)
	{
		stop = true;
		return;
	}

	audio_file.seekg(audio_file_pos);
	audio_file.read((char*) byte_buffer, BYTE_BUFFER_SIZE);
	audio_file_pos += BYTE_BUFFER_SIZE;

	return;
}

void buffer_play(void)
{
	unsigned int n_div = 0u;
	int n_return = 0;

	while(n_div < buffer_n_div)
	{
		n_return = snd_pcm_writei(audio_dev, pp_startpoints[n_div], n_frames);
		if(n_return == -EPIPE) snd_pcm_prepare(audio_dev);

		n_div++;
	}

	return;
}

void buffer_transfer(void)
{
	static unsigned int n_byte;

	n_byte = 0u;
	n_sample = 0u;

	while(n_sample < BUFFER_SIZE_FRAMES)
	{
		load_out[2u*n_sample] = ((byte_buffer[n_byte + 2u] << 16) | (byte_buffer[n_byte + 1u] << 8) | (byte_buffer[n_byte]));
		load_out[2u*n_sample + 1u] = ((byte_buffer[n_byte + 5u] << 16) | (byte_buffer[n_byte + 4u] << 8) | (byte_buffer[n_byte + 3u]));

		n_byte += 6u;
		n_sample++;
	}

	return;
}

void load_startpoints(void)
{
	pp_startpoints[0] = play_out;
	unsigned int n_div = 1u;
	while(n_div < buffer_n_div)
	{
		pp_startpoints[n_div] = &play_out[n_div*audio_buffer_size_samples];
		n_div++;
	}

	return;
}

void update_buf_cycle(void)
{
	curr_buf_cycle = !curr_buf_cycle;
	return;
}

void buffer_remap(void)
{
	if(curr_buf_cycle)
	{
		load_out = buffer_1;
		play_out = buffer_0;
	}
	else
	{
		load_out = buffer_0;
		play_out = buffer_1;
	}

	return;
}

