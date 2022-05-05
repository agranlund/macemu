/*
 *  sys_atari.cpp - System dependent routines, Atari implementation
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

#include "sysdeps.h"
#include "main.h"
#include "macos_util.h"
#include "prefs.h"
#include "user_strings.h"
#include "sys.h"
#include "zeropage.h"
#include "mint/cookie.h"

#define DEBUG 1
#include "debug.h"
#define SUPPORT_RAW			1
#define SUPPORT_DEV			1
#define SUPPORT_FLOPPY		0
#define SUPPORT_CACHE		1

#define FORCE_FLUSH_WRITES	0

//#define HEAVYDEBUG


// AHDI
#define PUN_DEV			0x1f	// device number
#define PUN_UNIT		0x07	// Unit number
#define PUN_ACSI		0x00	// ACSI
#define PUN_SCSI		0x08	// SCSI
#define PUN_IDE			0x10	// IDE
#define PUN_REMOVABLE	0x40	// Removable media
#define PUN_VALID		0x80	// zero if valid
#define PUN_FLOPPY		0x1000	// fake enum for floppy


struct ahdi_data
{	
	uint16	puns;					// Number of HDDs
	uint8	flags[16];
	uint32	partition_start[16];
	uint32	cookie;					// 'AHDI' if following valid
	uint32*	cookie_ptr;				// Points to 'cookie'
	uint16	version;				// AHDI version
	uint16	max_sect_siz;			// Max logical sec size
	uint32	reserved[16];
};

// File handles are pointers to these structures
struct file_handle
{
	union
	{
		int32 f;
		uint32 device;
	};
	uint16 id;
	loff_t cur_pos;			// current seek position
	loff_t start_byte;		// Size of file header (if any)
	loff_t size;			// Size of file/device (minus header)
	bool read_only;			// Copy of Sys_open() flag
	bool is_file;			// disk image
	bool is_ejected;		// ejected
	char fname[128];
};

typedef int32 (*xhdi_ptr)(...);
xhdi_ptr xhdi = 0;
ahdi_data* ahdi = 0;

static bool sys_cache_enabled = false;
static uint8 sys_tempbuf[256];

#define CACHE_BLOCK_COUNT	2
#define CACHE_BLOCK_SIZE	(16 * 1024)
#define CACHE_DEBUG			0

#if SUPPORT_CACHE
	struct cache_block
	{
		uint16 	id;
		loff_t	pos;
		loff_t	end;
		uint8	buf[CACHE_BLOCK_SIZE];
	};

	cache_block	cache[CACHE_BLOCK_COUNT];
	uint16 next_block = 0;
	#if CACHE_DEBUG
		uint32 cache_hits = 0;
		uint32 cache_miss = 0;
		uint32 cache_skip = 0;
	#endif

	inline uint16 cache_new(loff_t pos)
	{
		uint16 idx = next_block;
		next_block++;
		if (next_block >= CACHE_BLOCK_COUNT)
			next_block = 0;
		return idx;
	}
#endif
/*
class DiskAccess
{
public:
	DiskAccess()
	{
		oldSR = DisableInterrupts();
		oldZP = SetZeroPage(ZEROPAGE_TOS);
		oldsavptr = *((volatile uint32*)0x4a2);
		*((volatile uint32*)0x4a2) = &savptr[50];
		SetSR(0x2100);
	}
	~DiskAccess()
	{
		DisableInterrupts();
		*((volatile uint32*)0x4a2) = oldsavptr;
		SetZeroPage(oldZP);
		SetSR(oldSR);
	}
private:
	uint16 oldSR;
	uint16 oldZP;
	uint32* oldsavptr;
	uint8 savptr[50];
};
#define DISK_ACCESS()	DiskAccess disk_access;
*/
#define DISK_ACCESS()	TOS_CONTEXT()

/*
 *  Initialization
 */

