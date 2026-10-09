/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "globldef.h"
#include "filedef.h"
#include "delay.h"
#include "cstrdef.h"
#include "strdef.hpp"
#include "cppthread.hpp"

#include "shared.h"

#include <stdlib.h>
#include <string.h>
#include <poll.h>

#include <iostream>

#ifdef __TEXTFORMAT_USE_WCHAR
#define __STDCIN__ std::wcin
#define __STDCOUT__ std::wcout
#else
#define __STDCIN__ std::cin
#define __STDCOUT__ std::cout
#endif

#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define __PB_I16 1
#define __PB_I24 2

static __attribute__((__aligned__(PTR_SIZE_BITS))) AudioPB *p_audio = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) audiopb_params_t pb_params;
static __attribute__((__aligned__(PTR_SIZE_BITS))) const char *audio_dev_desc = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) std::thread audiothread;
static __attribute__((__aligned__(PTR_SIZE_BITS))) __string usercmd = __TEXT("");
static __attribute__((__aligned__(32))) int h_filein = -1;

static void app_deinit(void);
static void runtime_loop(void);

static bool filein_open(void);
static void filein_close(void);

static int filein_get_params(void);
static bool compare_signature(const char *auth, const uint8_t *buf);

static void cmdui_cmd_decode(void);
static void cmdui_print_help_text(void);
static __offset_t cmdui_parse_position(const __tchar_t *text_pos);

static void audiothread_proc(void);

int main(int argc, char **argv)
{
	int _nret = 0;

	if(argc < 3)
	{
		__STDCOUT__ << __TEXT("Error: missing arguments\nThis executable requires 2 arguments: <playback device id> <input file directory>\nThey must be in that order\n");
		return -1;
	}

	audio_dev_desc = argv[1];
	pb_params.file_dir = argv[2];

	if(!filein_open())
	{
		__STDCOUT__ << __TEXT("Error: failed to open file\n");
		goto _l_main_error;
	}

	_nret = filein_get_params();
	if(_nret < 0) goto _l_main_error;

	pb_params.audiobuffer_size_frames = _get_closest_power2_ceil(pb_params.sample_rate);
	pb_params.streambuffer_segment_size_frames = pb_params.audiobuffer_size_frames/4u;
	pb_params.streambuffer_n_segments = 2u;

	switch(_nret)
	{
		case __PB_I16:
			p_audio = new AudioPB_i16(&pb_params);
			break;

		case __PB_I24:
			p_audio = new AudioPB_i24(&pb_params);
			break;
	}

	if(p_audio == NULL)
	{
		__STDCOUT__ << __TEXT("Error: audio object instance failed.\n");
		goto _l_main_error;
	}

	p_audio->chooseDevice(audio_dev_desc, true);

	if(!p_audio->initialize())
	{
		__STDCOUT__ << p_audio->getLastErrorMessage() << std::endl;
		goto _l_main_error;
	}

	audiothread = std::thread(&audiothread_proc);

	runtime_loop();

	app_deinit();
	return 0;

_l_main_error:

	app_deinit();
	return -1;
}

static void app_deinit(void)
{
	cppthread_stop(&audiothread);

	filein_close();

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	return;
}

static void runtime_loop(void)
{
	int _nret;
	int _audiostatus;
	struct pollfd _poll_userinput;

	memset(&_poll_userinput, 0, sizeof(struct pollfd));

	_poll_userinput.fd = STDIN_FILENO;
	_poll_userinput.events = POLLIN;

	__STDCOUT__ << __TEXT("Playback Started\n");
	cmdui_print_help_text();

	while(true)
	{
		_audiostatus = p_audio->getStatus();
		if((_audiostatus < AudioPB::STATUS_READY) || (_audiostatus == AudioPB::STATUS_STOPPED)) break;

		_nret = poll(&_poll_userinput, 1, 1);
		if(_nret > 0)
		{
			usercmd = __TEXT("");
			__STDCIN__ >> usercmd;
			cmdui_cmd_decode();
		}
	}

	cppthread_wait(&audiothread);

	delay_ms(1024);
	__STDCOUT__ << __TEXT("Playback Finished\n");

	return;
}

