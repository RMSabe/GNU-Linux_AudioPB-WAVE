/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "AudioPB.hpp"
#include "cstrdef.h"
#include "delay.h"

#include <stdlib.h>
#include <string.h>

AudioPB::AudioPB(const audiopb_params_t *p_params)
{
	this->setParameters(p_params);
}

AudioPB::~AudioPB(void)
{
}

bool AudioPB::setParameters(const audiopb_params_t *p_params)
{
	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = __TEXT("AudioPB::setParameters: Error: cannot run method. Audio object is already initialized.");
		return false;
	}

	this->status = AudioPB::STATUS_UNINITIALIZED;

	if(p_params == NULL)
	{
		this->err_msg = __TEXT("AudioPB::setParameters: Error: given p_params is NULL.");
		return false;
	}

	if(p_params->file_dir == NULL)
	{
		this->err_msg = __TEXT("AudioPB::setParameters: Error: invalid file directory.");
		return false;
	}

	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->FILEIN_DIR = p_params->file_dir;
	this->AUDIOBUFFER_SIZE_FRAMES = p_params->audiobuffer_size_frames;
	this->STREAMBUFFER_SEGMENT_SIZE_FRAMES = p_params->streambuffer_segment_size_frames;
	this->STREAMBUFFER_N_SEGMENTS = p_params->streambuffer_n_segments;
	this->SAMPLE_RATE = (size_t) p_params->sample_rate;
	this->N_CHANNELS = (size_t) p_params->n_channels;

	return true;
}

