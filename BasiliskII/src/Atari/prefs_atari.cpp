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
	{"_disk",			TYPE_STRING,	true,  "Disabled disk"},
	{"diskdevmode",		TYPE_INT32,		false, "Device access mode"},
	{"diskcache", 		TYPE_BOOLEAN, 	false, "Disk image cache"},
	{"diskcachesize",	TYPE_INT32, 	false, "Disk image cache size"},
	{"logging", 		TYPE_STRING, 	false, "Debug log output"},
	{"logging_full", 	TYPE_BOOLEAN, 	false, "Log always"},
	{"irqsafe", 		TYPE_BOOLEAN, 	false, "Allow Mac VBL during TOS calls"},
	{"sound_driver",	TYPE_INT32, 	false, "Atari sound driver"},
	{"sound_freq",		TYPE_INT32, 	false, "Default sound frequency"},
	{"sound_channels",	TYPE_INT32, 	false, "Default num channels"},
	{"sound_bits",		TYPE_INT32, 	false, "Default sample size"},
	{"mouse_speed",		TYPE_INT32,		false, "Mouse speed"},
	{"video_mode",		TYPE_INT32,		false, "Video mode"},
	{"video_emu",		TYPE_BOOLEAN,	false, "Enable emulation"},
	{"video_mmu",		TYPE_BOOLEAN,	false, "MMU acceleration"},
	{"video_cmp",		TYPE_BOOLEAN,	false, "CMP acceleration"},
	{"video_debug",		TYPE_INT32,		false, "Video debugging"},
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
	// remove common default prefs that aren't useful for us
	PrefsRemoveItem("udptunnel");
	PrefsRemoveItem("udpport");
	PrefsRemoveItem("frameskip");
	PrefsRemoveItem("displaycolordepth");
	PrefsRemoveItem("noclipconversion");
	PrefsRemoveItem("jit");
	PrefsRemoveItem("swap_opt_cmd");
	PrefsRemoveItem("ignoresegv");
	PrefsRemoveItem("cpu");

	// new defaults for atari
	PrefsReplaceInt32("ramsize", 0);		// autodetect
	PrefsReplaceInt32("modelid", 0);		// autodetect
	PrefsReplaceBool("fpu", true);			// use fpu if available
	PrefsReplaceBool("nosound", false);		// sound enabled
	PrefsReplaceBool("nogui", false);		// gui enabled

	PrefsAddInt32("sound_driver", 1);		// DMA sound
	PrefsAddInt32("sound_freq", 22050);		// 22050hz by default
	PrefsAddInt32("sound_channels", 1);		// mono
	PrefsAddInt32("sound_bits", 8);			// 8bit

	PrefsAddString("logging", "file");		// log mode
	PrefsAddBool("logging_full", false);	// log init only
	PrefsAddBool("diskcache", true);		// disk cache enabled
	PrefsAddBool("irqsafe", false);			// tos irq workaround disabled

	PrefsAddInt32("diskdevmode", 3);		// device access enabled

	PrefsAddInt32("mouse_speed", 8);		// mouse speed

	PrefsAddBool("video_emu", true);		// enable emulation
	PrefsAddBool("video_mmu", true);		// mmu acceleration enabled
	PrefsAddBool("video_cmp", false);		// hash acceleration disabled
	PrefsAddBool("video_mode", 0);			// use desktop gfxmode
	PrefsAddInt32("video_debug", 0);		// debug options
	//PrefsReplaceInt32("frameskip", 2);
}
