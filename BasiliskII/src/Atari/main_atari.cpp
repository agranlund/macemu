/*
 *  main_atari.cpp - Startup code for Atari
 *
 *  Basilisk II (C) 1997-2008 Christian Bauer
 *  Atari port by Anders Granlund
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
#include "cpu_emulation.h"
#include "main.h"
#include "xpram.h"
#include "timer.h"
#include "sony.h"
#include "disk.h"
#include "cdrom.h"
#include "scsi.h"
#include "audio.h"
#include "video.h"
#include "serial.h"
#include "ether.h"
#include "clip.h"
#include "emul_op.h"
#include "rom_patches.h"
#include "prefs.h"
#include "prefs_editor.h"
#include "sys.h"
#include "user_strings.h"
#include "version.h"
#include "macos_util.h"
#include "adb.h"

#include "video_atari.h"
#include "input_atari.h"
#include "debug_atari.h"
#include "asm_support.h"
#include "zeropage.h"
#include "basilisk.h"

#include "mint/mintbind.h"
#include "mint/cookie.h"
#include "mint/sysvars.h"
#include <string.h>

#ifndef NDEBUG
#include "stdarg.h"
#endif

#define DEBUG 0
#include "debug.h"

#if DEBUG
#define DEBUG_FIXED_MEM			0			// put mac rom / ram in specific locations to ease debugging
#define DEBUG_PERIODIC_TICK		0			// log every second
#endif

#define QUIT_ON_RESET			1			// just quit until we've figure out why reset is crashing


char *strlwr(char *str)
{
	char *p = str;
	if (p) {
		while (*str) {
			char c = *str;
			if (c >= 'A' && c <= 'Z')
				*str = c + 'a' - 'A';
			++str;
		}
	}
	return p;
}

char *strupr(char *str)
{
	char *p = str;
	if (p) {
		while (*str) {
			char c = *str;
			if (c >= 'a' && c <= 'z')
				*str = c + 'A' - 'a';
			++str;
		}
	}
	return p;
}

// Constants
char RSC_FILE_NAME[] = "BASILISK.RSC";
static const char ROM_FILE_NAME[] = "ROM";
static const char __ver[] = "$VER: " VERSION_STRING " " __DATE__;
static const int SCRATCH_MEM_SIZE = (64 * 1024);
static const int HOST_STACK_SIZE = (1024 * 8);

// Global variables
uint8 *ScratchMem = 0;						// Scratch memory for Mac ROM writes
uint8* HostMemChunk = NULL;
uint16 EmulatedSR = 0;
uint32 InterruptFlags = 0;
bool TosIrqSafe = false;

OBJECT* RscMenuPtr = NULL;

int16 aesVersion = 0;
bool isEmuTos = false;
bool isMagic = false;
bool isMint  = false;
bool isFalcon = false;
bool isTT = false;
uint16 tosVersion = 0;
int diskCacheSize = 0;

// CPU and FPU type, addressing mode
int FPUType = 0;
int CPUType = 0;
int HostCPUType = 0;
int HostFPUType = 0;
bool TwentyFourBitAddressing = false;
bool EmulationStarted = false;
bool LogInitOnly = true;

uint32 cpu_cacr_override_tos = 0xFFFFFFFF;
uint32 cpu_cacr_override_mac = 0xFFFFFFFF;
int32 cpu_pcr_override = 0xFFFFFFFF;


// RAM and ROM pointers
uint32 RAMBaseMac;				// RAM base (Mac address space)
uint8 *RAMBaseHost;				// RAM base (host address space)
uint32 RAMSize;					// Size of RAM
uint32 ROMBaseMac;				// ROM base (Mac address space)
uint8 *ROMBaseHost;				// ROM base (host address space)
uint32 ROMSize;					// Size of ROM

extern void timer_atari_set();
extern void timer_atari_tick(suseconds_t micros);

void* operator new(size_t s)
{
	void* m = malloc(s);
	if (m) {
		memset(m, 0, s);
	}
	return m;
}

void* operator new[](size_t s)
{
	void* m = malloc(s);
	if (m) {
		memset(m, 0, s);
	}
	return m;
}

void operator delete(void* m)
{
	free(m);
}

char* astrdup(const char* s)
{
    const char* olds = s;
    size_t len = strlen(olds) + 1;
    char* news = malloc(len);
	if (news) {
		strcpy(news, olds);
	}
	return news;
}

#define MAC_START_STACK_SIZE		(64*1024)
#if MAC_START_STACK_SIZE
uint8 macStack[MAC_START_STACK_SIZE];
#endif

void Start680x0()
{
#if MAC_START_STACK_SIZE
	memset(macStack, 0, MAC_START_STACK_SIZE);
	uint8* stack = macStack + MAC_START_STACK_SIZE - 64;
#else
	uint8* stack = RAMBaseHost + 0x8000;
#endif

	log("Start emulation\n");
	log(" Mac RAM: %08lx (%d)\n", RAMBaseHost, RAMSize / 1024);
	log(" Mac ROM: %08lx (%d)\n", ROMBaseHost, ROMSize / 1024);
	log(" Mac SCR: %08lx (%d)\n", ScratchMem, SCRATCH_MEM_SIZE / 1024);
	log(" Mac SP:  %08lx\n", stack);

	log(" Setting timers...\n");
	timer_atari_set();

	log(" Disabling interrupts...\n");
	DisableInterrupts();

	log(" Setting zero page...\n");
	SetZeroPage(ZEROPAGE_MAC);

	log(" Clearing zero page...\n");
	void* zeroMem = (void*)0x00000000;
	memset(zeroMem, 0x00, 8*1024);

	log(" Init timers...\n");
	InitTimers();

	log(" Flushing caches...\n");
	FlushCache();
	FlushATC();

	if (LogInitOnly) {
		log(" Disabling logging...\n");
		log(" Booting Macintosh...\n");
		RestoreDebug();
	}
	else {
		log(" Booting Macintosh...\n");
	}
	BootMacintosh(stack);
}


void FlushCodeCache(void *start, uint32 size)
{
	FlushCache();
}

bool app_init()
{
	aes_global[0] = 0;
	appl_init();
	aesVersion = aes_global[0];
	if (aesVersion == 0)
		return true;

	if (rsrc_load(RSC_FILE_NAME) == 0)
	{
		strlwr(RSC_FILE_NAME);
		if (rsrc_load(RSC_FILE_NAME) == 0)
		{
			ErrorAlert("basilisk.rsc not found");
			return false;
		}
	}
	return true;
}

void app_exit()
{
	if (aesVersion > 0)
	{
		if (RscMenuPtr)
			menu_bar(RscMenuPtr, 0);
		rsrc_free();
	}
	appl_exit();
}

uint32 GetHexPrefs(const char* id, uint32 def)
{
	uint32 v = def;
	const char* str = PrefsFindString(id);
	if (str)
	{
		v = 0;
		uint16 count = 0;
		while (count < 8)
		{
			uint8 c = *str++;
			if (c >= '0' && c <= '9') {
				v <<= 4; v |= (c - '0');
			} else if (c >='a' && c <= 'f') {
				v <<= 4; v |= (c - 'a');
			} else if (c >= 'A' && c <= 'A') {
				v <<= 4; v |= c - 'A';
			} else {
				count = 8;
			}
			count++;
		}
	}
	return v;
}

// --------------------------------------------------------------------
//
// Main program
//
// --------------------------------------------------------------------
int super_main()
{
	if (!app_init())
		exit(0);

	// Get Atari specs from cookies.
	// todo: figure this out ourselves or at least detect MMU presence.
	uint32 cookie = 0;
	if (Getcookie('_CPU', &cookie) == 0)
		HostCPUType = cookie / 10;
	if (Getcookie('_FPU', &cookie) == 0)
	{
		HostFPUType = (cookie >> 17);
		switch (HostFPUType & 0x3)
		{
			case 1: HostFPUType = 1; break;	// 68881 or 68882
			case 2: HostFPUType = 1; break;	// 68881
			case 3: HostFPUType = 2; break;	// 68882
			default:
			{
				if (HostFPUType & 4)
					HostFPUType = 3;		// 68040 built-in
				else if (HostFPUType & 8)
					HostFPUType = 4;		// 68060 built-in
				else
					HostFPUType = 0;
			}
			break;
		}
	}

	// todo: bail out on unsupported hardware
	if (HostCPUType < 3)
	{
		ErrorAlert("Unsupported CPU|68030+ with MMU required");
		QuitEmulator();
	}

	// get tos info
	OSHEADER* oshdr = (OSHEADER*) *((uint32*)0x000004F2);
	isEmuTos = (strncmp((char*)(((uint32)oshdr->os_beg) + 0x2C), "ETOS", 4) == 0);
	isMagic = (Getcookie('MagX', &cookie) == 0);
	isMint  = (Getcookie('MiNT', &cookie) == 0);
	tosVersion = 0;
	if (isEmuTos) {
	} else {
		tosVersion = oshdr->os_version;
	}

	// Read preferences
	int argc = 0;
	char** argv = 0;
	PrefsInit(0, argc, argv);

	// Setup Mac CPU
	// todo: support 040 emulation on 030 host?
	TwentyFourBitAddressing = false;
	CPUType = HostCPUType;

	// TODO: need 060 FPU patches from Amiga Shapeshifter
	// (frestore has a different stack format on 060)
	if (HostCPUType > 4)
		HostFPUType = 0;

	// MacOS does not understand anything better than 040
	if (CPUType > 4)
		CPUType = 4;

	// pick a default model id based on CPU (MacIICI or Quadra800)
	if (PrefsFindInt32("modelid") <= 0)
		PrefsReplaceInt32("modelid", CPUType >= 4 ? 14 : 5);

	// cpu related overrides
	cpu_cacr_override_tos = GetHexPrefs("cpu_cacr_tos", 0xFFFFFFFF);
	cpu_cacr_override_mac = GetHexPrefs("cpu_cacr_mac", 0xFFFFFFFF);
	cpu_pcr_override = GetHexPrefs("cpu_pcr", 0xFFFFFFFF);

	// early low level init
	InitCPU();
	InitZeroPage();

	// Show preferences editor
	InitDebug();

	if (aesVersion > 0)
	{
		if (!PrefsFindBool("nogui"))
		{
			rsrc_gaddr(R_TREE, MENU_MAIN, &RscMenuPtr);
			menu_bar(RscMenuPtr, 1);
			if (!PrefsEditor())
				QuitEmulator();
		}
		else
		{
			rsrc_gaddr(R_TREE, MENU_DUMMY, &RscMenuPtr);
			menu_bar(RscMenuPtr, 1);
		}
	}

	// Setup Mac FPU
	FPUType = ((HostFPUType != 0) && PrefsFindBool("fpu")) ? 1 : 0;
	// todo: software fpu emulation?

	// Early exit if no rom file was configured
	if (!PrefsFindString("rom",0) || (*PrefsFindString("rom",0) == 0))
	{
		ErrorAlert("ROM file not specified");
		QuitEmulator();
	}

	// Early exit if no boot disk was configured
	bool haveBootDevice = 
		(PrefsFindString("floppy", 0)) ||
		(PrefsFindString("disk", 0)) ||
		(PrefsFindString("cdrom", 0));

	for (uint16 i=0; i<8 && !haveBootDevice; i++)
	{
		char tmp[8];
		sprintf(tmp, "scsi%d", i);
		if (PrefsFindString(tmp, 0))
			haveBootDevice = true;
	}
	if (!haveBootDevice)
	{
		ErrorAlert("Boot disk not specified");
		QuitEmulator();
	}

	// Initialize LineA
	linea0();	// init linea
	lineaa();	// hide mouse

	// We need at least timerC running
	SetSR(0x2300);

	// Initialize variables
	uint32 seed = *((volatile unsigned uint32*)0x4BAL);
	srand(seed);
	RAMBaseHost = 0;
	ROMBaseHost = 0;

	// Init debugging / logging
	InitDebug();

	if (PrefsFindBool("logging_full"))
		LogInitOnly = false;

	// Print some info
	log(GetString(STR_ABOUT_TEXT1), VERSION_MAJOR, VERSION_MINOR);
	log(" %s\n", GetString(STR_ABOUT_TEXT2));

	cookie = 0; Getcookie('_MCH', &cookie);
	isTT = (cookie == 0x00020000);
	isFalcon = ((cookie == 0x00030000) || (cookie == 0x00010100));
	log("Atari: 680%d0, FPU: %02x MCH: %08x", HostCPUType, HostFPUType, cookie);
	if (isEmuTos) {
		log(" TOS: EmuTOS");
	} else {
		log(" TOS: %04x", tosVersion);
	}
	if (isMagic) {
		log(" (MagiC)\n");
	} else if (isMint) {
		log(" (MiNT)\n");
	} else {
		log("\n");
	}
	log("Mac:   680%d0, FPU: %02d MDL: %d\n", CPUType, FPUType, PrefsFindInt32("modelid"));

	TosIrqSafe = PrefsFindBool("irqsafe");

	log("IrqSafe: %s\n", TosIrqSafe ? "Yes" : "No");

	// Open Macintosh ROM
	char rom_path[32] = "ROM";
	if (PrefsFindString("rom"))
		strncpy(rom_path, PrefsFindString("rom"), 31);

	log("Opening ROM: '%s'\n", rom_path);
	FILE* rom_fh = fopen(rom_path, "rb");
	if (!rom_fh)
	{
		// try lowercase
		strlwr(rom_path);
		log("Opening ROM: '%s'\n", rom_path);
		rom_fh = fopen(rom_path, "rb");
		if (!rom_fh)
		{
			// try uppercase
			strupr(rom_path);
			log("Opening ROM: '%s'\n", rom_path);
			rom_fh = fopen(rom_path, "rb");
		}
	}
	if (!rom_fh)
	{
		ErrorAlert(STR_NO_ROM_FILE_ERR);
		QuitEmulator();
	}
	fseek(rom_fh, 0, SEEK_END);
	ROMSize = ftell(rom_fh);
	if (ROMSize != 512*1024 && ROMSize != 1024*1024)
	{
		ErrorAlert(STR_ROM_SIZE_ERR);
		fclose(rom_fh);
		QuitEmulator();
	}
	fseek(rom_fh, 0, SEEK_SET);

	// Disable cpu cache when using 512K ROMs on 68040+
	if ((ROMSize < (1024*1024)) && (HostCPUType >= 4))
	{
		log("Disabling cpu cache (512k ROM on 68040+)\n");
		cpu_cacr_override_tos = 0;
		cpu_cacr_override_mac = 0;
	}

	// Allocate host memory
#if DEBUG_FIXED_MEM
	HostMemChunk = NULL;
	RAMSize		= 1024 * 1024 * 8;
	RAMBaseMac 	= (uint32) 0x02000000;
	ROMBaseMac 	= (uint32) 0x03000000;
	ScratchMem	= (uint8*) (0x03200000 + (SCRATCH_MEM_SIZE >> 1));
	RAMBaseHost = (uint8*) RAMBaseMac;
	ROMBaseHost = (uint8*) ROMBaseMac;
	memset(ROMBaseHost, 0, ROMSize);
	memset(RAMBaseHost, 0, RAMSize);
	memset(ScratchMem, 	0, SCRATCH_MEM_SIZE);
#else

	// Create area for Mac RAM and ROM (ROM must be higher in memory,
	// so we allocate one big chunk and put the ROM at the top of it)

	uint32 freeRam = Mxalloc(-1, 3);
	uint32 freeRamST = Mxalloc(-1, 0);
	uint32 freeRamTT = Mxalloc(-1, 1);
	log("Free ST-RAM: %u\n", freeRamST);
	log("Free TT-RAM: %u\n", freeRamTT);
	log("Free Block:  %u\n", freeRam);

	// round RAM size to 1MB increments unless we are very low on memory then go for 64KB
	static const uint32 ramsizeMask = freeRam < (4 * 1024 * 1024) ? 0xffff0000 : 0xfff00000;
	static const uint32 ramsizeMin = 512*1024L;			// minimum ram to start
	static const uint32 ramsizeMax = 512 * 1024 * 1024;	// maximum Mac ram

	RAMSize = PrefsFindInt32("ramsize");
	if (RAMSize > 0 && RAMSize < (16 * 1024))
	{
		// size was specified as MB
		RAMSize *= (1024*1024);
	}
	else if ((RAMSize == 0) || (RAMSize & 0x80000000))
	{
		// use entire block of ram
		RAMSize = freeRam;
	}
	RAMSize &= ramsizeMask;

	// clamp to mac min/max
	if (RAMSize > ramsizeMax)
		RAMSize = ramsizeMax;
	if (RAMSize < ramsizeMin)
		RAMSize = ramsizeMin;

	// reserve ram for misc stuff
	uint32 reserveMisc = 128 * 1024;

	// reserve for graphics emulation
	bool native;
	uint32 reserveGfx = 0;					// gfx emulation
	AtariScreenInfo(PrefsFindInt32("video_mode"), native, reserveGfx);

	// reserve for disk cache
	if (PrefsFindBool("diskcache") && !isMagic && !isMint)
	{
		diskCacheSize = PrefsFindInt32("diskcacheSize");
		if (diskCacheSize <= 0)
		{
			if (freeRam >= (30 * 1024 * 1024))		diskCacheSize = 4 * 1024 * 1024;
			else if (freeRam >= (14 * 1024 * 1024))	diskCacheSize = 2 * 1024 * 1024;
			else if (freeRam >= (6 * 1024 * 1024))	diskCacheSize = 1 * 1024 * 1024;
			else if (freeRam >= (3 * 1024 * 1024))	diskCacheSize = 128 * 1024;		
			else									diskCacheSize = 0;
		}
	}

	uint32 reserveRam = ROMSize + SCRATCH_MEM_SIZE + reserveMisc + reserveGfx + diskCacheSize;

	log("Mem request: %dKb (%d + %d + %d + %d + %d + %d)\n",
		(RAMSize + reserveRam) / 1024,
		RAMSize / 1024, ROMSize / 1024, SCRATCH_MEM_SIZE / 1024, reserveMisc / 1024, reserveGfx / 1024, diskCacheSize / 1024);

	if ((RAMSize + reserveRam) > freeRam)
	{
		RAMSize = (freeRam > reserveRam) ? ((freeRam - reserveRam) & ramsizeMask) : 0;
		if (RAMSize < ramsizeMin)
		{
			ErrorAlert(STR_NO_MEM_ERR);
			QuitEmulator();
		}
	}

	log("Allocating Host memory (%d + %d + %d Kb)\n", RAMSize/1024, ROMSize/1024, SCRATCH_MEM_SIZE/1024);
	uint32 hostMemSize = RAMSize + ROMSize + SCRATCH_MEM_SIZE + 16;
	HostMemChunk = (uint8 *)Mxalloc(hostMemSize, 3);
	memset(HostMemChunk, 0, hostMemSize);
	log("HostMemChunk: 0x%08x\n", HostMemChunk);
	if (HostMemChunk == NULL)
	{
		ErrorAlert(STR_NO_MEM_ERR);
		QuitEmulator();
	}
	RAMBaseHost = (uint8*) (((uint32)(HostMemChunk + 15)) & ~15L);
	ROMBaseHost = RAMBaseHost + RAMSize;
	ScratchMem = ROMBaseHost + ROMSize + (SCRATCH_MEM_SIZE / 2);	// ScratchMem points to middle of block
	RAMBaseMac = (uint32)RAMBaseHost;
	ROMBaseMac = (uint32)ROMBaseHost;
#endif

	// Load Macintosh ROM
	log("Reading ROM\n");
	if (fread(ROMBaseHost, ROMSize, 1, rom_fh) <= 0)
	{
		ErrorAlert(STR_ROM_FILE_READ_ERR);
		fclose(rom_fh);
		QuitEmulator();
	}
	fclose(rom_fh);

	// Init system routines
	SysInit();

	// Initialize zero page and vectors.
	log("Setup zero page\n");
	if (!SetupZeroPage())
	{
		//ErrorAlert(.....)
		log("InitZeroPage failed\n");
		QuitEmulator();
	}

	// Initialize everything
	log("Init emulation\n");
	if (!InitAll(0))
	{
		log("InitAll failed\n");
		QuitEmulator();
	}

	// Jump to ROM boot routine
	// todo: save SP
	EmulatedSR = 0x2700;
	Start680x0();
	QuitEmulator();
	return 0;
}

static const int ssp_size = (HOST_STACK_SIZE) >> 2;
uint32 s_new_ssp[ssp_size];
uint32 s_old_ssp;
uint32 s_old_usp;

void start_super()
{
   __asm__ __volatile__								\
   (												\
      "												\
      move.l	%0,%%a0;							\
      move.l	%%a0,%%d0;							\
      subq.l	#4,%%d0;							\
      and.w		#-16,%%d0;							\
      move.l	%%d0,%%a0;							\
      move.l	%%sp,-(%%a0);						\
      move.l	%%usp,%%a1;							\
      move.l	%%a1,-(%%a0);						\
      move.l	%%a0,%%sp;							\
      movem.l	%%d1-%%d7/%%a2-%%a6,-(%%sp);		\
      jsr		(%1);								\
      movem.l	(%%sp)+,%%d1-%%d7/%%a2-%%a6;		\
      move.l	(%%sp)+,%%a0;						\
      move.l	%%a0,%%usp;							\
      move.l	(%%sp)+,%%sp;						\
      "												\
      :												\
      : "p"(&s_new_ssp[ssp_size-16]),				\
	  	"a"(&super_main)							\
      : "%%d0", "%%a0", "%%a1", "cc"				\
   );
} 

int main()
{
	Supexec(&start_super);
	//Supexec(&super_main);
	return 0;
}


// --------------------------------------------------------------------
// Quit emulator.
//
//	Note that this can be called from an exception.
// --------------------------------------------------------------------
void QuitEmulator(void)
{
	log("QuitEmulator...\n");

	DisableInterrupts();
	RestoreZeroPage();

	RestoreDebug();
	RestoreTimers();

	FlushCache();
	SetSR(0x2300);
	RestoreInput();

	/*
	// todo: restore stack
	if (HostMemChunk)
	{
		Mfree(HostMemChunk);
	}
	*/

	ExitAll();
	SysExit();
	PrefsExit();
	FlushCache();

	RestoreCPU();

	linea0();
	linea9();

	app_exit();
	exit(0);
}


