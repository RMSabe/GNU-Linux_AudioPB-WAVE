#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE 4096
#define BUFFER_SIZE_BYTES (4*BUFFER_SIZE)
#define OUTPUT_BUFFER_SIZE (2*BUFFER_SIZE)
#define OUTPUT_BUFFER_SIZE_BYTES (4*OUTPUT_BUFFER_SIZE)
#define BYTE_BUFFER_SIZE (3*BUFFER_SIZE_BYTES/4)

char *audio_file_dir = NULL;
char *audio_dev_desc = NULL;
unsigned int audio_data_begin = 0;
unsigned int audio_data_end = 0;
unsigned int sample_rate = 0;

std::thread loadthread;
std::thread playthread;

std::fstream audio_file;
unsigned int audio_file_pos = 0;

snd_pcm_t *audio_dev = NULL;
snd_pcm_uframes_t n_frames;
unsigned int audio_buffer_size = 0;
unsigned int buffer_n_div = 1;
int **pp_startpoints = NULL;

unsigned char *byte_buffer = NULL;
int *buffer_output_0 = NULL;
int *buffer_output_1 = NULL;

bool curr_buf_cycle = false;

int *load_out = NULL;
int *play_out = NULL;

int n_sample = 0;
int n_byte = 0;

bool stop = false;

void update_buf_cycle(void)
{
	curr_buf_cycle = !curr_buf_cycle;
	return;
}

void buffer_remap(void)
{
	if(curr_buf_cycle)
	{
		load_out = buffer_output_1;
		play_out = buffer_output_0;
	}
	else
	{
		load_out = buffer_output_0;
		play_out = buffer_output_1;
	}

	return;
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
	unsigned int n_div = 0;
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
	n_sample = 0;
	n_byte = 0;
	while(n_sample < BUFFER_SIZE)
	{
		load_out[2*n_sample] = ((byte_buffer[n_byte + 2] << 16) | (byte_buffer[n_byte + 1] << 8) | (byte_buffer[n_byte]));
		load_out[2*n_sample + 1] = load_out[2*n_sample];

		n_byte += 3;
		n_sample++;
	}

	return;
}

void load_startpoints(void)
{
	pp_startpoints[0] = play_out;
	unsigned int n_div = 1;
	while(n_div < buffer_n_div)
	{
		pp_startpoints[n_div] = &play_out[n_div*audio_buffer_size];
		n_div++;
	}

	return;
}

void loadthread_proc(void)
{
	buffer_load();
	buffer_transfer();
	update_buf_cycle();
	return;
}

void playthread_proc(void)
{
	load_startpoints();
	buffer_play();
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

void playback(void)
{
	buffer_preload();
	while(!stop)
	{
		playthread = std::thread(playthread_proc);
		loadthread = std::thread(loadthread_proc);
		loadthread.join();
		playthread.join();
		buffer_remap();
	}

	return;
}

void buffer_malloc(void)
{
	audio_buffer_size = 2*n_frames;
	if(audio_buffer_size < OUTPUT_BUFFER_SIZE) buffer_n_div = OUTPUT_BUFFER_SIZE/audio_buffer_size;
	else buffer_n_div = 1;

	pp_startpoints = (int**) malloc(buffer_n_div*sizeof(int*));

	byte_buffer = (unsigned char*) malloc(BYTE_BUFFER_SIZE);
	buffer_output_0 = (int*) malloc(OUTPUT_BUFFER_SIZE_BYTES);
	buffer_output_1 = (int*) malloc(OUTPUT_BUFFER_SIZE_BYTES);

	memset(byte_buffer, 0, BYTE_BUFFER_SIZE);
	memset(buffer_output_0, 0, OUTPUT_BUFFER_SIZE_BYTES);
	memset(buffer_output_1, 0, OUTPUT_BUFFER_SIZE_BYTES);

	return;
}

void buffer_free(void)
{
	free(pp_startpoints);

	free(byte_buffer);
	free(buffer_output_0);
	free(buffer_output_1);
	return;
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
