/*
 *  xpram_atari.cpp - XPRAM handling, Atari implementation
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
#include "xpram.h"
#include "zeropage.h"

#define DEBUG 0
#include "debug.h"


// XPRAM file name and path
char XPRAM_FILE_NAME[] = "XPRAM";

/*
 *  Load XPRAM from settings file
 */

void LoadXPRAM(const char *vmdir)
{
	// called from init before emulation start	
	TOS_CONTEXT();
	D(bug("LoadXPRAM: '%s'\n", XPRAM_FILE_NAME));
	int32 f = Fopen(XPRAM_FILE_NAME, 0);
	if (f < 0)
	{
		strlwr(XPRAM_FILE_NAME);
		D(bug("LoadXPRAM: '%s'\n", XPRAM_FILE_NAME));
		f = Fopen(XPRAM_FILE_NAME, 0);
	}
	if (f >= 0)
	{
		Fread(f, 256, XPRAM);
		Fclose(f);
		D(bug(" Loaded\n"));
	}
	D(bug("LoadXPRAM end\n"));
}


/*
 *  Save XPRAM to settings file
 */

void SaveXPRAM(void)
{
	// called from 1hz irq while emulator is running
	if (IsSection(SECTION_TOS) || IsSection(SECTION_DISK))
		return;

	D(bug("SaveXPRAM '%s'\n", XPRAM_FILE_NAME));
	{
		TOS_CONTEXT();

		EnterSection(SECTION_TOS);
		EnterSection(SECTION_DISK);
		int32 f = Fcreate(XPRAM_FILE_NAME, 0);
		if (f < 0)
		{
			ExitSection(SECTION_DISK);
			ExitSection(SECTION_TOS);
			D(bug(" Err: (%d) failed creating file '%s'\n", f, XPRAM_FILE_NAME))
			return;
		}
		int32 result = Fwrite(f, 256, XPRAM);
		Fclose(f);
		if (result != 256)
		{
			Fdelete(XPRAM_FILE_NAME);
			ExitSection(SECTION_DISK);
			ExitSection(SECTION_TOS);
			D(bug(" Err: failed writing file '%s'\n", XPRAM_FILE_NAME));
		}
		else
		{
			ExitSection(SECTION_DISK);
			ExitSection(SECTION_TOS);
		}
	}

	D(bug("SaveXPRAM end\n"));
}


/*
 *  Delete PRAM file
 */

void ZapPRAM(void)
{
	// not called. could be called from prefs editor.
	TOS_CONTEXT();
	D(bug("ZapXPRAM begin\n"));
	char fname[16];
	strcpy(fname, XPRAM_FILE_NAME);
	strupr(fname);
	Fdelete(fname);
	strlwr(fname);
	Fdelete(fname);
	D(bug("ZapXPRAM end\n"));
}
