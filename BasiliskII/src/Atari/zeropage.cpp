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

#define DEBUG 1
#include "debug.h"

extern void pmmu_68030_init(void);
extern "C" void setup_68040_pmmu(void);


#define CACR_OVERRIDE       1
#define CACR_RESPECT_MAC    0

ZeroPageState ZPState[3];
volatile uint16 currentZeroPage = ZEROPAGE_OLD;
volatile int16 section[NUM_SECTIONS] = { 0 };

uint32  vbrTableTos[256];
uint32  vbrTableMac[256];
uint16  vecDirectJumpTable[256*3];

__LINEA *__aline;
__FONT  **__fonts;
short  (**__funcs) (void);

struct MFPTimer
{
    uint8 en;
    uint8 ctrl;
    uint8 data;
};
MFPTimer mfpTimerOld[4];
bool timersInited = false;

extern bool isEmuTos;
extern bool isMagic;
extern bool isMint;
extern uint16 tosVersion;
extern uint32 ROMSize;

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
    SetSR(sr);
    return prevZeroPage;
}

void InitTimers()
{
    if (timersInited)
        return;

    // backup timer settings
    mfpTimerOld[0].en = TIMERA_MASK_ENABLE & (*TIMERA_REG_ENABLE);
    mfpTimerOld[0].ctrl = TIMERA_MASK_CTRL & (*TIMERA_REG_CTRL);
    mfpTimerOld[0].data = *TIMERA_REG_DATA;
    mfpTimerOld[1].en = TIMERB_MASK_ENABLE & (*TIMERB_REG_ENABLE);
    mfpTimerOld[1].ctrl = TIMERB_MASK_CTRL & (*TIMERB_REG_CTRL);
    mfpTimerOld[1].data = *TIMERB_REG_DATA;
    mfpTimerOld[2].en = TIMERC_MASK_ENABLE & (*TIMERC_REG_ENABLE);
    mfpTimerOld[2].ctrl = TIMERC_MASK_CTRL & (*TIMERC_REG_CTRL);
    mfpTimerOld[2].data = *TIMERC_REG_DATA;
    mfpTimerOld[3].en = TIMERD_MASK_ENABLE & (*TIMERD_REG_ENABLE);
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
        #if TIMERB_200HZ
            SetMacVector(TIMERB_VECTOR, VecTimer1);
            SetTosVector(TIMERB_VECTOR, VecTimer1);
            const unsigned char ctrl = (5 << TIMERB_SHIFT_CTRL);    // divide base clock by 64
            const unsigned char data = 192;                         // 200 hz ((2457600 / (64 * 192))
            *TIMERB_REG_CTRL = (((~TIMERB_MASK_CTRL) & (*TIMERB_REG_CTRL)) | (ctrl << TIMERB_SHIFT_CTRL));
            *TIMERB_REG_DATA = data;
            *TIMERB_REG_ENABLE  |= TIMERB_MASK_ENABLE;
        #endif
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
        SetMacVector(TIMERC_VECTOR, VecTimer2);
        SetTosVector(TIMERC_VECTOR, VecTimer2);
        // new params
        const unsigned char ctrl = (4 << TIMERC_SHIFT_CTRL);    // divide base clock by 50
        const unsigned char data = 49;                          // 1003hz ((2457600 / (50 * 49))
        *TIMERC_REG_CTRL = ((~TIMERC_MASK_CTRL) & (*TIMERC_REG_CTRL)) | ctrl;
        *TIMERC_REG_DATA = data;
        *TIMERC_REG_ENABLE  |= TIMERC_MASK_ENABLE;
    }

    // timer-D (baud rate generator)
    {
        // used by debug_atari.cpp, serial_atari.cpp
        //*TIMERD_REG_ENABLE &= ~TIMERD_MASK_ENABLE;
        //*TIMERD_REG_PENDING &= ~TIMERD_MASK_ENABLE;
        //*TIMERD_REG_SERVICE &= ~TIMERD_MASK_ENABLE;
    }

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
    GetMMU(&ZPState[ZEROPAGE_OLD].mmu);
    memcpy(&ZPState[ZEROPAGE_TOS], &ZPState[ZEROPAGE_OLD], sizeof(ZeroPageState));
    memcpy(&ZPState[ZEROPAGE_MAC], &ZPState[ZEROPAGE_OLD], sizeof(ZeroPageState));
    return true;
}

void RestoreZeroPage()
{
    SetZeroPage(ZEROPAGE_OLD);
    memcpy(&ZPState[ZEROPAGE_TOS], &ZPState[ZEROPAGE_OLD], sizeof(ZeroPageState));
    memcpy(&ZPState[ZEROPAGE_MAC], &ZPState[ZEROPAGE_OLD], sizeof(ZeroPageState));
}

