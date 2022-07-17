/*
 *  zeropage.cpp - handles access to memory region at 0x0000 - 0x2000
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
#include "emul_op.h"
#include "zeropage.h"
#include "asm_support.h"
#include "debug_atari.h"

#define DEBUG 0
#include "debug.h"

extern void pmmu_68030_init(void);
extern "C" void setup_68040_pmmu(void);

#define CACR_OVERRIDE       1
#define CACR_RESPECT_MAC    0
#define NO_TOS_AUTOVECTOR   0
#define SUPPORT_ALT_TOSVBR  1
#define SUPPORT_FPEMU       1

ZeroPageState ZPState[3];
volatile uint16 currentZeroPage = ZEROPAGE_OLD;
volatile int16 section[NUM_SECTIONS] = { 0 };

uint32  vbrTableTos[256];
uint32  vbrTableMac[256];

#if SUPPORT_ALT_TOSVBR
uint16  vecDirectJumpTableMac[256*3];
uint16  vecDirectJumpTableTos[256*4];
#else
uint16  vecDirectJumpTable[256*3];
#endif

__LINEA *__aline;
__FONT  **__fonts;
short  (**__funcs) (void);

struct MFPTimer
{
    uint8 en;
    uint8 mask;
    uint8 ctrl;
    uint8 data;
};
MFPTimer mfpTimerOld[4];
bool timersInited = false;

extern bool isEmuTos;
extern bool isMagic;
extern bool isMint;
extern bool isTT;
extern uint16 tosVersion;
extern uint32 ROMSize;

extern uint32 cpu_cacr_override_tos;
extern uint32 cpu_cacr_override_mac;
extern uint32 cpu_pcr_override;

/*
68060
11.1.2.3 CONTEXT SWITCH INTERRUPT HANDLERS. Context switch interrupt handlers
that use the same virtual address to map into multiple physical address locations must flush
the branch cache via the MOVEC to CACR instruction. The reason for this is that the branch
cache is a logical cache and not a physical cache. For systems that transparently translate
logical addresses to physical addresses, the branch cache need not be flushed.

The branch cache must be cleared by the operating system on all context switches (using
the MOVEC to CACR instruction), because it is virtually-mapped.

The branch cache is automatically cleared by the hardware as part of any cache invalidate
(CINV) or any cache push and invalidate (CPUSH) instruction operating on the instruction
cache.

// CACR:  31  30  29  28  27  26  24 23  22	  21   20      16 15  14  13  12	      0
//        EDC NAD ESB DPI FOC 0 0 0  EBC CABC CUBC 0 0 0 0 0  EIC NAI FIC 0000000000000

    EDC = Enable data cache
    NAD = No allocate Mode (data cache)
    ESB = Enable store buffer
    DPI = Disable CPUSH invalidation
    FOC = 1/2 cache operation enable (data cache)
    EBC = Enable branch cache
    CABC = Clear all entries in branch cache
    CUBC = Clear user entries in branch cache
    EIC = Enable instruction cache
    NAI = No allocate mode (instruction cache)
    FIC = 1/2 cache operation enable (instruction cache)
*/

extern "C" uint16 SetZeroPage(uint16 page)
{
    uint16 sr = DisableInterrupts();
    if (page == currentZeroPage)
    {
        SetSR(sr);
        return page;
    }
    SetMMU(&ZPState[page].mmu);
    SetVBR(ZPState[page].vbr);
    *TIMERC_REG_DATA = ZPState[page].tcdata;
    *TIMERC_REG_CTRL = ((~TIMERC_MASK_CTRL) & (*TIMERC_REG_CTRL)) | ZPState[page].tcctrl;
    *TIMERC_REG_ENABLE = (*TIMERC_REG_ENABLE & ~TIMERC_MASK_ENABLE) | ZPState[page].tcen;

#if CACR_OVERRIDE
    #if CACR_RESPECT_MAC
        // update cacr for mac context
        if (currentZeroPage == ZEROPAGE_MAC) {
            ZPState[ZEROPAGE_MAC].cacr = GetCACR();
        }
    #endif
    SetCACR(ZPState[page].cacr);
#else
    FlushCache();
#endif
    uint16 prevZeroPage = currentZeroPage;
    currentZeroPage = page;

#if NO_TOS_AUTOVECTOR
    static uint16 savedSR = 0x2600;
    if (prevZeroPage == ZEROPAGE_MAC) {
        savedSR = sr;
        sr = 0x2600;
    } else if (currentZeroPage == ZEROPAGE_MAC) {
        sr = savedSR;
    }
#endif

    SetSR(sr);
    return prevZeroPage;
}

