/*
 *  asm_support.h
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

#ifndef ATARI_ASM_INC_H
#define ATARI_ASM_INC_H

#include "sysdeps.h"

#if 0
#define MMU030_PAGESIZE		1024
#define MMU030_PAGESHIFT	10
#else
#define MMU030_PAGESIZE		256
#define MMU030_PAGESHIFT	8
#endif

#define MMU040_PAGESIZE		4096
#define MMU040_PAGESHIFT	12

extern int HostCPUType;
extern int HostFPUType;

struct MMURegs030
{
    uint32  srp[2];
    uint32  crp[2];
    uint32  ttr0;
    uint32  ttr1;
    uint32  tc;
};

struct MMURegs040
{
	uint32	srp;
	uint32	urp;
	uint32	itt0;
	uint32	dtt0;
	uint32	itt1;
	uint32	dtt1;
	uint32	tc;
};

struct MMURegs
{
	union
	{
		MMURegs030	m030;
		MMURegs040	m040;
	};
};

#ifdef __cplusplus
extern "C" {
#endif

extern void BootMacintosh(uint8* stack);
extern void ExecuteTosInterrupt(uint16 vec);

extern void VecRts();
extern void VecRte();
extern void VecNoHbl();
extern void VecOriginal();
extern void VecMacLineA();
extern void VecMacHandler();
extern void VecAcia();
extern void VecTimer1();
extern void VecTimer2();
extern void VecTosIrq();
extern void VecMacException();
extern void VecMacFpuException();

extern void FlushCache030();
extern void FlushATC030();
extern void InitMMU030();
extern void GetMMU030(MMURegs030* r);
extern void SetMMU030(MMURegs030* r);
extern uint32 GetCACR030();
extern void SetCACR030(uint32 r);

extern void FlushCache040();
extern void FlushATC040();
extern void InitMMU040();
extern void GetMMU040(MMURegs040* r);
extern void SetMMU040(MMURegs040* r);
extern uint32 GetCACR040();
extern void SetCACR040(uint32 r);

extern void Setup060(void);

// Motorola 68060 support package
extern void Vec060_isp_unimp();
extern void Vec060_isp_cas();
extern void Vec060_isp_cas2();
extern void Vec060_isp_cas_finish();
extern void Vec060_isp_cas2_finish();
extern void Vec060_isp_cas_inrange();
extern void Vec060_isp_cas_terminate();
extern void Vec060_isp_cas_restart();
extern void Vec060_fpsp_snan();
extern void Vec060_fpsp_operr();
extern void Vec060_fpsp_ovfl();
extern void Vec060_fpsp_unfl();
extern void Vec060_fpsp_dz();
extern void Vec060_fpsp_inex();
extern void Vec060_fpsp_fline();
extern void Vec060_fpsp_unsupp();
extern void Vec060_fpsp_effadd();

#ifdef __cplusplus
}
#endif


//------------------------------------------------------------------------------------------

inline uint16 GetSR() {
	uint16 retvalue;
	__asm__ volatile (          \
		"	move.w  sr,%0\n"    \
		: "=d"(retvalue) : : "cc");
    return retvalue;
}

inline void SetSR(uint16 newSR) {
	__asm__ volatile (          \
		"	move.w  %0,sr\n"    \
		: : "d"(newSR) : "cc");    
}

//------------------------------------------------------------------------------------------

inline uint16 DisableInterrupts() {
	uint16 oldSR = GetSR();
	SetSR(0x2700);
	return oldSR;
}

//------------------------------------------------------------------------------------------

inline uint32* GetVBR() {
	uint32* retvalue;
	__asm__ volatile (          \
		"	movec   vbr,%0\n"   \
		: "=r"(retvalue) : : "cc");
    return retvalue;
}

inline void SetVBR(uint32* newVBR) {
	__asm__ volatile (          \
		"	movec  %0,vbr\n"    \
		: : "r"(newVBR) : "cc");
}


//------------------------------------------------------------------------------------------
inline uint32 GetCACR() {
	if (HostCPUType >= 4)
		return GetCACR040();
	else
		return GetCACR030();
}

inline void SetCACR(uint32 r) {
	if (HostCPUType >= 4)
		SetCACR040(r);
	else
		SetCACR030(r);
}

//------------------------------------------------------------------------------------------

inline void SetMMU(MMURegs* regs) {
	if (HostCPUType >= 4)
		SetMMU040(&regs->m040);
	else
		SetMMU030(&regs->m030);
}

inline void GetMMU(MMURegs* regs) {
	if (HostCPUType >= 4)
		GetMMU040(&regs->m040);
	else
		GetMMU030(&regs->m030);
}

//------------------------------------------------------------------------------------------

inline void FlushATC() {
	if (HostCPUType >= 4)
		FlushATC040();
	else
		FlushATC030();
}


#endif
