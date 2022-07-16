/*
 *  prefs_atari.cpp - Preferences handling, Atari implementation
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prefs.h"



// Platform-specific preferences items
prefs_desc platform_prefs_items[] = {
	PREFS_ITEM("_disk",			TYPE_STRING,	true,  "Disabled disk"),
	PREFS_ITEM("_extfs",		TYPE_STRING,	true,  "Disabled extfs"),
	PREFS_ITEM("diskdevmode",	TYPE_INT32,		false, "Device access mode"),
	PREFS_ITEM("diskcache", 	TYPE_BOOLEAN, 	false, "Disk image cache"),
	PREFS_ITEM("diskcachesize",	TYPE_INT32, 	false, "Disk image cache size"),
	PREFS_ITEM("logging", 		TYPE_STRING, 	false, "Debug log output"),
	PREFS_ITEM("logging_full", 	TYPE_BOOLEAN, 	false, "Log always"),
	PREFS_ITEM("irqsafe", 		TYPE_BOOLEAN, 	false, "Allow Mac VBL during TOS calls"),
	PREFS_ITEM("sound_driver",	TYPE_INT32, 	false, "Atari sound driver"),
	PREFS_ITEM("sound_freq",	TYPE_INT32, 	false, "Default sound frequency"),
	PREFS_ITEM("sound_channels",TYPE_INT32, 	false, "Default num channels"),
	PREFS_ITEM("sound_bits",	TYPE_INT32, 	false, "Default sample size"),
	PREFS_ITEM("mouse_speed",	TYPE_INT32,		false, "Mouse speed"),
	PREFS_ITEM("video_mode",	TYPE_INT32,		false, "Video mode"),
	PREFS_ITEM("video_emu",		TYPE_BOOLEAN,	false, "Enable emulation"),
	PREFS_ITEM("video_mmu",		TYPE_BOOLEAN,	false, "MMU acceleration"),
	PREFS_ITEM("video_cmp",		TYPE_BOOLEAN,	false, "CMP acceleration"),
	PREFS_ITEM("video_debug",	TYPE_INT32,		false, "Video debugging"),
	PREFS_ITEM("cpu_cacr_tos",	TYPE_STRING,	false, "Force CACR for TOS"),
	PREFS_ITEM("cpu_cacr_mac",	TYPE_STRING,	false, "Force CACR for MAC"),
	PREFS_ITEM("cpu_pcr",		TYPE_STRING,	false, "Force 68060 PCR"),
	{NULL, TYPE_END, false, NULL} // End of list
};

// Prefs file name and path
char PREFS_FILE_NAME[16] = "BASILISK.INF";


/*
 *  Load preferences from settings file
 */

void LoadPrefs(const char* vmdir)
{
	// Read preferences from settings file
	FILE *f = fopen(PREFS_FILE_NAME, "r");
	if (f == NULL)
	{
		strlwr(PREFS_FILE_NAME);
		f = fopen(PREFS_FILE_NAME, "r");
	}

	if (f != NULL)
	{
		// Prefs file found, load settings
		LoadPrefsFromStream(f);
		fclose(f);
	}
	else
	{
		// No prefs file, save defaults
		SavePrefs();
	}
}


/*
 *  Save preferences to settings file
 */

void SavePrefs(void)
{
	FILE*f = fopen(PREFS_FILE_NAME, "w");
	if (f != NULL)
	{
		SavePrefsToStream(f);
		fclose(f);
	}
}


/*
 *  Add defaults of platform-specific prefs items
 *  You may also override the defaults set in PrefsInit()
 */

void AddPlatformPrefsDefaults(void)
{
	// new defaults for atari
	PrefsReplaceInt32("frameskip", 1);		// frameskip for emulated graphics
	PrefsReplaceInt32("ramsize", 0);		// autodetect
	PrefsReplaceInt32("modelid", 0);		// autodetect
	PrefsReplaceBool("fpu", true);			// use fpu if available
	PrefsReplaceBool("nosound", false);		// sound enabled
	PrefsReplaceBool("nogui", false);		// gui enabled

	uint32 cookie = 0;
	Getcookie('_SND', &cookie);
	if (cookie & 2)
	{
		PrefsAddInt32("sound_driver", 1);		// DMA sound
		PrefsAddInt32("sound_freq", 22050);		// 22050hz by default
		PrefsAddInt32("sound_channels", 1);		// mono
		PrefsAddInt32("sound_bits", 8);			// 8bit
	}
	else
	{
		PrefsAddInt32("sound_driver", 2);		// YM sound
		PrefsAddInt32("sound_freq", 11025);		// 11025hz by default
		PrefsAddInt32("sound_channels", 1);		// mono
		PrefsAddInt32("sound_bits", 8);			// 8bit
	}

	PrefsAddString("logging", "file");		// log mode
	PrefsAddBool("logging_full", false);	// log init only
	PrefsAddBool("diskcache", true);		// disk cache enabled
	PrefsAddBool("irqsafe", false);			// tos irq workaround disabled

	PrefsAddInt32("diskdevmode", 3);		// device access enabled

	PrefsAddInt32("mouse_speed", 8);		// mouse speed

	PrefsAddBool("video_emu", true);		// enable emulation
	PrefsAddBool("video_mmu", true);		// mmu acceleration enabled
	PrefsAddBool("video_mode", 0);			// use desktop gfxmode
	PrefsAddInt32("video_debug", 0);		// debug options
}
