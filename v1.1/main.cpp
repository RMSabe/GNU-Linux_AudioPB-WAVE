/*
 * WAVE file audio playback
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include <fstream>
#include <iostream>
#include <string>

#define PB_16BIT2CH 0
#define PB_24BIT2CH 1
#define PB_16BIT1CH 2
#define PB_24BIT1CH 3

#define BYTEBUF_SIZE 2048U

std::fstream audio_file;
char *audio_dev = NULL;
char *file_dir = NULL;

unsigned int audio_file_size = 0u;
unsigned int audio_data_begin = 0u;
unsigned int audio_data_end = 0u;
unsigned int sample_rate = 0u;

bool open_audio_file(void);
bool check_file_ext(void);

int audio_file_get_params(void);
bool compare_signature(const char *auth, const char *bytebuf, unsigned int offset);

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		std::cout << "Error: missing arguments\nThis executable requires two arguments: <Audio Device> <Audio File Directory>\nThey must be in this order\n";
		return 0;
	}

	audio_dev = argv[1];
	file_dir = argv[2];

	if(!check_file_ext())
	{
		std::cout << "Error: incompatible file type\n";
		return 0;
	}

	if(!open_audio_file())
	{
		std::cout << "Error opening audio file\nError code: " << errno << std::endl;
		return 0;
	}

	int n_return = audio_file_get_params();
	audio_file.close();

	if(n_return < 0)
	{
		std::cout << "Error: audio format not supported\n";
		return 0;
	}

	std::string cmd_line = "";
	switch(n_return)
	{
		case PB_16BIT2CH:
			cmd_line = "./pb_16bit2ch.elf ";
			break;

		case PB_24BIT2CH:
			cmd_line = "./pb_24bit2ch.elf ";
			break;

		case PB_16BIT1CH:
			cmd_line = "./pb_16bit1ch.elf ";
			break;

		case PB_24BIT1CH:
			cmd_line = "./pb_24bit1ch.elf ";
			break;
	}

	cmd_line += file_dir;
	cmd_line += ' ';
	cmd_line += audio_dev;
	cmd_line += ' ';
	cmd_line += std::to_string(audio_data_begin);
	cmd_line += ' ';
	cmd_line += std::to_string(audio_data_end);
	cmd_line += ' ';
	cmd_line += std::to_string(sample_rate);

	system(cmd_line.c_str());
	return 0;
}

bool open_audio_file(void)
{
	audio_file.open(file_dir, std::ios_base::in);
	if(!audio_file.is_open()) return false;

	audio_file.seekg(0, audio_file.end);
	audio_file_size = audio_file.tellg();
	return true;
}

bool check_file_ext(void)
{
	unsigned int len = 0u;
	while(file_dir[len] != '\0') len++;

	if(len < 5u) return false;

	if(compare_signature(".wav", file_dir, (len - 4u))) return true;
	if(compare_signature(".WAV", file_dir, (len - 4u))) return true;

	return false;
}

int audio_file_get_params(void)
{
	bool stereo = false;
	unsigned short bit_depth = 0u;

	char *header_info = (char*) malloc(BYTEBUF_SIZE);
	unsigned short *pushort = NULL;
	unsigned int *puint = NULL;

	audio_file.seekg(0);
	audio_file.read(header_info, BYTEBUF_SIZE);
	unsigned int byte_pos = 0u;

	//Error Check: Invalid Chunk Signature
	if(!compare_signature("RIFF", header_info, byte_pos)) return -1;

	//Error Check: Invalid Format Signature
	if(!compare_signature("WAVE", header_info, (byte_pos + 8u))) return -1;

	byte_pos += 12u;

	//Fetch "fmt " Subchunk
	while(!compare_signature("fmt ", header_info, byte_pos))
	{
		if(byte_pos > (BYTEBUF_SIZE - 256u)) return -1; //Error: Subchunk not found

		puint = (unsigned int*) &header_info[byte_pos + 4u];
		byte_pos += (puint[0] + 8u);
	}

	//Error Check: Encoding Type Not Supported
	pushort = (unsigned short*) &header_info[byte_pos + 8u];
	if(pushort[0] != 1u) return -1;

	//Error Check: Number of Channels Not Supported
	if(pushort[1] > 2u) return -1;

	stereo = (pushort[1] == 2u);

	puint = (unsigned int*) &header_info[byte_pos + 12u];
	sample_rate = puint[0];

	pushort = (unsigned short*) &header_info[byte_pos + 20u];
	bit_depth = pushort[1];

	puint = (unsigned int*) &header_info[byte_pos + 4u];
	byte_pos += (puint[0] + 8u);

	//Fetch "data" Subchunk
	while(!compare_signature("data", header_info, byte_pos))
	{
		if(byte_pos > (BYTEBUF_SIZE - 256u)) return -1; //Error: Subchunk not found

		puint = (unsigned int*) &header_info[byte_pos + 4u];
		byte_pos += (puint[0] + 8u);
	}

	puint = (unsigned int*) &header_info[byte_pos + 4u];
	audio_data_begin = (byte_pos + 8u);
	audio_data_end = (audio_data_begin + puint[0]);

	free(header_info);

	if(stereo && (bit_depth == 16u)) return PB_16BIT2CH;
	if(stereo && (bit_depth == 24u)) return PB_24BIT2CH;
	if(bit_depth == 16u) return PB_16BIT1CH;
	if(bit_depth == 24u) return PB_24BIT1CH;
	return -1;
}

bool compare_signature(const char *auth, const char *bytebuf, unsigned int offset)
{
	if(auth[0] != bytebuf[offset]) return false;
	if(auth[1] != bytebuf[offset + 1u]) return false;
	if(auth[2] != bytebuf[offset + 2u]) return false;
	if(auth[3] != bytebuf[offset + 3u]) return false;

	return true;
}