//--------------------------------------------------------
//  Message dialogs
//--------------------------------------------------------
void ErrorAlert(const char *text)
{
	// show dialog and quit
	if (!PrefsFindBool("nogui") && (aesVersion > 0))
	{
		static char str[1024];

		// suspend emulator
		uint16 sr = DisableInterrupts();
		uint16 zp = SetZeroPage(ZEROPAGE_OLD);
		SetSR(0x2300);
		linea9(); // show mouse

		sprintf(str, "[3][%s|%s][Ok]", GetString(STR_ERROR_ALERT_TITLE), text);
		form_alert(1, str);

		lineaa(); // hide mouse
		DisableInterrupts();
		SetZeroPage(zp);
		SetSR(sr);
	}
	else if (currentZeroPage == ZEROPAGE_OLD)
	{
		printf("ERROR: %s\n", text);
	}
	else
	{
		log("ERROR: %s\n", text);
	}
}

void WarningAlert(const char *text)
{
	if (!PrefsFindBool("nogui") && (aesVersion > 0))
	{
		return;
	}
	else if (currentZeroPage == ZEROPAGE_OLD)
	{
		printf("WARNING: %s\n", text);
	}
	else
	{
		log("WARNING: %s\n");
	}
}

bool ChoiceAlert(const char *text, const char *pos, const char *neg)
{
	return false;
	/*
	char str[256];
	sprintf(str, "%s|%s", pos, neg);
	EasyStruct req;
	req.es_StructSize = sizeof(EasyStruct);
	req.es_Flags = 0;
	req.es_Title = (UBYTE *)GetString(STR_WARNING_ALERT_TITLE);
	req.es_TextFormat = (UBYTE *)GetString(STR_GUI_WARNING_PREFIX);
	req.es_GadgetFormat = (UBYTE *)str;
	return EasyRequest(NULL, &req, NULL, (ULONG)text);
	*/
}



