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

#include "strdef.hpp"

__string str_tolower(__string input)
{
	__string output;
	size_t input_len;
	size_t n_char;

	input_len = (size_t) input.length();

	output = __TEXT("");
	n_char = 0u;
	while(n_char < input_len)
	{
		if((input[n_char] >= 0x41) && (input[n_char] <= 0x5a)) output += (__tchar_t) (((uint32_t) input[n_char]) | 0x20);
		else output += input[n_char];

		n_char++;
	}

	return output;
}

__string str_toupper(__string input)
{
	__string output;
	size_t input_len;
	size_t n_char;

	input_len = (size_t) input.length();

	output = __TEXT("");
	n_char = 0u;
	while(n_char < input_len)
	{
		if((input[n_char] >= 0x61) && (input[n_char] <= 0x7a)) output += (__tchar_t) (((uint32_t) input[n_char]) & 0xdf);
		else output += input[n_char];

		n_char++;
	}

	return output;
}

#ifdef __TEXTFORMAT_USE_WCHAR

std::string _str_char_tolower(std::string input)
{
	std::string output;
	size_t input_len;
	size_t n_char;

	input_len = (size_t) input.length();

	output = "";
	n_char = 0u;
	while(n_char < input_len)
	{
		if((input[n_char] >= 0x41) && (input[n_char] <= 0x5a)) output += (char) (((uint32_t) input[n_char]) | 0x20);
		else output += input[n_char];

		n_char++;
	}

	return output;
}

std::string _str_char_toupper(std::string input)
{
	std::string output;
	size_t input_len;
	size_t n_char;

	input_len = (size_t) input.length();

	output = "";
	n_char = 0u;
	while(n_char < input_len)
	{
		if((input[n_char] >= 0x61) && (input[n_char] <= 0x7a)) output += (char) (((uint32_t) input[n_char]) & 0xdf);
		else output += input[n_char];

		n_char++;
	}

	return output;
}

#endif