static char* debugReg(uint32 reg, char* regstring)
{
    uint8 b = *((volatile uint8*)reg);
    regstring[0] = (b & (1 << 7)) ? '1' : '0';
    regstring[1] = (b & (1 << 6)) ? '1' : '0';
    regstring[2] = (b & (1 << 5)) ? '1' : '0';
    regstring[3] = (b & (1 << 4)) ? '1' : '0';
    regstring[4] = (b & (1 << 3)) ? '1' : '0';
    regstring[5] = (b & (1 << 2)) ? '1' : '0';
    regstring[6] = (b & (1 << 1)) ? '1' : '0';
    regstring[7] = (b & (1 << 0)) ? '1' : '0';
    regstring[8] = 0;
    return regstring;
}

static void debugMfpRegs(const char* title)
{
    char rs[9*4];
    log(" %s MFP1:\n", title);
    for (uint32 r = 0xFFFA01; r <= 0xFFFA25; r+=8) {
        log("   %08x: %s %s %s %s\n", r, debugReg(r,&rs[9*0]), debugReg(r+2,&rs[9*1]), debugReg(r+4,&rs[9*2]), debugReg(r+6,&rs[9*3]));
    }
    if (isTT) {
        log(" %s MFP2\n", title);
        for (uint32 r = 0xFFFA81; r <= 0xFFFAA5; r+=8) {
            log("   %08x: %s %s %s %s\n", r, debugReg(r,&rs[9*0]), debugReg(r+2,&rs[9*1]), debugReg(r+4,&rs[9*2]), debugReg(r+6,&rs[9*3]));
        }
    }
}

