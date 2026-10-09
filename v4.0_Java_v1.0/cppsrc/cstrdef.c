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

#include "cstrdef.h"
#include <string.h>

ssize_t cstr_getlength(const __tchar_t *str)
{
	size_t len;

	if(str == NULL) return -1;

	len = 0u;
	while(str[len] != '\0') len++;

	return (ssize_t) len;
}

ssize_t cstr_locatechar(const __tchar_t *str, __tchar_t c)
{
	size_t len;
	size_t n_char;

	if(str == NULL) return -1;

	len = (size_t) cstr_getlength(str);

	n_char = 0u;
	while(n_char < len)
	{
		if(str[n_char] == c) return (ssize_t) n_char;
		n_char++;
	}

	return -1;
}

bool cstr_compare(const __tchar_t *str1, const __tchar_t *str2)
{
	size_t num1;
	size_t num2;

	if(str1 == NULL) return false;
	if(str2 == NULL) return false;

	num1 = (size_t) cstr_getlength(str1);
	num2 = (size_t) cstr_getlength(str2);

	if(num1 != num2) return false;

	num1 = 0u;
	while(num1 < num2)
	{
		if(str1[num1] != str2[num1]) return false;
		num1++;
	}

	return true;
}

bool cstr_compare_upto_len(const __tchar_t *str1, const __tchar_t *str2, size_t stop_index, bool fail_if_nolen)
{
	size_t num1;
	size_t num2;

	if(str1 == NULL) return false;
	if(str2 == NULL) return false;

	num1 = (size_t) cstr_getlength(str1);
	num2 = (size_t) cstr_getlength(str2);

	if((num1 < stop_index) && (num2 < stop_index))
	{
		if(fail_if_nolen) return false;

		return cstr_compare(str1, str2);
	}

	if((num1 < stop_index) || (num2 < stop_index)) return false;

	num1 = 0u;
	while(num1 < stop_index)
	{
		if(str1[num1] != str2[num1]) return false;
		num1++;
	}

	return true;
}

bool cstr_compare_upto_char(const __tchar_t *str1, const __tchar_t *str2, __tchar_t stop_char, bool fail_if_nochar)
{
	ssize_t num1;
	ssize_t num2;

	if(str1 == NULL) return false;
	if(str2 == NULL) return false;

	num1 = cstr_locatechar(str1, stop_char);
	num2 = cstr_locatechar(str2, stop_char);

	if((num1 < 0) && (num2 < 0))
	{
		if(fail_if_nochar) return false;

		return cstr_compare(str1, str2);
	}

	if(num1 < 0) goto _l_cstr_compare_upto_char_compareloop;

	if((num2 < 0) || (num1 < num2)) num2 = num1;

_l_cstr_compare_upto_char_compareloop:

	num1 = 0u;
	while(num1 < num2)
	{
		if(str1[num1] != str2[num1]) return false;
		num1++;
	}

	return true;
}

bool cstr_copy(const __tchar_t *input_str, __tchar_t *output_str, size_t bufferout_length)
{
	size_t stop_index;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	stop_index = (size_t) cstr_getlength(input_str);

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	memcpy(output_str, input_str, (stop_index*sizeof(__tchar_t)));

	output_str[stop_index] = '\0';

	return true;
}

bool cstr_copy_upto_len(const __tchar_t *input_str, __tchar_t *output_str, size_t bufferout_length, size_t stop_index, bool append_nullchar)
{
	size_t input_len;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	output_str[bufferout_length - 1u] = '\0'; /*Write null char terminator to the end of output buffer, for safety*/

	input_len = (size_t) cstr_getlength(input_str);
	if(input_len < stop_index) stop_index = input_len;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	memcpy(output_str, input_str, (stop_index*sizeof(__tchar_t)));

	if(append_nullchar) output_str[stop_index] = '\0';

	return true;
}

bool cstr_copy_upto_char(const __tchar_t *input_str, __tchar_t *output_str, size_t bufferout_length, __tchar_t stop_char, bool append_nullchar)
{
	ssize_t num1;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	num1 = cstr_locatechar(input_str, stop_char);

	if(num1 < 0) num1 = cstr_getlength(input_str);

	return cstr_copy_upto_len(input_str, output_str, bufferout_length, (size_t) num1, append_nullchar);
}

