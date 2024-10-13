/*
 * WAVE audio file playback app v2.0.1 for GNU-Linux
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPLAYBACK_24BIT1CH_HPP
#define AUDIOPLAYBACK_24BIT1CH_HPP

#include "AudioPlayback.hpp"

class AudioPlayback_24bit1ch : public AudioPlayback {
	public:
		AudioPlayback_24bit1ch(audio_playback_params_t *params);
		~AudioPlayback_24bit1ch(void);

	private:
		size_t AUDIOBUFFER_SIZE_SAMPLES = 0u;
		size_t AUDIOBUFFER_SIZE_BYTES = 0u;

		std::uint8_t *bytebuf = nullptr;

		bool audio_hw_init(void) override;
		void buffer_malloc(void) override;
		void buffer_free(void) override;

		void buffer_load(void) override;
};

#endif //AUDIOPLAYBACK_24BIT1CH_HPP

