/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0
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
		~AudioPB_i24(void) override;

	private:
		__attribute__((__aligned__(PTR_SIZE_BITS))) size_t INPUTBUFFER_SIZE_BYTES = 0u;
		__attribute__((__aligned__(PTR_SIZE_BITS))) uint8_t *p_inputbuffer = NULL;

		bool buffer_alloc(void) override;
		void buffer_free(void) override;
		void buffer_load(void) override;
};

#endif /*AUDIOPB_I24_HPP*/