bool cstr_tolower(__tchar_t *str, size_t buffer_length)
{
	size_t stop_index;
	size_t n_char;

	if(str == NULL) return false;
	if(!buffer_length) return false;

	str[buffer_length - 1u] = '\0'; /*Ensure the buffer is null terminated.*/

	stop_index = (size_t) cstr_getlength(str);

	n_char = 0u;
	while(n_char < stop_index)
	{
		if((str[n_char] >= 0x41) && (str[n_char] <= 0x5a)) str[n_char] |= 0x20;
		n_char++;
	}

	return true;
}

bool cstr_toupper(__tchar_t *str, size_t buffer_length)
{
	size_t stop_index;
	size_t n_char;

	if(str == NULL) return false;
	if(!buffer_length) return false;

	str[buffer_length - 1u] = '\0'; /*Ensure the buffer is null terminated.*/

	stop_index = (size_t) cstr_getlength(str);

	n_char = 0u;
	while(n_char < stop_index)
	{
		if((str[n_char] >= 0x61) && (str[n_char] <= 0x7a)) str[n_char] &= 0xdf;
		n_char++;
	}

	return true;
}

bool cstr_copy_text8_to_text16(const uint8_t *input_str, uint16_t *output_str, size_t bufferout_length)
{
	size_t stop_index;
	size_t n_char;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (uint16_t) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return true;
}

bool cstr_copy_text8_to_text32(const uint8_t *input_str, uint32_t *output_str, size_t bufferout_length)
{
	size_t stop_index;
	size_t n_char;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (uint32_t) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return true;
}

bool cstr_copy_text16_to_text8(const uint16_t *input_str, uint8_t *output_str, size_t bufferout_length)
{
	size_t stop_index;
	size_t n_char;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (uint8_t) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return true;
}

bool cstr_copy_text16_to_text32(const uint16_t *input_str, uint32_t *output_str, size_t bufferout_length)
{
	size_t stop_index;
	size_t n_char;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (uint32_t) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return true;
}

bool cstr_copy_text32_to_text8(const uint32_t *input_str, uint8_t *output_str, size_t bufferout_length)
{
	size_t stop_index;
	size_t n_char;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (uint8_t) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return true;
}

bool cstr_copy_text32_to_text16(const uint32_t *input_str, uint16_t *output_str, size_t bufferout_length)
{
	size_t stop_index;
	size_t n_char;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	stop_index = 0u;
	while(input_str[stop_index] != '\0') stop_index++;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	n_char = 0u;
	while(n_char < stop_index)
	{
		output_str[n_char] = (uint16_t) input_str[n_char];
		n_char++;
	}

	output_str[stop_index] = '\0';

	return true;
}

#ifdef __TEXTFORMAT_USE_WCHAR

ssize_t _cstr_char_getlength(const char *str)
{
	size_t len;

	if(str == NULL) return -1;

	len = 0u;
	while(str[len] != '\0') len++;

	return (ssize_t) len;
}

ssize_t _cstr_char_locatechar(const char *str, char c)
{
	size_t len;
	size_t n_char;

	if(str == NULL) return -1;

	len = (size_t) _cstr_char_getlength(str);

	n_char = 0u;
	while(n_char < len)
	{
		if(str[n_char] == c) return (ssize_t) n_char;
		n_char++;
	}

	return -1;
}

bool _cstr_char_compare(const char *str1, const char *str2)
{
	size_t num1;
	size_t num2;

	if(str1 == NULL) return false;
	if(str2 == NULL) return false;

	num1 = (size_t) _cstr_char_getlength(str1);
	num2 = (size_t) _cstr_char_getlength(str2);

	if(num1 != num2) return false;

	num1 = 0u;
	while(num1 < num2)
	{
		if(str1[num1] != str2[num1]) return false;
		num1++;
	}

	return true;
}