//--------------------------------------------------------
//  Mutexes
//--------------------------------------------------------
struct B2_mutex {
	int dummy;	//!!
};
B2_mutex *B2_create_mutex(void) {
	return new B2_mutex;
}
void B2_lock_mutex(B2_mutex *mutex) {
}
void B2_unlock_mutex(B2_mutex *mutex) {
}
void B2_delete_mutex(B2_mutex *mutex) {
	delete mutex;
}


//--------------------------------------------------------
// Mac interrupts
//--------------------------------------------------------
int16 blockInts = 0;
int16 blockVideoInts = 0;

static inline void SetInterruptFlag(uint32 flag)
{
	InterruptFlags |= flag;
}

static inline void ClearInterruptFlag(uint32 flag)
{
	InterruptFlags &= ~flag;
}

static inline void TriggerInterrupt(void)
{
	if (/*((GetSR() & 0x0700) == 0) &&*/ blockInts == 0)
	{
		M68kRegisters r;
		EmulOp(M68K_EMUL_OP_IRQ, &r);
	}
}

static inline void TriggerNMI(void)
{

}


//----------------------------------------------------------
//
// Exception handler
//
//----------------------------------------------------------
struct trap_regs {	// This must match the layout of M68kRegisters
	uint32 d[8];
	uint32 a[8];
	uint16 sr;
	uint32 pc;
	uint16 format;
};


