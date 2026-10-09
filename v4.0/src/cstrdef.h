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

#ifndef CSTRDEF_H
#define CSTRDEF_H 1

#include "globldef.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __TEXTFORMAT_USE_WCHAR
#define __SPRINTF(output, size, input, ...) swprintf(output, size, input, ## __VA_ARGS__)
#define __CSTRTOL(nptr, endptr, base) wcstol(nptr, endptr, base)
#define __CSTRTOUL(nptr, endptr, base) wcstoul(nptr, endptr, base)
#define __CSTRTOLL(nptr, endptr, base) wcstoll(nptr, endptr, base)
#define __CSTRTOULL(nptr, endptr, base) wcstoull(nptr, endptr, base)
#else
#define __SPRINTF(output, size, input, ...) snprintf(output, size, input, ## __VA_ARGS__)
#define __CSTRTOL(nptr, endptr, base) strtol(nptr, endptr, base)
#define __CSTRTOUL(nptr, endptr, base) strtoul(nptr, endptr, base)
#define __CSTRTOLL(nptr, endptr, base) strtoll(nptr, endptr, base)
#define __CSTRTOULL(nptr, endptr, base) strtoull(nptr, endptr, base)
#endif

/*
 * cstr_getlength()
 * returns the length of a null-terminated C string, or -1 if error.
 */

__EXTERNC__ ssize_t cstr_getlength(const __tchar_t *str);

/*
 * cstr_locatechar()
 * search for a specific char (c) in a null-terminated C string.
 *
 * returns the index of the first matching character found,
 * returns -1 if character is not in the string or error.
 */

__EXTERNC__ ssize_t cstr_locatechar(const __tchar_t *str, __tchar_t c);

/*
 * cstr_compare()
 * compares 2 null-terminated C strings.
 *
 * returns true if they're equal.
 * returns false if they're not equal or error
 */

__EXTERNC__ bool cstr_compare(const __tchar_t *str1, const __tchar_t *str2);

/*
 * cstr_compare_upto_len()
 * compares 2 null-terminated C strings up to a specified length (stop_index).
 *
 * fail_if_nolen sets the outcome if both strings are shorter than stop_index:
 * if both strings are shorter than stop_index and fail_if_nolen is set to true, then function will return false, even if strings are equal.
 * if both strings are shorter than stop_index and fail_if_nolen is set to false, then function will perform normal cstr_compare between input strings.
 *
 * returns true if both strings are equal up to stop_index.
 * returns false if not equal or error.
 */

__EXTERNC__ bool cstr_compare_upto_len(const __tchar_t *str1, const __tchar_t *str2, size_t stop_index, bool fail_if_nolen);

/*
 * cstr_compare_upto_char()
 * compares 2 null-terminated C strings until it reaches a specified character (stop_char) in either string.
 *
 * fail_if_nochar sets the outcome if neither strings have the stop_char:
 * if neither strings have stop_char and fail_if_nochar is set to true, then function will return false, even if strings are equal.
 * if neither strings have stop_char and fail_if_nochar is set to false, then function will perform normal cstr_compare between input strings.
 */

__EXTERNC__ bool cstr_compare_upto_char(const __tchar_t *str1, const __tchar_t *str2, __tchar_t stop_char, bool fail_if_nochar);

/*
 * cstr_copy()
 * copies the content of an input null-terminated C string, to an output string. Appends null-terminator character to output string.
 *
 * if input string length is longer than output buffer length - 1, all remaining input characters past buffer length - 1 will be truncated from output string.
 *
 * returns true if successful, false if error.
 */

__EXTERNC__ bool cstr_copy(const __tchar_t *input_str, __tchar_t *output_str, size_t bufferout_length);

/*
 * cstr_copy_upto_len()
 * copy the content of an input null-terminated C string to an output string up to a specified length (stop_index).
 *
 * if input string is shorter than given stop_index, then function will copy the whole input string.
 * if input string is longer than output buffer length - 1, then all remaining input characters past that index will be truncated in output string.
 *
 * if append_nullchar is true, function will append a null-terminator character to the copied content in output buffer.
 * function will always write a null-terminator character at the very end of output buffer, even if append_nullchar is false, for safety.
 *
 * returns true if successful, false if error.
 */

__EXTERNC__ bool cstr_copy_upto_len(const __tchar_t *input_str, __tchar_t *output_str, size_t bufferout_length, size_t stop_index, bool append_nullchar);

