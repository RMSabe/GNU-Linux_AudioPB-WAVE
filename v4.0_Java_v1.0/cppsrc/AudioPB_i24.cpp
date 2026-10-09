/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB_i24.hpp"
#include <stdlib.h>
#include <string.h>

AudioPB_i24::AudioPB_i24(const audiopb_params_t *p_params) : AudioPB(p_params)
{
	this->AUDIO_BYTES_PER_SAMPLE = 4u;
	this->FILE_BYTES_PER_SAMPLE = 3u;
	this->AUDIODEV_FORMAT = SND_PCM_FORMAT_S24_LE;
}

AudioPB_i24::~AudioPB_i24(void)
{
	this->deinitialize();
}

bool AudioPB_i24::buffer_alloc(void)
{
	this->buffer_free();

	this->INPUTBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->FILE_BYTES_PER_SAMPLE);

	this->p_inputbuffer = (uint8_t*) malloc(this->INPUTBUFFER_SIZE_BYTES);
	this->p_streambuffer = malloc(this->STREAMBUFFER_SIZE_BYTES);

	if(this->p_inputbuffer == NULL)
	{
		this->buffer_free();
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = __TEXT("AudioPB_i24::buffer_alloc: Error: memory allocate failed.");
		return false;
	}

	if(this->p_streambuffer == NULL)
	{
		this->buffer_free();
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = __TEXT("AudioPB_i24::buffer_alloc: Error: memory allocate failed.");
		return false;
	}

	memset(this->p_inputbuffer, 0, this->INPUTBUFFER_SIZE_BYTES);
	memset(this->p_streambuffer, 0, this->STREAMBUFFER_SIZE_BYTES);
	return true;
}

void AudioPB_i24::buffer_free(void)
{
	if(this->p_inputbuffer != NULL)
	{
		free(this->p_inputbuffer);
		this->p_inputbuffer = NULL;
	}

	if(this->p_streambuffer != NULL)
	{
		free(this->p_streambuffer);
		this->p_streambuffer = NULL;
	}

	return;
}

void AudioPB_i24::buffer_load(void)
{
	int32_t *_p_input = NULL;
	size_t _nsample;
	size_t _nbyte;

	if(this->filein_pos >= this->AUDIO_DATA_END)
	{
		this->status = AudioPB::STATUS_STOPPED;
		return;
	}

	_p_input = (int32_t*) (((uintptr_t) (this->p_streambuffer)) + (this->streambuffer_nseg_load)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	memset(this->p_inputbuffer, 0, this->INPUTBUFFER_SIZE_BYTES);

	__LSEEK(this->h_filein, this->filein_pos, SEEK_SET);
	read(this->h_filein, this->p_inputbuffer, this->INPUTBUFFER_SIZE_BYTES);
	this->filein_pos += (__offset_t) this->INPUTBUFFER_SIZE_BYTES;

	_nbyte = 0u;
	for(_nsample = 0u; _nsample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; _nsample++)
	{
		_p_input[_nsample] = ((this->p_inputbuffer[_nbyte + 2u] << 16) | (this->p_inputbuffer[_nbyte + 1u] << 8) | (this->p_inputbuffer[_nbyte]));

		if(_p_input[_nsample] & 0x00800000) _p_input[_nsample] |= 0xff800000;

		_nbyte += this->FILE_BYTES_PER_SAMPLE;
	}

	return;
}