void SysInit(void)
{
	log("SysInit\n");

	Getcookie('XHDI', (int32*)&xhdi);
	ahdi = (*((ahdi_data**) 0x516L));
	if (ahdi && (ahdi->cookie != 'AHDI'))
		ahdi = 0;

#if SUPPORT_CACHE
	sys_cache_enabled = PrefsFindBool("diskcache");
	D(bug(" Cache %s\n", sys_cache_enabled ? "enabled" : "disabled"));
	for (uint16 i=0; i<CACHE_BLOCK_COUNT; i++)
		cache[i].id = 0xFFFF;
#endif

	int32 devmode = PrefsFindInt32("diskdevmode");
	D(bug(" XHDI: %d : %s\n", (devmode & 1) ? 1 : 0, xhdi ? "Yes" : "No"));
	D(bug(" AHDI: %d : %04x\n", (devmode & 2) ? 1 : 0, ahdi ? ahdi->version : 0));

	if ((devmode & 1) == 0)
		xhdi = 0;

	if ((devmode & 2) == 0)
		ahdi = 0;

	// AHDI needs to be at least version 3.0 or greater
	if (ahdi && (ahdi->version < 0x300))
		ahdi = 0;
}


/*
 *  Deinitialization
 */

void SysExit(void)
{
}


/*
 *  This gets called when no "floppy" prefs items are found
 *  It scans for available floppy drives and adds appropriate prefs items
 */

void SysAddFloppyPrefs(void)
{

}


/*
 *  This gets called when no "disk" prefs items are found
 *  It scans for available HFS volumes and adds appropriate prefs items
 */

void SysAddDiskPrefs(void)
{
	// TOS doesn't support MacOS partitioning, so this probably doesn't make much sense...
}


/*
 *  This gets called when no "cdrom" prefs items are found
 *  It scans for available CD-ROM drives and adds appropriate prefs items
 */

void SysAddCDROMPrefs(void)
{

}


/*
 *  Add default serial prefs (must be added, even if no ports present)
 */

void SysAddSerialPrefs(void)
{
	PrefsAddString("seriala", "ser");
	PrefsAddString("serialb", "par");
}


/*
 *  Open file/device, create new file handle (returns NULL on error)
 *
 *	Can be a path to a disk image file, a RAW disk partition or
 *  direct device access.
 *
 *	Format for raw partitions: 	raw:<drive letter>:<offset>:<num blocks>
 *	Format for device names: 	dev:<major>.<minor>:<start block>:<num blocks>
 *
 *  Examples
 * 		e:\folder\test.dsk		(absolute path)
 * 		folder\disk.iso			(relative path)
 * 		raw:g					(RAW partition 'g')
 *		raw:g:0:1024			(RAW partition 'g', 1024 blocks at offset 0 of partition)
 * 		dev:0.0:2:1024			(device 0.0, 1024 blocks at offset 2 on device)
 *
 */
