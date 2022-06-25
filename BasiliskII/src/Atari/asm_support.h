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

// Vectors
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

// Motorola 68060 support vectors
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

// 68030
extern void FlushCache030();
extern void FlushATC030();
extern void InitMMU030();
extern void GetMMU030(MMURegs030* r);
extern void SetMMU030(MMURegs030* r);
extern uint32 GetCACR030();
extern void SetCACR030(uint32 r);

// 68040
extern void FlushCache040();
extern void FlushATC040();
extern void InitMMU040();
extern void GetMMU040(MMURegs040* r);
extern void SetMMU040(MMURegs040* r);
extern uint32 GetCACR040();
extern void SetCACR040(uint32 r);

// 68060
extern void FlushCache060();
#define FlushATC060 FlushATC040
#define InitMMU060 InitMMU040
#define GetMMU060 GetMMU040
#define SetMMU060 SetMMU040
#define GetCACR060 GetCACR040
extern void SetCACR060(uint32 r);
extern void SetPCR060(uint32 r);
extern uint32 GetPCR060();


// CPU access
typedef void (*flush_cache_func)();
typedef void (*flush_atc_func)();
typedef uint32 (*get_cacr_func)();
typedef void (*set_cacr_func)(uint32 r);
typedef void (*get_mmu_func)(MMURegs * r);
typedef void (*set_mmu_func)(MMURegs * r);

extern flush_cache_func FlushCache;
extern flush_atc_func	FlushATC;
extern set_cacr_func	SetCACR;
extern get_cacr_func	GetCACR;
extern get_mmu_func		GetMMU;
extern set_mmu_func		SetMMU;
extern uint32			oldPCR;

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------------------------
inline void InitCPU()
{
	if (HostCPUType < 4)
	{
		// 68030
		FlushCache 		= FlushCache030;
		FlushATC 		= FlushATC030;
		SetCACR			= SetCACR030;
		GetCACR			= GetCACR030;
		GetMMU			= GetMMU030;
		SetMMU			= SetMMU030;
	}
	else if (HostCPUType < 6)
	{
		// 68040
		FlushCache 		= FlushCache040;
		FlushATC 		= FlushATC040;
		SetCACR			= SetCACR040;
		GetCACR			= GetCACR040;
		GetMMU			= GetMMU040;
		SetMMU			= SetMMU040;
	}
	else
	{
		// 68060
		FlushCache 		= FlushCache060;
		FlushATC 		= FlushATC060;
		SetCACR			= SetCACR060;
		GetCACR			= GetCACR060;
		GetMMU			= GetMMU060;
		SetMMU			= SetMMU060;

		oldPCR = GetPCR060();
		// bit 0 = enable super scalar
		// bit 1 = disable fpu
		// bit 5 = disable super bypass (undocumented)
		uint32 pcr = oldPCR;
		pcr |= (1 << 5);
		SetPCR060(pcr);
	}

	FlushCache();
}

inline void RestoreCPU()
{
	if (HostCPUType >= 6)
	{
		SetPCR060(oldPCR);
	}
}

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

#endif