void InitTimers()
{
    if (timersInited)
        return;

    // log mfp registers
    debugMfpRegs("Old");

    // backup timer settings
    mfpTimerOld[0].en = TIMERA_MASK_ENABLE & (*TIMERA_REG_ENABLE);
    mfpTimerOld[0].mask = TIMERA_MASK_ENABLE & (*TIMERA_REG_MASK);
    mfpTimerOld[0].ctrl = TIMERA_MASK_CTRL & (*TIMERA_REG_CTRL);
    mfpTimerOld[0].data = *TIMERA_REG_DATA;
    mfpTimerOld[1].en = TIMERB_MASK_ENABLE & (*TIMERB_REG_ENABLE);
    mfpTimerOld[1].mask = TIMERB_MASK_ENABLE & (*TIMERB_REG_MASK);
    mfpTimerOld[1].ctrl = TIMERB_MASK_CTRL & (*TIMERB_REG_CTRL);
    mfpTimerOld[1].data = *TIMERB_REG_DATA;
    mfpTimerOld[2].en = TIMERC_MASK_ENABLE & (*TIMERC_REG_ENABLE);
    mfpTimerOld[2].mask = TIMERC_MASK_ENABLE & (*TIMERC_REG_MASK);
    mfpTimerOld[2].ctrl = TIMERC_MASK_CTRL & (*TIMERC_REG_CTRL);
    mfpTimerOld[2].data = *TIMERC_REG_DATA;
    mfpTimerOld[3].en = TIMERD_MASK_ENABLE & (*TIMERD_REG_ENABLE);
    mfpTimerOld[3].mask = TIMERD_MASK_ENABLE & (*TIMERD_REG_MASK);
    mfpTimerOld[3].ctrl = TIMERD_MASK_CTRL & (*TIMERD_REG_CTRL);
    mfpTimerOld[3].data = *TIMERD_REG_DATA;

    // timer-A
    {
        // audio_atari.cpp installs this timer if needed
        *TIMERA_REG_ENABLE &= ~TIMERA_MASK_ENABLE;
        *TIMERA_REG_PENDING &= ~TIMERA_MASK_ENABLE;
        *TIMERA_REG_SERVICE &= ~TIMERA_MASK_ENABLE;
    }

    // timer-B
    {
        *TIMERB_REG_ENABLE &= ~TIMERB_MASK_ENABLE;
        *TIMERB_REG_PENDING &= ~TIMERB_MASK_ENABLE;
        *TIMERB_REG_SERVICE &= ~TIMERB_MASK_ENABLE;
        SetMacVector(TIMERB_VECTOR, VecMacTimerB);
        SetTosVector(TIMERB_VECTOR, VecTosTimerB);
        const unsigned char ctrl = (7 << TIMERB_SHIFT_CTRL);    // divide base clock by 200
        const unsigned char data = 205;                         // 59.9 hz ((2457600 / (200 * 205))
        *TIMERB_REG_CTRL = ((~TIMERB_MASK_CTRL) & (*TIMERB_REG_CTRL)) | ctrl;
        *TIMERB_REG_DATA = data;
        *TIMERB_REG_ENABLE |= TIMERB_MASK_ENABLE;
        *TIMERB_REG_MASK |= TIMERB_MASK_ENABLE;
    }

    // timer-C (tos system timer)
    {
        // used as emulation "thread"
        *TIMERC_REG_ENABLE &= ~TIMERC_MASK_ENABLE;
        *TIMERC_REG_PENDING &= ~TIMERC_MASK_ENABLE;
        *TIMERC_REG_SERVICE &= ~TIMERC_MASK_ENABLE;
        // params to restore 200hz (2457600 / (64 * 192))
        mfpTimerOld[2].en = TIMERC_MASK_ENABLE;
        mfpTimerOld[2].ctrl = (5 << TIMERC_SHIFT_CTRL);
        mfpTimerOld[2].data = 192;
        // new vectors
        SetMacVector(TIMERC_VECTOR, VecMacTimerC);
        SetTosVector(TIMERC_VECTOR, VecTosTimerC);
        ZPState[ZEROPAGE_MAC].tcen = 0;
        ZPState[ZEROPAGE_MAC].tcctrl = (4 << TIMERC_SHIFT_CTRL);    // divide base clock by 50
        ZPState[ZEROPAGE_MAC].tcdata = 49;                          // 1003hz ((2457600 / (50 * 49))
        // new params
        const unsigned char ctrl = (5 << TIMERC_SHIFT_CTRL);    // divide base clock by 64
        const unsigned char data = 192;                         // 200 hz ((2457600 / (64 * 192))
        //const unsigned char ctrl = (4 << TIMERC_SHIFT_CTRL);    // divide base clock by 50
        //const unsigned char data = 49;                          // 1003hz ((2457600 / (50 * 49))
        *TIMERC_REG_CTRL = ((~TIMERC_MASK_CTRL) & (*TIMERC_REG_CTRL)) | ctrl;
        *TIMERC_REG_DATA = data;
        *TIMERC_REG_ENABLE |= TIMERC_MASK_ENABLE;
        *TIMERC_REG_MASK |= TIMERC_MASK_ENABLE;
    }

    // timer-D (baud rate generator)
    {
        // used by debug_atari.cpp, serial_atari.cpp
        //*TIMERD_REG_ENABLE &= ~TIMERD_MASK_ENABLE;
        //*TIMERD_REG_PENDING &= ~TIMERD_MASK_ENABLE;
        //*TIMERD_REG_SERVICE &= ~TIMERD_MASK_ENABLE;
    }

    // log mfp registers
    debugMfpRegs("New");


    timersInited = true;
}

