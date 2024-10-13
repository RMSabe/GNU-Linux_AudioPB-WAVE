/*
 * WAVE audio file playback app v2.0 for GNU-Linux
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

bool AudioPlayback_16bit2ch::runPlayback(void)
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
	if(this->buffer_0 == nullptr) this->buffer_0 = (std::int16_t*) std::malloc(this->BUFFER_SIZE_BYTES);
	if(this->buffer_1 == nullptr) this->buffer_1 = (std::int16_t*) std::malloc(this->BUFFER_SIZE_BYTES);

	memset(this->buffer_0, 0, this->BUFFER_SIZE_BYTES);
	memset(this->buffer_1, 0, this->BUFFER_SIZE_BYTES);

	return;
}

void AudioPlayback_16bit2ch::buffer_free(void)
{
	if(this->buffer_0 != nullptr)
	{
		std::free(this->buffer_0);
		this->buffer_0 = nullptr;
	}

	if(this->buffer_1 != nullptr)
	{
		std::free(this->buffer_1);
		this->buffer_1 = nullptr;
	}

	this->load_buf = nullptr;
	this->play_buf = nullptr;
	return;
}

void AudioPlayback_16bit2ch::playback_proc(void)
{
	this->stop = false;
	this->filein_pos = this->audio_data_begin;

	this->playback_init();
	this->playback_loop();
	return;
}

void AudioPlayback_16bit2ch::playback_init(void)
{
	this->buffer_remap();

	this->buffer_load();

	this->curr_buf_cycle = !this->curr_buf_cycle;
	this->buffer_remap();

	return;
}

void AudioPlayback_16bit2ch::playback_loop(void)
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

void AudioPlayback_16bit2ch::buffer_remap(void)
{
	if(this->curr_buf_cycle)
	{
		this->load_buf = this->buffer_1;
		this->play_buf = this->buffer_0;
	}
	else
	{
		this->load_buf = this->buffer_0;
		this->play_buf = this->buffer_1;
	}

	return;
}

void AudioPlayback_16bit2ch::buffer_load(void)
{
	if(this->filein_pos >= this->audio_data_end)
	{
		this->stop = true;
		return;
	}

	memset(this->load_buf, 0, this->BUFFER_SIZE_BYTES);

	__LSEEK(this->filein, this->filein_pos, SEEK_SET);
	read(this->filein, this->load_buf, this->BUFFER_SIZE_BYTES);
	this->filein_pos += (__offset) this->BUFFER_SIZE_BYTES;

	return;
}

void AudioPlayback_16bit2ch::buffer_play(void)
{
	int n_ret = snd_pcm_writei(this->audio_dev, this->play_buf, (snd_pcm_uframes_t) this->BUFFER_SIZE_FRAMES);
	if(n_ret == -EPIPE) snd_pcm_prepare(this->audio_dev);
	return;
}

