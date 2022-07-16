/*
 *  prefs_items.cpp - Common preferences items
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

#include "sys.h"
#include "prefs.h"


// Common preferences items (those which exist on all platforms)
// Except for "disk", "floppy", "cdrom", "scsiX", "screen", "rom" and "ether",
// these are guaranteed to be in the prefs.

prefs_desc common_prefs_items[] = {
	PREFS_ITEM("disk", TYPE_STRING, true,       "device/file name of Mac volume"),
	PREFS_ITEM("floppy", TYPE_STRING, true,     "device/file name of Mac floppy drive"),
	PREFS_ITEM("cdrom", TYPE_STRING, true,      "device/file names of Mac CD-ROM drive"),
	PREFS_ITEM("extfs", TYPE_STRING, false,     "root path of ExtFS"),
	PREFS_ITEM("seriala", TYPE_STRING, false,   "device name of Mac serial port A"),
	PREFS_ITEM("serialb", TYPE_STRING, false,   "device name of Mac serial port B"),
#ifndef ATARI
	PREFS_ITEM("scsi0", TYPE_STRING, false,     "SCSI target for Mac SCSI ID 0"),
	PREFS_ITEM("scsi1", TYPE_STRING, false,     "SCSI target for Mac SCSI ID 1"),
	PREFS_ITEM("scsi2", TYPE_STRING, false,     "SCSI target for Mac SCSI ID 2"),
	PREFS_ITEM("scsi3", TYPE_STRING, false,     "SCSI target for Mac SCSI ID 3"),
	PREFS_ITEM("scsi4", TYPE_STRING, false,     "SCSI target for Mac SCSI ID 4"),
	PREFS_ITEM("scsi5", TYPE_STRING, false,     "SCSI target for Mac SCSI ID 5"),
	PREFS_ITEM("scsi6", TYPE_STRING, false,     "SCSI target for Mac SCSI ID 6"),
	PREFS_ITEM("screen", TYPE_STRING, false,    "video mode"),
	PREFS_ITEM("ether", TYPE_STRING, false,     "device name of Mac ethernet adapter"),
	PREFS_ITEM("etherconfig", TYPE_STRING, false,"path of network config script"),
	PREFS_ITEM("udptunnel", TYPE_BOOLEAN, false, "tunnel all network packets over UDP"),
	PREFS_ITEM("udpport", TYPE_INT32, false,    "IP port number for tunneling"),
#endif
	PREFS_ITEM("rom", TYPE_STRING, false,       "path of ROM file"),
	PREFS_ITEM("bootdrive", TYPE_INT32, false,  "boot drive number"),
	PREFS_ITEM("bootdriver", TYPE_INT32, false, "boot driver number"),
	PREFS_ITEM("ramsize", TYPE_INT32, false,    "size of Mac RAM in bytes"),
	PREFS_ITEM("frameskip", TYPE_INT32, false,  "number of frames to skip in refreshed video modes"),
	PREFS_ITEM("modelid", TYPE_INT32, false,    "Mac Model ID (Gestalt Model ID minus 6)"),
	PREFS_ITEM("fpu", TYPE_BOOLEAN, false,      "enable FPU emulation"),
	PREFS_ITEM("nocdrom", TYPE_BOOLEAN, false,  "don't install CD-ROM driver"),
	PREFS_ITEM("nosound", TYPE_BOOLEAN, false,  "don't enable sound output"),
	PREFS_ITEM("nogui", TYPE_BOOLEAN, false,    "disable GUI"),
	PREFS_ITEM("keyboardtype", TYPE_INT32, false, "hardware keyboard type"),
	PREFS_ITEM("mousewheelmode", TYPE_INT32, false, "mouse wheel support (0=page up/down, 1=cursor up/down)"),
	PREFS_ITEM("mousewheellines", TYPE_INT32, false, "number of lines to scroll in mouse wheel mode 1"),
	PREFS_ITEM("yearofs", TYPE_INT32, 0,			"year offset"),
	PREFS_ITEM("dayofs", TYPE_INT32, 0,			"day offset"),
	PREFS_ITEM("sound_buffer", TYPE_INT32, false,	"sound buffer length"),
#ifndef ATARI
	PREFS_ITEM("redir", TYPE_STRING, true,      "port forwarding for slirp"),
	PREFS_ITEM("host_domain", TYPE_STRING, true,	"handle DNS requests for this domain on the host (slirp only)"),
	PREFS_ITEM("cpu", TYPE_INT32, false,        "CPU type (0 = 68000, 1 = 68010 etc.)"),
	PREFS_ITEM("displaycolordepth", TYPE_INT32, false, "display color depth"),
	PREFS_ITEM("noclipconversion", TYPE_BOOLEAN, false, "don't convert clipboard contents"),
	PREFS_ITEM("jit", TYPE_BOOLEAN, false,         "enable JIT compiler"),
	PREFS_ITEM("jitfpu", TYPE_BOOLEAN, false,      "enable JIT compilation of FPU instructions"),
	PREFS_ITEM("jitdebug", TYPE_BOOLEAN, false,    "enable JIT debugger (requires mon builtin)"),
	PREFS_ITEM("jitcachesize", TYPE_INT32, false,  "translation cache size in KB"),
	PREFS_ITEM("jitlazyflush", TYPE_BOOLEAN, false, "enable lazy invalidation of translation cache"),
	PREFS_ITEM("jitinline", TYPE_BOOLEAN, false,   "enable translation through constant jumps"),
	PREFS_ITEM("jitblacklist", TYPE_STRING, false, "blacklist opcodes from translation"),
	PREFS_ITEM("keycodes", TYPE_BOOLEAN, false, "use keycodes rather than keysyms to decode keyboard"),
	PREFS_ITEM("keycodefile", TYPE_STRING, false, "path of keycode translation file"),
	PREFS_ITEM("hotkey",TYPE_INT32,false,"hotkey modifier"),
	PREFS_ITEM("scale_nearest",TYPE_BOOLEAN,false,"nearest neighbor scaling"),
	PREFS_ITEM("scale_integer",TYPE_BOOLEAN,false,"integer scaling"),
	PREFS_ITEM("mag_rate", TYPE_INT32, 0,			"rate of magnification"),
	PREFS_ITEM("gammaramp", TYPE_STRING, false,	"gamma ramp (on, off or fullscreen)"),
	PREFS_ITEM("swap_opt_cmd", TYPE_BOOLEAN, false,	"swap option and command key"),
	PREFS_ITEM("ignoresegv", TYPE_BOOLEAN, false,    "ignore illegal memory accesses"),
	PREFS_ITEM("name_encoding", TYPE_INT32, false,	"file name encoding"),
	PREFS_ITEM("title", TYPE_STRING, false,	"window title"),
	PREFS_ITEM("delay", TYPE_INT32, false,	"additional delay [uS] every 64k instructions"),
#endif
	{NULL, TYPE_END, false, NULL} // End of list
};


/*
 *  Set default values for preferences items
 */