bool AudioPB::initialize(void)
{
	if(this->status > AudioPB::STATUS_UNINITIALIZED) return true;

	this->status = AudioPB::STATUS_UNINITIALIZED;

	if(!this->SAMPLE_RATE)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid sample rate.");
		return false;
	}

	if(this->N_CHANNELS < AudioPB::N_CHANNELS_MIN)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid number of channels.");
		return false;
	}

	if(this->STREAMBUFFER_N_SEGMENTS < AudioPB::STREAMBUFFER_N_SEGMENTS_MIN)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid stream buffer segment count.");
		return false;
	}

	if(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES < AudioPB::STREAMBUFFER_SEGMENT_SIZE_FRAMES_MIN)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid stream buffer size.");
		return false;
	}

	if(this->AUDIOBUFFER_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)
	{
		this->status = AudioPB::STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = __TEXT("AudioPB::initialize: Error: invalid audio buffer size. (audio buffer smaller than stream buffer segment).");
		return false;
	}

	this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES = (this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SEGMENT_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->STREAMBUFFER_SIZE_FRAMES = (this->STREAMBUFFER_N_SEGMENTS)*(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);
	this->STREAMBUFFER_SIZE_SAMPLES = (this->STREAMBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	if(!this->filein_open()) return false;

	if(!this->audio_hw_init())
	{
		this->filein_close();
		return false;
	}

	if(!this->buffer_alloc())
	{
		this->filein_close();
		this->audio_hw_deinit();
		return false;
	}

	this->status = AudioPB::STATUS_READY;
	return true;
}

bool AudioPB::runPlayback(void)
{
	if(this->status != AudioPB::STATUS_READY)
	{
		this->err_msg = __TEXT("AudioPB::runPlayback: Error: cannot run method. Audio object is either not initialized or already running.");
		return false;
	}

	this->playback_proc();
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	return true;
}

void AudioPB::pausePlayback(void)
{
	if(this->status == AudioPB::STATUS_RUNNING)
	{
		this->status = AudioPB::STATUS_PAUSED;
		snd_pcm_drain(this->audiodev.p_device);
	}

	return;
}

void AudioPB::resumePlayback(void)
{
	if(this->status == AudioPB::STATUS_PAUSED)
	{
		snd_pcm_prepare(this->audiodev.p_device);
		this->status = AudioPB::STATUS_RUNNING;
	}

	return;
}

void AudioPB::stopPlayback(void)
{
	if(this->status > AudioPB::STATUS_READY) this->status = AudioPB::STATUS_STOPPED;
	return;
}

bool AudioPB::loadAudioDeviceList(void)
{
	void **_pp_hints = NULL;
	char *_p_devname = NULL;
	char *_p_devdesc = NULL;
	char *_p_devioid = NULL;
	char *_p_audiodevicelist_entry_name = NULL;
	char *_p_audiodevicelist_entry_desc = NULL;

	size_t _n_hint;
	size_t _n_sndctl;
	size_t _n_sndctl_count;
	size_t _audiodevicelist_byteindex;

	int _nret;
	int _i32_sndctlindex;

	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	if(this->audiodevlist.p_entries != NULL)
	{
		free(this->audiodevlist.p_entries);
		this->audiodevlist.p_entries = NULL;
	}

	this->audiodevlist.n_entries = 0u;

	_n_sndctl_count = 0u;
	_i32_sndctlindex = -1;

	while(true)
	{
		_nret = snd_card_next(&_i32_sndctlindex);
		if(_nret < 0)
		{
			this->status = AudioPB::STATUS_ERROR_AUDIOHW;
			this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_card_next failed.");
			return false;
		}

		if(_i32_sndctlindex < 0) break;

		_n_sndctl_count++;
	}

	_n_sndctl = 0u;
	while(_n_sndctl < _n_sndctl_count)
	{
		_nret = snd_device_name_hint((int) _n_sndctl, "pcm", &_pp_hints);
		if((_nret < 0) || (_pp_hints == NULL))
		{
			this->status = AudioPB::STATUS_ERROR_AUDIOHW;
			this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_hint failed.");
			return false;
		}

		_n_hint = 0u;
		while(_pp_hints[_n_hint] != NULL)
		{
			_p_devname = snd_device_name_get_hint((const char*) _pp_hints[_n_hint], "NAME");
			if(_p_devname == NULL)
			{
				snd_device_name_free_hint(_pp_hints);
				this->status = AudioPB::STATUS_ERROR_AUDIOHW;
				this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
				return false;
			}

			/*Consider only hardware interface devices ("hw:..." "plughw:...")*/

			if(_cstr_char_compare_upto_char(_p_devname, "hw", ':', true) || _cstr_char_compare_upto_char(_p_devname, "plughw", ':', true))
			{
				_p_devioid = snd_device_name_get_hint((const char*) _pp_hints[_n_hint], "IOID");

				/*Count only playback devices*/
				if((_p_devioid == NULL) || _cstr_char_compare(_p_devioid, "Output")) this->audiodevlist.n_entries++;

				if(_p_devioid != NULL)
				{
					free(_p_devioid);
					_p_devioid = NULL;
				}
			}

			free(_p_devname);
			_p_devname = NULL;

			_n_hint++;
		}

		snd_device_name_free_hint(_pp_hints);
		_pp_hints = NULL;

		_n_sndctl++;
	}

	if(!this->audiodevlist.n_entries)
	{
		this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: no playback devices found.");
		return false;
	}

	this->audiodevlist.p_entries = (audiodevicelist_entry_t*) malloc((this->audiodevlist.n_entries)*sizeof(audiodevicelist_entry_t));
	if(this->audiodevlist.p_entries == NULL)
	{
		this->status = AudioPB::STATUS_ERROR_MEMORY;
		this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: failed to allocate heap memory.");
		return false;
	}

	_audiodevicelist_byteindex = 0u;
	_n_sndctl = 0u;

	while(_n_sndctl < _n_sndctl_count)
	{
		_nret = snd_device_name_hint((int) _n_sndctl, "pcm", &_pp_hints);
		if((_nret < 0) || (_pp_hints == NULL))
		{
			this->status = AudioPB::STATUS_ERROR_AUDIOHW;
			this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_hint failed.");
			return false;
		}

		_n_hint = 0u;
		while(_pp_hints[_n_hint] != NULL)
		{
			_p_devname = snd_device_name_get_hint((const char*) _pp_hints[_n_hint], "NAME");
			if(_p_devname == NULL)
			{
				snd_device_name_free_hint(_pp_hints);
				this->status = AudioPB::STATUS_ERROR_AUDIOHW;
				this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
				return false;
			}

			if(_cstr_char_compare_upto_char(_p_devname, "hw", ':', true) || _cstr_char_compare_upto_char(_p_devname, "plughw", ':', true))
			{
				_p_devioid = snd_device_name_get_hint((const char*) _pp_hints[_n_hint], "IOID");

				if((_p_devioid == NULL) || _cstr_char_compare(_p_devioid, "Output"))
				{
					_p_devdesc = snd_device_name_get_hint((const char*) _pp_hints[_n_hint], "DESC");
					if(_p_devdesc == NULL)
					{
						if(_p_devioid != NULL) free(_p_devioid);
						free(_p_devname);
						snd_device_name_free_hint(_pp_hints);
						this->status = AudioPB::STATUS_ERROR_AUDIOHW;
						this->err_msg = __TEXT("AudioPB::loadAudioDeviceList: Error: ALSA snd_device_name_get_hint failed.");
						return false;
					}

					_p_audiodevicelist_entry_name = ((audiodevicelist_entry_t*) (((uintptr_t) this->audiodevlist.p_entries) + _audiodevicelist_byteindex))->name;
					_p_audiodevicelist_entry_desc = ((audiodevicelist_entry_t*) (((uintptr_t) this->audiodevlist.p_entries) + _audiodevicelist_byteindex))->desc;

					_cstr_char_copy(_p_devname, _p_audiodevicelist_entry_name, __AUDIODEVICELIST_ENTRY_TEXTLENGTH);
					_cstr_char_copy(_p_devdesc, _p_audiodevicelist_entry_desc, __AUDIODEVICELIST_ENTRY_TEXTLENGTH);

					_audiodevicelist_byteindex += sizeof(audiodevicelist_entry_t);

					free(_p_devdesc);
					_p_devdesc = NULL;
				}

				if(_p_devioid != NULL)
				{
					free(_p_devioid);
					_p_devioid = NULL;
				}
			}

			free(_p_devname);
			_p_devname = NULL;

			_n_hint++;
		}

		snd_device_name_free_hint(_pp_hints);
		_pp_hints = NULL;

		_n_sndctl++;
	}

	this->status = AudioPB::STATUS_UNINITIALIZED;
	return true;
}

ssize_t AudioPB::getAudioDeviceListEntryCount(void)
{
	return (ssize_t) this->audiodevlist.n_entries;
}

const audiodevicelist_entry_t* AudioPB::getAudioDeviceListEntry(size_t index)
{
	if(this->audiodevlist.p_entries == NULL)
	{
		this->err_msg = __TEXT("AudioPB::getAudioDeviceListEntry: Error: audio device list is not loaded.");
		return NULL;
	}

	if(index >= this->audiodevlist.n_entries)
	{
		this->err_msg = __TEXT("AudioPB::getAudioDeviceListEntry: Error: given index is out of bounds.");
		return NULL;
	}

	return (const audiodevicelist_entry_t*) (((uintptr_t) (this->audiodevlist.p_entries)) + index*sizeof(audiodevicelist_entry_t));
}

bool AudioPB::chooseDevice(const char *name, bool enable_resampling)
{
	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	if(name == NULL)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: given name parameter is NULL.");
		return false;
	}

	this->audiodev.device_name = name;
	this->audiodev.enable_resampling = (int) enable_resampling;
	return true;
}

