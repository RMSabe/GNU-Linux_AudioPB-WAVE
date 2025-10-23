/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 3.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB_i24.hpp"
#include <stdlib.h>
#include <string.h>

AudioPB_i24::AudioPB_i24(const audiopb_params_t *p_params) : AudioPB(p_params)
{
}

AudioPB_i24::~AudioPB_i24(void)
{
	this->stop_playback = true;

	cppthread_stop(&(this->userthread));
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();
}

bool AudioPB_i24::audio_hw_init(void)
{
	snd_pcm_hw_params_t *p_hwparams = NULL;
	snd_pcm_uframes_t n_frames = 0u;
	int n_ret = -1;
	/*unsigned int rate = 0u;*/

	this->audio_hw_deinit(); /*Clear any previous instance of audio device*/

	/*OPEN AUDIO DEVICE*/

	n_ret = snd_pcm_open(&(this->p_audiodev), this->AUDIODEV_DESC.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(n_ret < 0)
	{
		this->p_audiodev = NULL;
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_open failed.";
		return false;
	}

	/*ALLOCATE DEVICE PARAMS OBJ*/

	n_ret = snd_pcm_hw_params_malloc(&p_hwparams);
	if((n_ret < 0) || (p_hwparams == NULL))
	{
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_malloc failed.";
		return false;
	}

	/*PRELOAD DEVICE PARAMETERS*/

	n_ret = snd_pcm_hw_params_any(this->p_audiodev, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_any failed.";
		return false;
	}

	/*ENABLE/DISABLE DEVICE RESAMPLING*/
	/*
	 * 0 = disable (better performance)
	 * 1 = enable (better compatibility)
	 */

	n_ret = snd_pcm_hw_params_set_rate_resample(this->p_audiodev, p_hwparams, 1u);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_rate_resample failed.";
		return false;
	}

	/*SET DEVICE ACCESS*/

	n_ret = snd_pcm_hw_params_set_access(this->p_audiodev, p_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_access failed.";
		return false;
	}

	/*SET DEVICE FORMAT*/

	n_ret = snd_pcm_hw_params_set_format(this->p_audiodev, p_hwparams, SND_PCM_FORMAT_S24_LE);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_format failed.";
		return false;
	}

	/*SET DEVICE CHANNELS*/

	n_ret = snd_pcm_hw_params_set_channels(this->p_audiodev, p_hwparams, (unsigned int) this->N_CHANNELS);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_channels failed.";
		return false;
	}

	/*SET DEVICE SAMPLING RATE*/

	n_ret = snd_pcm_hw_params_set_rate(this->p_audiodev, p_hwparams, (unsigned int) this->SAMPLE_RATE, 0);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_rate failed.";
		return false;
	}

	/*SET DEVICE SAMPLING RATE (USING ALTERNATIVE FUNCTION)*/

	/*rate = (unsigned int) this->SAMPLE_RATE;
	n_ret = snd_pcm_hw_params_set_rate_near(this->p_audiodev, p_hwparams, &rate, NULL);

	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_rate_near failed.";
		return false;
	}
	
	if(rate != ((unsigned int) this->SAMPLE_RATE))
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_rate_near: rate is incompatible.";
		return false;
	}
	*/

	/*GET/SET DEVICE CURRENT BUFFER SIZE*/

	n_ret = snd_pcm_hw_params_get_buffer_size(p_hwparams, &n_frames);
	if(n_ret < 0)
	{
		n_frames = (snd_pcm_uframes_t) _get_closest_power2_ceil((size_t) this->SAMPLE_RATE);
		n_ret = snd_pcm_hw_params_set_buffer_size_near(this->p_audiodev, p_hwparams, &n_frames);

		if(n_ret < 0)
		{
			snd_pcm_hw_params_free(p_hwparams);
			this->audio_hw_deinit();
			this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_buffer_size_near failed.";
			return false;
		}
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (size_t) n_frames;
	this->AUDIOBUFFER_SIZE_SAMPLES = (this->AUDIOBUFFER_SIZE_FRAMES)*((size_t) this->N_CHANNELS);
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*4u;

	/*SET DEVICE BUFFER SEGMENT SIZE (PERIOD SIZE)*/

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(this->AUDIOBUFFER_SIZE_FRAMES/4u);

	n_ret = snd_pcm_hw_params_set_period_size(this->p_audiodev, p_hwparams, (snd_pcm_uframes_t) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, 0);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_period_size failed.";
		return false;
	}

	/*SET DEVICE BUFFER SEGMENT SIZE (PERIOD SIZE) (USING ALTERNATIVE FUNCTION)*/

	/*n_frames = (snd_pcm_uframes_t) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES;
	n_ret = snd_pcm_hw_params_set_period_size_near(this->p_audiodev, p_hwparams, &n_frames, NULL);

	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_period_size_near failed.";
		return false;
	}

	if(n_frames != ((snd_pcm_uframes_t) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES))
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params_set_period_size_near: period is not compatible.";
		return false;
	}*/

	/*APPLY SETTINGS TO DEVICE*/

	n_ret = snd_pcm_hw_params(this->p_audiodev, p_hwparams);
	if(n_ret < 0)
	{
		snd_pcm_hw_params_free(p_hwparams);
		this->audio_hw_deinit();
		this->err_msg = "AudioPB_i24::audio_hw_init: Error: snd_pcm_hw_params failed.";
		return false;
	}

	n_ret = snd_pcm_hw_params_get_period_size(p_hwparams, &n_frames, NULL);

	if(n_ret >= 0) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = (size_t) n_frames; /*Not really necessary, but just to be safe.*/

	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = (this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES)*((size_t) this->N_CHANNELS);
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*4u;

	this->BYTEBUF_SIZE = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*3u;

	snd_pcm_hw_params_free(p_hwparams);
	return true;
}