void AddPrefsDefaults(void)
{
	SysAddSerialPrefs();
	PrefsAddBool("udptunnel", false);
	PrefsAddInt32("udpport", 6066);
	PrefsAddInt32("bootdriver", 0);
	PrefsAddInt32("bootdrive", 0);
	PrefsAddInt32("ramsize", 8 * 1024 * 1024);
	PrefsAddInt32("frameskip", 6);
	PrefsAddInt32("modelid", 5);	// Mac IIci
	PrefsAddInt32("cpu", 3);		// 68030
	PrefsAddInt32("displaycolordepth", 0);
	PrefsAddBool("fpu", false);
	PrefsAddBool("nocdrom", false);
	PrefsAddBool("nosound", false);
	PrefsAddBool("noclipconversion", false);
	PrefsAddBool("nogui", false);
	
#if USE_JIT
	// JIT compiler specific options
//	PrefsAddBool("jit", true);
	PrefsAddBool("jitfpu", true);
	PrefsAddBool("jitdebug", false);
	PrefsAddInt32("jitcachesize", 8192);
	PrefsAddBool("jitlazyflush", true);
	PrefsAddBool("jitinline", true);
#else
	PrefsAddBool("jit", false);
#endif

    PrefsAddInt32("keyboardtype", 5);

#ifdef __APPLE__
	PrefsAddBool("swap_opt_cmd", false);
#else
	PrefsAddBool("swap_opt_cmd", true);
#endif
	PrefsAddBool("ignoresegv", true);
}
