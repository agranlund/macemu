/*
 *  posix_emu.h
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

#ifndef _POSIX_EMU_H_
#define _POSIX_EMU_H_

void init_posix_emu(void);
void final_posix_emu(void);

// access() mode: exists?
#ifndef F_OK
  #define F_OK 0
#endif
// access() mode: can do r/w?
#ifndef W_OK
  #define W_OK 6
#endif

#ifdef LIBCMINI

  // libcmini is using different defines than unix/mintlib
  #ifdef O_CREAT
    #undef O_CREAT
    #define O_CREAT  0x20
  #endif
  #ifdef O_TRUNC
    #undef O_TRUNC
    #define O_TRUNC  0x40
  #endif
  #ifdef O_EXCL
    #undef O_EXCL
    #define O_EXCL  0x80
  #endif

  #ifndef EPERM
    #define EPERM       38
  #endif
  #ifndef EBUSY
    #define EBUSY       2
  #endif
  #ifndef ENOTEMPTY
    #define ENOTEMPTY   83
  #endif
  #ifndef ENOSPC
    #define ENOSPC      94
  #endif
  #ifndef EROFS
    #define EROFS       13
  #endif
  #ifndef EMFILE
    #define EMFILE      35
  #endif
  #ifndef EIO
    #define EIO         90
  #endif
#endif

struct utimbuf
{
	time_t actime;      // access time
	time_t modtime;     // modification time
};

typedef struct __dirstream DIR;

extern "C" {
int my_stat( const char *, struct my_stat * );
int my_fstat( int, struct my_stat * );
int my_open( const char *, int, ... );
int my_rename( const char *, const char * );
int my_access( const char *, int );
int my_mkdir( const char *path, int mode );
int my_remove( const char * );
int my_creat( const char *path, int mode );
int my_creat( const char *path, int mode );
int my_close( int fd );
long my_lseek( int fd, long, int);
int my_read( int fd, void *, unsigned int);
int my_write( int fd, const void *, unsigned int);
int my_ftruncate( int fd, unsigned int size );
int my_locking( int fd, int mode, long nbytes );
int my_utime( const char *path, struct utimbuf * );

DIR* my_opendir( const char* path);
struct dirent* my_readdir(DIR *d);
int my_closedir(DIR *d);
};



#ifndef NO_POSIX_API_HOOK
# ifdef fstat
#  undef fstat
# endif
# define stat my_stat
# define fstat my_fstat
# define open my_open
# define rename my_rename
# define access my_access
# define mkdir my_mkdir
# define rmdir my_remove
# define remove my_remove
# define creat my_creat
# define close my_close
# ifdef lseek
#  undef lseek
# endif
# define lseek my_lseek
# define read my_read
# define write my_write
# define ftruncate my_ftruncate
# define locking my_locking
# define utime my_utime

#define opendir my_opendir
#define readdir my_readdir
#define closedir my_closedir
#endif //!NO_POSIX_API_HOOK

// can't #define "stat" unless there's a replacement for "struct stat"

struct my_st_mtim {
	unsigned long tv_nsec;
};

struct my_stat {
  __dev_t st_dev;		/* Device.  */
  __ino_t st_ino;		/* File serial number.  */
  __mode_t st_mode;		/* File mode.  */
  __nlink_t st_nlink;		/* (Hard) link count.  */
  __uid_t st_uid;		/* User ID of the file's owner.  */
  __gid_t st_gid;		/* Group ID of the file's group.  */
  __dev_t st_rdev;		/* Device number, if device.  */
  long __st_high_atime;
  __time_t st_atime;		/* Time of last access, UTC.  */
  struct my_st_mtim st_atim;
  long __st_high_mtime;
  __time_t st_mtime;		/* Time of last access, UTC.  */
  struct my_st_mtim st_mtim;
  long __st_high_ctime;
  __time_t st_ctime;		/* Time of last status change, UTC.  */
  struct my_st_mtim st_ctim;
  unsigned long __st_hi_size;	/* Upper 4 bytes of st_size.  */
  __off_t st_size;		/* File size, in bytes.  */
  unsigned long __st_hi_blocks; /* Upper 4 bytes of st_blocks.  */
  __off_t st_blocks;		/* Number of 512-bytes blocks allocated.  */
  unsigned long int st_blksize;	/* Optimal blocksize for I/O.  */
  unsigned long	int st_flags;	/* User defined flags for file.  */
  unsigned long	int st_gen;	/* File generation number.  */
  long int __res[7];
};


#endif //_POSIX_EMU_H_