bool AudioPB::chooseDevice(size_t index, bool enable_resampling)
{
	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	if(this->audiodevlist.p_entries == NULL)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: audio device list is not loaded.");
		return false;
	}

	if(index >= this->audiodevlist.n_entries)
	{
		this->err_msg = __TEXT("AudioPB::chooseDevice: Error: given index is out of bounds.");
		return false;
	}

	this->audiodev.device_name = this->audiodevlist.p_entries[index].name;
	this->audiodev.enable_resampling = (int) enable_resampling;
	return true;
}

bool AudioPB::chooseDefaultDevice(bool enable_resampling)
{
	if(this->status > AudioPB::STATUS_UNINITIALIZED)
	{
		this->err_msg = __TEXT("AudioPB::chooseDefaultDevice: Error: cannot run method. Audio object already initialized.");
		return false;
	}

	this->audiodev.device_name = "default";
	this->audiodev.enable_resampling = (int) enable_resampling;
	return true;
}

__offset_t AudioPB::getAudioDataSizeFrames(void)
{
	__offset_t _file_bytes_per_frame;

	if(this->status < AudioPB::STATUS_READY) return -1;

	_file_bytes_per_frame = (__offset_t) ((this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS));

	return (this->AUDIO_DATA_END - this->AUDIO_DATA_BEGIN)/_file_bytes_per_frame;
}

