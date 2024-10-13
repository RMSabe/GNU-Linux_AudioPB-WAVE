/*
 * WAVE audio file playback app v2.0.1 for GNU-Linux
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPlayback.hpp"

AudioPlayback::AudioPlayback(audio_playback_params_t *params)
{
	this->setParameters(params);
}

bool AudioPlayback::setParameters(audio_playback_params_t *params)
{
	if(params == nullptr) return false;
	if(params->audio_dev_desc == nullptr) return false;
	if(params->filein_dir == nullptr) return false;

	this->audio_dev_desc = params->audio_dev_desc;
	this->filein_dir = params->filein_dir;
	this->audio_data_begin = params->audio_data_begin;
	this->audio_data_end = params->audio_data_end;
	this->sample_rate = params->sample_rate;

	this->status = STATUS_INITIALIZED;
	return true;
}

bool AudioPlayback::runPlayback(void)
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

std::string AudioPlayback::getLastErrorMessage(void)
{
	return this->error_msg;
}

bool AudioPlayback::filein_open(void)
{
	this->filein = open(this->filein_dir.c_str(), O_RDONLY);
	if(this->filein < 0) return false;

	this->filein_size = __LSEEK(this->filein, 0, SEEK_END);
	return true;
}

void AudioPlayback::filein_close(void)
{
	if(this->filein < 0) return;

	close(this->filein);
	this->filein = -1;
	this->filein_size = 0;
	return;
}

void AudioPlayback::audio_hw_deinit(void)
{
	if(this->audio_dev == nullptr) return;

	snd_pcm_drain(this->audio_dev);
	snd_pcm_close(this->audio_dev);
	this->audio_dev = nullptr;
	return;
}

void AudioPlayback::playback_proc(void)
{
	this->stop = false;
	this->filein_pos = this->audio_data_begin;

	this->playback_init();
	this->playback_loop();
	return;
}

void AudioPlayback::playback_init(void)
{
	this->buffer_remap();

	this->buffer_load();

	this->curr_buf_cycle = !this->curr_buf_cycle;
	this->buffer_remap();

	return;
}

void AudioPlayback::playback_loop(void)
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

void AudioPlayback::buffer_remap(void)
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

void AudioPlayback::buffer_play(void)
{
	int n_ret = snd_pcm_writei(this->audio_dev, this->playout_buf, (snd_pcm_uframes_t) this->BUFFER_SIZE_FRAMES);
	if(n_ret == -EPIPE) snd_pcm_prepare(this->audio_dev);
	return;
}

