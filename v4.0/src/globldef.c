/*
 * Some functions and definitions to ease development on GNU-Linux systems.
 * Version 2.1.2
 *
 * config.h is the macro settings file. Used to define behavior and variables.
 * globldef.h is the global definition file. It should be the first file included in all other source files.
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 *
 * GitHub Repository: https://github.com/RMSabe/GNU-Linux_Lib
 */

#include "globldef.h"

__attribute__((__aligned__(PTR_SIZE_BITS))) __tchar_t textbuf[TEXTBUF_SIZE_CHARS] = {'\0'};

uintptr_t _get_closest_power2_round(uintptr_t input)
{
	uintptr_t _numptr1;
	uintptr_t _numptr2;

	if(!input) return 0u;
	if(_is_power2(input)) return input;

	if(input > PTR_MSB_VALUE)
	{
		_numptr1 = input - PTR_MSB_VALUE;
		_numptr2 = PTR_MAX_VALUE - input;

		if(_numptr1 < _numptr2) return PTR_MSB_VALUE;
		return 0u;
	}

	_numptr2 = _get_closest_power2_ceil(input);
	_numptr1 = (_numptr2 >> 1);

	if((input - _numptr1) < (_numptr2 - input)) return _numptr1;

	return _numptr2;
}

uintptr_t _get_closest_power2_floor(uintptr_t input)
{
	uintptr_t _numptr;

	if(!input) return 0u;
	if(_is_power2(input)) return input;

	if(input > PTR_MSB_VALUE) return PTR_MSB_VALUE;

	_numptr = _get_closest_power2_ceil(input);
	_numptr = (_numptr >> 1);

	return _numptr;
}

uintptr_t _get_closest_power2_ceil(uintptr_t input)
{
	uintptr_t _numptr;

	if(!input) return 0u;
	if(_is_power2(input)) return input;

	_numptr = 1u;

	while(_numptr)
	{
		if(_numptr > input) break;
		_numptr = (_numptr << 1);
	}

	return _numptr;
}

bool _is_power2(uintptr_t input)
{
	uintptr_t _numptr;

	if(!input) return false;

	_numptr = 1u;

	while(_numptr)
	{
		if(_numptr == input) return true;
		_numptr = (_numptr << 1);
	}

	return false;
}

