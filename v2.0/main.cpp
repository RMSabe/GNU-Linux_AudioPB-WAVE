/*
 * WAVE audio file playback app v2.0 for GNU-Linux
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "globaldef.h"
#include <iostream>
#include <string>

#include "AudioPlayback.hpp"
#include "AudioPlayback_16bit1ch.hpp"
#include "AudioPlayback_16bit2ch.hpp"
#include "AudioPlayback_24bit1ch.hpp"
#include "AudioPlayback_24bit2ch.hpp"

#define PB_16BIT1CH 1
#define PB_16BIT2CH 2
#define PB_24BIT1CH 3
#define PB_24BIT2CH 4

#define BYTEBUF_SIZE 4096U

AudioPlayback *pb_obj = nullptr;
audio_playback_params_t audio_params;

int fd = -1;
__offset fsize = 0;

bool file_ext_check(void);

bool file_open(void);
void file_close(void);

int file_get_params(void);
bool compare_signature(const char *auth, const char *bytebuf, size_t offset);

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		std::cout << "Error: missing arguments\nThis executable requires two arguments: <Audio Device> <Audio File Directory>\nThey must be in this order\n";
		return 0;
	}

	audio_params.audio_dev_desc = argv[1];
	audio_params.filein_dir = argv[2];

	if(!file_ext_check())
	{
		std::cout << "Error: file format is not supported\n";
		return 1;
	}

	if(!file_open())
	{
		std::cout << "Error: could not open audio file\n";
		return 1;
	}

	int n_ret = file_get_params();

	if(n_ret < 0)
	{
		std::cout << "Error: audio format not supported\n";
		return 1;
	}

	switch(n_ret)
	{
		case PB_16BIT1CH:
			pb_obj = new AudioPlayback_16bit1ch(&audio_params);
			break;

		case PB_16BIT2CH:
			pb_obj = new AudioPlayback_16bit2ch(&audio_params);
			break;

		case PB_24BIT1CH:
			pb_obj = new AudioPlayback_24bit1ch(&audio_params);
			break;

		case PB_24BIT2CH:
			pb_obj = new AudioPlayback_24bit2ch(&audio_params);
			break;
	}

	if(!pb_obj->runPlayback())
	{
		std::cout << "Error: " << pb_obj->getLastErrorMessage() << std::endl;
		delete pb_obj;
		return 1;
	}

	delete pb_obj;
	return 0;
}

bool file_ext_check(void)
{
	if(audio_params.filein_dir == nullptr) return false;

	size_t len = 0u;
	while(audio_params.filein_dir[len] != '\0') len++;

	if(len < 5u) return false;

	if(compare_signature(".wav", audio_params.filein_dir, (len - 4u))) return true;
	if(compare_signature(".WAV", audio_params.filein_dir, (len - 4u))) return true;

	return false;
}

bool file_open(void)
{
	if(audio_params.filein_dir == nullptr) return false;

	fd = open(audio_params.filein_dir, O_RDONLY);
	if(fd < 0) return false;

	fsize = __LSEEK(fd, 0, SEEK_END);
	return true;
}

void file_close(void)
{
	if(fd < 0) return;

	close(fd);
	fd = -1;
	fsize = 0;
	return;
}

int file_get_params(void)
{
	char *header_info = (char*) std::malloc(BYTEBUF_SIZE);
	std::uint16_t *pu16 = NULL;
	std::uint32_t *pu32 = NULL;

	size_t bytepos = 0u;

	std::uint16_t n_channels = 0u;
	std::uint32_t bit_depth = 0u;

	__LSEEK(fd, 0, SEEK_SET);
	read(fd, header_info, BYTEBUF_SIZE);
	file_close();

	//Error Check: Invalid Chunk Signature
	if(!compare_signature("RIFF", header_info, 0u)) return -1;

	//Error Check: Invalid Format Signature
	if(!compare_signature("WAVE", header_info, 8u)) return -1;

	bytepos = 12u;

	//Fetch "fmt " Subchunk
	while(!compare_signature("fmt ", header_info, bytepos))
	{
		//Error: subchunk "fmt " not found
		if(bytepos > (BYTEBUF_SIZE - 256u))
		{
			std::free(header_info);
			return -1;
		}

		pu32 = (std::uint32_t*) &header_info[bytepos + 4u];
		bytepos += (size_t) (*pu32 + 8u);
	}

	pu16 = (std::uint16_t*) &header_info[bytepos + 8u];

	//Error Check: Encoding Format Not Supported
	if(pu16[0] != 1u)
	{
		std::free(header_info);
		return -1;
	}

	n_channels = pu16[1];

	pu32 = (std::uint32_t*) &header_info[bytepos + 12u];
	audio_params.sample_rate = *pu32;

	pu16 = (std::uint16_t*) &header_info[bytepos + 22u];
	bit_depth = *pu16;

	pu32 = (std::uint32_t*) &header_info[bytepos + 4u];
	bytepos += (size_t) (*pu32 + 8u);

	//Fetch "data" Subchunk
	while(!compare_signature("data", header_info, bytepos))
	{
		//Error: subchunk "data" not found
		if(bytepos > (BYTEBUF_SIZE - 256u))
		{
			std::free(header_info);
			return -1;
		}

		pu32 = (std::uint32_t*) &header_info[bytepos + 4u];
		bytepos += (size_t) (*pu32 + 8u);
	}

	pu32 = (std::uint32_t*) &header_info[bytepos + 4u];

	audio_params.audio_data_begin = (__offset) (bytepos + 8u);
	audio_params.audio_data_end = audio_params.audio_data_begin + ((__offset) *pu32);

	std::free(header_info);

	if((bit_depth == 16u) && (n_channels == 1u)) return PB_16BIT1CH;
	if((bit_depth == 16u) && (n_channels == 2u)) return PB_16BIT2CH;
	if((bit_depth == 24u) && (n_channels == 1u)) return PB_24BIT1CH;
	if((bit_depth == 24u) && (n_channels == 2u)) return PB_24BIT2CH;

	return -1;
}

bool compare_signature(const char *auth, const char *bytebuf, size_t offset)
{
	if(auth == nullptr) return false;
	if(bytebuf == nullptr) return false;

	if(auth[0] != bytebuf[offset]) return false;
	if(auth[1] != bytebuf[offset + 1u]) return false;
	if(auth[2] != bytebuf[offset + 2u]) return false;
	if(auth[3] != bytebuf[offset + 3u]) return false;

	return true;
}

