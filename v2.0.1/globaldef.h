/*
 * WAVE audio file playback app v2.0.1 for GNU-Linux
 *
 * Author: Rafael Sabe
 * Email: rafaelmsabe@gmail.com
 */

#ifndef GLOBALDEF_H
#define GLOBALDEF_H

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef _LARGEFILE64_SOURCE
typedef off64_t __offset;
#define __LSEEK(fd, offset, whence) lseek64(fd, offset, whence)
#else
typedef off_t __offset;
#define __LSEEK(fd, offset, whence) lseek(fd, offset, whence)
#endif

#endif //GLOBALDEF_H

