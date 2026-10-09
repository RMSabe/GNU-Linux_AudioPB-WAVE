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

#ifndef STRDEF_HPP
#define STRDEF_HPP 1

#include "globldef.h"
#include <string>

#ifdef __TEXTFORMAT_USE_WCHAR
typedef std::wstring __string;
#define __TOSTRING(val) std::to_wstring(val)
#else
typedef std::string __string;
#define __TOSTRING(val) std::to_string(val)
#endif

/*
 * str_tolower() & str_toupper()
 * converts all the characters from an input string to lower-case or upper-case.
 *
 * returns the converted string.
 * WARNING: Might not work too well with wstrings.
 */

extern __string str_tolower(__string input);
extern __string str_toupper(__string input);

/*
 * The following functions are the ANSI char version of the previous functions.
 * Since many GNU-Linux APIs use ASCII/ANSI text format, I decided to add the same functions for ASCII/ANSI text format,
 * independent from the __string type.
 */

#ifdef __TEXTFORMAT_USE_WCHAR

extern std::string _str_char_tolower(std::string input);
extern std::string _str_char_toupper(std::string input);

#else

static inline std::string _str_char_tolower(std::string input)
{
	return (std::string) str_tolower((__string) input);
}

static inline std::string _str_char_toupper(std::string input)
{
	return (std::string) str_toupper((__string) input);
}

#endif

#endif /*STRDEF_HPP*/