void *Sys_open(const char *name, bool read_only)
{
	static uint16 num_disks = 0;
	file_handle* fh = (file_handle*)NULL;
	bool dev_valid = false;
	int16 dev_major = -1;
	int16 dev_minor = -1;
	uint32 dev_blocks = 0;
	uint32 dev_blocksize = 0;
	uint32 dev_blockstart = 0;
	char dev_name[33] = {0};

	#if DEBUG
	char* fixedname = (char*)sys_tempbuf;
	strncpy(fixedname, name, 254);
	for (uint16 i=0; i<strlen(fixedname); i++)
		if (fixedname[i] == '\\')
			fixedname[i] = '/';
	D(bug("Sys_open: '%s'\n", fixedname));
	#endif

	// direct device access, requires xhdi
	if (strncmp(name, "dev:", 4) == 0)
	{
	#if SUPPORT_DEV
		int32 dmaj = -1;
		int32 dmin = -1;
		if (sscanf(name, "dev:%d.%d:%d:%d", &dmaj, &dmin, &dev_blockstart, &dev_blocks) < 4)
		{
			D(bug(" Err: invalid dev format\n"));
			return NULL;
		}
		dev_major = (int16) dmaj;
		dev_minor = (int16) dmin;

		if (xhdi)
		{
			// device caps
			{
				uint32 totalblocks = 0;
				uint32 blocksize = 0;
				struct XHGetCapacityParams { uint16 opcode; uint16 major; uint16 minor; uint32 *blocks; uint32 *blocksize; };
				XHGetCapacityParams p = {14, dev_major, dev_minor, &totalblocks, &blocksize };
				int32 result = xhdi(p);
				if (result >= 0) {
					if ((dev_blockstart + dev_blocks) > totalblocks) {
						D(bug(" Err: Requested block range does not fit (%d - %d : %d)\n", dev_blockstart, dev_blockstart + dev_blocks, totalblocks));
						return NULL;
					}
				}
			}
			// device params
			{
				struct XHInqTargetParams { uint16 opcode; uint16 major; uint16 minor; uint32* blocksize; uint32* deviceflags; char* productname; };
				XHInqTargetParams p = {1, dev_major, dev_minor, &dev_blocksize, 0, dev_name };
				int32 result = xhdi(p);
				if (result < 0) {
					D(bug(" Err: XHInqTarget for device %d.%d returned %d\n", dev_major, dev_minor, result));
					return NULL;
				}
			}
			dev_valid = true;
		}
		else if (ahdi)
		{
			strcpy(dev_name, "AHDI");
			dev_blocksize = 512;
			dev_valid = true;
		}

	#else
	D(bug(" Err: dev support is disabled\n"));
	return NULL;
	#endif //SUPPORT_DEV
	}

	// direct partition access using xhdi or bios
	else if (strncmp(name, "raw:", 4) == 0)
	{
	#if SUPPORT_RAW

		// sanity check driver letter
		uint32 req_partition_offset = 0;
		uint32 req_partition_size = 0;
		char dev_letter = 0;
		sscanf(name, "raw:%c:%d:%d", &dev_letter, &req_partition_offset, &req_partition_size);
		if ((dev_letter >= 'A') && (dev_letter <= 'Z'))
			dev_letter = (dev_letter - 'A') + 'a';
		if ((dev_letter < 'a') || (dev_letter >'z')) {
			D(bug(" Err: Invalid drive letter '%c'\n", dev_letter));
			return NULL;
		}
		int16 biosdev = dev_letter - 'a';

		// floppy
		if ((biosdev >= 0) && (biosdev < 2))
		{
		#if SUPPORT_FLOPPY
			// todo: we're just going to assume this drive is a 1.4MB one, can we find out for sure?
			sprintf(dev_name, "Floppy", biosdev);
			dev_major = PUN_FLOPPY;
			dev_minor = biosdev;
			dev_blockstart = 1;
			dev_blocksize = 512;
			dev_blocks = 2880;
			dev_valid = true;
		#else
			D(bug(" Err: floppy is not yet supported\n"));
			return NULL;
		#endif
		}
		// hdd - xdhi
		else if (xhdi)
		{
			int32 result;
			// partition info
			{
				char partid[4] = {0};
				_BPB devbpb;
				struct XHInqDevParams2 { uint16 opcode; uint16 bios_device; uint16* major; uint16* minor; uint32* startsec; _BPB* bpb; uint32* blocks; char* partid; };
				XHInqDevParams2 p = {12, biosdev, &dev_major, &dev_minor, &dev_blockstart, &devbpb, &dev_blocks, partid};
				result = xhdi(p);
				if (result < 0) {
					D(bug(" Err: XHInqDev for '%c' returned %d\n", dev_letter, result));
					return NULL;
				}
				if ((strncmp(partid, "RAW", 3) != 0) && (strncmp(partid, "MAC", 3) == 0)) {
					D(bug(" Err: device '%c' is not a RAW or MAC partition ['%s']\n", dev_letter, partid));
					return NULL;
				}
			}
			// driver info
			{
				char driver_name[34];
				char driver_ver[8];
				uint16 driver_ipl;
				uint16 driver_ahdi;
				struct XHInqDriverParams { uint16 opcode; uint16 dev; char* name; char* ver; char* company; uint16* ahdi; uint16* ipl; };
				XHInqDriverParams p = {8, biosdev, driver_name, driver_ver, 0, &driver_ahdi, &driver_ipl };
				result = xhdi(p);
				if (result < 0) {
					D(bug(" Err: XHInqDriver for '%c' returned %d\n", dev_letter, result));
					return NULL;
				}
				D(bug(" Driver: %s %s\n", driver_name, driver_ver));
			}
			// device info
			{
				uint32 flags = 0;
				struct XHInqTargetParams { uint16 opcode; uint16 major; uint16 minor; uint32* blocksize; uint32* deviceflags; char* productname; };
				XHInqTargetParams p = {1, dev_major, dev_minor, &dev_blocksize, &flags, dev_name };
				result = xhdi(p);
				if (result < 0) {
					D(bug(" Err: XHInqTarget for device %d.%d returned %d\n", dev_major, dev_minor, result));
					return NULL;
				}
			}
			dev_valid = true;
		}
		// hdd - ahdi
		else if (ahdi)
		{
			if (biosdev >= 16) {
				D(bug(" Err: AHDI device letter '%c' is not supported (max '%d')\n", dev_letter, 15 + 'a'));
				return NULL;
			}
			if (ahdi->flags[biosdev] & PUN_VALID) {
				D(bug(" Err: AHDI device '%c' is not valid\n", dev_letter));
				return NULL;
			}

			strcpy(dev_name, "AHDI");
			dev_major = (ahdi->flags[biosdev] & (PUN_DEV & ~PUN_UNIT));
			dev_minor = ahdi->flags[biosdev] & (PUN_DEV & PUN_UNIT);
			dev_blockstart = ahdi->partition_start[biosdev];
			dev_blocksize = 512;
			dev_blocks = 0;

			// guess partition size by looking at next partition on same device
			if ((dev_blocks == 0) && (biosdev < 15)) {
				if (!(ahdi->flags[biosdev+1] & PUN_VALID))
					if ((ahdi->flags[biosdev+1] & PUN_DEV) == (ahdi->flags[biosdev] & PUN_DEV))
						if (ahdi->partition_start[biosdev+1] > (dev_blockstart + 32))
							dev_blocks = ahdi->partition_start[biosdev+1] - dev_blockstart - 32;
			}

			if (req_partition_size + dev_blocks == 0) {
				D(bug(" Err: Unable to resolve size for '%c'\n", dev_letter));
				return NULL;
			}

			dev_valid = true;
		}

		if (!dev_valid)
			return NULL;

		// user requested partition offset
		dev_blockstart += req_partition_offset;
		if (req_partition_size)
		{
			// user override size
			dev_blocks = req_partition_size;
		}
		else
		{
			// use entire partition
			if (req_partition_offset)
			{
				// but subtract custom offset if one is used
				if (dev_blocks <= req_partition_offset) {
					D(bug(" Err: partition offset %d is too large for size %d\n", req_partition_offset, dev_blocks));
					return NULL;
				}
				dev_blocks -= req_partition_offset;
			}
		}
	#else
		D(bug(" Err: raw support is disabled\n"));
		return NULL;
	#endif // SUPPORT_RAW
	}

	// setup device
	if (dev_valid)
	{
		// sanity check results
		if ((dev_major < 0) || (dev_minor < 0)) {
			D(bug(" Err: Invalid device %d.%d\n", dev_major, dev_minor));
			return NULL;
		}
		if (dev_blocks == 0) {
			D(bug(" Err: Invalid block count (%d)\n", dev_blocks));
			return NULL;
		}
		if (dev_blocksize != 512) {
			D(bug(" Err: Invalid block size (%d)\n", dev_blocksize));
			return NULL;
		}

		// good to go!
		D(bug(" Device: %d.%d : %s\n", dev_major, dev_minor, dev_name));
		D(bug(" Size:   %d.%1dMB (%d blocks of %d from %d)\n",
			(dev_blocks * dev_blocksize) / (1024 * 1024),
			((dev_blocks * dev_blocksize) % (1024 * 1024)) / 100000,
			dev_blocks,
			dev_blocksize,
			dev_blockstart));

		fh = new file_handle;
		if (fh == NULL)
		{
			D(bug(" Err: File handle is null. out of memory?\n"));
			return NULL;
		}
		fh->id = num_disks++;
		fh->device = (((uint32)dev_major) | (((uint32)(dev_minor)) << 16) | 0x80000000);
		fh->read_only = false;
		fh->is_file = false;
		fh->is_ejected = false;
		fh->cur_pos = 0;
		fh->size = dev_blocks * dev_blocksize;
		fh->start_byte = dev_blockstart * dev_blocksize;
		return fh;
	}
	// disk image
	else
	{
		// open disk image
		char* fname = name;
		int32 f = Fopen(fname, read_only ? 0 : 2);
		if ((f < 0) && !read_only)
		{
			// failed, try as read-only
			f = Fopen(fname, 0);
			if (f >= 0)
				read_only = true;
		}
		if (f < 0)
		{
			// try lowercase filename
			fname = (char*)sys_tempbuf;
			strncpy(fname, name, 255);
			strlwr(fname);
			f = Fopen(fname, read_only ? 0 : 2);
			if ((f < 0) && !read_only)
			{
				// failed, try as read-only
				f = Fopen(fname, 0);
				if (f >= 0)
					read_only = true;
			}
		}
		if (f < 0)
		{
			// try uppercase filename
			fname = (char*)sys_tempbuf;
			strncpy(fname, name, 255);
			strupr(fname);
			f = Fopen(fname, read_only ? 0 : 2);
			if ((f < 0) && !read_only)
			{
				// failed, try as read-only
				f = Fopen(fname, 0);
				if (f >= 0)
					read_only = true;
			}
		}

		if (f >= 0)
		{

			fh = new file_handle;
			if (fh == NULL)
			{
				D(bug(" Err: File handle is null. out of memory?\n"));
				return NULL;
			}
			fh->id = num_disks++;
			fh->f = f;
			fh->read_only = read_only;
			fh->is_file = true;
			fh->is_ejected = false;
			strncpy(fh->fname, fname, 128);

			// Detect disk image file layout
			int32 size = Fseek(0, f, SEEK_END);
			Fseek(f, 0, SEEK_SET);
			D(bug(" Disk size = %d\n", size));
			Fread(f, 256, sys_tempbuf);
			FileDiskLayout(size, sys_tempbuf, fh->start_byte, fh->size);
			fh->cur_pos = 256;
			return fh;
		}
	}

	D(bug(" Err: Failed\n"));
	return NULL;
}	


