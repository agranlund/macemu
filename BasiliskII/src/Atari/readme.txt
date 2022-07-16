
  Basilisk II
  A 68k Macintosh emulator

  Copyright (C) 1997-2008 Christian Bauer et al.
  http://basilisk.cebix.net/

  Atari port by by Anders Granlund
  http://www.happydaze.se

  With help and/or contributions by:
  EmuTOS, Hatari, FreeMiNT, Motorola, Vincent Rivière, mfro, stephen_usher,
  derkom, Badwolf, SWE/YesCREW, Odd Skancke, Miro Kropáček,
  and everyone else who has helped testing this.
 
  Version: 220716
  
--------------------
License
--------------------
Basilisk II is available under the terms of the GNU General Public License.
See license.txt for details.


--------------------
Features
--------------------
- Native code execution with low emulation overhead
- Native graphics on Graphics card or Atari mono.
- Emulated graphics on most Atari screen modes
- Sound through YM/DMA/LPT/Cartridge
- Disk images, shared folder, direct hdd access


--------------------
Requirements
--------------------
- Atari ST/STe/TT/Falcon
- 68030 or better. MMU is required, FPU is optional.
- 4MB+ RAM minimum (8MB+ for non-native graphics)
- A hard drive
- A MacintoshII ROM dump


--------------------
Recommended settings
--------------------
- 68030 : modelid 5,  Macintosh LC-III rom
- 68040 : modelid 14, Macintosh Quadra rom
- 68060 : modelid 14, Macintosh Quadra rom

These are general recommendations, you may have better or worse luck
using roms from other Macintosh models.

(68030)
If you are low on memory then these 512K roms may work:
MacII-CI (requires an FPU)
MacII-SI (works with or without FPU)

(68040-60)
I am unsure which Quadra rom is best, try one from a 650/700/800/900.
Some 68030 roms may work but I recommend using a proper 68040 rom.


--------------------
Graphics
--------------------
Native:
  MacOS can render directly and without emulation to:
    - ST and TT mono resolutions
    - Graphics card with a Mac compatible display mode
      (256 color mode is the safest bet)

Emulated:
  BasiliskII will emulate graphics when the Atari display mode
  is not directly compatible with MacOS.
  This is *much* slower than native graphics.

  You need at least 8MB RAM to use emulated graphics.

  BasiliskII can translate the following display modes:

   Atari 4bit  <-> Mac 4bit + 8bit(*)
   Atari 8bit  <-> Mac 8bit
   Atari 16bit <-> Mac 16bit + 8bit(*)
   Other 16bit <-> Mac 16bit

   (*) These 8bit modes are even slower.
       You'll get the best performance when Atari and MacOS
       bit depths are matching.
     
  Special cases:
    - ST-Low and TT-Low will emulate a 640x480x256 color Mac display.
      Press the UNDO key to toggle viewport zoom.

    - A graphics card set to an incompatible 16bit mode will use emulation
      to correct the color channels.
      This is obviously a lot slower than native rendering but
      colors will be correct.
  

--------------------
Sound
--------------------
Sound is only supported for Sound Manager 3.x
(And at the moment, there are issues with Sound Manager 3.2.1 and later)

System 7.0:
  Install QuickTime 2.1 + Sound Manager 3.1

System 7.1:
  Install System Update 3.
  (or install QuickTime 2.1 + Sound Manager 3.1)

System 7.5:
  Comes with Sound Manager 3.2 and should work out of the box.

MacOS 8.0:
  Comes with Sound Manager 3.2.1 built in, which currently does not work.


A note on QuickTime:
 QuickTime 2.5+ will upgrade your system to Sound Manager 3.2.1+
 and thus stop sound from working at the moment.
 In this case you could try downgrading from Sound Manager that came
 with QuickTime to a compatible version.


--------------------
Performance help
--------------------
* Sound is quite expensive on Ataris that don't have DMA.
  Try lowering the quality. You can do so in basilisk.inf
  or from the Sound prefs in the MacOS control Panel.

* Try an older version of MacOS with lower system requirements