#if DEBUG && 0
static void dumpRegs(trap_regs* r)
{
	D(bug(	"d0 %08lx d1 %08lx d2 %08lx d3 %08lx\n"
			"d4 %08lx d5 %08lx d6 %08lx d7 %08lx\n"
			"a0 %08lx a1 %08lx a2 %08lx a3 %08lx\n"
			"a4 %08lx a5 %08lx a6 %08lx a7 %08lx\n"
			"sr %04x vbr %08x\n",
			r->d[0], r->d[1], r->d[2], r->d[3], r->d[4], r->d[5], r->d[6], r->d[7],
			r->a[0], r->a[1], r->a[2], r->a[3], r->a[4], r->a[5], r->a[6], r->a[7],
			r->sr, (uint32)ZPState[currentZeroPage].vbr));
}
#endif

extern "C" uint16 VecMacExceptionC(trap_regs *r)
{
	uint16 vec = r->format & 0x0FFF;

#if 0
	D(bug(	"Vec 0x%08x, PC 0x%08x\n", vec, r->pc));
	D(dumpRegs(r));
#endif

	if (vec == 0x10)
	{
		uint16 opcode = *(uint16*)(r->pc);
#if 1
		if ((opcode & 0xff00) != 0x7100)
			return 0;
#elif 0
		if ((opcode & 0xff00) != 0x7100)
		{
			D(bug("Illegal Instruction %04x at %08lx\n", *(uint16 *)(r->pc), r->pc));
			D(dumpRegs(r));
			QuitEmulator();
		}
#endif

		uint16 sr = GetSR();
		blockInts++;
		#if EMULOP_DEBUG
			D(bug("emulop: SR = %04x (%04x)\n", r->sr, sr));
		#endif
		EnterSection(SECTION_MAC_EMUOP);
		EmulOp(opcode, (M68kRegisters*)r);
		ExitSection(SECTION_MAC_EMUOP);
		r->pc += 2;
		if (opcode == M68K_EMUL_OP_RESET)
		{
			D(bug("EmulOp RESET\n"));
			// reset, keep interrupts disabled
			blockInts--;
			r->sr = 0x2700;
			#if QUIT_ON_RESET
			{
				static bool firstReset = true;
				if (!firstReset)
					QuitEmulator();
				firstReset = false;
			}
			#endif
		}
		else
		{
			SetSR(sr);
			blockInts--;
		}
		return 1;
	}
	
#if LINEA_DEBUG
	else if (vec == 0x28)
	{
		uint16 opcode = *(uint16*)(r->pc);
		#if 1
			D(bug("LineA: 0x%04x\n", opcode));
		#else
			D(bug("LineA: 0x%04x : %s\n", opcode, GetLineaFuncname(opcode)));
		#endif
		SetSR(r->sr);
		return 0;
	}
#endif

#if 0
	else
	{
		D(bug("Unhandled vector 0x%08x at 0x%08x\n", vec, r->pc));
		D(dumpRegs(r));
		QuitEmulator();
	}
#endif

    // let original vector deal with it
	return 0;
}



