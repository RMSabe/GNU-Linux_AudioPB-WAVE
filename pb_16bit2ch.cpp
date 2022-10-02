#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <alsa/asoundlib.h>

std::fstream audio_file;
char *audio_file_dir = NULL;
unsigned int audio_data_begin = 0;
unsigned int audio_data_end = 0;
unsigned int audio_file_pos = 0;
unsigned int sample_rate = 0;

snd_pcm_t *audio_dev = NULL;
char *audio_dev_desc = NULL;
snd_pcm_uframes_t n_frames;
unsigned int buffer_size_bytes = 0;

short *buffer_0 = NULL;
short *buffer_1 = NULL;

bool next_buf = false;
bool stop = false;

void buffer_load(void)
{
	if(audio_file_pos >= audio_data_end)
	{
		stop = true;
		return;
	}

	short *buf_in = NULL;
	if(next_buf) buf_in = buffer_1;
	else buf_in = buffer_0;

	audio_file.seekg(audio_file_pos);
	audio_file.read((char*) buf_in, buffer_size_bytes);
	audio_file_pos += buffer_size_bytes;

	next_buf = !next_buf;
	return;
}

void buffer_play(void)
{
	short *buf_out = NULL;
	if(next_buf) buf_out = buffer_0;
	else buf_out = buffer_1;

	int n_return = snd_pcm_writei(audio_dev, buf_out, n_frames);
	if(n_return == -EPIPE) snd_pcm_prepare(audio_dev);

	return;
}

void playback(void)
{
	buffer_load();
	while(!stop)
	{
		buffer_play();
		buffer_load();
	}

	return;
}

void buffer_malloc(void)
{
	buffer_size_bytes = 4*n_frames;
	buffer_0 = (short*) malloc(buffer_size_bytes);
	buffer_1 = (short*) malloc(buffer_size_bytes);

	memset(buffer_0, 0, buffer_size_bytes);
	memset(buffer_1, 0, buffer_size_bytes);
	return;
}

void buffer_free(void)
{
	free(buffer_0);
	free(buffer_1);
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

	n_return = snd_pcm_hw_params_set_format(audio_dev, hw_params, SND_PCM_FORMAT_S16_LE);
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
