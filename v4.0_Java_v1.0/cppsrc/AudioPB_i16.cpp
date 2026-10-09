/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB_i16.hpp"
#include <stdlib.h>
#include <string.h>

AudioPB_i16::AudioPB_i16(const audiopb_params_t *p_params) : AudioPB(p_params)
{
	this->AUDIO_BYTES_PER_SAMPLE = 2u;
	this->FILE_BYTES_PER_SAMPLE = 2u;
	this->AUDIODEV_FORMAT = SND_PCM_FORMAT_S16_LE;
}

AudioPB_i16::~AudioPB_i16(void)
{
	this->deinitialize();
}

bool AudioPB_i16::buffer_alloc(void)
{
	this->buffer_free();

	this->p_streambuffer = malloc(this->STREAMBUFFER_SIZE_BYTES);
	if(this->p_streambuffer == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = __TEXT("AudioPB_i16::buffer_alloc: Error: memory allocate failed.");
		return false;
	}

	memset(this->p_streambuffer, 0, this->STREAMBUFFER_SIZE_BYTES);
	return true;
}

void AudioPB_i16::buffer_free(void)
{
	if(this->p_streambuffer != NULL)
	{
		free(this->p_streambuffer);
		this->p_streambuffer = NULL;
	}

	return;
}

void AudioPB_i16::buffer_load(void)
{
	void *_p_input = NULL;

	if(this->filein_pos >= this->AUDIO_DATA_END)
	{
		this->status = AudioPB::STATUS_STOPPED;
		return;
	}

	_p_input = (void*) (((uintptr_t) (this->p_streambuffer)) + (this->streambuffer_nseg_load)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));
	memset(_p_input, 0, this->STREAMBUFFER_SEGMENT_SIZE_BYTES);

	__LSEEK(this->h_filein, this->filein_pos, SEEK_SET);
	read(this->h_filein, _p_input, this->STREAMBUFFER_SEGMENT_SIZE_BYTES);
	this->filein_pos += (__offset_t) this->STREAMBUFFER_SEGMENT_SIZE_BYTES;

	return;
}

