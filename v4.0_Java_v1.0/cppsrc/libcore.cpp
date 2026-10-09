/*
 * WAVE Audio Playback for GNU-Linux systems.
 * Version 4.0 (Interop Java version 1.0)
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#include "globldef.h"
#include "filedef.h"
#include "delay.h"
#include "cstrdef.h"
#include "strdef.hpp"

#include "shared.h"

#include <stdlib.h>
#include <string.h>

#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#include <jni.h>

#define __PB_I16 1
#define __PB_I24 2

static __attribute__((__aligned__(PTR_SIZE_BITS))) AudioPB *p_audio = NULL;
static __attribute__((__aligned__(PTR_SIZE_BITS))) audiopb_params_t pb_params;
static __attribute__((__aligned__(PTR_SIZE_BITS))) std::string filein_dir = "";
static __attribute__((__aligned__(PTR_SIZE_BITS))) __string err_msg = __TEXT("");
static __attribute__((__aligned__(32))) int h_filein = -1;

#ifdef __TEXTFORMAT_USE_WCHAR
#if __SIZEOF_WCHAR_T__ == 4
static __attribute__((__aligned__(PTR_SIZE_BITS))) uint16_t jchar_textbuf[TEXTBUF_SIZE_CHARS];
#endif
#endif

__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_initialize(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT void JNICALL Java_Core_deinitialize(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jstring JNICALL Java_Core_getLastErrorMessage(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_setFileInDirectory(JNIEnv *p_jnienv, jclass jcls, jstring fileDirectory);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_loadFile_1createAudioObject(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_loadAudioDeviceList(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jint JNICALL Java_Core_getAudioDeviceListEntryCount(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jstring JNICALL Java_Core_getAudioDeviceListEntry_1friendlyName(JNIEnv *p_jnienv, jclass jcls, jint index);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_chooseDevice(JNIEnv *p_jnienv, jclass jcls, jint index, jboolean enableResampling);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_chooseDefaultDevice(JNIEnv *p_jnienv, jclass jcls, jboolean enableResampling);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_initializeAudioObject(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jint JNICALL Java_Core_getStatus(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_runPlayback(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT void JNICALL Java_Core_pausePlayback(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT void JNICALL Java_Core_resumePlayback(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT void JNICALL Java_Core_stopPlayback(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jlong JNICALL Java_Core_getAudioDataSizeFrames(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jlong JNICALL Java_Core_getAudioDataPositionFrames(JNIEnv *p_jnienv, jclass jcls);
__EXTERNC__ JNIEXPORT jboolean JNICALL Java_Core_setAudioDataPositionFrames(JNIEnv *p_jnienv, jclass jcls, jlong position);

static bool core_init(void);
static void core_deinit(void);
static bool filein_open(void);
static void filein_close(void);
static int filein_get_params(void);
static bool compare_signature(const char *auth, const uint8_t *buf);
static jstring tstr_to_jstr(JNIEnv *p_jnienv, const __tchar_t *tstr);
static __string jstr_to_tstr(JNIEnv *p_jnienv, jstring jstr);

JNIEXPORT jboolean JNICALL Java_Core_initialize(JNIEnv *p_jnienv, jclass jcls)
{
	return (jboolean) core_init();
}

JNIEXPORT void JNICALL Java_Core_deinitialize(JNIEnv *p_jnienv, jclass jcls)
{
	core_deinit();
	return;
}

JNIEXPORT jstring JNICALL Java_Core_getLastErrorMessage(JNIEnv *p_jnienv, jclass jcls)
{
	return tstr_to_jstr(p_jnienv, err_msg.c_str());
}

JNIEXPORT jboolean JNICALL Java_Core_setFileInDirectory(JNIEnv *p_jnienv, jclass jcls, jstring fileDirectory)
{
	const char *_fdir = NULL;
	jboolean _jboolean_ret;

	_fdir = p_jnienv->GetStringUTFChars(fileDirectory, NULL);

	if(_fdir == NULL) _jboolean_ret = JNI_FALSE;
	else
	{
		filein_dir = _fdir;
		_jboolean_ret = JNI_TRUE;
	}

	p_jnienv->ReleaseStringUTFChars(fileDirectory, _fdir);

	return _jboolean_ret;
}

JNIEXPORT jboolean JNICALL Java_Core_loadFile_1createAudioObject(JNIEnv *p_jnienv, jclass jcls)
{
	int _n_ret;

	if(!filein_open())
	{
		err_msg = __TEXT("Error: could not open input file.");
		return JNI_FALSE;
	}

	_n_ret = filein_get_params();
	if(_n_ret < 0) return JNI_FALSE;

	pb_params.file_dir = filein_dir.c_str();
	pb_params.audiobuffer_size_frames = _get_closest_power2_ceil((uintptr_t) pb_params.sample_rate);
	pb_params.streambuffer_segment_size_frames = pb_params.audiobuffer_size_frames/4u;
	pb_params.streambuffer_n_segments = 2u;

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	switch(_n_ret)
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
		err_msg = __TEXT("Error: audio object instance failed.");
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_Core_loadAudioDeviceList(JNIEnv *p_jnienv, jclass jcls)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return JNI_FALSE;
	}

	if(!p_audio->loadAudioDeviceList())
	{
		err_msg = p_audio->getLastErrorMessage();
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

JNIEXPORT jint JNICALL Java_Core_getAudioDeviceListEntryCount(JNIEnv *p_jnienv, jclass jcls)
{
	ssize_t _ssize;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return -1;
	}

	_ssize = p_audio->getAudioDeviceListEntryCount();
	if(_ssize < 0) err_msg = p_audio->getLastErrorMessage();

	return (jint) _ssize;
}

JNIEXPORT jstring JNICALL Java_Core_getAudioDeviceListEntry_1friendlyName(JNIEnv *p_jnienv, jclass jcls, jint index)
{
	const audiodevicelist_entry_t *_p_entry = NULL;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return p_jnienv->NewStringUTF("");
	}

	if(index < 0)
	{
		err_msg = __TEXT("Error: invalid index value.");
		return p_jnienv->NewStringUTF("");
	}

	_p_entry = p_audio->getAudioDeviceListEntry((size_t) index);
	if(_p_entry == NULL)
	{
		err_msg = p_audio->getLastErrorMessage();
		return p_jnienv->NewStringUTF("");
	}

	return p_jnienv->NewStringUTF(_p_entry->desc);
}

JNIEXPORT jboolean JNICALL Java_Core_chooseDevice(JNIEnv *p_jnienv, jclass jcls, jint index, jboolean enableResampling)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return JNI_FALSE;
	}

	if(index < 0)
	{
		err_msg = __TEXT("Error: invalid index value.");
		return JNI_FALSE;
	}

	if(!p_audio->chooseDevice((size_t) index, (bool) enableResampling))
	{
		err_msg = p_audio->getLastErrorMessage();
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_Core_chooseDefaultDevice(JNIEnv *p_jnienv, jclass jcls, jboolean enableResampling)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return JNI_FALSE;
	}

	if(!p_audio->chooseDefaultDevice((bool) enableResampling))
	{
		err_msg = p_audio->getLastErrorMessage();
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_Core_initializeAudioObject(JNIEnv *p_jnienv, jclass jcls)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return JNI_FALSE;
	}

	if(!p_audio->initialize())
	{
		err_msg = p_audio->getLastErrorMessage();
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

JNIEXPORT jint JNICALL Java_Core_getStatus(JNIEnv *p_jnienv, jclass jcls)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return -1;
	}

	return (jint) p_audio->getStatus();
}

JNIEXPORT jboolean JNICALL Java_Core_runPlayback(JNIEnv *p_jnienv, jclass jcls)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return JNI_FALSE;
	}

	if(!p_audio->runPlayback())
	{
		err_msg = p_audio->getLastErrorMessage();
		return JNI_FALSE;
	}

	delete p_audio;
	p_audio = NULL;

	return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_Core_pausePlayback(JNIEnv *p_jnienv, jclass jcls)
{
	if(p_audio != NULL) p_audio->pausePlayback();
	return;
}

JNIEXPORT void JNICALL Java_Core_resumePlayback(JNIEnv *p_jnienv, jclass jcls)
{
	if(p_audio != NULL) p_audio->resumePlayback();
	return;
}

JNIEXPORT void JNICALL Java_Core_stopPlayback(JNIEnv *p_jnienv, jclass jcls)
{
	if(p_audio != NULL) p_audio->stopPlayback();
	return;
}

JNIEXPORT jlong JNICALL Java_Core_getAudioDataSizeFrames(JNIEnv *p_jnienv, jclass jcls)
{
	__offset_t _n_off;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return -1;
	}

	_n_off = p_audio->getAudioDataSizeFrames();
	if(_n_off < 0) err_msg = p_audio->getLastErrorMessage();

	return (jlong) _n_off;
}

JNIEXPORT jlong JNICALL Java_Core_getAudioDataPositionFrames(JNIEnv *p_jnienv, jclass jcls)
{
	__offset_t _n_off;

	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return -1;
	}

	_n_off = p_audio->getAudioDataPositionFrames();
	if(_n_off < 0) err_msg = p_audio->getLastErrorMessage();

	return (jlong) _n_off;
}

JNIEXPORT jboolean JNICALL Java_Core_setAudioDataPositionFrames(JNIEnv *p_jnienv, jclass jcls, jlong position)
{
	if(p_audio == NULL)
	{
		err_msg = __TEXT("Error: no audio object instance.");
		return JNI_FALSE;
	}

	if(!p_audio->setAudioDataPositionFrames((__offset_t) position))
	{
		err_msg = p_audio->getLastErrorMessage();
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

static bool core_init(void)
{
	/*No initialization required for this version*/
	return true;
}

