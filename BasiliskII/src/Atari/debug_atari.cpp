/*
 *  debug_atari.cpp - debug support
 *
 *  Basilisk II (C), 1997-2008 Christian Bauer
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option), any later version.
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
#include "debug_atari.h"
#include "zeropage.h"
#include "prefs.h"
#include "mint/mintbind.h"

#define DEBUG 0
#include "debug.h"

#define DEBUG_ATARI_ENABLED             !NDEBUG
#define DEBUG_ATARI_SERIAL_FAST         1           // output with 307200 baud

#define LOGGING_MODE_OFF                0
#define LOGGING_MODE_FILE               1
#define LOGGING_MODE_SERIAL             2
#define LOGGING_MODE_SCREEN             3
static int16 loggingMode = LOGGING_MODE_OFF;

#define LOGGING_FILENAME "basilisk.log"


void InitDebug()
{
#if DEBUG_ATARI_ENABLED
    const char* loggingModeStr = PrefsFindString("logging", 0);
    if (loggingModeStr == NULL)
        return;

    int16 newLoggingMode = LOGGING_MODE_OFF;    
    if (strcmp(loggingModeStr, "serial") == 0)
        newLoggingMode = LOGGING_MODE_SERIAL;
    else if (strcmp(loggingModeStr, "file") == 0)
        newLoggingMode = LOGGING_MODE_FILE;
    else if (strcmp(loggingModeStr, "screen") == 0)
        newLoggingMode = LOGGING_MODE_SCREEN;

    if (loggingMode != newLoggingMode)
    {
        RestoreDebug();

        if (newLoggingMode == LOGGING_MODE_FILE)
        {
            int16 fh = Fcreate(LOGGING_FILENAME, 0);
            Fwrite(fh, 2, "\n\r");
            Fclose(fh);
        }
        else if (newLoggingMode == LOGGING_MODE_SERIAL)
        {
            #if DEBUG_ATARI_SERIAL_FAST
            // Configure serial port to 19200.8.N.1
            uint32 cnf = Rsconf(-1, -1, -1, -1, -1, -1);
            uint16 ucr = 0x81 & (cnf >> 24);
            ucr |= (0 << 1);    // parity type
            ucr |= (0 << 2);    // no parity
            ucr |= (1 << 3);    // 1 stop bit
            ucr |= (0 << 5);    // 8 bits
            uint16 bau = 0;     // 19200 baud
            uint16 ctr = 0;     // no flow control
            Rsconf(bau, ctr, ucr, -1, -1, -1);
            // remove timer-D divider for 307200 baud
            *((volatile uint8*)0x00FFFA29) &= 0x7F;
            #endif
            // redirect standard output to serial port
            Fforce(2, Fopen("AUX:", 1));
            Fforce(1, Fdup(2));
        }
    }
    
    loggingMode = newLoggingMode;
#endif
}

void RestoreDebug()
{
#if DEBUG_ATARI_ENABLED
    // todo: restore
    if (loggingMode == LOGGING_MODE_FILE)
    {
    }
    else if (loggingMode == LOGGING_MODE_SERIAL)
    {
        #if DEBUG_ATARI_SERIAL_FAST
        // todo: restore serial port config and timer-D
        #endif
        // todo: restore output redirection
    }
    loggingMode = LOGGING_MODE_OFF;
#endif
}

void aprintf_do(const char* buf, uint32 len)
{
    if (len == 0)
        return;

    EnterSection(SECTION_TOS);
    {
        if (loggingMode == LOGGING_MODE_FILE)
        {
            int16 fh = Fopen(LOGGING_FILENAME, 2);
            if (fh >= 0)
            {
                Fseek(0, fh, SEEK_END);
                Fwrite(fh, len, buf);
                Fclose(fh);
            }
        }
        else
        {
            Fwrite(1, len, buf);
        }
    }
    ExitSection(SECTION_TOS);
}

void aprintf(const char *fmt, ...)
{
#if DEBUG_ATARI_ENABLED
    static char buf[256];
    if (loggingMode != LOGGING_MODE_OFF)
    {
        uint16 sr = DisableInterrupts();
        uint16 zp = GetZeroPage();
        bool safeToPrint = !IsSection(SECTION_TOS) && !IsSection(SECTION_DEBUG);
        if (!safeToPrint)
        {
            // todo: buffer and print later...
            SetSR(sr);
            return;
        }

        EnterSection(SECTION_DEBUG);
        {
            // format
            va_list va;
            va_start(va, fmt);
            vsnprintf(buf, 253, fmt, va);
            va_end(va);
            uint32 len = strlen(buf);
            if (buf[0] <= 0)
                len = 0;
            if ((len > 0) && (buf[len-1] == '\n'))
            {
                buf[len++] = '\r';
                buf[len] = 0;
            }

            if (zp == ZEROPAGE_MAC)
            {
                TOS_CONTEXT();
                aprintf_do(buf, len);
            }
            else
            {
                SetSR(0x2500);
                aprintf_do(buf, len);
            }
        }
        ExitSection(SECTION_DEBUG);
        SetSR(sr);
    }
#endif
}


#if LINEA_DEBUG

struct LineaInfo {
    uint16 code;
    const char* name;
};

#define LINEAINFO(code, name) { code, name }

LineaInfo lineaInfos[]= {
    // SysTraps.txt
    LINEAINFO(0xA000+0, "open"),
    LINEAINFO(0xA000+1, "close"),
    LINEAINFO(0xA000+2, "read"),
    LINEAINFO(0xA000+3, "write"),
    LINEAINFO(0xA000+4, "control"),
    LINEAINFO(0xA000+5, "status"),
    LINEAINFO(0xA000+6, "killio"),
    LINEAINFO(0xA000+7, "GetVolInfo"),
    LINEAINFO(0xA000+8, "Create"),
    LINEAINFO(0xA000+9, "Delete"),
    LINEAINFO(0xA000+10, "OpenRF"),
    LINEAINFO(0xA000+11, "ReName"),
    LINEAINFO(0xA000+12, "GetFileInfo"),
    LINEAINFO(0xA000+13, "SetFileInfo"),
    LINEAINFO(0xA000+14, "UnMountVol"),
    LINEAINFO(0xA000+15, "MountVol"),
    LINEAINFO(0xA000+16, "Allocate"),
    LINEAINFO(0xA000+17, "GetEOF"),
    LINEAINFO(0xA000+18, "SetEOF"),
    LINEAINFO(0xA000+19, "FlushVol"),
    LINEAINFO(0xA000+20, "GetVol"),
    LINEAINFO(0xA000+21, "SetVol"),
    LINEAINFO(0xA000+22, "FInitQueue"),
    LINEAINFO(0xA000+23, "Eject"),
    LINEAINFO(0xA000+24, "GetFPos"),
    LINEAINFO(0xA000+25, "InitZone"),
    LINEAINFO(0xA000+26, "GetZone"),
    LINEAINFO(0xA000+27, "SetZone"),
    LINEAINFO(0xA000+28, "FreeMem"),
    LINEAINFO(0xA000+29, "MaxMem"),
    LINEAINFO(0xA000+30, "NewPtr"),
    LINEAINFO(0xA000+31, "DisposPtr"),
    LINEAINFO(0xA000+32, "SetPtrSize"),
    LINEAINFO(0xA000+33, "GetPtrSize"),
    LINEAINFO(0xA000+34, "NewHandle"),
    LINEAINFO(0xA000+35, "DisposHandle"),
    LINEAINFO(0xA000+36, "SetHandleSize"),
    LINEAINFO(0xA000+37, "GetHandleSize"),
    LINEAINFO(0xA000+38, "HandleZone"),
    LINEAINFO(0xA000+39, "ReAllocHandle"),
    LINEAINFO(0xA000+40, "RecoverHGandle"),
    LINEAINFO(0xA000+41, "HLock"),
    LINEAINFO(0xA000+42, "HUnlock"),
    LINEAINFO(0xA000+43, "EmptyHandle"),
    LINEAINFO(0xA000+44, "InitApplZone"),
    LINEAINFO(0xA000+45, "SetApplLimit"),
    LINEAINFO(0xA000+46, "BlockMove"),
    LINEAINFO(0xA000+47, "PostEvent"),
    LINEAINFO(0xA000+48, "OSEventAvail"),
    LINEAINFO(0xA000+49, "GetOSEvent"),
    LINEAINFO(0xA000+50, "FlushEvents"),
    LINEAINFO(0xA000+51, "VInstall"),
    LINEAINFO(0xA000+52, "VRemove"),
    LINEAINFO(0xA000+53, "OffLine"),
    LINEAINFO(0xA000+54, "MoreMasters"),
    LINEAINFO(0xA000+56, "WriteParam"),
    LINEAINFO(0xA000+57, "ReadDateTime"),
    LINEAINFO(0xA000+58, "SetDateTime"),
    LINEAINFO(0xA000+59, "Delay"),
    LINEAINFO(0xA000+60, "CmpString"),
    LINEAINFO(0xA000+61, "DrvrInstall"),
    LINEAINFO(0xA000+62, "DrvrRemove"),
    LINEAINFO(0xA000+63, "InitUtil"),
    LINEAINFO(0xA000+64, "ResrvMem"),
    LINEAINFO(0xA000+65, "SetFilLock"),
    LINEAINFO(0xA000+66, "RstFilLock"),
    LINEAINFO(0xA000+67, "SetFilType"),
    LINEAINFO(0xA000+68, "SetFPos"),
    LINEAINFO(0xA000+69, "FlushFile"),
    LINEAINFO(0xA000+70, "GetTrapAddress"),
    LINEAINFO(0xA000+71, "SetTrapAddress"),
    LINEAINFO(0xA000+72, "PtrZone"),
    LINEAINFO(0xA000+73, "HPurge"),
    LINEAINFO(0xA000+74, "HNoPurge"),
    LINEAINFO(0xA000+75, "SetGrowZone"),
    LINEAINFO(0xA000+76, "CompactMem"),
    LINEAINFO(0xA000+77, "PurgeMem"),
    LINEAINFO(0xA000+78, "AddDrive"),
    LINEAINFO(0xA000+79, "RDrvrInstall"),
    LINEAINFO(0xA000+84, "UprString"),
    LINEAINFO(0xA000+87, "SetAppBase"),
 
    // QuickTraps.txt
    // ....
 
    // ToolTraps.txt
    // ....
    LINEAINFO(0xA992, "DetachResource"),
    LINEAINFO(0xA993, "SetResPurge"),
    LINEAINFO(0xA994, "CurResFile"),
    LINEAINFO(0xA995, "InitResources"),
    LINEAINFO(0xA996, "RsrcZoneInit"),
    LINEAINFO(0xA997, "OpenResFile"),
    LINEAINFO(0xA998, "UseResFile"),
    LINEAINFO(0xA999, "UpdateResFile"),
    LINEAINFO(0xA99A, "CloseResFile"),
    LINEAINFO(0xA99B, "SetResLoad"),
    LINEAINFO(0xA99C, "CountResource"),
    LINEAINFO(0xA99D, "GetIndResource"),
    LINEAINFO(0xA99E, "CountTypes"),
    LINEAINFO(0xA99F, "GetIndType"),
    LINEAINFO(0xA9A0, "GetResource"),
    LINEAINFO(0xA9A1, "GetNamedResource"),
    LINEAINFO(0xA9A2, "LoadResource"),
    LINEAINFO(0xA9A3, "ReleaseResource"),
    LINEAINFO(0xA9A4, "HomeResFile"),
    LINEAINFO(0xA9A5, "SizeRsrc"),
    LINEAINFO(0xA9A6, "GetResAttrs"),
    LINEAINFO(0xA9A7, "SetResAttrs"),
    LINEAINFO(0xA9A8, "GetResInfo"),
    LINEAINFO(0xA9A9, "SetResInfo"),
    LINEAINFO(0xA9AA, "ChangedResource"),
    LINEAINFO(0xA9AB, "AddResource"),
    LINEAINFO(0xA9AD, "RmveResource"),
    LINEAINFO(0xA9AF, "ResError"),
    LINEAINFO(0xA9B0, "WriteResource"),
    LINEAINFO(0xA9B1, "CreateResFile"),
    LINEAINFO(0xA9B2, "SystemEvent"),
    LINEAINFO(0xA9B3, "SystemClick"),
    LINEAINFO(0xA9B4, "SystemTask"),
    LINEAINFO(0xA9B5, "SystemMenu"),
    // ....

};


const char* GetLineaFuncname(uint16 code)
{
    const uint16 numFuncs = sizeof(lineaInfos) / sizeof(LineaInfo);
    for (uint16 i = 0; i<numFuncs; i++)
        if (code == lineaInfos[i].code)
            return lineaInfos[i].name;
    return "unknown";
}

#endif // LINEA_DEBUG