* Emulated graphics is very expensive.
  You may want to experiment with different Atari screen modes
  or use Mono (or a graphics card) which doesn't have
  any emulation overhead.

  Non-matching bit-depth are even more expensive.
  For example, 8bit MacOS on 4bit Atari is much slower than
  4bit MacOS on 4bit Atari (or even 8bit MacOS on 8bit Atari).
  You can normally change MacOS mode from "Monitor" prefs
  in the control panel.
  
  You can try with a higher frameskip option.

  Emulated graphics is optimized for fastram and you're not
  going to have a good experience if you only have ST-RAM.

* Graphics card and 16bit
  Most Atari graphics cards are not going to match the pixel format
  that MacOS wants.
  If this happens then emulation is going to kick in and it will
  be slow. 8bit mode is generally a safer bet for graphics cards.

* Disk image access is much faster in MiNT or MagiC compared
  to when running plain TOS or EmuTOS.


--------------------
Configuring
--------------------

basilisk.inf contains some settings you may want to modify.
At the very least to specify the rom file and to add one or
more disks/cds to the Mac.

nogui <enable>
  Set to true to disable the startup configuration GUI.

rom <filename>
  This item specifies the file name of the Mac ROM file to be used by
  Basilisk II. If no "rom" line is given, the ROM file has to be named
  "ROM" and put in the same directory as the Basilisk II executable.

modelid <MacOS model ID>
  Specifies the Macintosh model ID that Basilisk II should report to MacOS.
  The default is "5" which corresponds to a Mac IIci.
  If you want to run MacOS 8, you have to set this to "14" (Quadra 900).

ramsize <bytes>
  Allocate "bytes" bytes of RAM for MacOS system and application memory.
  Set it to 0 to give it all available Atari memory.

fpu <enable>
  Enable fpu if available
  0 = don't use fpu, 1 = use fpu if available

nosound <enable>
  Set to true to disable virtual Mac sound hardware.

sound_driver <driver>
  0=Silent, 1=DMA, 2=YM, 3=Covox, 4=MV16, 5=Replay8, 6=Replay8S, 7=Replay16

sound_freq <hz>
  Wanted playback frequency. It does not have to be exact,
  you'll get the closest matching frequency supported by the sound driver.

sound_channels <number of channels>
  Number of channels (1 for mono, 2 for stereo)
  Sound driver may choose another format if your setting is not supported.

sound_bits <bits per sample>
  Wanted bits per sample (8 or 16)
  Sound driver may choose another format if your setting is not supported.

mouse_speed <speed>
  Speed of Mac mouse cursor. Default is 8.

video_emu <enable>
  Enable video emulation support

video_mmu <enable>
  Enable video emulation MMU acceleration

frameskip <value>
  Frameskip when using emulated video.

bootdrive <drive number>
  Specify MacOS drive number of boot volume. "0" (the default) means
  "boot from first bootable volume".

bootdriver <driver number>
  Specify MacOS driver number of boot volume. "0" (the default) means
  "boot from first bootable volume". Use "-62" to boot from CD-ROM.

floppy <floppy drive description>
  This item describes one floppy drive to be used by Basilisk II. There
  can be multiple "floppy" lines in the preferences file.
  The format of "floppy drive description" is the same as for "disk" lines.

cdrom <CD-ROM drive description>
  This item describes one CD-ROM drive to be used by Basilisk II. There
  can be multiple "cdrom" lines in the preferences file.
  The format of "CD-ROM drive description" is the same as for "disk" lines.

extfs <folder>
  This item describes an Atari folder which should be shared to MacOS.
  There can be only one shared folder.
  This feature requires MiNT or MagiC and the folder should be on a
  partition that has long filenames enabled.
  The folder will be littered with many (hidden) files so I recommend
  using a dedicated folder for this purpose rather than sharing
  something like your home folder.

diskcache <enable>
  Enable disk image cache

diskdevmode <mode>
  Enables direct hdd feature, default is 3
    0 = disabled
    1 = enabled in AHDI mode only
    2 = enabled in XHDI mode only
    3 = enabled in XHDI and AHDI mode (XHDI preferred)