void RestoreTimers()
{
    if (!timersInited)
        return;

    timersInited = false;

    // timer-A
    {
        *TIMERA_REG_ENABLE  = (~TIMERA_MASK_ENABLE) & (*TIMERA_REG_ENABLE);
        *TIMERA_REG_PENDING &= ~TIMERA_MASK_ENABLE;
        *TIMERA_REG_SERVICE &= ~TIMERA_MASK_ENABLE;
        *TIMERA_REG_CTRL = ((~TIMERA_MASK_CTRL) & (*TIMERA_REG_CTRL)) | mfpTimerOld[0].ctrl;
        *TIMERA_REG_DATA = mfpTimerOld[0].data;
        *TIMERA_REG_ENABLE  |= mfpTimerOld[0].en;
    }

    // timer-B
    {
        *TIMERB_REG_ENABLE = (~TIMERB_MASK_ENABLE) & (*TIMERB_REG_ENABLE);
        *TIMERB_REG_PENDING &= ~TIMERB_MASK_ENABLE;
        *TIMERB_REG_SERVICE &= ~TIMERB_MASK_ENABLE;
        *TIMERB_REG_CTRL = ((~TIMERB_MASK_CTRL) & (*TIMERB_REG_CTRL)) | mfpTimerOld[1].ctrl;
        *TIMERB_REG_DATA = mfpTimerOld[1].data;
        *TIMERB_REG_ENABLE  |= mfpTimerOld[1].en;
    }

    // timer-C
    {
        *TIMERC_REG_ENABLE  = (~TIMERC_MASK_ENABLE) & (*TIMERC_REG_ENABLE);
        *TIMERC_REG_PENDING &= ~TIMERC_MASK_ENABLE;
        *TIMERC_REG_SERVICE &= ~TIMERC_MASK_ENABLE;
        *TIMERC_REG_CTRL    = ((~TIMERC_MASK_CTRL) & (*TIMERC_REG_CTRL)) | mfpTimerOld[2].ctrl;
        *TIMERC_REG_DATA    = mfpTimerOld[2].data;
        *TIMERC_REG_ENABLE  |= mfpTimerOld[2].en;
    }

    // timer-D
    {
        *TIMERD_REG_ENABLE  = (~TIMERD_MASK_ENABLE) & (*TIMERD_REG_ENABLE);
        *TIMERD_REG_PENDING &= ~TIMERD_MASK_ENABLE;
        *TIMERD_REG_SERVICE &= ~TIMERD_MASK_ENABLE;
        *TIMERD_REG_CTRL    = ((~TIMERD_MASK_CTRL) & (*TIMERD_REG_CTRL)) | mfpTimerOld[3].ctrl;
        *TIMERD_REG_DATA    = mfpTimerOld[3].data;
        *TIMERD_REG_ENABLE  |= mfpTimerOld[3].en;
    }
}

void SetMacVector(uint16 v, uint32 f)
{
    vbrTableMac[v>>2] = f;
}

void SetTosVector(uint16 v, uint32 f)
{
    vbrTableTos[v>>2] = f;
}