/*
 *  Close file/device, delete file handle
 */

void Sys_close(void *arg)
{
	DISK_ACCESS();
	D(bug("Sys_close(%08lx)\n", arg));

	file_handle *fh = (file_handle *)arg;
	if (!fh)
		return;

	// disk image file
	if (fh->is_file)
	{
		EnterSection(SECTION_TOS);
		Fclose(fh->f);
		ExitSection(SECTION_TOS);
		#if SUPPORT_CACHE
		for (uint16 i=0; i<CACHE_BLOCK_COUNT; i++)
			if (cache[i].id == fh->id)
				cache[i].id = 0xFFFF;
		#endif
	}


	delete fh;
}



/*
 *  Read "length" bytes from file/device, starting at "offset", to "buffer",
 *  returns number of bytes read (or 0)
 */

size_t Sys_read(void *arg, void *buffer, loff_t offset, size_t length)
{
	Section s(SECTION_DISK);

	file_handle *fh = (file_handle *)arg;
	if (!fh)
	{
		D(bug("Sys_read error: NULL file handle\n"));
		return 0;
	}

	#if SUPPORT_CACHE
	bool cacheable = (sys_cache_enabled && (length < CACHE_BLOCK_SIZE) && (fh->is_file));
	if (cacheable)
	{
		for (uint16 i=0; i<CACHE_BLOCK_COUNT; i++)
		{
			cache_block& block = cache[i];
			if (fh->id == block.id)
			{
				if ((offset >= block.pos) && ((offset+length) <= block.end))
				{
					memcpy(buffer, &block.buf[offset - block.pos], length);
					#if CACHE_DEBUG
					cache_hits++;
					D(bug("Sys_read [%d] cache (HIT)   : %d : %d  (hms: %d / %d / %d)\n", fh->id, offset / 512, length, cache_hits, cache_miss, cache_skip));
					#endif
					return length;
				}
			}
		}
		#if CACHE_DEBUG
		cache_miss++;
		D(bug("Sys_read [%d] cache (MISS)  : %d : %d  (hms: %d / %d / %d)\n", fh->id, offset / 512, length, cache_hits, cache_miss, cache_skip));
		#endif			
	}
	#if CACHE_DEBUG
	else
	{
		cache_skip++;
		D(bug("Sys_read [%d] cache (SKIP) : %d : %d  (mhs: %d / %d / %d)\n", fh->id, offset / 512, length, cache_hits, cache_miss, cache_skip));
	}
	#endif
	#endif

	// device / raw
	if (!fh->is_file)
	{
		DISK_ACCESS();
		uint16 dev_major = (fh->device & 0xFFFF);
		uint16 dev_minor = ((fh->device >> 16) & 0x7FFF);
		uint32 start = (fh->start_byte + offset) >> 9;
		uint16 count = length >> 9;
		int32 result = -1;
		#ifdef HEAVYDEBUG
		D(bug("Sys_read [%02d] device %d.%d : %d blocks at %d\n", fh->id, dev_major, dev_minor, count, start));
		if ((length & 511) || ((fh->start_byte + offset) & 511))
		{
			D(bug("bad start/length: %d, %d\n", (fh->start_byte + offset), length));
			QuitEmulator();
		}
		#endif

		#if SUPPORT_FLOPPY
		if (dev_major == PUN_FLOPPY)
		{
			const uint32 sectors_per_track = 18;
			const uint32 bytes_per_sector = 512;
			uint16 block_start = start;
			uint16 block_count = count;
			D(bug("floppy read %d,%d (%d, %d)\n", offset, length, block_start, block_count));
			uint8* buf = buffer;
			while(block_count > 0)
			{
				uint16 track = block_start / sectors_per_track;
				uint16 sector = 1 + (block_start - (track * sectors_per_track));
				D(bug("floprd %d %d\n", sector, track));
				EnterSection(SECTION_TOS);
				int16 result = Floprd(buf, 0, dev_minor, sector, track, 0, 1);
				ExitSection(SECTION_TOS);
				if (result != 0) {
					D(bug(" fail = %d\n", result));
				}
				buf += bytes_per_sector;
				block_start++;
				block_count--;
			}
			result = 0;
		} else
		#endif
		if (xhdi)
		{
			// xhdi
			if (cacheable)
			{
				uint16 cache_block_idx = cache_new(offset);
				cache_block& block = cache[cache_block_idx];
				struct XHReadWriteParams { uint16 opcode; uint16 major; uint16 minor; uint16 rwflag; uint32 recno; uint16 count; void *buf; };
				XHReadWriteParams p = {10, dev_major, dev_minor, 0, start, CACHE_BLOCK_SIZE>>9, block.buf};
				EnterSection(SECTION_TOS);
				result = xhdi(p);
				ExitSection(SECTION_TOS);
				if (result >= 0)
				{
					block.id = fh->id;
					block.pos = offset;
					block.end = offset + CACHE_BLOCK_SIZE;
					memcpy(buffer, (void*)block.buf, length);
					return length;
				}
			}
			struct XHReadWriteParams { uint16 opcode; uint16 major; uint16 minor; uint16 rwflag; uint32 recno; uint16 count; void *buf; };
			XHReadWriteParams p = {10, dev_major, dev_minor, 0, start, count, buffer};
			EnterSection(SECTION_TOS);
			result = xhdi(p);
			ExitSection(SECTION_TOS);
		}
		else if (ahdi)
		{
			// ahdi
			uint16 dev = 2 + ((dev_major | dev_minor) & PUN_DEV);
			if (cacheable)
			{
				uint16 cache_block_idx = cache_new(offset);
				cache_block& block = cache[cache_block_idx];
				EnterSection(SECTION_TOS);
				result = Rwabs(8, block.buf, CACHE_BLOCK_SIZE>>9, -1, dev, start);
				ExitSection(SECTION_TOS);
				if (result == 0)
				{
					block.id = fh->id;
					block.pos = offset;
					block.end = offset + CACHE_BLOCK_SIZE;
					memcpy(buffer, (void*)block.buf, length);
					return length;
				}
			}
			EnterSection(SECTION_TOS);
			result = Rwabs(8, buffer, count, -1, dev, start);
			ExitSection(SECTION_TOS);
		}

		if (result < 0) {
			D(bug(" Sys_read: Err: %d\n", result));
			return 0;
		}

		return length;
	}

	// disk image file
	else
	{
		DISK_ACCESS();
		uint32 p = offset + fh->start_byte;
		if (fh->cur_pos != p)
		{
			EnterSection(SECTION_TOS);
			int32 result = Fseek(p, fh->f, SEEK_SET);
			ExitSection(SECTION_TOS);
			if (result < 0)
			{
				D(bug("Sys_read [%d] file seek error! %d -> %d\n", fh->id, fh->cur_pos / 512, p / 512));
				return 0;
			}
			fh->cur_pos = result;
		}

		// Read data
		#ifdef HEAVYDEBUG
		D(bug("Sys_read [%d] file read %d : %d\n", fh->id, p / 512, length / 512));
		#endif
		int32 actual = -1;

		#if SUPPORT_CACHE
		if (cacheable)
		{
			uint16 cache_block_idx = cache_new(offset);
			cache_block& block = cache[cache_block_idx];
			EnterSection(SECTION_TOS);
			actual = Fread(fh->f, CACHE_BLOCK_SIZE, block.buf);
			ExitSection(SECTION_TOS);
			if (actual >= length)
			{
				block.id = fh->id;
				block.pos = offset;
				block.end = offset + actual;
				memcpy(buffer, (void*)block.buf, length);
				fh->cur_pos += actual;
				return length;
			}
			D(bug(" Err: cache write. got %d/%d\n", actual, length));
			EnterSection(SECTION_TOS);
			Fseek(fh->cur_pos, fh->f, SEEK_SET);
			ExitSection(SECTION_TOS);
		}
		#endif

		EnterSection(SECTION_TOS);
		actual = Fread(fh->f, length, buffer);
		ExitSection(SECTION_TOS);
		if (actual < 0)
		{
			D(bug(" Err: read %d / %d\n", actual, length));
			return 0;
		}
		else
		{
			fh->cur_pos += actual;
			return actual;
		}
	}

	return 0;
}


