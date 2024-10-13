/*
 * WAVE audio file playback app v2.0.1 for GNU-Linux
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPLAYBACK_HPP
#define AUDIOPLAYBACK_HPP

#include "globaldef.h"
#include <iostream>
#include <string>

#include <alsa/asoundlib.h>

struct audio_playback_params {
	char *audio_dev_desc;
	char *filein_dir;
	__offset audio_data_begin;
	__offset audio_data_end;
	std::uint32_t sample_rate;
};

typedef struct audio_playback_params audio_playback_params_t;

class AudioPlayback {
	public:
		AudioPlayback(audio_playback_params_t *params);

		bool setParameters(audio_playback_params_t *params);
		bool runPlayback(void);

		std::string getLastErrorMessage(void);

	protected:
		enum Status {
			STATUS_ERROR_NOFILE = -3,
			STATUS_ERROR_AUDIOHW = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_INITIALIZED = 1
		};

		int status = STATUS_UNINITIALIZED;

		std::uint32_t sample_rate = 0u;

		int filein = -1;
		__offset filein_size = 0;
		__offset filein_pos = 0;

		__offset audio_data_begin = 0;
		__offset audio_data_end = 0;

		std::string filein_dir = "";
		std::string audio_dev_desc = "";

		std::string error_msg = "";

		snd_pcm_t *audio_dev = nullptr;

		size_t BUFFER_SIZE_FRAMES = 0u;
		size_t BUFFER_SIZE_SAMPLES = 0u;
		size_t BUFFER_SIZE_BYTES = 0u;

		void *bufferout_0 = nullptr;
		void *bufferout_1 = nullptr;

		void *loadout_buf = nullptr;
		void *playout_buf = nullptr;

		bool curr_buf_cycle = false;
		bool stop = false;

		bool filein_open(void);
		void filein_close(void);

		virtual bool audio_hw_init(void) = 0;
		void audio_hw_deinit(void);

		virtual void buffer_malloc(void) = 0;
		virtual void buffer_free(void) = 0;

		void playback_proc(void);
		void playback_init(void);
		void playback_loop(void);
		void buffer_remap(void);

		virtual void buffer_load(void) = 0;
		void buffer_play(void);
};

#endif //AUDIOPLAYBACK_HPP

