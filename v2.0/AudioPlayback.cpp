/*
 * WAVE audio file playback app v2.0 for GNU-Linux
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