bool InitZeroPage()
{
    currentZeroPage = ZEROPAGE_OLD;
    ZPState[ZEROPAGE_OLD].vbr = GetVBR();
    ZPState[ZEROPAGE_OLD].cacr = GetCACR();
    ZPState[ZEROPAGE_OLD].tcen = TIMERC_MASK_ENABLE;
    ZPState[ZEROPAGE_OLD].tcctrl = (5 << TIMERC_SHIFT_CTRL);    // divide base clock by 64
    ZPState[ZEROPAGE_OLD].tcdata = 192;                         // 200 hz ((2457600 / (64 * 192))

    GetMMU(&ZPState[ZEROPAGE_OLD].mmu);
    memcpy(&ZPState[ZEROPAGE_TOS], &ZPState[ZEROPAGE_OLD], sizeof(ZeroPageState));
    memcpy(&ZPState[ZEROPAGE_MAC], &ZPState[ZEROPAGE_OLD], sizeof(ZeroPageState));

    if (HostCPUType >= 6)
    {
        uint32 cacr = ZPState[ZEROPAGE_OLD].cacr & 0x80008000;  // data + instruction caches only
        ZPState[ZEROPAGE_TOS].cacr = cacr;
        ZPState[ZEROPAGE_MAC].cacr = cacr;
    }

    // disable caches during init
    SetCACR(0);
    return true;
}

void RestoreZeroPage()
{
    uint16 sr = DisableInterrupts();
    SetZeroPage(ZEROPAGE_OLD);
    SetCACR(ZPState[ZEROPAGE_OLD].cacr);
    memcpy(&ZPState[ZEROPAGE_TOS], &ZPState[ZEROPAGE_OLD], sizeof(ZeroPageState));
    memcpy(&ZPState[ZEROPAGE_MAC], &ZPState[ZEROPAGE_OLD], sizeof(ZeroPageState));
    SetSR(sr);
}

