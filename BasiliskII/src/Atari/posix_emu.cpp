/*
 *  posix_emu.cpp -- posix and virtual desktop
 *
 *  Basilisk II (C) 1997-2008 Christian Bauer
 *
 *  Windows platform specific code copyright (C) Lauri Pesonen
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

#include "sysdeps.h"
#define NO_POSIX_API_HOOK
#include "posix_emu.h"
#include "user_strings.h"
#include "main.h"
#include "extfs_defs.h"
#include "prefs.h"
#include "zeropage.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#ifdef LIBCMINI
	#include "errno.h"
	#include <ext.h>
	#include <dirent.h>
	#define	__S_IFMT				0170000
	#define	__S_IFDIR				0040000	
	#define	__S_ISTYPE(mode, mask)	(((mode) & __S_IFMT) == (mask))
	#define	S_ISDIR(mode)	 		__S_ISTYPE((mode), __S_IFDIR)
#else
	#include <errno.h>
	#include <fcntl.h>
	#include <stdarg.h>
	#include <sys/dirent.h>
	#include <sys/stat.h>	
#endif


#include "mint/dcntl.h"
#include "mint/mintbind.h"

#ifdef MINTLIB
	#define dta _dta
#endif

#define DEBUG 0
#include "debug.h"


#define FILE_ACCESS_START()	\
	TOSContext tos_context(0x2500); \
	EnterSection(SECTION_TOS);

#define FILE_ACCESS_DONE()	\
	ExitSection(SECTION_TOS);


extern bool isMint;
extern bool isMagic;

bool extfs_supported = false;

#define EMPTYBUF_SIZE (1 * 1024)
static char emptybuf[EMPTYBUF_SIZE];

void init_posix_emu(void)
{
	memset(emptybuf, 0, EMPTYBUF_SIZE);
	if (isMint || isMagic)
		extfs_supported = true;
}

void final_posix_emu(void)
{
}

extern "C" {

int my_stat( const char *path, struct my_stat *st )
{
	if (!extfs_supported)
		return -1;

	FILE_ACCESS_START();
	int result = stat(path, (struct stat*) st);
	FILE_ACCESS_DONE();
	D(bug("stat(%s) = %d [mode:%x]\n", path, result, st->st_mode));
	return result;
}

int my_fstat( int fd, struct my_stat *st )
{
	FILE_ACCESS_START();
	int result = fstat(fd, (struct stat*) st);
	FILE_ACCESS_DONE();
	D(bug("fstat(%d) = %d\n", fd, result));
	return result;
}

int my_open( const char *path, int mode, ... )
{
	FILE_ACCESS_START();
	va_list args;
	va_start(args, mode);
	int result = open(path, mode, args);
	va_end(args);
	FILE_ACCESS_DONE();
	D(bug("open [%s][%x] = %d\n", path, mode, result));
	return result;
}

int my_rename( const char *old_path, const char *new_path )
{
	FILE_ACCESS_START();
	int result = rename(old_path, new_path);
	Sync();
	FILE_ACCESS_DONE();
	D(bug("rename [%s] [%s] = %d\n", old_path, new_path, result));
	return result;
}

int my_access( const char *path, int mode )
{
#ifdef LIBCMINI
	// todo: missing in libcmini
	int result = 0;
	if (mode == F_OK)
	{
		// exist check
		FILE_ACCESS_START();
		struct stat st;
		if (stat(path, &st) == 0) {
			result = 0;
		} else {
			result = -1;
		}
		FILE_ACCESS_DONE();
		D(bug("access F_OK [%s] = %d\n", path, result));
		return result;
	}
	// permission check: R_OK / W_OK / X_OK
	return result;
#else
	FILE_ACCESS_START();
	int result = access(path, mode);
	FILE_ACCESS_DONE();
	return result;
#endif
}

int my_mkdir( const char *path, int mode )
{
	FILE_ACCESS_START();
	int result;
#ifdef LIBCMINI
	// todo: missing in libcmini
	struct stat st;
	if ((stat(path, &st) == 0) && S_ISDIR(st.st_mode)) {
		result = 0;
	} else {
		result = Dcreate(path);
	}
#else
	result = mkdir(path, mode);
#endif
	Sync();
	FILE_ACCESS_DONE();
	D(bug("mkdir [%s] = %d\n", path, result));
	return result;
}

int my_remove( const char *path )
{
	FILE_ACCESS_START();
#ifdef LIBCMINI
	// todo: broken in libcmini
	int result = 0;
	struct stat st;
	if (path && *path && (stat(path, &st) == 0)) {
		if (S_ISDIR(st.st_mode)) {
			result = Ddelete(path);
		} else {
			result = Fdelete(path);
		}
		if ((result == ENOTDIR) || (result == ENOENT))
			result = 0;
	}
#else
	int result = remove(path);
#endif
	Sync();
	FILE_ACCESS_DONE();
	D(bug("remove [%s] = %d\n", path, result));
	return result;
}

int my_creat( const char *path, int mode )
{
	D(bug("my_creat\n"));
	int result = my_open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
	if (result >= 0)
	{
		FILE_ACCESS_START();
		Fsync(result);
		FILE_ACCESS_DONE();
	}
	return result;
}

int my_ftruncate( int fd, unsigned int sz )
{
	int result = 0;
	FILE_ACCESS_START();
	int32 pos = lseek(fd, 0, SEEK_CUR);
	int32 size = lseek(fd, 0, SEEK_END);
	int32 newsize = (int32) sz;
	if (newsize > size)
	{
		int32 cnt = newsize - size;
		while(cnt >= EMPTYBUF_SIZE) {
			write(fd, emptybuf, EMPTYBUF_SIZE);
			cnt -= EMPTYBUF_SIZE;
		}
		if (cnt > 0) {
			write(fd, emptybuf, cnt);
		}
		lseek(fd, pos, SEEK_SET);
		result = 0;
	}
	else
	{
		lseek(fd, pos, SEEK_SET);
		if (newsize < size) {
			result = Fcntl(fd, &newsize, FTRUNCATE);
		}
	}
	FILE_ACCESS_DONE();
	D(bug("ftruncate (%d), %d -> %d = %d\n", fd, newsize, size, result));
	return result;
}

int my_close( int fd )
{
	FILE_ACCESS_START();
	int result = close(fd);
	Sync();
	FILE_ACCESS_DONE();
	D(bug("close %d = %d\n", fd, result));
	return result;
}

long my_lseek( int fd, long offset, int origin )
{
	FILE_ACCESS_START();
	off_t result = lseek(fd, offset, origin);
	FILE_ACCESS_DONE();
	D(bug("seek %d,%d,%d = %d\n", fd, offset, origin, result));
	return (long) result;
}

int my_read( int fd, void *buffer, unsigned int count )
{
	FILE_ACCESS_START();
	int result = read(fd, buffer, count);
	FILE_ACCESS_DONE();
	D(bug("read %d,%d = %d\n", fd, count, result));
	return result;
}

int my_write( int fd, const void *buffer, unsigned int count )
{
	FILE_ACCESS_START();
	int result = write(fd, buffer, count);
	FILE_ACCESS_DONE();
	D(bug("write %d,%d = %d\n", fd, count, result));
	return result;
}


DIR* my_opendir( const char* path)
{
	FILE_ACCESS_START();
	DIR* d = opendir(path);
	FILE_ACCESS_DONE();
	D(bug("opendir %s = %x\n", path, d));
	return d;
}

struct dirent* my_readdir(DIR *d)
{
	FILE_ACCESS_START();
	struct dirent* de = readdir(d);
	FILE_ACCESS_DONE();
	D(bug("readdir %x = %x\n", d, de));
	return de;
}

int my_closedir(DIR *d)
{
	FILE_ACCESS_START();
	int result = closedir(d);
	FILE_ACCESS_DONE();
	D(bug("closedir %x = %d\n", d, result));
	return result;
}

int my_utime( const char *path, struct utimbuf * my_times )
{
	FILE_ACCESS_START();
	unsigned long modtime;
	unsigned long* dtime;
	if (my_times) {
		dtime = (unsigned long*) &my_times->modtime;
	} else {
		modtime = ((long) Tgettime() << 16) | (Tgetdate() & 0xFFFF);
		dtime = &modtime;
	}

	int16 fh = open(path, O_WRONLY | O_CREAT);
	if (fh < 0) {
		FILE_ACCESS_DONE();
		D(bug("my_utime - failed to open %s (%d)\n", path, fh));
		return -1;
	}

	int result = Fdatime(dtime, fh, 1);
	close(fh);
	FILE_ACCESS_DONE();

	D(bug("my_utime %s = %d\n", path, result));
	return result;
}

};