//----------------------------------------------------------
//
// Atari interrupts
//
//----------------------------------------------------------
#define HIRES_TIMER_THRESHOLD	17
#define IRQ_TIMERB				1
#define IRQ_TIMERC				2
#define tck1000					(( 49 * 1000000) / (2457600 /  50))
#define tck200 					((192 * 1000000) / (2457600 /  64))
#define tck60					((205 * 1000000) / (2457600 / 200))

#define COUNT_MISSED_VBLS		1

static int16 irqActive = 0;				// timer b/c irq is active
static uint16 missedVbls = 0;			// ignored vbls

extern tm_time_t globalTimerTime;		// timer_atari.cpp
extern tm_time_t wakeup_time;			// timer.cpp

static inline bool IsTimerExpired()
{
	if (timer_cmp_time(wakeup_time, globalTimerTime) <= 0)
	{
		D(bug("ite: wakeup = [%d:%d] (%x:%d)\n", wakeup_time.tv_sec, wakeup_time.tv_usec, wakeup_time.tv_sec, wakeup_time.tv_usec));
		D(bug("ite: global = [%d:%d] (%x:%d)\n", globalTimerTime.tv_sec, globalTimerTime.tv_usec, globalTimerTime.tv_sec, globalTimerTime.tv_usec));
		D(bug("TIMER EXPIRED: %d\n", wakeup_time.tv_sec - globalTimerTime.tv_sec));
		return true;
	}
	return false;
}