/*
 * cstr_copy_upto_char()
 * copy the content of an input null-terminated C string to an output string up to a specified character (stop_char).
 *
 * if input string does not have stop_char, then function will copy the whole input string.
 * if input string is longer than output buffer length - 1, then all remaining input characters past that index will be truncated in output string.
 *
 * if append_nullchar is true, function will append a null-terminator character to the copied content in output buffer.
 * function will always write a null-terminator character at the very end of output buffer, even if append_nullchar is false, for safety.
 *
 * returns true if successful, false if error.
 */

__EXTERNC__ bool cstr_copy_upto_char(const __tchar_t *input_str, __tchar_t *output_str, size_t bufferout_length, __tchar_t stop_char, bool append_nullchar);

/*
 * cstr_tolower() & cstr_toupper()
 *
 * converts all letters in a null-terminated C string to lower-case or upper-case.
 *
 * returns true if successful, false if error.
 * WARNING: Might not work very well with wchar_t C strings.
 */

__EXTERNC__ bool cstr_tolower(__tchar_t *str, size_t buffer_length);
__EXTERNC__ bool cstr_toupper(__tchar_t *str, size_t buffer_length);

/*
 * cstr_copy_text..._to_text...()
 * Variations of cstr_copy().
 * Copy/convert texts using different character sizes/types (UTF-8, UTF-16, UTF-32, etc).
 *
 * Converting from a bigger character to a smaller character will truncate the higher bits.
 *
 * returns true if successful, false if error.
 */

__EXTERNC__ bool cstr_copy_text8_to_text16(const uint8_t *input_str, uint16_t *output_str, size_t bufferout_length);
__EXTERNC__ bool cstr_copy_text8_to_text32(const uint8_t *input_str, uint32_t *output_str, size_t bufferout_length);

__EXTERNC__ bool cstr_copy_text16_to_text8(const uint16_t *input_str, uint8_t *output_str, size_t bufferout_length);
__EXTERNC__ bool cstr_copy_text16_to_text32(const uint16_t *input_str, uint32_t *output_str, size_t bufferout_length);

__EXTERNC__ bool cstr_copy_text32_to_text8(const uint32_t *input_str, uint8_t *output_str, size_t bufferout_length);
__EXTERNC__ bool cstr_copy_text32_to_text16(const uint32_t *input_str, uint16_t *output_str, size_t bufferout_length);

#if __SIZEOF_WCHAR_T__ == 4

static inline bool cstr_copy_char_to_wchar(const char *input_str, wchar_t *output_str, size_t bufferout_length)
{
	return cstr_copy_text8_to_text32((const uint8_t*) input_str, (uint32_t*) output_str, bufferout_length);
}

static inline bool cstr_copy_wchar_to_char(const wchar_t *input_str, char *output_str, size_t bufferout_length)
{
	return cstr_copy_text32_to_text8((const uint32_t*) input_str, (uint8_t*) output_str, bufferout_length);
}

#endif

#if __SIZEOF_WCHAR_T__ == 2

static inline bool cstr_copy_char_to_wchar(const char *input_str, wchar_t *output_str, size_t bufferout_length)
{
	return cstr_copy_text8_to_text16((const uint8_t*) input_str, (uint16_t*) output_str, bufferout_length);
}

static inline bool cstr_copy_wchar_to_char(const wchar_t *input_str, char *output_str, size_t bufferout_length)
{
	return cstr_copy_text16_to_text8((const uint16_t*) input_str, (uint8_t*) output_str, bufferout_length);
}

#endif

#if __SIZEOF_WCHAR_T__ == 1

static inline bool cstr_copy_char_to_wchar(const char *input_str, wchar_t *output_str, size_t bufferout_length)
{
	return cstr_copy((const __tchar_t*) input_str, (__tchar_t*) output_str, bufferout_length);
}

static inline bool cstr_copy_wchar_to_char(const wchar_t *input_str, char *output_str, size_t bufferout_length)
{
	return cstr_copy((const __tchar_t*) input_str, (__tchar_t*) output_str, bufferout_length);
}

#endif

static inline bool cstr_copy_char_to_tchar(const char *input_str, __tchar_t *output_str, size_t bufferout_length)
{
#ifdef __TEXTFORMAT_USE_WCHAR
	return cstr_copy_char_to_wchar(input_str, (wchar_t*) output_str, bufferout_length);
#else
	return cstr_copy((const __tchar_t*) input_str, output_str, bufferout_length);
#endif
}

static inline bool cstr_copy_tchar_to_char(const __tchar_t *input_str, char *output_str, size_t bufferout_length)
{
#ifdef __TEXTFORMAT_USE_WCHAR
	return cstr_copy_wchar_to_char((const wchar_t*) input_str, output_str, bufferout_length);
#else
	return cstr_copy(input_str, (__tchar_t*) output_str, bufferout_length);
#endif
}