static bool filein_open(void)
{
	if(pb_params.file_dir == NULL) return false;

	filein_close();

	h_filein = open(pb_params.file_dir, O_RDONLY);

	return (h_filein >= 0);
}

static void filein_close(void)
{
	if(h_filein < 0) return;

	close(h_filein);
	h_filein = -1;
	return;
}

static int filein_get_params(void)
{
	const uintptr_t _BUFFERSIZE = 8192u;
	uintptr_t _bufferindex;
	uint8_t *_p_headerinfo = NULL;

	uint32_t _u32;
	uint16_t _u16;

	uint16_t _bitdepth;

	_p_headerinfo = (uint8_t*) malloc(_BUFFERSIZE);
	if(_p_headerinfo == NULL)
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: memory allocate failed.\n");
		goto _l_filein_get_params_error;
	}

	memset(_p_headerinfo, 0, _BUFFERSIZE);

	__LSEEK(h_filein, 0, SEEK_SET);
	read(h_filein, _p_headerinfo, _BUFFERSIZE);
	filein_close();

	if(!compare_signature("RIFF", _p_headerinfo))
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: file format not supported.\n");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const uint8_t*) (((uintptr_t) _p_headerinfo) + 8u)))
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: file format not supported.\n");
		goto _l_filein_get_params_error;
	}

	_bufferindex = 12u;

	while(true)
	{
		if(_bufferindex > (_BUFFERSIZE - 8u))
		{
			__STDCOUT__ << __TEXT("filein_get_params: Error: broken file header.\n");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const uint8_t*) (((uintptr_t) _p_headerinfo) + _bufferindex))) break;

		_u32 = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _bufferindex + 4u));
		_bufferindex += (uintptr_t) (_u32 + 8u);
	}

	if(_bufferindex > (_BUFFERSIZE - 24u))
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: broken file header.\n");
		goto _l_filein_get_params_error;
	}

	_u16 = *((uint16_t*) (((uintptr_t) _p_headerinfo) + _bufferindex + 8u));
	if(_u16 != 1u)
	{
		__STDCOUT__ << __TEXT("filein_get_params: Error: audio encoding format not supported.\n");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = *((uint16_t*) (((uintptr_t) _p_headerinfo) + _bufferindex + 10u));
	pb_params.sample_rate = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _bufferindex + 12u));
	_bitdepth = *((uint16_t*) (((uintptr_t) _p_headerinfo) + _bufferindex + 22u));

	_u32 = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _bufferindex + 4u));
	_bufferindex += (uintptr_t) (_u32 + 8u);

	while(true)
	{
		if(_bufferindex > (_BUFFERSIZE - 8u))
		{
			__STDCOUT__ << __TEXT("filein_get_params: Error: broken file header.\n");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const uint8_t*) (((uintptr_t) _p_headerinfo) + _bufferindex))) break;

		_u32 = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _bufferindex + 4u));
		_bufferindex += (uintptr_t) (_u32 + 8u);
	}

	_u32 = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _bufferindex + 4u));

	pb_params.audio_data_begin = (__offset_t) (_bufferindex + 8u);
	pb_params.audio_data_end = pb_params.audio_data_begin + ((__offset_t) _u32);

	free(_p_headerinfo);
	_p_headerinfo = NULL;

	switch(_bitdepth)
	{
		case 16u:
			return __PB_I16;

		case 24u:
			return __PB_I24;
	}

	__STDCOUT__ << __TEXT("filein_get_params: Error: audio format not supported.\n");

_l_filein_get_params_error:

	filein_close();
	if(_p_headerinfo != NULL) free(_p_headerinfo);
	return -1;
}

static bool compare_signature(const char *auth, const uint8_t *buf)
{
	size_t _nchar;

	if(auth == NULL) return false;
	if(buf == NULL) return false;

	for(_nchar = 0u; _nchar < 4u; _nchar++) if(auth[_nchar] != ((char) buf[_nchar])) return false;

	return true;
}

