/*
 * WAVE audio file playback app v2.0.1 for GNU-Linux
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPlayback_16bit2ch.hpp"

AudioPlayback_16bit2ch::AudioPlayback_16bit2ch(audio_playback_params_t *params) : AudioPlayback(params)
{
}

AudioPlayback_16bit2ch::~AudioPlayback_16bit2ch(void)
{
	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();
}

bool AudioPlayback_16bit2ch::audio_hw_init(void)
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

	n_ret = snd_pcm_hw_params_set_format(this->audio_dev, hw_params, SND_PCM_FORMAT_S16_LE);
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
	this->BUFFER_SIZE_SAMPLES = 2u*this->BUFFER_SIZE_FRAMES;
	this->BUFFER_SIZE_BYTES = 2u*this->BUFFER_SIZE_SAMPLES;

	return true;
}

void AudioPlayback_16bit2ch::buffer_malloc(void)
{
	if(this->bufferout_0 == nullptr) this->bufferout_0 = std::malloc(this->BUFFER_SIZE_BYTES);
	if(this->bufferout_1 == nullptr) this->bufferout_1 = std::malloc(this->BUFFER_SIZE_BYTES);

	memset(this->bufferout_0, 0, this->BUFFER_SIZE_BYTES);
	memset(this->bufferout_1, 0, this->BUFFER_SIZE_BYTES);

	return;
}

void AudioPlayback_16bit2ch::buffer_free(void)
{
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

void AudioPlayback_16bit2ch::buffer_load(void)
{
	if(this->filein_pos >= this->audio_data_end)
	{
		this->stop = true;
		return;
	}

	memset(this->loadout_buf, 0, this->BUFFER_SIZE_BYTES);

	__LSEEK(this->filein, this->filein_pos, SEEK_SET);
	read(this->filein, this->loadout_buf, this->BUFFER_SIZE_BYTES);
	this->filein_pos += (__offset) this->BUFFER_SIZE_BYTES;

	return;
}

