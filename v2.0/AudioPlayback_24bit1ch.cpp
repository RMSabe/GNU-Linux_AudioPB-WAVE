/*
 * WAVE audio file playback app v2.0 for GNU-Linux
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPlayback_24bit1ch.hpp"

AudioPlayback_24bit1ch::AudioPlayback_24bit1ch(audio_playback_params_t *params) : AudioPlayback(params)
{
}

AudioPlayback_24bit1ch::~AudioPlayback_24bit1ch(void)
{
	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();
}

bool AudioPlayback_24bit1ch::runPlayback(void)
{
	if(this->status < 1)
	{
		this->error_msg = "Audio Playback object has not been initialized.";
		return false;
	}

	if(!this->filein_open())
	{
		this->error_msg = "Could not open input file.";
		this->status = this->STATUS_ERROR_NOFILE;
		return false;
	}

	if(!this->audio_hw_init())
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->filein_close();
		return false;
	}

	this->buffer_malloc();

	std::cout << "Playback started\n";
	this->playback_proc();
	std::cout << "Playback finished\n";

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	return true;
}

bool AudioPlayback_24bit1ch::audio_hw_init(void)
{
	snd_pcm_hw_params_t *hw_params = nullptr;
	snd_pcm_uframes_t nframes = 0u;
	int n_ret = 0;
	std::uint32_t rate = this->sample_rate;

	n_ret = snd_pcm_open(&this->audio_dev, this->audio_dev_desc.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
	if(n_ret < 0)
	{
		this->error_msg = "Audio HW Init: could not open audio device.";
		return false;
	}

	snd_pcm_hw_params_malloc(&hw_params);
	snd_pcm_hw_params_any(this->audio_dev, hw_params);

	n_ret = snd_pcm_hw_params_set_access(this->audio_dev, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(n_ret < 0)
	{
		this->error_msg = "Audio HW Init: could not set device access.";
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_close(this->audio_dev);
		this->audio_dev = nullptr;
		return false;
	}

	n_ret = snd_pcm_hw_params_set_format(this->audio_dev, hw_params, SND_PCM_FORMAT_S24_LE);
	if(n_ret < 0)
	{
		this->error_msg = "Audio HW Init: could not set device format.";
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_close(this->audio_dev);
		this->audio_dev = nullptr;
		return false;
	}

	n_ret = snd_pcm_hw_params_set_channels(this->audio_dev, hw_params, 2);
	if(n_ret < 0)
	{
		this->error_msg = "Audio HW Init: could not set device channels.";
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_close(this->audio_dev);
		this->audio_dev = nullptr;
		return false;
	}

	n_ret = snd_pcm_hw_params_set_rate_near(this->audio_dev, hw_params, &rate, 0);
	if((n_ret < 0) || (rate < this->sample_rate))
	{
		this->error_msg = "Audio HW Init: could not set device sampling rate.";
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_close(this->audio_dev);
		this->audio_dev = nullptr;
		return false;
	}

	n_ret = snd_pcm_hw_params(this->audio_dev, hw_params);
	if(n_ret < 0)
	{
		this->error_msg = "Audio HW Init: could not apply device params.";
		snd_pcm_hw_params_free(hw_params);
		snd_pcm_close(this->audio_dev);
		this->audio_dev = nullptr;
		return false;
	}

	snd_pcm_hw_params_get_period_size(hw_params, &nframes, 0);
	snd_pcm_hw_params_free(hw_params);

	this->BUFFER_SIZE_FRAMES = (size_t) nframes;

	this->BUFFER_SIZE_SAMPLES = this->BUFFER_SIZE_FRAMES;
	this->BUFFER_SIZE_BYTES = 3u*this->BUFFER_SIZE_SAMPLES;

	this->AUDIOBUFFER_SIZE_SAMPLES = 2u*this->BUFFER_SIZE_FRAMES;
	this->AUDIOBUFFER_SIZE_BYTES = 4u*this->AUDIOBUFFER_SIZE_SAMPLES;

	return true;
}

void AudioPlayback_24bit1ch::buffer_malloc(void)
{
	if(this->bytebuf == nullptr) this->bytebuf = (std::uint8_t*) std::malloc(this->BUFFER_SIZE_BYTES);
	if(this->bufferout_0 == nullptr) this->bufferout_0 = (std::int32_t*) std::malloc(this->AUDIOBUFFER_SIZE_BYTES);
	if(this->bufferout_1 == nullptr) this->bufferout_1 = (std::int32_t*) std::malloc(this->AUDIOBUFFER_SIZE_BYTES);

	memset(this->bytebuf, 0, this->BUFFER_SIZE_BYTES);
	memset(this->bufferout_0, 0, this->AUDIOBUFFER_SIZE_BYTES);
	memset(this->bufferout_1, 0, this->AUDIOBUFFER_SIZE_BYTES);

	return;
}

void AudioPlayback_24bit1ch::buffer_free(void)
{
	if(this->bytebuf != nullptr)
	{
		std::free(this->bytebuf);
		this->bytebuf = nullptr;
	}

	if(this->bufferout_0 != nullptr)
	{
		std::free(this->bufferout_0);
		this->bufferout_0 = nullptr;
	}

	if(this->bufferout_1 != nullptr)
	{
		std::free(this->bufferout_1);
		this->bufferout_1 = nullptr;
	}

	this->loadout_buf = nullptr;
	this->playout_buf = nullptr;
	return;
}

void AudioPlayback_24bit1ch::playback_proc(void)
{
	this->stop = false;
	this->filein_pos = this->audio_data_begin;

	this->playback_init();
	this->playback_loop();
	return;
}

void AudioPlayback_24bit1ch::playback_init(void)
{
	this->buffer_remap();

	this->buffer_load();

	this->curr_buf_cycle = !this->curr_buf_cycle;
	this->buffer_remap();

	return;
}

void AudioPlayback_24bit1ch::playback_loop(void)
{
	while(!this->stop)
	{
		this->buffer_play();
		this->buffer_load();
		this->curr_buf_cycle = !this->curr_buf_cycle;
		this->buffer_remap();
	}

	return;
}

void AudioPlayback_24bit1ch::buffer_remap(void)
{
	if(this->curr_buf_cycle)
	{
		this->loadout_buf = this->bufferout_1;
		this->playout_buf = this->bufferout_0;
	}
	else
	{
		this->loadout_buf = this->bufferout_0;
		this->playout_buf = this->bufferout_1;
	}

	return;
}

void AudioPlayback_24bit1ch::buffer_load(void)
{
	size_t n_frame = 0u;
	size_t n_sample = 0u;
	size_t n_byte = 0u;

	if(this->filein_pos >= this->audio_data_end)
	{
		this->stop = true;
		return;
	}

	memset(this->bytebuf, 0, this->BUFFER_SIZE_BYTES);

	__LSEEK(this->filein, this->filein_pos, SEEK_SET);
	read(this->filein, this->bytebuf, this->BUFFER_SIZE_BYTES);
	this->filein_pos += (__offset) this->BUFFER_SIZE_BYTES;

	n_byte = 0u;
	for(n_frame = 0u; n_frame < this->BUFFER_SIZE_FRAMES; n_frame++)
	{
		n_sample = 2u*n_frame;

		this->loadout_buf[n_sample] = ((this->bytebuf[n_byte + 2u] << 16) | (this->bytebuf[n_byte + 1u] << 8) | (this->bytebuf[n_byte]));

		if(this->loadout_buf[n_sample] & 0x00800000) this->loadout_buf[n_sample] |= 0xff800000;
		else this->loadout_buf[n_sample] &= 0x007fffff; //Not really necessary, but just to be safe.

		this->loadout_buf[n_sample + 1u] = this->loadout_buf[n_sample];

		n_byte += 3u;
	}

	return;
}

void AudioPlayback_24bit1ch::buffer_play(void)
{
	int n_ret = snd_pcm_writei(this->audio_dev, this->playout_buf, (snd_pcm_uframes_t) this->BUFFER_SIZE_FRAMES);
	if(n_ret == -EPIPE) snd_pcm_prepare(this->audio_dev);
	return;
}