bool AudioPB_i24::buffer_alloc(void)
{
	this->buffer_free(); /*Clear any previous allocations*/

	this->p_buffer0 = malloc(this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	this->p_buffer1 = malloc(this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	this->p_bytebuf = (uint8_t*) malloc(this->BYTEBUF_SIZE);

	if(this->p_buffer0 == NULL)
	{
		this->buffer_free();
		return false;
	}

	if(this->p_buffer1 == NULL)
	{
		this->buffer_free();
		return false;
	}

	if(this->p_bytebuf == NULL)
	{
		this->buffer_free();
		return false;
	}

	memset(this->p_buffer0, 0, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	memset(this->p_buffer1, 0, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	memset(this->p_bytebuf, 0, this->BYTEBUF_SIZE);

	return true;
}

void AudioPB_i24::buffer_free(void)
{
	if(this->p_buffer0 != NULL)
	{
		free(this->p_buffer0);
		this->p_buffer0 = NULL;
	}

	if(this->p_buffer1 != NULL)
	{
		free(this->p_buffer1);
		this->p_buffer1 = NULL;
	}

	if(this->p_bytebuf != NULL)
	{
		free(this->p_bytebuf);
		this->p_bytebuf = NULL;
	}

	this->p_loadbuf = NULL;
	this->p_playbuf = NULL;
	return;
}

void AudioPB_i24::buffer_load(void)
{
	size_t n_sample = 0u;
	size_t n_byte = 0u;
	int32_t sample = 0;

	if(this->filein_pos >= this->AUDIO_DATA_END)
	{
		this->stop_playback = true;
		return;
	}

	memset(this->p_bytebuf, 0, this->BYTEBUF_SIZE);

	__LSEEK(this->h_filein, this->filein_pos, SEEK_SET);
	read(this->h_filein, this->p_bytebuf, this->BYTEBUF_SIZE);
	this->filein_pos += (__offset) this->BYTEBUF_SIZE;

	n_byte = 0u;
	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		sample = ((this->p_bytebuf[n_byte + 2u] << 16) | (this->p_bytebuf[n_byte + 1u] << 8) | (this->p_bytebuf[n_byte]));

		if(sample & 0x00800000) sample |= 0xff800000;
		else sample &= 0x007fffff; /*Not really necessary, but just to be safe.*/

		((int32_t*) (this->p_loadbuf))[n_sample] = sample;

		n_byte += 3u;
	}

	return;
}