static void cmdui_cmd_decode(void)
{
	const __tchar_t *_cmd = NULL;
	__offset_t _n_off;

	usercmd = str_tolower(usercmd);
	_cmd = usercmd.c_str();

	if(cstr_compare(__TEXT("stop"), _cmd))
	{
		p_audio->stopPlayback();
		return;
	}

	if(cstr_compare(__TEXT("pause"), _cmd))
	{
		p_audio->pausePlayback();
		__STDCOUT__ << __TEXT("Playback Paused\n");
		return;
	}

	if(cstr_compare(__TEXT("play"), _cmd))
	{
		p_audio->resumePlayback();
		__STDCOUT__ << __TEXT("Playback Resumed\n");
		return;
	}

	if(cstr_compare(__TEXT("help"), _cmd) || cstr_compare(__TEXT("--help"), _cmd))
	{
		cmdui_print_help_text();
		return;
	}

	if(cstr_compare(__TEXT("getsize"), _cmd))
	{
		_n_off = p_audio->getAudioDataSizeFrames();
		if(_n_off < 0) __STDCOUT__ << p_audio->getLastErrorMessage() << std::endl;

		__STDCOUT__ << __TEXT("Data size: ") << __TOSTRING(_n_off) << __TEXT(" frames\n");
		return;
	}

	if(cstr_compare(__TEXT("getpos"), _cmd))
	{
		_n_off = p_audio->getAudioDataPositionFrames();
		if(_n_off < 0) __STDCOUT__ << p_audio->getLastErrorMessage() << std::endl;

		__STDCOUT__ << __TEXT("Current data position: ") << __TOSTRING(_n_off) << __TEXT(" frames\n");
		return;
	}

	if(cstr_compare_upto_char(__TEXT("setpos"), _cmd, ':', true))
	{
		_n_off = cmdui_parse_position(&_cmd[7]);
		if(_n_off < 0) return;

		if(p_audio->setAudioDataPositionFrames(_n_off)) __STDCOUT__ << __TEXT("Parameter updated\n");
		else __STDCOUT__ << p_audio->getLastErrorMessage() << std::endl;

		return;
	}

	__STDCOUT__ << __TEXT("Error: invalid command entered\n");
	return;
}

static void cmdui_print_help_text(void)
{
	__STDCOUT__ << __TEXT("User Command List:\n\n");
	__STDCOUT__ << __TEXT("\"help\" or \"--help\" : print this list\n");
	__STDCOUT__ << __TEXT("\"stop\" : stop playback and quit application\n");
	__STDCOUT__ << __TEXT("\"pause\" : pause playback (when playing)\n");
	__STDCOUT__ << __TEXT("\"play\" : resume playback (when paused)\n");
	__STDCOUT__ << __TEXT("\"getsize\" : print the size of audio data (in number of frames)\n");
	__STDCOUT__ << __TEXT("\"getpos\" : print the current position of audio data (in number of frames)\n");
	__STDCOUT__ << __TEXT("\"setpos:<number>\" : set the current position of audio data (in number of frames)\n\n");

	return;
}

static __offset_t cmdui_parse_position(const __tchar_t *text_pos)
{
	__offset_t _n_pos;

	if(text_pos == NULL) return -1;

	try { _n_pos = (__offset_t) std::stoll(text_pos); }
	catch(...)
	{
		__STDCOUT__ << __TEXT("Error: invalid position value entered\n");
		return -1;
	}

	if(_n_pos < 0)
	{
		__STDCOUT__ << __TEXT("Error: invalid position value entered\n");
		return -1;
	}

	return _n_pos;
}

static void audiothread_proc(void)
{
	p_audio->runPlayback();
	return;
}

void __attribute__((__noreturn__)) app_exit(int exit_code, const __tchar_t *exit_msg)
{
	app_deinit();

	if(exit_msg != NULL) __STDCOUT__ << __TEXT("PROCESS EXIT CALLED\n") << exit_msg << std::endl;

	exit(exit_code);

	while(true) delay_ms(10);
}

