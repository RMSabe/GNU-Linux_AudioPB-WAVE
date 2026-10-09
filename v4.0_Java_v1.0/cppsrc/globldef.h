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

#ifndef GLOBLDEF_H
#define GLOBLDEF_H 1

#include "config.h"

#ifdef __cplusplus
#define __EXTERNC__ extern "C"
#else
#define __EXTERNC__ extern
#endif

#define PTR_SIZE_BYTES (sizeof(void*))
#define PTR_SIZE_BITS (PTR_SIZE_BYTES*8U)

#ifdef __FILE64
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#else
#ifdef _LARGEFILE64_SOURCE
#undef _LARGEFILE64_SOURCE
#endif
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/types.h>

#define PTR_MAX_VALUE ((uintptr_t) ~((uintptr_t) 0U))
#define PTR_MSB_VALUE ((uintptr_t) (((uintptr_t) 1U) << (PTR_SIZE_BITS - 1U)))

#ifdef __TEXTFORMAT_USE_WCHAR
typedef wchar_t __tchar_t;
#define __TEXT(cstr) L##cstr
#else
typedef char __tchar_t;
#define __TEXT(cstr) cstr
#endif

#define TEXTBUF_SIZE_BYTES (TEXTBUF_SIZE_CHARS*sizeof(__tchar_t))

__EXTERNC__ __tchar_t textbuf[];

/*
 * These functions are meant to be used mostly with buffer allocation sizes.
 * They will return the closest power of 2 value from the input value.
 * ..._round() will round the value to the closest power of 2
 * ..._floor() will return the closest power of 2 below input
 * ..._ceil() will return the closest power of 2 above input
 * if input is 0 or if function fails, it will return 0
 */

__EXTERNC__ uintptr_t _get_closest_power2_round(uintptr_t input);
__EXTERNC__ uintptr_t _get_closest_power2_floor(uintptr_t input);
__EXTERNC__ uintptr_t _get_closest_power2_ceil(uintptr_t input);

/*This function returns true if input is a power of 2, false otherwise*/

__EXTERNC__ bool _is_power2(uintptr_t input);

#endif /*GLOBLDEF_H*/

