/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 3.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPB_I24_HPP
#define AUDIOPB_I24_HPP

#include "AudioPB.hpp"

class AudioPB_i24 : public AudioPB {
	public:
		AudioPB_i24(const audiopb_params_t *p_params);
		~AudioPB_i24(void);

	private:
		size_t BYTEBUF_SIZE = 0u;

		uint8_t *p_bytebuf = NULL;

		bool audio_hw_init(void) override;
		bool buffer_alloc(void) override;
		void buffer_free(void) override;
		void buffer_load(void) override;
};

#endif /*AUDIOPB_I24_HPP*/