__offset_t AudioPB::getAudioDataPositionFrames(void)
{
	__offset_t _file_bytes_per_frame;

	if(this->status < AudioPB::STATUS_READY) return -1;

	_file_bytes_per_frame = (__offset_t) ((this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS));

	return (this->filein_pos - this->AUDIO_DATA_BEGIN)/_file_bytes_per_frame;
}

bool AudioPB::setAudioDataPositionFrames(__offset_t position)
{
	__offset_t _file_bytes_per_frame;
	__offset_t _file_data_size_frames;

	if(this->status < AudioPB::STATUS_READY) return false;

	_file_data_size_frames = this->getAudioDataSizeFrames();
	if(_file_data_size_frames < 0) return false;

	if((position < 0) || (position >= _file_data_size_frames))
	{
		this->err_msg = __TEXT("AudioPB::setAudioDataPositionFrames: Error: invalid position value.");
		return false;
	}

	_file_bytes_per_frame = (__offset_t) ((this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS));

	this->filein_pos = this->AUDIO_DATA_BEGIN + (position*_file_bytes_per_frame);
	return true;
}

int AudioPB::getStatus(void)
{
	return this->status;
}

__string AudioPB::getLastErrorMessage(void)
{
	if(this->status == AudioPB::STATUS_UNINITIALIZED)
		return (__TEXT("Error: Audio object not initialized.\nExtended error message: ") + this->err_msg);

	return this->err_msg;
}

void AudioPB::deinitialize(void)
{
	this->status = AudioPB::STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit();
	this->buffer_free();

	if(this->audiodevlist.p_entries != NULL)
	{
		free(this->audiodevlist.p_entries);
		this->audiodevlist.p_entries = NULL;
	}

	this->audiodevlist.n_entries = 0u;
	return;
}

bool AudioPB::filein_open(void)
{
	this->filein_close();

	this->h_filein = open(this->FILEIN_DIR.c_str(), O_RDONLY);
	if(this->h_filein < 0)
	{
		this->status = AudioPB::STATUS_ERROR_NOFILE;
		this->err_msg = __TEXT("AudioPB::filein_open: Error: failed to open input file.");
		return false;
	}

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

bool AudioPB::audio_hw_init(void)
{
	snd_pcm_hw_params_t *_p_hwparams = NULL;
	snd_pcm_uframes_t _nframes;
	int _nret;

	this->audio_hw_deinit();

	_nret = snd_pcm_open(&(this->audiodev.p_device), this->audiodev.device_name.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(_nret < 0)
	{
		this->audiodev.p_device = NULL;
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_open failed.");
		return false;
	}

	_nret = snd_pcm_hw_params_malloc(&_p_hwparams);
	if((_nret < 0) || (_p_hwparams == NULL))
	{
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_malloc failed.");
		return false;
	}

	_nret = snd_pcm_hw_params_any(this->audiodev.p_device, _p_hwparams);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_any failed.");
		return false;
	}

	_nret = snd_pcm_hw_params_set_rate_resample(this->audiodev.p_device, _p_hwparams, this->audiodev.enable_resampling);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_rate_resample failed.");
		return false;
	}

	_nret = snd_pcm_hw_params_set_access(this->audiodev.p_device, _p_hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_access failed.");
		return false;
	}

	_nret = snd_pcm_hw_params_set_format(this->audiodev.p_device, _p_hwparams, (snd_pcm_format_t) this->AUDIODEV_FORMAT);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_format failed.");
		return false;
	}

	_nret = snd_pcm_hw_params_set_channels(this->audiodev.p_device, _p_hwparams, (unsigned int) this->N_CHANNELS);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_channels failed.");
		return false;
	}

	_nret = snd_pcm_hw_params_set_rate(this->audiodev.p_device, _p_hwparams, (unsigned int) this->SAMPLE_RATE, 0);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_rate failed.");
		return false;
	}

	_nframes = (snd_pcm_uframes_t) this->AUDIOBUFFER_SIZE_FRAMES;
	_nret = snd_pcm_hw_params_set_buffer_size_near(this->audiodev.p_device, _p_hwparams, &_nframes);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_buffer_size_near failed.");
		return false;
	}

	_nret = snd_pcm_hw_params_set_period_size(this->audiodev.p_device, _p_hwparams, (snd_pcm_uframes_t) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES, 0);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params_set_period_size failed.");
		return false;
	}

	_nret = snd_pcm_hw_params(this->audiodev.p_device, _p_hwparams);
	if(_nret < 0)
	{
		snd_pcm_hw_params_free(_p_hwparams);
		this->audio_hw_deinit();
		this->status = AudioPB::STATUS_ERROR_AUDIOHW;
		this->err_msg = __TEXT("AudioPB::audio_hw_init: Error: snd_pcm_hw_params failed.");
		return false;
	}

	snd_pcm_hw_params_free(_p_hwparams);
	return true;
}

