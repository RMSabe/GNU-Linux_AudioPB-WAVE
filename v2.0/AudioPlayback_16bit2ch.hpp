/*
 * WAVE audio file playback app v2.0 for GNU-Linux
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPLAYBACK_16BIT2CH_HPP
#define AUDIOPLAYBACK_16BIT2CH_HPP

#include "AudioPlayback.hpp"

class AudioPlayback_16bit2ch : public AudioPlayback {
	public:
		AudioPlayback_16bit2ch(audio_playback_params_t *params);
		~AudioPlayback_16bit2ch(void);

		bool runPlayback(void) override;

	private:
		std::int16_t *buffer_0 = nullptr;
		std::int16_t *buffer_1 = nullptr;

		std::int16_t *load_buf = nullptr;
		std::int16_t *play_buf = nullptr;

		bool curr_buf_cycle = false;
		bool stop = false;

		bool audio_hw_init(void) override;
		void buffer_malloc(void) override;
		void buffer_free(void) override;

		void playback_proc(void);
		void playback_init(void);
		void playback_loop(void);
		void buffer_remap(void);
		void buffer_load(void);
		void buffer_play(void);
};

#endif //AUDIOPLAYBACK_16BIT2CH_HPP