bool SetupZeroPage()
{
    D(bug("Setting up VBR\n"));
    uint32* tosVbr = vbrTableTos;
    uint32* macVbr = vbrTableMac;
    ZPState[ZEROPAGE_TOS].vbr = tosVbr;
    ZPState[ZEROPAGE_MAC].vbr = macVbr;

    D(bug(" oldVbr = 0x%08x\n", ZPState[ZEROPAGE_OLD].vbr));
    D(bug(" tosVbr = 0x%08x\n", ZPState[ZEROPAGE_TOS].vbr));
    D(bug(" macVbr = 0x%08x\n", ZPState[ZEROPAGE_MAC].vbr));

    if ((HostCPUType >= 4) && (ROMSize < (1024 * 1024)))
    {
        D(bug("Disabling cpu cache (512k ROM on 68040+)\n"));
        SetCACR(0);
    }

    uint32 cacr = GetCACR();
    ZPState[ZEROPAGE_TOS].cacr = cacr;
    ZPState[ZEROPAGE_MAC].cacr = cacr;

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

    D(bug(" oldCacr = 0x%08x\n", ZPState[ZEROPAGE_OLD].cacr));
    D(bug(" tosCacr = 0x%08x\n", ZPState[ZEROPAGE_TOS].cacr));
    D(bug(" macCacr = 0x%08x\n", ZPState[ZEROPAGE_MAC].cacr));

    //---------------------------------------------------------------------
    //  Vectors
    //---------------------------------------------------------------------

    // jump to the real Tos/Mac vector by default
    for(uint16 i=0,j=0; i<0x400; i+=4)
    {
        SetMacVector(i, &vecDirectJumpTable[j]);
        SetTosVector(i, &vecDirectJumpTable[j]);
        vecDirectJumpTable[j++] = 0x2F38;   // move.l (addr).w,-(sp)
        vecDirectJumpTable[j++] = i;        // addr
        vecDirectJumpTable[j++] = 0x4E75;   // rts
    }

    // exceptions
    SetMacVector(0x10, VecMacException);    // illegal instruction
    SetMacVector(0x14, VecRte);             // divide by zero
    SetMacVector(0x20, VecMacException);    // priviledged instruction
#if LINEA_DEBUG
    SetMacVector(0x28, VecMacException);    // linea
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
        D(bug(" Original:\n"));
        D(bug(" tc:  %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.tc));
        D(bug(" srp: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.srp));
        D(bug(" urp: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.urp));
        D(bug(" it0: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.itt0));
        D(bug(" dt0: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.dtt0));
        D(bug(" it1: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.itt1));
        D(bug(" dt1: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m040.dtt1));
    }
    else
    {
        D(bug(" tc:  %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.tc));
        D(bug(" srp: %08x %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.srp[0], ZPState[ZEROPAGE_TOS].mmu.m030.srp[1])); 
        D(bug(" crp: %08x %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.crp[0], ZPState[ZEROPAGE_TOS].mmu.m030.crp[1])); 
        D(bug(" tt0: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.ttr0));
        D(bug(" tt1: %08x\n", ZPState[ZEROPAGE_TOS].mmu.m030.ttr1));
        if ((ZPState[ZEROPAGE_OLD].mmu.m030.tc & 0x80000000) == 0)
        {
            memset(&ZPState[ZEROPAGE_OLD].mmu, 0, sizeof(ZPState[ZEROPAGE_OLD].mmu));
            memset(&ZPState[ZEROPAGE_TOS].mmu, 0, sizeof(ZPState[ZEROPAGE_TOS].mmu));
        }
    }

    D(bug("Create MAC MMU config\n"));
    if (HostCPUType >= 4)
    {
        InitMMU040();
        D(bug(" tc:  %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.tc));
        D(bug(" srp: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.srp));
        D(bug(" urp: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.urp));
        D(bug(" it0: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.itt0));
        D(bug(" dt0: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.dtt0));
        D(bug(" it1: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.itt1));
        D(bug(" dt1: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m040.dtt1));        
    }
    else
    {
        InitMMU030();
        D(bug(" tc:  %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.tc));
        D(bug(" srp: %08x %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.srp[0], ZPState[ZEROPAGE_MAC].mmu.m030.srp[1])); 
        D(bug(" crp: %08x %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.crp[0], ZPState[ZEROPAGE_MAC].mmu.m030.crp[1])); 
        D(bug(" tt0: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.ttr0));
        D(bug(" tt1: %08x\n", ZPState[ZEROPAGE_MAC].mmu.m030.ttr1));
    }


    D(bug("Flushing cache\n"));
    SetCACR(ZPState[ZEROPAGE_TOS].cacr);
    D(bug("ZeroPage setup complete\n"));
    return true;
}

void InitMMU030()
{
    D(bug(" InitMMU030: Start\n"));
    pmmu_68030_init();
    D(bug(" InitMMU030: Created\n"));
}

void InitMMU040()
{
    uint16 sr = DisableInterrupts();
    D(bug(" InitMMU040: Start\n"));
    setup_68040_pmmu();
    GetMMU040(&ZPState[ZEROPAGE_MAC].mmu.m040);
    SetMMU040(&ZPState[ZEROPAGE_OLD].mmu.m040);
    D(bug(" InitMMU040: Created\n"));
    D(bug(" InitMMU040: Flush ATC\n"));
    FlushATC040();
    D(bug(" InitMMU040: Flush Cache\n"));
    SetCACR040(ZPState[ZEROPAGE_OLD].cacr);
    SetSR(sr);
    D(bug(" InitMMU040: Done\n"));
}