/*
 *  Write "length" bytes from "buffer" to file/device, starting at "offset",
 *  returns number of bytes written (or 0)
 */

size_t Sys_write(void *arg, void *buffer, loff_t offset, size_t length)
{
	Section s(SECTION_DISK);
	file_handle *fh = (file_handle *)arg;
	if (!fh)
	{
		D(bug("Sys_write error: NULL file handle\n"));
		return 0;
	}

	#if SUPPORT_CACHE
	if (sys_cache_enabled)
	{
		for (uint16 i=0; i<CACHE_BLOCK_COUNT; i++)
			if (cache[i].id == fh->id)
				cache[i].id = 0xFFFF;
	}
	#endif

	DISK_ACCESS();

	// device
	if (!fh->is_file)
	{
		uint16 dev_major = (fh->device & 0xFFFF);
		uint16 dev_minor = ((fh->device >> 16) & 0x7FFF);
		uint32 start = (fh->start_byte + offset) >> 9;
		uint16 count = length >> 9;
		int32 result;

		#ifdef HEAVYDEBUG
		D(bug("Sys_write [%02d] device %d.%d : %d blocks at %d\n", fh->id, dev_major, dev_minor, count, start));
		if ((length & 511) || ((fh->start_byte + offset) & 511))
		{
			D(bug("bad start/length: %d\n", (fh->start_byte + offset), length));
			QuitEmulator();
		}
		#endif

		#if SUPPORT_FLOPPY
		if (dev_major == PUN_FLOPPY)
		{
			const uint32 sectors_per_track = 18;
			const uint32 bytes_per_sector = 512;

			uint16 block_start = start;
			uint16 block_count = count;

			D(bug("floppy write %d,%d (%d, %d)\n", offset, length, block_start, block_count));
			uint8* buf = buffer;
			while(block_count > 0)
			{
				uint16 track = block_start / sectors_per_track;
				uint16 sector = 1 + (block_start - (track * sectors_per_track));
				D(bug("flopwr %d %d\n", sector, track));
				EnterSection(SECTION_TOS);
				int16 result = Flopwr(buf, 0, dev_minor, sector, track, 0, 1);
				ExitSection(SECTION_TOS);
				if (result != 0) {
					D(bug(" fail = %d\n", result));
				}
				buf += bytes_per_sector;
				block_start++;
				block_count--;
			}
			result = 0;
		} else
		#endif
		if (xhdi)
		{
			// xhdi
			struct XHReadWriteParams { uint16 opcode; uint16 major; uint16 minor; uint16 rwflag; uint32 recno; uint16 count; void *buf; };
			XHReadWriteParams p = {10, dev_major, dev_minor, 1, start, count, buffer};
			EnterSection(SECTION_TOS);
			result = xhdi(p);
			ExitSection(SECTION_TOS);
		}
		else if (ahdi)
		{
			// bios
			uint16 dev = 2 + ((dev_major | dev_minor) & PUN_DEV);
			EnterSection(SECTION_TOS);
			result = Rwabs(9, buffer, count, -1, dev, start);
			ExitSection(SECTION_TOS);
		}

		if (result < 0)
		{
			D(bug(" Sys_write: Err %d\n", result));
			return 0;
		}
		return length;
	}

	// disk image file
	else
	{
		uint32 p = offset + fh->start_byte;
		if (fh->cur_pos != p)
		{
			EnterSection(SECTION_TOS);
			int32 result = Fseek(p, fh->f, SEEK_SET);
			ExitSection(SECTION_TOS);
			if (result < 0)
			{
				D(bug("Sys_write [%d] seek error %d -> %d\n", fh->id, fh->cur_pos / 512, p / 512));
				return 0;
			}
			fh->cur_pos = result;
		}

		#ifdef HEAVYDEBUG
		D(bug("Sys_write [%d] file write %d : %d\n", fh->id, p / 512, length / 512));		
		#endif
		EnterSection(SECTION_TOS);
		int32 actual = Fwrite(fh->f, length, buffer);
		ExitSection(SECTION_TOS);
		if (actual != length)
		{
			D(bug(" Err: wrote %d / %d\n", actual, length));
			return 0;
		}
		else
		{
			#if FORCE_FLUSH_WRITES
			{
				EnterSection(SECTION_TOS);
				Fclose(fh->f);
				fh->f = Fopen(fh->fname, fh->read_only ? 0 : 2);
				fh->cur_pos = 0;
				ExitSection(SECTION_TOS);
				if (fh->f < 0)
				{
					D(bug("Fatal: couldn't reopen '%s'\n", fh->fname));
					QuitEmulator();
				}
			}
			#else
			{
				fh->cur_pos += actual;
			}
			#endif
			return actual;
		}
	}

	return 0;
}


