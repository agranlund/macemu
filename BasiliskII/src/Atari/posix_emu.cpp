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
#include <ext.h>
#include <dirent.h>
#include "mint/dcntl.h"
#include "mint/mintbind.h"

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

void init_posix_emu(void)
{
	if (isMint || isMagic)
		extfs_supported = true;
}

void final_posix_emu(void)
{
}

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
	FILE_ACCESS_DONE();
	D(bug("rename [%s] [%s] = %d\n", old_path, new_path, result));
	return result;
}

int my_access( const char *path, int mode )
{
	if (mode == F_OK)
	{
		// exist check
		FILE_ACCESS_START();
		ffblk dta;
		int result = findfirst(path, &dta, 0xFF);
		D(bug("access F_OK [%s] = %d\n", path, result));
		FILE_ACCESS_DONE();
		return result;
	}

	// permission check: R_OK / W_OK / X_OK
	errno = 0;
	return 0;
}

int my_mkdir( const char *path, int mode )
{
	FILE_ACCESS_START();
	int result = Dcreate(path);
	FILE_ACCESS_DONE();
	D(bug("mkdir [%s] = %d\n", path, result));
	return result;
}

int my_remove( const char *path )
{
	FILE_ACCESS_START();
	int result = remove(path);
	FILE_ACCESS_DONE();
	D(bug("remove [%s] = %d\n", path, result));
	return result;
}

int my_creat( const char *path, int mode )
{
	return my_open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

int my_chsize( int fd, unsigned int sz )
{
	FILE_ACCESS_START();
	int result = Fcntl (fd, &sz, FTRUNCATE);
	FILE_ACCESS_DONE();
	D(bug("chsize %d , %d = %d\n", fd, sz, result));
	return result;
}

int my_close( int fd )
{
	FILE_ACCESS_START();
	int result = close(fd);
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

/*
static FILETIME get_file_time(time_t time) {
	FILETIME ft;
	unsigned long long result = 11644473600LL;
	result += time;
	result *= 10000000LL;
	ft.dwHighDateTime = (result >> 32);
	ft.dwLowDateTime = (result & 0xFFFFFFFF);
	return ft;
}
*/
int my_utime( const char *path, struct utimbuf * my_times )
{
	//FILE_ACCESS_START();
	// todo
	//FILE_ACCESS_DONE();

	my_times->actime = 0;
	my_times->modtime = 0;
	return 0;
/*
	// todo
	return -1;
	auto tpath = tstr(path);
	LPCTSTR p = MRP(tpath.get());
	HANDLE f = CreateFile(p, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f != INVALID_HANDLE_VALUE) {
		FILETIME crTime = get_file_time(my_times->actime);
		FILETIME modTime = get_file_time(my_times->modtime);
		SetFileTime(f, &crTime, NULL, &modTime);
		CloseHandle(f);
		return 0;
	}
	return -1;
	*/
}




