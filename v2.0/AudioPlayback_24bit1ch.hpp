/*
 * WAVE audio file playback app v2.0 for GNU-Linux
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

		bool runPlayback(void) override;

	private:
		size_t AUDIOBUFFER_SIZE_SAMPLES = 0u;
		size_t AUDIOBUFFER_SIZE_BYTES = 0u;

		std::uint8_t *bytebuf = nullptr;
		std::int32_t *bufferout_0 = nullptr;
		std::int32_t *bufferout_1 = nullptr;

		std::int32_t *loadout_buf = nullptr;
		std::int32_t *playout_buf = nullptr;

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

#endif //AUDIOPLAYBACK_24BIT1CH_HPP