static inline bool cstr_copy_wchar_to_tchar(const wchar_t *input_str, __tchar_t *output_str, size_t bufferout_length)
{
#ifdef __TEXTFORMAT_USE_WCHAR
	return cstr_copy((const __tchar_t*) input_str, output_str, bufferout_length);
#else
	return cstr_copy_wchar_to_char(input_str, (char*) output_str, bufferout_length);
#endif
}

static inline bool cstr_copy_tchar_to_wchar(const __tchar_t *input_str, wchar_t *output_str, size_t bufferout_length)
{
#ifdef __TEXTFORMAT_USE_WCHAR
	return cstr_copy(input_str, (__tchar_t*) output_str, bufferout_length);
#else
	return cstr_copy_char_to_wchar((const char*) input_str, output_str, bufferout_length);
#endif
}

/*
 * The following functions are the ANSI char version of the previous functions.
 * Since many GNU-Linux APIs use ASCII/ANSI text format, I decided to add the same functions for ASCII/ANSI text format,
 * independent from the __tchar_t type.
 */

#ifdef __TEXTFORMAT_USE_WCHAR

__EXTERNC__ ssize_t _cstr_char_getlength(const char *str);
__EXTERNC__ ssize_t _cstr_char_locatechar(const char *str, char c);

__EXTERNC__ bool _cstr_char_compare(const char *str1, const char *str2);
__EXTERNC__ bool _cstr_char_compare_upto_len(const char *str1, const char *str2, size_t stop_index, bool fail_if_nolen);
__EXTERNC__ bool _cstr_char_compare_upto_char(const char *str1, const char *str2, char stop_char, bool fail_if_nochar);

__EXTERNC__ bool _cstr_char_copy(const char *input_str, char *output_str, size_t bufferout_length);
__EXTERNC__ bool _cstr_char_copy_upto_len(const char *input_str, char *output_str, size_t bufferout_length, size_t stop_index, bool append_nullchar);
__EXTERNC__ bool _cstr_char_copy_upto_char(const char *input_str, char *output_str, size_t bufferout_length, char stop_char, bool append_nullchar);

__EXTERNC__ bool _cstr_char_tolower(char *str, size_t buffer_length);
__EXTERNC__ bool _cstr_char_toupper(char *str, size_t buffer_length);

#else

static inline ssize_t _cstr_char_getlength(const char *str)
{
	return cstr_getlength((const __tchar_t*) str);
}

static inline ssize_t _cstr_char_locatechar(const char *str, char c)
{
	return cstr_locatechar((const __tchar_t*) str, (__tchar_t) c);
}

static inline bool _cstr_char_compare(const char *str1, const char *str2)
{
	return cstr_compare((const __tchar_t*) str1, (const __tchar_t*) str2);
}

static inline bool _cstr_char_compare_upto_len(const char *str1, const char *str2, size_t stop_index, bool fail_if_nolen)
{
	return cstr_compare_upto_len((const __tchar_t*) str1, (const __tchar_t*) str2, stop_index, fail_if_nolen);
}

static inline bool _cstr_char_compare_upto_char(const char *str1, const char *str2, char stop_char, bool fail_if_nochar)
{
	return cstr_compare_upto_char((const __tchar_t*) str1, (const __tchar_t*) str2, (__tchar_t) stop_char, fail_if_nochar);
}

static inline bool _cstr_char_copy(const char *input_str, char *output_str, size_t bufferout_length)
{
	return cstr_copy((const __tchar_t*) input_str, (__tchar_t*) output_str, bufferout_length);
}

static inline bool _cstr_char_copy_upto_len(const char *input_str, char *output_str, size_t bufferout_length, size_t stop_index, bool append_nullchar)
{
	return cstr_copy_upto_len((const __tchar_t*) input_str, (__tchar_t*) output_str, bufferout_length, stop_index, append_nullchar);
}

static inline bool _cstr_char_copy_upto_char(const char *input_str, char *output_str, size_t bufferout_length, char stop_char, bool append_nullchar)
{
	return cstr_copy_upto_char((const __tchar_t*) input_str, (__tchar_t*) output_str, bufferout_length, (__tchar_t) stop_char, append_nullchar);
}

static inline bool _cstr_char_tolower(char *str, size_t buffer_length)
{
	return cstr_tolower((__tchar_t*) str, buffer_length);
}

static inline bool _cstr_char_toupper(char *str, size_t buffer_length)
{
	return cstr_toupper((__tchar_t*) str, buffer_length);
}

#endif

#endif /*CSTRDEF_H*/

