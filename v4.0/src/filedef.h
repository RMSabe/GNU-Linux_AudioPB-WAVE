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

#ifndef FILEDEF_H
#define FILEDEF_H 1

#include "globldef.h"

#include <fcntl.h>
#include <unistd.h>

#ifdef __FILE64
typedef off64_t __offset_t;
#define __LSEEK(fd, offset, whence) lseek64(fd, offset, whence)
#else
typedef off_t __offset_t;
#define __LSEEK(fd, offset, whence) lseek(fd, offset, whence)
#endif

#endif /*FILEDEF_H*/

