/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef AUDIOPB_I16_HPP
#define AUDIOPB_I16_HPP

#include "AudioPB.hpp"

class AudioPB_i16 : public AudioPB {
	public:
		AudioPB_i16(const audiopb_params_t *p_params);
		~AudioPB_i16(void) override;

	private:
		bool buffer_alloc(void) override;
		void buffer_free(void) override;
		void buffer_load(void) override;
};

#endif /*AUDIOPB_I16_HPP*/

