/*
 * WAVE audio file playback app v2.0.1 for GNU-Linux
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

	private:
		bool audio_hw_init(void) override;
		void buffer_malloc(void) override;
		void buffer_free(void) override;

		void buffer_load(void) override;
};

#endif //AUDIOPLAYBACK_16BIT2CH_HPP

