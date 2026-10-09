/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPB_HPP
#define AUDIOPB_HPP

#include "globldef.h"
#include "filedef.h"
#include "strdef.hpp"

#include "shared.h"

#include <alsa/asoundlib.h>

constexpr size_t __AUDIODEVICELIST_ENTRY_TEXTLENGTH = 256u;

struct _audiodevicelist_entry {
	char name[__AUDIODEVICELIST_ENTRY_TEXTLENGTH];
	char desc[__AUDIODEVICELIST_ENTRY_TEXTLENGTH];
};

typedef struct _audiodevicelist_entry audiodevicelist_entry_t;

struct _audiodevicelist {
	__attribute__((__aligned__(PTR_SIZE_BITS))) audiodevicelist_entry_t *p_entries;
	__attribute__((__aligned__(PTR_SIZE_BITS))) size_t n_entries;
};

typedef struct _audiodevicelist audiodevicelist_t;

struct _audiodevice {
	__attribute__((__aligned__(PTR_SIZE_BITS))) snd_pcm_t *p_device;
	__attribute__((__aligned__(PTR_SIZE_BITS))) std::string device_name;
	__attribute__((__aligned__(32))) int enable_resampling;
};

typedef struct _audiodevice audiodevice_t;

struct _audiopb_params {
	__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t audio_data_begin;
	__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t audio_data_end;
	__attribute__((__aligned__(PTR_SIZE_BITS))) const char *file_dir;
	__attribute__((__aligned__(PTR_SIZE_BITS))) size_t audiobuffer_size_frames;
	__attribute__((__aligned__(PTR_SIZE_BITS))) size_t streambuffer_segment_size_frames;
	__attribute__((__aligned__(PTR_SIZE_BITS))) size_t streambuffer_n_segments;
	__attribute__((__aligned__(32))) uint32_t sample_rate;
	__attribute__((__aligned__(16))) uint16_t n_channels;
};

typedef struct _audiopb_params audiopb_params_t;

class AudioPB {
	public:
		AudioPB(const audiopb_params_t *p_params);
		virtual ~AudioPB(void);

		bool setParameters(const audiopb_params_t *p_params);
		bool initialize(void);
		bool runPlayback(void);
		void pausePlayback(void);
		void resumePlayback(void);
		void stopPlayback(void);

		bool loadAudioDeviceList(void);
		ssize_t getAudioDeviceListEntryCount(void);
		const audiodevicelist_entry_t* getAudioDeviceListEntry(size_t index);

		bool chooseDevice(const char *name, bool enable_resampling);
		bool chooseDevice(size_t index, bool enable_resampling);
		bool chooseDefaultDevice(bool enable_resampling);

		__offset_t getAudioDataSizeFrames(void);
		__offset_t getAudioDataPositionFrames(void);
		bool setAudioDataPositionFrames(__offset_t position);

		int getStatus(void);
		__string getLastErrorMessage(void);

		enum Status {
			STATUS_ERROR_INVALIDPARAMS = -5,
			STATUS_ERROR_MEMORY = -4,
			STATUS_ERROR_AUDIOHW = -3,
			STATUS_ERROR_NOFILE = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_RUNNING = 2,
			STATUS_PAUSED = 3,
			STATUS_STOPPED = 4
		};

	protected:
		static constexpr size_t N_CHANNELS_MIN = 1u;
		static constexpr size_t STREAMBUFFER_N_SEGMENTS_MIN = 2u;
		static constexpr size_t STREAMBUFFER_SEGMENT_SIZE_FRAMES_MIN = 32u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIOBUFFER_SIZE_FRAMES = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SIZE_SAMPLES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SIZE_BYTES = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_N_SEGMENTS = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t STREAMBUFFER_SEGMENT_SIZE_BYTES = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t AUDIO_BYTES_PER_SAMPLE = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t FILE_BYTES_PER_SAMPLE = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t SAMPLE_RATE = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t N_CHANNELS = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t AUDIO_DATA_BEGIN = 0;
		__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t AUDIO_DATA_END = 0;

		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t streambuffer_nseg_load = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t streambuffer_nseg_play = 0u;

		__attribute__((__aligned__(PTR_SIZE_BITS))) void *p_streambuffer = NULL;

		__attribute__((__aligned__(PTR_SIZE_BITS))) audiodevice_t audiodev = {
			.p_device = NULL,
			.device_name = "",
			.enable_resampling = 0
		};

		__attribute__((__aligned__(PTR_SIZE_BITS))) audiodevicelist_t audiodevlist = {
			.p_entries = NULL,
			.n_entries = 0u
		};

		__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t filein_size = 0;
		__attribute__((__aligned__(PTR_SIZE_BITS))) __offset_t filein_pos = 0;

		__attribute__((__aligned__(PTR_SIZE_BITS))) std::string FILEIN_DIR = "";
		__attribute__((__aligned__(PTR_SIZE_BITS))) __string err_msg = __TEXT("");

		__attribute__((__aligned__(32))) int AUDIODEV_FORMAT = -1;
		__attribute__((__aligned__(32))) int h_filein = -1;
		__attribute__((__aligned__(32))) int status = AudioPB::STATUS_UNINITIALIZED;

		void deinitialize(void);

		bool filein_open(void);
		void filein_close(void);

		bool audio_hw_init(void);
		void audio_hw_deinit(void);

		virtual bool buffer_alloc(void) = 0;
		virtual void buffer_free(void) = 0;

		void playback_proc(void);
		void playback_init(void);
		void playback_loop(void);

		void streambuffer_nseg_update(void);

		virtual void buffer_load(void) = 0;
		void buffer_play(void);
};

#endif /*AUDIOPB_HPP*/