void AudioPB::audio_hw_deinit(void)
{
	if(this->audiodev.p_device == NULL) return;

	snd_pcm_drop(this->audiodev.p_device);
	snd_pcm_hw_free(this->audiodev.p_device);
	snd_pcm_close(this->audiodev.p_device);

	this->audiodev.p_device = NULL;
	return;
}

void AudioPB::playback_proc(void)
{
	this->playback_init();
	this->playback_loop();

	snd_pcm_drain(this->audiodev.p_device);
	return;
}

void AudioPB::playback_init(void)
{
	this->streambuffer_nseg_load = 0u;
	this->streambuffer_nseg_play = (this->STREAMBUFFER_N_SEGMENTS)/2u;

	this->filein_pos = this->AUDIO_DATA_BEGIN;

	this->status = AudioPB::STATUS_RUNNING;
	return;
}

void AudioPB::playback_loop(void)
{
	while(true)
	{
		if((this->status < AudioPB::STATUS_READY) || (this->status == AudioPB::STATUS_STOPPED)) break;

		if(this->status == AudioPB::STATUS_PAUSED)
		{
			delay_ms(1);
			continue;
		}

		this->buffer_play();
		this->buffer_load();
		this->streambuffer_nseg_update();

		snd_pcm_wait(this->audiodev.p_device, -1);
	}

	return;
}

void AudioPB::streambuffer_nseg_update(void)
{
	this->streambuffer_nseg_load++;
	this->streambuffer_nseg_load %= this->STREAMBUFFER_N_SEGMENTS;

	this->streambuffer_nseg_play++;
	this->streambuffer_nseg_play %= this->STREAMBUFFER_N_SEGMENTS;

	return;
}

void AudioPB::buffer_play(void)
{
	void *_p_output = NULL;
	ssize_t _nret;

	if(this->status != AudioPB::STATUS_RUNNING) return;

	_p_output = (void*) (((uintptr_t) (this->p_streambuffer)) + (this->streambuffer_nseg_play)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	_nret = (ssize_t) snd_pcm_writei(this->audiodev.p_device, _p_output, (snd_pcm_uframes_t) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);
	if(_nret < 0)
	{
		if(_nret == -EPIPE)
		{
			_nret = (ssize_t) snd_pcm_prepare(this->audiodev.p_device);
			if(_nret < 0) app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioPB::buffer_play: Error: snd_pcm_prepare failed."));
		}
		else app_exit(-1, __TEXT("CRITICAL ERROR OCCURRED: AudioPB::buffer_play: Error: snd_pcm_writei failed."));
	}

	return;
}