static inline bool IsHiresTimerEnabled()
{
	return (ZPState[ZEROPAGE_MAC].tcen != 0);
}

static inline void EnableHiresTimer()
{
	ZPState[ZEROPAGE_MAC].tcen = TIMERC_MASK_ENABLE;
	*TIMERC_REG_ENABLE = (*TIMERC_REG_ENABLE & ~TIMERC_MASK_ENABLE) | ZPState[ZEROPAGE_MAC].tcen;
}

static inline void DisableHiresTimer()
{
	*TIMERC_REG_ENABLE = (*TIMERC_REG_ENABLE & ~TIMERC_MASK_ENABLE);
	ZPState[ZEROPAGE_MAC].tcen = 0;
}

static inline bool HiresTimerNeeded()
{
	int32 sec = wakeup_time.tv_sec - globalTimerTime.tv_sec;
	if (sec > 1)
		return false;
	if (sec < 0)
		return true;
#if 1
	#define TIMERMUL1000(x) ((x)<<10)
	#define TIMERDIV1000(x) ((x)>>10)
#else
	#define TIMERMUL1000(x) ((x)*1000)
	#define TIMERDIV1000(x) ((x)/1000)
#endif
	int32 msec = TIMERMUL1000(sec);
	msec += TIMERDIV1000((int32)wakeup_time.tv_usec - (int32)globalTimerTime.tv_usec);
	D(bug("ms = %d [%d] [%d , %d]\n", msec, wakeup_time.tv_sec - globalTimerTime.tv_sec, wakeup_time.tv_usec, globalTimerTime.tv_usec));
	return (msec < HIRES_TIMER_THRESHOLD);
}