bool _cstr_char_compare_upto_len(const char *str1, const char *str2, size_t stop_index, bool fail_if_nolen)
{
	size_t num1;
	size_t num2;

	if(str1 == NULL) return false;
	if(str2 == NULL) return false;

	num1 = (size_t) _cstr_char_getlength(str1);
	num2 = (size_t) _cstr_char_getlength(str2);

	if((num1 < stop_index) && (num2 < stop_index))
	{
		if(fail_if_nolen) return false;

		return _cstr_char_compare(str1, str2);
	}

	if((num1 < stop_index) || (num2 < stop_index)) return false;

	num1 = 0u;
	while(num1 < stop_index)
	{
		if(str1[num1] != str2[num1]) return false;
		num1++;
	}

	return true;
}

bool _cstr_char_compare_upto_char(const char *str1, const char *str2, char stop_char, bool fail_if_nochar)
{
	ssize_t num1;
	ssize_t num2;

	if(str1 == NULL) return false;
	if(str2 == NULL) return false;

	num1 = _cstr_char_locatechar(str1, stop_char);
	num2 = _cstr_char_locatechar(str2, stop_char);

	if((num1 < 0) && (num2 < 0))
	{
		if(fail_if_nochar) return false;

		return _cstr_char_compare(str1, str2);
	}

	if(num1 < 0) goto _l__cstr_char_compare_upto_char_compareloop;

	if((num2 < 0) || (num1 < num2)) num2 = num1;

_l__cstr_char_compare_upto_char_compareloop:

	num1 = 0u;
	while(num1 < num2)
	{
		if(str1[num1] != str2[num1]) return false;
		num1++;
	}

	return true;
}

bool _cstr_char_copy(const char *input_str, char *output_str, size_t bufferout_length)
{
	size_t stop_index;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	stop_index = (size_t) _cstr_char_getlength(input_str);

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	memcpy(output_str, input_str, stop_index);

	output_str[stop_index] = '\0';

	return true;
}

bool _cstr_char_copy_upto_len(const char *input_str, char *output_str, size_t bufferout_length, size_t stop_index, bool append_nullchar)
{
	size_t input_len;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	output_str[bufferout_length - 1u] = '\0'; /*Write null char terminator to the end of output buffer, for safety*/

	input_len = (size_t) _cstr_char_getlength(input_str);
	if(input_len < stop_index) stop_index = input_len;

	if(stop_index >= bufferout_length) stop_index = bufferout_length - 1u;

	memcpy(output_str, input_str, stop_index);

	if(append_nullchar) output_str[stop_index] = '\0';

	return true;
}

bool _cstr_char_copy_upto_char(const char *input_str, char *output_str, size_t bufferout_length, char stop_char, bool append_nullchar)
{
	ssize_t num1;

	if(input_str == NULL) return false;
	if(output_str == NULL) return false;
	if(!bufferout_length) return false;

	num1 = _cstr_char_locatechar(input_str, stop_char);

	if(num1 < 0) num1 = _cstr_char_getlength(input_str);

	return _cstr_char_copy_upto_len(input_str, output_str, bufferout_length, (size_t) num1, append_nullchar);
}

bool _cstr_char_tolower(char *str, size_t buffer_length)
{
	size_t stop_index;
	size_t n_char;

	if(str == NULL) return false;
	if(!buffer_length) return false;

	str[buffer_length - 1u] = '\0'; /*Ensure the buffer is null terminated.*/

	stop_index = (size_t) _cstr_char_getlength(str);

	n_char = 0u;
	while(n_char < stop_index)
	{
		if((str[n_char] >= 0x41) && (str[n_char] <= 0x5a)) str[n_char] |= 0x20;
		n_char++;
	}

	return true;
}

bool _cstr_char_toupper(char *str, size_t buffer_length)
{
	size_t stop_index;
	size_t n_char;

	if(str == NULL) return false;
	if(!buffer_length) return false;

	str[buffer_length - 1u] = '\0'; /*Ensure the buffer is null terminated.*/

	stop_index = (size_t) _cstr_char_getlength(str);

	n_char = 0u;
	while(n_char < stop_index)
	{
		if((str[n_char] >= 0x61) && (str[n_char] <= 0x7a)) str[n_char] &= 0xdf;
		n_char++;
	}

	return true;
}

#endif

