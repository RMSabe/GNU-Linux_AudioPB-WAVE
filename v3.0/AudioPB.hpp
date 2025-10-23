/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 3.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPB_HPP
#define AUDIOPB_HPP

#include "globldef.h"
#include "filedef.h"
#include "strdef.hpp"
#include "cppthread.hpp"

#include "shared.hpp"

#include <alsa/asoundlib.h>

struct _audiopb_params {
	const char *audio_dev_desc;
	const char *filein_dir;
	__offset audio_data_begin;
	__offset audio_data_end;
	uint32_t sample_rate;
	uint16_t n_channels;
};

typedef struct _audiopb_params audiopb_params_t;

class AudioPB {
	public:
		AudioPB(const audiopb_params_t *p_params);

		bool setParameters(const audiopb_params_t *p_params);
		bool initialize(void);
		bool runPlayback(void);

		std::string getLastErrorMessage(void);

		enum Status {
			STATUS_ERROR_MEMALLOC = -4,
			STATUS_ERROR_AUDIOHW = -3,
			STATUS_ERROR_NOFILE = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_PLAYING = 2
		};

	protected:
		size_t AUDIOBUFFER_SIZE_FRAMES = 0u;
		size_t AUDIOBUFFER_SIZE_SAMPLES = 0u;
		size_t AUDIOBUFFER_SIZE_BYTES = 0u;

		size_t AUDIOBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		size_t AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		size_t AUDIOBUFFER_SEGMENT_SIZE_BYTES = 0u;

		void *p_buffer0 = NULL;
		void *p_buffer1 = NULL;

		void *p_loadbuf = NULL;
		void *p_playbuf = NULL;

		snd_pcm_t *p_audiodev = NULL;

		int h_filein = -1;
		__offset filein_size = 0;
		__offset filein_pos = 0;

		std::string AUDIODEV_DESC = "";
		std::string FILEIN_DIR = "";

		__offset AUDIO_DATA_BEGIN = 0;
		__offset AUDIO_DATA_END = 0;

		uint32_t SAMPLE_RATE = 0u;
		uint16_t N_CHANNELS = 0u;

		std::thread userthread;

		std::string usr_cmd = "";
		std::string err_msg = "";

		int status = this->STATUS_UNINITIALIZED;

		bool buf_cycle = false;
		bool stop_playback = false;

		bool filein_open(void);
		void filein_close(void);

		virtual bool audio_hw_init(void) = 0;
		void audio_hw_deinit(void);

		virtual bool buffer_alloc(void) = 0;
		virtual void buffer_free(void) = 0;

		void playback_proc(void);
		void playback_init(void);
		void playback_loop(void);

		virtual void buffer_load(void) = 0;

		void buffer_play(void);
		void buffer_remap(void);

		void cmdui_cmd_decode(void);

		void userthread_proc(void);
};

#endif /*AUDIOPB_HPP*/