disk <volume description>
  This item describes one MacOS volume to be mounted by Basilisk II.
  There can be multiple "disk" lines in the preferences file. Basilisk II
  can handle hardfiles (byte-per-byte images of HFS volumes in a file on
  the host system) or access an Atari device directly.
  The "volume description" is either the pathname of a hardfile or a
  platform-dependant description of an HFS partition or drive.
  If the volume description is prefixed by an asterisk ("*"),
  the volume is write protected for MacOS.

  Basilisk II can also handle some types of Mac "disk image" files directly,
  as long as they are uncompressed and unencoded.


  ----------
  Disk image
  ----------
  The easiest, and safest, is to use disk images stored on an Atari partition.
  Disk images can be very slow, depending on TOS and driver.
  They are generally quite ok in MiNT and MagiC, but slow in Atari TOS
  and extremely slow in EmuTOS.

  The path can be absolute or relative.
  Example:
    "disk hdd0.dsk"
    "disk c:\basilisk\disks\hdd0.dsk"


  -----------------
  Direct HDD access
  -----------------

  The following two options will let the Mac access a harddrive directly.
  This is generally faster than using disk images.

  **** USE WITH CAUTION ****
  Unless you are confident in what you are doing this could destroy your data.

  Backup your existing drive(s) before trying this.
  To limit the risk of destroying Atari partitions, use a different drive, and
  if possible a different device entierly for you Mac stuff. At least until:
   a) You are comfortable with Atari devices, partitions and so on.
   b) BasiliskII is more mature. It cannot be guaranteed there are no
      potentially fatal bugs in the system. It has had limited testing
      with an Ultrasatan, HDDriver and EmuTOS built-in driver.

  The device must have a physical block size of 512 bytes.
  This is usually the case, but if in doubt, double check using some software
  that can display this information. For example HDDrutil from HDDriver.

  You hard disk driver must be either XHDI or AHDI 3.0 compliant.
  I recommend HDDriver or EmuTOS built in drivers.
  The utility that comes with HDDriver is great for partitioning or
  just generally for retrieving information about devices and partitions.


  -------------
  RAW partition
  -------------
  This is the easier and safer of the two direct-access options.
  Your hard disk driver must be capable of recognizing and assigning
  driver letters to RAW partitions.

  Usage;
   disk raw:<drive letter>
   or
   disk raw:<drive letter>:<start block>:<size>

   "start block" and "size" are given in blocks and are completely optional.
   "start block" is the starting offset inside the partition.
   "size" is how many blocks it should occupy.
   If these are not specified it will use the entire partition.

  Examples:
    "disk raw:g"
      Let Mac use the entire partition G: as a harddrive

    "disk raw:g:1024:4096"
      Let mac use blocks 1024 to 1024+4096 on G: as a harddrive

  You can assign several Mac drives from the same RAW parition
  by using the start+size option. Or just make multiple partitions.

  Special note on AHDI:
   BasiliskII will not be able to automatically determine the
   size of the last RAW partion on a device when AHDI is used.
   Use the optional parameters to specify the size manually.


  --------------------
  Direct device access
  --------------------
  Same as above but you are not restricted to partitions in any way.
  You can assign any physical location on a device as a Mac disk with this.
  Use with extra caution.

  Usage:
    disk dev:<major>.<minor>:<start block>:<size>
       "major" is the Major device ID
       "minor" is the Minor device ID
       "start block" is the block where the Mac disk should start
       "size" is the size of the Mac disk, given in blocks.

  Example:
     "disk dev:0.0:2:4096"
       Let Mac use blocks 2 to 2+4096 on device 0.0 as a hard drive.

  You can assign several Mac drives on the same device, just take care
  with the start and size so that they don't overlap.

  The device id is specified in XHDI major/minor format but you can still
  use it through AHDI.
  The Major device ID's are: 0 = ACSI, 8 = SCSI, 16 = IDE
  And the minor ID is the device number on that particular bus.