/*
 *  Return size of file/device (minus header)
 */

loff_t SysGetFileSize(void *arg)
{
	file_handle *fh = (file_handle *)arg;
	if (!fh)
		return true;

	return fh->size;
}


/*
 *  Eject volume (if applicable)
 */

void SysEject(void *arg)
{
	file_handle *fh = (file_handle *)arg;
	if (!fh)
		return;

	fh->is_ejected = true;
}


/*
 *  Format volume (if applicable)
 */

bool SysFormat(void *arg)
{
	file_handle *fh = (file_handle *)arg;
	if (!fh)
		return false;

	//!!
	return true;
}


/*
 *  Check if file/device is read-only (this includes the read-only flag on Sys_open())
 */

bool SysIsReadOnly(void *arg)
{
	file_handle *fh = (file_handle *)arg;
	if (!fh)
		return true;

	return fh->read_only;
}


/*
 *  Check if the given file handle refers to a fixed or a removable disk
 */

bool SysIsFixedDisk(void *arg)
{
	file_handle *fh = (file_handle *)arg;
	if (!fh)
		return true;

	if (fh->is_file)
		return true;

	return true;
}


/*
 *  Check if a disk is inserted in the drive (always true for files)
 */

bool SysIsDiskInserted(void *arg)
{
	file_handle *fh = (file_handle *)arg;
	if (!fh)
		return false;

	if (fh->is_file)
		return true;

	return true;
}


