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

#ifndef ATARI_ZEROPAGE_H
#define ATARI_ZEROPAGE_H

#include "sysdeps.h"
#include "asm_support.h"

#define TIMERB_200HZ 	0


#define ZEROPAGE_OLD    0
#define ZEROPAGE_MAC    1
#define ZEROPAGE_TOS    2

#define MFP(offs)               ((volatile unsigned char*)(0x00FFFA00+offs))

#define TIMERA_VECTOR           0x134
#define TIMERA_MASK_ENABLE      (1<<5)
#define TIMERA_MASK_CTRL        0x0F
#define TIMERA_SHIFT_CTRL       0
#define TIMERA_REG_ENABLE       MFP(0x07)
#define TIMERA_REG_PENDING      MFP(0x0B)
#define TIMERA_REG_SERVICE      MFP(0x0F)
#define TIMERA_REG_MASK     	MFP(0x13)
#define TIMERA_REG_CTRL         MFP(0x19)
#define TIMERA_REG_DATA         MFP(0x1F)

#define TIMERB_VECTOR           0x120
#define TIMERB_MASK_ENABLE      (1<<0)
#define TIMERB_MASK_CTRL        0x0F
#define TIMERB_SHIFT_CTRL       0
#define TIMERB_REG_ENABLE       MFP(0x07)
#define TIMERB_REG_PENDING      MFP(0x0B)
#define TIMERB_REG_SERVICE      MFP(0x0F)
#define TIMERB_REG_MASK     	MFP(0x13)
#define TIMERB_REG_CTRL         MFP(0x1B)
#define TIMERB_REG_DATA         MFP(0x21)

#define TIMERC_VECTOR           0x114
#define TIMERC_MASK_ENABLE      (1<<5)
#define TIMERC_MASK_CTRL        0x70
#define TIMERC_SHIFT_CTRL       4
#define TIMERC_REG_ENABLE       MFP(0x09)
#define TIMERC_REG_PENDING      MFP(0x0D)
#define TIMERC_REG_SERVICE      MFP(0x11)
#define TIMERC_REG_MASK		    MFP(0x15)
#define TIMERC_REG_CTRL         MFP(0x1D)
#define TIMERC_REG_DATA         MFP(0x23)

#define TIMERD_VECTOR           0x110
#define TIMERD_MASK_ENABLE      (1<<4)
#define TIMERD_MASK_CTRL        0x07
#define TIMERD_SHIFT_CTRL       0
#define TIMERD_REG_ENABLE       MFP(0x09)
#define TIMERD_REG_PENDING      MFP(0x0D)
#define TIMERD_REG_SERVICE      MFP(0x11)
#define TIMERD_REG_MASK		    MFP(0x15)
#define TIMERD_REG_CTRL         MFP(0x1D)
#define TIMERD_REG_DATA         MFP(0x25)

#define SECTION_IRQ				0
#define SECTION_TOS				1
#define SECTION_DISK			2
#define SECTION_DEBUG			3
#define SECTION_MAC_IRQ			4
#define SECTION_MAC_EMUOP		5
#define NUM_SECTIONS 			6

extern volatile uint16 currentZeroPage;

extern bool InitZeroPage();
extern bool SetupZeroPage();
extern void RestoreZeroPage();
inline uint16 GetZeroPage() { return currentZeroPage; }
extern "C" uint16 SetZeroPage(uint16 page);

extern void InitTimers();
extern void RestoreTimers();

extern void SetTosVector(uint16 v, uint32 f);
extern void SetMacVector(uint16 v, uint32 f);

extern volatile int16 section[NUM_SECTIONS];

#define EnterSection(_sec)		{ section[_sec]++; }
#define ExitSection(_sec)		{ section[_sec]--; }
#define IsSection(_sec)			( section[_sec] != 0 )

class Section
{
public:
	Section(uint16 s) { EnterSection(s); _s = s; }
	~Section() { ExitSection(_s); }
private:
	uint16 _s;
};


struct ZeroPageState
{
    MMURegs mmu;
    uint32* vbr;
	uint32	cacr;
};

extern ZeroPageState ZPState[3];
extern volatile uint16 currentZeroPage;


// Helper class for calling bios/xbios from anywhere
// Temporarily replaces bios save register block (_savptr)
// The block is 3 * 23 words large

#define TOS_SAV_PTR		((volatile uint32*)0x4a2)
#define TOS_SAV_SIZE	(12*3)
#define TOS_SAV_TOTAL	(1 + TOS_SAV_SIZE)

class TOSContext
{
public:
	TOSContext(uint16 sr)
	{
		oldSR = DisableInterrupts();
		oldZP = SetZeroPage(ZEROPAGE_TOS);
		savptr[0] = *TOS_SAV_PTR;
		*TOS_SAV_PTR = &savptr[TOS_SAV_TOTAL];
		SetSR(sr);
	}

	~TOSContext()
	{
		DisableInterrupts();
		*TOS_SAV_PTR = savptr[0];
		SetZeroPage(oldZP);
		SetSR(oldSR);
	}
private:
	uint16 oldSR;
	uint16 oldZP;
	uint32 savptr[TOS_SAV_TOTAL];
};

#define TOS_CONTEXT()			TOSContext tos_context(0x2500);
#define TOS_CONTEXT_NOIRQ(sr)	TOSContext tos_context(0x2700);


#endif //ATARI_ZEROPAGE_H
