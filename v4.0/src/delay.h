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

#ifndef DELAY_H
#define DELAY_H 1

#include "globldef.h"
#include <time.h>

__EXTERNC__ void delay_ms(clock_t time_ms);
__EXTERNC__ void delay_us(clock_t time_us);

#endif /*DELAY_H*/