//----------------------------------------------------------
// Mac hires timer (100hz)
//----------------------------------------------------------
extern "C" void IntTimerCMac()	// 1000hz
{
	timer_atari_tick(tck1000);
	if (irqActive || blockInts) {
		*TIMERC_REG_SERVICE = ~TIMERC_MASK_ENABLE;
		return;
	}
	if (IsTimerExpired()) {
		irqActive = IRQ_TIMERC;							// busy
		//*TIMERB_REG_MASK &= ~TIMERB_MASK_ENABLE;		// pause 60hz interrupts
		//*TIMERC_REG_ENABLE &= ~TIMERC_MASK_ENABLE;	// disable 1000hz interrupts
		*TIMERC_REG_SERVICE = ~TIMERC_MASK_ENABLE;		// unblock mfp
		SetSR(0x2500);									// enable mfp interrupts
		TimerInterrupt();								// mac timer interrupt
		// compensate for timers with even higher resolution
		if (wakeup_time.tv_sec == globalTimerTime.tv_sec) {
			if (wakeup_time.tv_usec < globalTimerTime.tv_usec)
				wakeup_time.tv_usec = globalTimerTime.tv_usec;
		}
		irqActive = 0;									// not busy
		//*TIMERB_REG_MASK |= TIMERB_MASK_ENABLE;			// resume 60hz interrupts
		//*TIMERC_REG_ENABLE |= TIMERC_MASK_ENABLE;		// enable 1000hz interrupts
		return;
	}
	*TIMERC_REG_SERVICE = ~TIMERC_MASK_ENABLE;
	return;
}