bool SetupZeroPage()
{
    log("Setting up VBR\n");
    uint32* tosVbr = vbrTableTos;
    uint32* macVbr = vbrTableMac;
    ZPState[ZEROPAGE_TOS].vbr = tosVbr;
    ZPState[ZEROPAGE_MAC].vbr = macVbr;

    log(" oldVbr  = 0x%08x\n", ZPState[ZEROPAGE_OLD].vbr);
    log(" tosVbr  = 0x%08x\n", ZPState[ZEROPAGE_TOS].vbr);
    log(" macVbr  = 0x%08x\n", ZPState[ZEROPAGE_MAC].vbr);

#if CACR_OVERRIDE
    #if CACR_RESPECT_MAC
        ZPState[ZEROPAGE_MAC].cacr = 0;                 // Mac sets up the cache
    #else
        if (HostCPUType >= 4)
            ZPState[ZEROPAGE_MAC].cacr = 0x80008000;     // data+instr cache
        else
            ZPState[ZEROPAGE_MAC].cacr = 0x00003111;     // data+instr cache, burst enabled, write allocate
    #endif
#endif

    if (cpu_cacr_override_tos != 0xFFFFFFFF)
        ZPState[ZEROPAGE_TOS].cacr = cpu_cacr_override_tos;

    if (cpu_cacr_override_mac != 0xFFFFFFFF)
        ZPState[ZEROPAGE_MAC].cacr = cpu_cacr_override_mac;

    if (HostCPUType >= 6)
    {
        if (cpu_pcr_override != 0xFFFFFFFF)
            SetPCR060(cpu_pcr_override);

        log(" oldPcr  = 0x%08x\n", oldPCR);
        log(" newPcr  = 0x%08x\n", GetPCR060());
    }

    log(" oldCacr = 0x%08x\n", ZPState[ZEROPAGE_OLD].cacr);
    log(" tosCacr = 0x%08x\n", ZPState[ZEROPAGE_TOS].cacr);
    log(" macCacr = 0x%08x\n", ZPState[ZEROPAGE_MAC].cacr);


    //---------------------------------------------------------------------
    //  Vectors
    //---------------------------------------------------------------------

#if SUPPORT_ALT_TOSVBR
    // mac jumptable
    for(uint16 i=0,j=0; i<0x400; i+=4)
    {
        SetMacVector(i, &vecDirectJumpTableMac[j]);
        vecDirectJumpTableMac[j++] = 0x2F38;   // move.l (addr).w,-(sp)
        vecDirectJumpTableMac[j++] = i;        // addr
        vecDirectJumpTableMac[j++] = 0x4E75;   // rts
    }
    // tos jumptable
    uint32 vbr = ZPState[ZEROPAGE_OLD].vbr;
    if (vbr < 0x10000) {
        for (uint16 i=0, j=0; i<0x400; i+=4, vbr+=4) {
            SetTosVector(i, &vecDirectJumpTableTos[j]);
            vecDirectJumpTableTos[j++] = 0x2F38;    // move.l (addr).w,-(sp)
            vecDirectJumpTableTos[j++] = vbr;       // addr
            vecDirectJumpTableTos[j++] = 0x4E75;    // rts
        }
    } else {
        for (uint16 i=0, j=0; i<0x400; i+=4, vbr+=4) {
            SetTosVector(i, &vecDirectJumpTableTos[j]);
            vecDirectJumpTableTos[j++] = 0x2F39;            // move.l (addr).w,-(sp)
            vecDirectJumpTableTos[j++] = (vbr >> 16);       // addr hi
            vecDirectJumpTableTos[j++] = (vbr & 0xFFFF);    // addr lo
            vecDirectJumpTableTos[j++] = 0x4E75;            // rts
        }
    }
#else
    for(uint16 i=0,j=0; i<0x400; i+=4)
    {
        SetMacVector(i, &vecDirectJumpTable[j]);
        SetTosVector(i, &vecDirectJumpTable[j]);
        vecDirectJumpTable[j++] = 0x2F38;   // move.l (addr).w,-(sp)
        vecDirectJumpTable[j++] = i;        // addr
        vecDirectJumpTable[j++] = 0x4E75;   // rts
    }
#endif

    // exceptions
    SetMacVector(0x10, VecMacIllegalInstruction);   // illegal instruction
    SetMacVector(0x14, VecRte);                     // divide by zero
    SetMacVector(0x20, VecMacException);            // priviledged instruction
#if LINEA_DEBUG
    SetMacVector(0x28, VecMacException);    // linea
#endif

    // tos linef emulator
#if SUPPORT_FPEMU
    extern int FPUType;
    if (FPUType != 0) {
        uint32 fline = *((volatile uint32*) (0x2C));
        while (fline > 12) {
            char* xbra = (char*)fline - 12;
            fline = 0;
            if (strncmp(xbra, "XBRA", 4) == 0) {
                if (strncmp(xbra+4, "FPE", 3) == 0) {
                    log(" lineF emulator at %08x\n", (uint32) (xbra + 12));
                    SetMacVector(0x2C, (uint32) (xbra + 12));
                } else {
                    fline = *((uint32*)(xbra + 8));
                }
            }
        }
    }
#endif


#if 0
    extern int FPUType;
    if (FPUType == 0)
        SetMacVector(0x2C, VecMacException);    // linef
    for (uint16 i=0x2C+4; i<0x60; i+=4)
        SetMacVector(i, VecMacException);
    for (uint16 i=0xC0; i<0x100; i+=4)
        SetMacVector(i, VecMacException);
#endif

    if (HostCPUType == 6)
    {
        SetMacVector(0x2C, Vec060_fpsp_fline);      // linef
        //SetMacVector(0xC0, Vec060_bsun);          // bsun
        SetMacVector(0xC4, Vec060_fpsp_inex);       // inex1/2
        SetMacVector(0xC8, Vec060_fpsp_dz);         // dz
        SetMacVector(0xCC, Vec060_fpsp_unfl);       // unfl
        SetMacVector(0xD0, Vec060_fpsp_operr);      // operr
        SetMacVector(0xD4, Vec060_fpsp_ovfl);       // ovfl
        SetMacVector(0xD8, Vec060_fpsp_snan);       // snan
        SetMacVector(0xDC, Vec060_fpsp_unsupp);     // fpu data format
        SetMacVector(0xf0, Vec060_fpsp_effadd);     // unimplemented effective address
        SetMacVector(0xf4, Vec060_isp_unimp);       // unimplemented integer instruction
    }
    else
    {
        SetMacVector(0xC0, VecMacFpuException);     // bsun
        SetMacVector(0xC4, VecMacFpuException);     // inex1/2
        SetMacVector(0xC8, VecMacFpuException);     // dz
        SetMacVector(0xCC, VecMacFpuException);     // unfl
        SetMacVector(0xD0, VecMacFpuException);     // operr
        SetMacVector(0xD4, VecMacFpuException);     // ovfl
        SetMacVector(0xD8, VecMacFpuException);     // snan
        SetMacVector(0xDC, VecMacFpuException);     // fpu data format
    }

    // disable all autovector interrupts
    for (uint16 i=0x60; i<0x80; i+=4)
    {
        SetMacVector(i, VecRte);
        SetTosVector(i, VecRte);
    }


    // MFP interrupts (two of them)
    for (uint16 i=0x100; i<0x180; i+=4)
        SetMacVector(i, VecTosIrq);


    //---------------------------------------------------------------------
    //  MMU
    //---------------------------------------------------------------------
    if (HostCPUType >= 4)
    {
        log(" Original:\n");
        log(" tc:  %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.tc);
        log(" srp: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.srp);
        log(" urp: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.urp);
        log(" it0: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.itt0);
        log(" dt0: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.dtt0);
        log(" it1: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.itt1);
        log(" dt1: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.dtt1);
    }
    else
    {
        log(" tc:  %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.tc);
        log(" srp: %08x %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.srp[0], ZPState[ZEROPAGE_TOS].mmu.m030.srp[1]);
        log(" crp: %08x %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.crp[0], ZPState[ZEROPAGE_TOS].mmu.m030.crp[1]);
        log(" tt0: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.ttr0);
        log(" tt1: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.ttr1);
        if ((ZPState[ZEROPAGE_OLD].mmu.m030.tc & 0x80000000) == 0)
        {
            memset(&ZPState[ZEROPAGE_OLD].mmu, 0, sizeof(ZPState[ZEROPAGE_OLD].mmu));
            memset(&ZPState[ZEROPAGE_TOS].mmu, 0, sizeof(ZPState[ZEROPAGE_TOS].mmu));
        }
    }

    log("Create MAC MMU config\n");
    if (HostCPUType >= 4)
    {
        InitMMU040();
        log(" tc:  %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.tc);
        log(" srp: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.srp);
        log(" urp: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.urp);
        log(" it0: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.itt0);
        log(" dt0: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.dtt0);
        log(" it1: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.itt1);
        log(" dt1: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.dtt1);        
    }
    else
    {
        InitMMU030();
        log(" tc:  %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.tc);
        log(" srp: %08x %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.srp[0], ZPState[ZEROPAGE_MAC].mmu.m030.srp[1]);
        log(" crp: %08x %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.crp[0], ZPState[ZEROPAGE_MAC].mmu.m030.crp[1]);
        log(" tt0: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.ttr0);
        log(" tt1: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.ttr1);
    }


    log("Flushing cache\n");
    SetCACR(ZPState[ZEROPAGE_TOS].cacr);
    log("ZeroPage setup complete\n");
    return true;
}

void InitMMU030()
{
    log(" InitMMU030: Start\n");
    pmmu_68030_init();
    log(" InitMMU030: Created\n");
}

void InitMMU040()
{
    uint16 sr = DisableInterrupts();
    log(" InitMMU040: Start\n");
    setup_68040_pmmu();
    GetMMU040(&ZPState[ZEROPAGE_MAC].mmu.m040);
    SetMMU040(&ZPState[ZEROPAGE_OLD].mmu.m040);
    log(" InitMMU040: Created\n");
    log(" InitMMU040: Flush ATC\n");
    FlushATC040();
    log(" InitMMU040: Flush Cache\n");
    SetCACR040(ZPState[ZEROPAGE_OLD].cacr);
    SetSR(sr);
    log(" InitMMU040: Done\n");
}
