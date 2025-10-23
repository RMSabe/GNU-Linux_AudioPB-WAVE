/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 3.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB.hpp"

#include <string.h>
#include <poll.h>
#include <iostream>

AudioPB::AudioPB(const audiopb_params_t *p_params)
{
	this->setParameters(p_params);
}

bool AudioPB::setParameters(const audiopb_params_t *p_params)
{
	if(this->status > 0)
	{
		this->err_msg = "AudioPB::setParameters: Error: audio object is already initialized.";
		return false;
	}

	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL)
	{
		this->err_msg = "AudioPB::setParameters: Error: given playback params object is NULL.";
		return false;
	}

	if(p_params->audio_dev_desc == NULL)
	{
		this->err_msg = "AudioPB::setParameters: Error: params argument \"audio_dev_desc\" is invalid.";
		return false;
	}

	if(p_params->filein_dir == NULL)
	{
		this->err_msg = "AudioPB::setParameters: Error: params argument \"filein_dir\" is invalid.";
		return false;
	}

	this->AUDIODEV_DESC = p_params->audio_dev_desc;
	this->FILEIN_DIR = p_params->filein_dir;
	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->SAMPLE_RATE = p_params->sample_rate;
	this->N_CHANNELS = p_params->n_channels;

	return true;
}

bool AudioPB::initialize(void)
{
	if(this->status > 0) return true;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		this->err_msg = "AudioPB::initialize: Error: failed to open input file.";
		return false;
	}

	if(!this->audio_hw_init())
	{
		this->filein_close();
		this->status = this->STATUS_ERROR_AUDIOHW;
		return false;
	}

	if(!this->buffer_alloc())
	{
		this->filein_close();
		this->audio_hw_deinit();
		this->status = this->STATUS_ERROR_MEMALLOC;
		this->err_msg = "AudioPB::initialize: Error: memory allocate failed.";
		return false;
	}

	this->status = this->STATUS_READY;
	return true;
}

bool AudioPB::runPlayback(void)
{
	if(this->status != this->STATUS_READY)
	{
		this->err_msg = "AudioPB::runPlayback: Error: audio object is either uninitialized or already playing.";
		return false;
	}

	std::cout << "Playback started\n";

	this->userthread = std::thread(&AudioPB::userthread_proc, this);

	this->playback_proc();

	cppthread_wait(&(this->userthread));

	std::cout << "Playback finished\n";

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;

	return true;
}

std::string AudioPB::getLastErrorMessage(void)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return "Error: audio object has not been initialized.\nExtended error message: " + this->err_msg;

	return this->err_msg;
}

bool AudioPB::filein_open(void)
{
	this->filein_close(); /*Close previous handle instance*/

	this->h_filein = open(this->FILEIN_DIR.c_str(), O_RDONLY);
	if(this->h_filein < 0) return false;

	this->filein_size = __LSEEK(this->h_filein, 0, SEEK_END);
	return true;
}

void AudioPB::filein_close(void)
{
	if(this->h_filein < 0) return;

	close(this->h_filein);
	this->h_filein = -1;
	this->filein_size = 0;

	return;
}

void AudioPB::audio_hw_deinit(void)
{
	if(this->p_audiodev == NULL) return;

	snd_pcm_drop(this->p_audiodev);
	snd_pcm_close(this->p_audiodev);
	this->p_audiodev = NULL;

	return;
}

void AudioPB::playback_proc(void)
{
	this->playback_init();
	this->playback_loop();

	snd_pcm_drain(this->p_audiodev);
	return;
}

void AudioPB::playback_init(void)
{
	this->buf_cycle = false;
	this->stop_playback = false;
	this->filein_pos = this->AUDIO_DATA_BEGIN;

	this->buffer_remap();
	return;
}

void AudioPB::playback_loop(void)
{
	while(!this->stop_playback)
	{
		this->buffer_play();
		this->buffer_load();

		snd_pcm_wait(this->p_audiodev, -1);
		this->buffer_remap();
	}

	return;
}

void AudioPB::buffer_play(void)
{
	ssize_t n_ret = 0;

	n_ret = (ssize_t) snd_pcm_writei(this->p_audiodev, this->p_playbuf, (snd_pcm_uframes_t) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES);
	if(n_ret < 0)
	{
		if(n_ret == -EPIPE)
		{
			n_ret = (ssize_t) snd_pcm_prepare(this->p_audiodev);
			if(n_ret < 0) app_exit(1, "AudioPB::buffer_play: Error: snd_pcm_prepare failed.");
		}
		else app_exit(1, "AudioPB::buffer_play: Error: snd_pcm_writei failed.");
	}

	return;
}

void AudioPB::buffer_remap(void)
{
	if(this->buf_cycle)
	{
		this->p_loadbuf = this->p_buffer1;
		this->p_playbuf = this->p_buffer0;
	}
	else
	{
		this->p_loadbuf = this->p_buffer0;
		this->p_playbuf = this->p_buffer1;
	}

	this->buf_cycle = !this->buf_cycle;
	return;
}

void AudioPB::cmdui_cmd_decode(void)
{
	this->usr_cmd = str_tolower(this->usr_cmd);

	if(!this->usr_cmd.compare("stop"))
	{
		this->stop_playback = true;
		return;
	}

	std::cout << "Error: invalid command entered\n";
	return;
}

void AudioPB::userthread_proc(void)
{
	int n_ret = 0;
	struct pollfd poll_userinput;

	memset(&poll_userinput, 0, sizeof(struct pollfd));

	poll_userinput.fd = STDIN_FILENO;
	poll_userinput.events = POLLIN;

	std::cout << "Enter \"stop\" to stop playback\n";

	while(!this->stop_playback)
	{
		n_ret = poll(&poll_userinput, 1, 1);

		if(n_ret > 0)
		{
			this->usr_cmd = "";
			std::cin >> this->usr_cmd;
			this->cmdui_cmd_decode();
		}
	}

	return;
}