//----------------------------------------------------------
// Mac system timer (60hz)
//----------------------------------------------------------
extern "C" void IntTimerBMac()
{
	#define cntXpramInterval 	60			// check+save xpram every minute
	#define cnt1hzInterval 		60			// 1hz interrupt every 60th 60hz interrupt
	static int16 cntXpram = 10;				// perform first xpram save earlier
	static int16 cnt1hz = cnt1hzInterval;

	// no reentrant past this point
	if (irqActive)
	{
		if (!IsHiresTimerEnabled())
			timer_atari_tick(tck60);
		#if COUNT_MISSED_VBLS
		missedVbls++;
		#endif
		*TIMERB_REG_SERVICE = ~TIMERB_MASK_ENABLE;
		return;	
	}
	
	#if COUNT_MISSED_VBLS
	uint16 vbls = 1 + missedVbls;
	missedVbls = 0;
	#endif

	// allow mfp interrupts to interrupt us
	irqActive = IRQ_TIMERB;
	*TIMERB_REG_SERVICE = ~TIMERB_MASK_ENABLE;
	SetSR(0x2500);

	blockInts++;
	uint32 irq = INTFLAG_AUDIO | INTFLAG_60HZ;

	// Mac timer interrupt
	if (!IsHiresTimerEnabled()) {
		timer_atari_tick(tck60);
		if (IsTimerExpired()) {
			TimerInterrupt();
			if (HiresTimerNeeded()) {
				D(bug("Enabling hires timer\n"));
				EnableHiresTimer();
			}
		}
	} else {
		if (!HiresTimerNeeded()) {
			D(bug("Disabling hires timer\n"));
			DisableHiresTimer();
		}
	}

	// Mac 1hz interrupt
	cnt1hz--;
	if (cnt1hz == 0)
	{
		cnt1hz = cnt1hzInterval;
		uint32 tdt = TimerDateTime();
		WriteMacInt32(0x20c, tdt);
		irq |= INTFLAG_1HZ;

		#if DEBUG_PERIODIC_TICK
		uint32 adsi = audio_data + adatStreamInfo;
		D(bug("tick 1hz [%u, %u, %d, %d, cacr:%08x, a=%08x %08x %08x %d]\n", tdt, ReadMacInt32(0x16a), blockInts, section[SECTION_MAC_EMUOP], GetCACR(),
			audio_data, *((uint32*)(audio_data + adatStreamInfo)), AudioStatus.mixer, AudioStatus.num_sources));
		#endif			

		// save xpram to disk every now and again
		cntXpram--;
		if (cntXpram == 0)
		{
			cntXpram = cntXpramInterval;
			static uint32 last_xpram[XPRAM_SIZE/4];
			if (!IsSection(SECTION_TOS) && !IsSection(SECTION_DISK)) {
				volatile uint32* s = (volatile uint32*)XPRAM;
				uint32* d = (uint32*)last_xpram;
				for (uint16 i=0; i<XPRAM_SIZE/4; i++) {
					uint32 sd = *s++; uint32 dd = *d;
					if (sd == dd) {
						d++;
					} else {
						D(bug("diff at %d (%08x -> %08x)\n", i, sd, dd));
						*d++ = sd;
						for (i++;i<XPRAM_SIZE/4;i++) {
							*d++ = *s++;
						}
						SaveXPRAM();
					}
				}
				D(bug("xpram diff = %d\n", memcmp(XPRAM, last_xpram, XPRAM_SIZE)));
			}
		}
	}

#if COUNT_MISSED_VBLS
	// count ignored vbls (TriggerInterrupt increments by 1)
	WriteMacInt32(0x16a, ReadMacInt32(0x16a) + (vbls - 1));
#endif

	// trigger interrupts
	if (blockVideoInts)
	{
		SetInterruptFlag(irq);
		blockInts--;
		TriggerInterrupt();
	}
	else
	{
		UpdateInput();
		SetInterruptFlag(irq);
		blockInts--;
		TriggerInterrupt();
		RequestInput();
		irqActive = 0;
		AtariScreenUpdate();
	}

	irqActive = 0;
}


//----------------------------------------------------------
// Tos hires timer (200hz)
//----------------------------------------------------------
extern "C" void IntTimerCTos()
{
	// mac system timer
	timer_atari_tick(tck200);
	// tos 200hz counter
	*((volatile uint32*)0x04BA) += 1;
	*TIMERC_REG_SERVICE = ~TIMERC_MASK_ENABLE;
	return;
}

//----------------------------------------------------------
// Tos system timer (60hz)
//----------------------------------------------------------
extern "C" void IntTimerBTos()
{
	if (irqActive || TosIrqSafe)
	{
		#if COUNT_MISSED_VBLS
		missedVbls++;
		#endif
		*TIMERB_REG_SERVICE = ~TIMERB_MASK_ENABLE;
		return;
	}

	// limited vblank to update mouse cursor
	irqActive = IRQ_TIMERB;
	*TIMERB_REG_SERVICE = ~TIMERB_MASK_ENABLE;
	SetSR(0x2500);
	blockInts++;
	UpdateInput();
	SetZeroPage(ZEROPAGE_MAC);
	WriteMacInt32(0x16a, ReadMacInt32(0x16a) + 1);
	if (HasMacStarted())
	{
		AudioInterrupt();
		ADBInterrupt();
		//VideoInterrupt();
		//if (ROMVersion == ROM_VERSION_32)
		{
			M68kRegisters r2;
			r2.d[0] = 0;
			Execute68kTrap(0xa072, &r2);
		}
	}
	AtariScreenUpdate();
	SetZeroPage(ZEROPAGE_TOS);
	RequestInput();
	blockInts--;
	irqActive = 0;
	return;
}
