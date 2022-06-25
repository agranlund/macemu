/*
 *  sysdeps.h - System dependent definitions for Atari
 *
 *  Basilisk II (C) 1997-2008 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SYSDEPS_H
#define SYSDEPS_H

#include "assert.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "sys/types.h"
#include "sys/time.h"

#include "mint/osbind.h"
#include "mint/ostruct.h"
#include "gem.h"
#include "gemx.h"
#ifndef __GNUC_INLINE__
#define __GNUC_INLINE__
#include "mint/linea.h"
#undef __GNUC_INLINE__
#else
#include "mint/linea.h"
#endif

#include "user_strings_atari.h"

#ifdef ATARI
#pragma GCC diagnostic ignored "-fpermissive"
#endif

// Mac and host address space are the same
#define REAL_ADDRESSING 1

// Using 68k natively
#define EMULATED_68K 0

// Can read/write 0x00000000-0x00000007 if zero page is remapped by MMU
//#define ATARI_ZERO_WRITABLE

// Mac ROM is not write protected
#define ROM_IS_WRITE_PROTECTED 0
#define USE_SCRATCHMEM_SUBTERFUGE 1

#define PRECISE_TIMING 1

// ExtFS is supported
#define SUPPORTS_EXTFS 1

// mon is not supported
#undef ENABLE_MON

// Data types
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned long uint32;
typedef signed long int32;
typedef unsigned long long uint64;
typedef signed long long int64;

// Time data type for Time Manager emulation
typedef struct timeval tm_time_t;

// Endianess conversion (not needed)
#define ntohs(x) (x)
#define ntohl(x) (x)
#define htons(x) (x)
#define htonl(x) (x)

extern char* strlwr(char* str);
extern char* strupr(char* str);

extern "C" int Getcookie(long, long*);

#ifdef printf
#undef printf
#endif
extern "C" void aprintf(const char *fmt, ...);
#define printf aprintf
#define log aprintf

#ifndef strdup
#define strdup astrdup
extern "C" char* astrdup(const char* s);
#endif

#endif
