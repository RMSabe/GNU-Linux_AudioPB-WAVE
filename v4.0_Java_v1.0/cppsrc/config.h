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

#ifndef CONFIG_H
#define CONFIG_H 1

/*
 * __FILE64: File Size and Position Definition Macro
 * If defined, file size and position will be 64 bits. off64_t and lseek64() will be used.
 * Else, file size and position will be long sized. off_t and lseek() will be used.
 */

#define __FILE64

/*
 * __TEXTFORMAT_USE_WCHAR: Default Text Format Definition Macro
 * If defined, default text char type (__tchar_t) and default C++ string type (__string) will be defined as wchar_t and std::wstring respectivelly.
 * Else, default text char type (__tchar_t) and default C++ string type (__string) will be defined as char and std::string respectivelly.
 */

#define __TEXTFORMAT_USE_WCHAR

/*
 * TEXTBUF_SIZE_CHARS
 * Define the size (in number of characters) of the global __tchar_t[] textbuf buffer.
 */

#define TEXTBUF_SIZE_CHARS 1024U

#endif /*CONFIG_H*/