/*
 *  Prevent medium removal (if applicable)
 */

void SysPreventRemoval(void *arg)
{

}


/*
 *  Allow medium removal (if applicable)
 */

void SysAllowRemoval(void *arg)
{

}


/*
 *  Read CD-ROM TOC (binary MSF format, 804 bytes max.)
 */

bool SysCDReadTOC(void *arg, uint8 *toc)
{
	return false;
}


/*
 *  Read CD-ROM position data (Sub-Q Channel, 16 bytes, see SCSI standard)
 */

bool SysCDGetPosition(void *arg, uint8 *pos)
{
	return false;
}


/*
 *  Play CD audio
 */

bool SysCDPlay(void *arg, uint8 start_m, uint8 start_s, uint8 start_f, uint8 end_m, uint8 end_s, uint8 end_f)
{
	return false;
}


/*
 *  Pause CD audio
 */

bool SysCDPause(void *arg)
{
	return false;
}


/*
 *  Resume paused CD audio
 */

bool SysCDResume(void *arg)
{
	return false;
}


/*
 *  Stop CD audio
 */

bool SysCDStop(void *arg, uint8 lead_out_m, uint8 lead_out_s, uint8 lead_out_f)
{
	return false;
}


/*
 *  Perform CD audio fast-forward/fast-reverse operation starting from specified address
 */

bool SysCDScan(void *arg, uint8 start_m, uint8 start_s, uint8 start_f, bool reverse)
{
	return false;
}


/*
 *  Set CD audio volume (0..255 each channel)
 */

void SysCDSetVolume(void *arg, uint8 left, uint8 right)
{

}


/*
 *  Get CD audio volume (0..255 each channel)
 */

void SysCDGetVolume(void *arg, uint8 &left, uint8 &right)
{

}