static void core_deinit(void)
{
	filein_close();

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	return;
}

static bool filein_open(void)
{
	filein_close();

	h_filein = open(filein_dir.c_str(), O_RDONLY);
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
	const uintptr_t _BUFFER_SIZE = 8192u;
	uint8_t *_p_headerinfo = NULL;

	uintptr_t _buffer_index;

	uint32_t _u32;
	uint16_t _u16;

	uint16_t _bit_depth;

	_p_headerinfo = (uint8_t*) malloc(_BUFFER_SIZE);
	if(_p_headerinfo == NULL)
	{
		err_msg = __TEXT("filein_get_params: Error: memory allocate failed.");
		goto _l_filein_get_params_error;
	}

	memset(_p_headerinfo, 0, _BUFFER_SIZE);

	__LSEEK(h_filein, 0, SEEK_SET);
	read(h_filein, _p_headerinfo, _BUFFER_SIZE);
	filein_close();

	if(!compare_signature("RIFF", _p_headerinfo))
	{
		err_msg = __TEXT("filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const uint8_t*) (((uintptr_t) _p_headerinfo) + 8u)))
	{
		err_msg = __TEXT("filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	_buffer_index = 12u;

	while(true)
	{
		if(_buffer_index > (_BUFFER_SIZE - 8u))
		{
			err_msg = __TEXT("filein_get_params: Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const uint8_t*) (((uintptr_t) _p_headerinfo) + _buffer_index))) break;

		_u32 = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _buffer_index + 4u));
		_buffer_index += (uintptr_t) (_u32 + 8u);
	}

	if(_buffer_index > (_BUFFER_SIZE - 24u))
	{
		err_msg = __TEXT("filein_get_params: Error: broken file header.");
		goto _l_filein_get_params_error;
	}

	_u16 = *((uint16_t*) (((uintptr_t) _p_headerinfo) + _buffer_index + 8u));
	if(_u16 != 1u)
	{
		err_msg = __TEXT("filein_get_params: Error: audio encoding format not supported.");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = *((uint16_t*) (((uintptr_t) _p_headerinfo) + _buffer_index + 10u));
	pb_params.sample_rate = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _buffer_index + 12u));

	_bit_depth = *((uint16_t*) (((uintptr_t) _p_headerinfo) + _buffer_index + 22u));

	_u32 = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _buffer_index + 4u));
	_buffer_index += (uintptr_t) (_u32 + 8u);

	while(true)
	{
		if(_buffer_index > (_BUFFER_SIZE - 8u))
		{
			err_msg = __TEXT("filein_get_params: Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const uint8_t*) (((uintptr_t) _p_headerinfo) + _buffer_index))) break;

		_u32 = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _buffer_index + 4u));
		_buffer_index += (uintptr_t) (_u32 + 8u);
	}

	_u32 = *((uint32_t*) (((uintptr_t) _p_headerinfo) + _buffer_index + 4u));

	pb_params.audio_data_begin = (__offset_t) (_buffer_index + 8u);
	pb_params.audio_data_end = pb_params.audio_data_begin + ((__offset_t) _u32);

	free(_p_headerinfo);
	_p_headerinfo = NULL;

	switch(_bit_depth)
	{
		case 16u:
			return __PB_I16;

		case 24u:
			return __PB_I24;
	}

	err_msg = __TEXT("filein_get_params: Error: audio format not supported.");

_l_filein_get_params_error:

	filein_close();
	if(_p_headerinfo != NULL) free(_p_headerinfo);
	return -1;
}

static bool compare_signature(const char *auth, const uint8_t *buf)
{
	size_t _index;

	if(auth == NULL) return false;
	if(buf == NULL) return false;

	for(_index = 0u; _index < 4u; _index++) if(auth[_index] != ((char) buf[_index])) return false;

	return true;
}

static jstring tstr_to_jstr(JNIEnv *p_jnienv, const __tchar_t *tstr)
{
	size_t _len;

	if(p_jnienv == NULL) return (jstring) NULL;

	if(tstr == NULL) return p_jnienv->NewStringUTF("");

#ifdef __TEXTFORMAT_USE_WCHAR

#if __SIZEOF_WCHAR_T__ == 4
	cstr_copy_text32_to_text16((const uint32_t*) tstr, jchar_textbuf, TEXTBUF_SIZE_CHARS);
	_len = 0u;
	while(jchar_textbuf[_len] != '\0') _len++;

	return p_jnienv->NewString((const jchar*) jchar_textbuf, (jsize) _len);
#endif

#if __SIZEOF_WCHAR_T__ == 2
	_len = (size_t) cstr_getlength(tstr);
	return p_jnienv->NewString((const jchar*) tstr, (jsize) _len);
#endif

#if __SIZEOF_WCHAR_T__ == 1
	return p_jnienv->NewStringUTF((const char*) tstr);
#endif

#else
	return p_jnienv->NewStringUTF((const char*) tstr);
#endif
}

static __string jstr_to_tstr(JNIEnv *p_jnienv, jstring jstr)
{
	const void *_strtext = NULL;
	ssize_t _len;
	ssize_t _nchar;
	__string _tstr;

	_tstr = __TEXT("");

	if(p_jnienv == NULL) return _tstr;
	if(jstr == NULL) return _tstr;

#ifdef __TEXTFORMAT_USE_WCHAR
	_len = (ssize_t) p_jnienv->GetStringLength(jstr);
	_strtext = (const void*) p_jnienv->GetStringChars(jstr, NULL);

	if((_strtext != NULL) && (_len > 0))
		for(_nchar = 0; _nchar < _len; _nchar++)
			_tstr += (__tchar_t) ((const jchar*) _strtext)[_nchar];

	p_jnienv->ReleaseStringChars(jstr, (const jchar*) _strtext);
#else
	_strtext = (const void*) p_jnienv->GetStringUTFChars(jstr, NULL);

	if(_strtext != NULL)
	{
		_nchar = 0;
		while(((const char*) _strtext)[_nchar] != '\0')
		{
			_tstr += (__tchar_t) ((const char*) _strtext)[_nchar];
			_nchar++;
		}
	}

	p_jnienv->ReleaseStringUTFChars(jstr, (const char*) _strtext);
#endif

	return _tstr;
}

void __attribute__((__noreturn__)) app_exit(int exit_code, const __tchar_t *exit_msg)
{
	core_deinit();

	exit(exit_code);

	while(true) delay_ms(16);
}

