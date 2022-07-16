/*
* asm_support.S - Atari utility functions in assembly language
*
* Basilisk II (C) 1997-2001 Christian Bauer
*
* This program is free software// you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation// either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY// without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program// if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

	.bss
	.even
_FlushCache:	ds.l 1
_FlushATC:		ds.l 1
_SetCACR:		ds.l 1
_GetCACR:		ds.l 1
_GetMMU:		ds.l 1
_SetMMU:		ds.l 1
_oldPCR:		ds.l 1

	.globl	_FlushCache
	.globl	_FlushATC
	.globl	_SetCACR
	.globl	_GetCACR
	.globl	_GetMMU
	.globl	_SetMMU
	.globl	_oldPCR

	.text
	.even
	.globl	_SetZeroPage			// from ZeroPage.cpp
	.globl	newAcia					// from acia.S

	.globl	_BootMacintosh
	.globl	_Execute68k
	.globl	_Execute68kTrap
	.globl	_ExecuteTosInterrupt

	.globl	_VecRts
	.globl	_VecRte
	.globl	_VecNoHbl
	.globl	_VecTosIrq
	.globl	_VecMacLineA

	.globl	_VecAcia
	.globl	_VecAciaC

	.globl	_VecMacIllegalInstruction
	.globl	_VecMacException
	.globl	_VecMacExceptionC
	.globl	_VecMacFpuException

	.globl	_VecMacTimerB
	.globl	_VecTosTimerB
	.globl	_VecMacTimerC
	.globl	_VecTosTimerC

	.globl	_IntTimerBMac
	.globl	_IntTimerBTos
	.globl	_IntTimerCMac
	.globl	_IntTimerCTos


// In Hatari, there is a situation where the SR register becomes broken when
// an MFP interrupt is entered directly after setting SR to 0x2000
// It _looks_ like it is getting confused when setting the interrupt mask bits.
// Instead of mask 6 as you'd expect it appears to set 45 for MFP interrupt 0x118 and that
// wreaks havoc on all kinds of bits in the register.
// The SR value in the exception frame is correct though

#define FIX_BROKEN_MFP_SR	1

#if FIX_BROKEN_MFP_SR
#define MFP_SR_FIX	move.w	#0x2600,sr
#else
#define MFP_SR_FIX
#endif


//--------------------------------------------------------------
// void BootMacintosh(uint8* stack)
//
// Set stack in Macintosh RAM and start executing the ROM
//--------------------------------------------------------------
_BootMacintosh:
	move.l	4(sp),a0
	move.l	a0,sp
	move.l	_ROMBaseHost,a0		// jump to mac rom
	lea		0x2a(a0),a0
	jmp		(a0)


//--------------------------------------------------------------
// void Execute68k(uint32 addr, M68kRegisters *r)//
//
// Execute 68k subroutine (must be ended with rts)
// r->a[7] and r->sr are unused!
//--------------------------------------------------------------
_Execute68k:
	move.l	4(sp),d0				// Get arguments
	move.l	8(sp),a0
	movem.l	d2-d7/a2-a6,-(sp)		// Save registers
	move.l	a0,-(sp)				// Push pointer to M68kRegisters on stack
	pea		.e68kdone				// Push return address on stack
	move.l	d0,-(sp)				// Push pointer to 68k routine on stack
	movem.l	(a0),d0-d7/a0-a6		// Load registers from M68kRegisters
	rts								// Jump into 68k routine
.e68kdone:
	move.l	a6,-(sp)				// Save a6
	move.l	4(sp),a6				// Get pointer to M68kRegisters
	movem.l	d0-d7/a0-a5,(a6)		// Save d0-d7/a0-a5 to M68kRegisters
	move.l	(sp)+,56(a6)			// Save a6 to M68kRegisters
	addq.l	#4,sp					// Remove pointer from stack
	movem.l	(sp)+,d2-d7/a2-a6		// Restore registers
	rts


//--------------------------------------------------------------
// void Execute68kTrap(uint16 trap, M68kRegisters *r)//
//
// Execute MacOS 68k trap
// r->a[7] and r->sr are unused!
//--------------------------------------------------------------
_Execute68kTrap:
	move.l	4(sp),d0				// Get arguments
	move.l	8(sp),a0
	movem.l	d2-d7/a2-a6,-(sp)		// Save registers
	move.l	a0,-(sp)				// Push pointer to M68kRegisters on stack
	move.w	d0,-(sp)				// Push trap word on stack
	subq.l	#8,sp					// Create fake A-Line exception frame
	movem.l	(a0),d0-d7/a0-a6		// Load registers from M68kRegisters
	move.l	a2,-(sp)				// Save a2 and d2
	move.l	d2,-(sp)
	lea		.e68ktdone,a2			// a2 points to return address
	move.w	16(sp),d2				// Load trap word into d2
	jmp		([0x28.w],10)			// Jump into MacOS A-Line handler
.e68ktdone:
	move.l	a6,-(sp)				// Save a6
	move.l	6(sp),a6				// Get pointer to M68kRegisters
	movem.l	d0-d7/a0-a5,(a6)		// Save d0-d7/a0-a5 to M68kRegisters
	move.l	(sp)+,56(a6)			// Save a6 to M68kRegisters
	addq.l	#6,sp					// Remove pointer and trap word from stack
	movem.l	(sp)+,d2-d7/a2-a6		// Restore registers
	rts

//--------------------------------------------------------------
// void ExecuteTosInterrupt(uint16 vector)
//
//
//--------------------------------------------------------------
_ExecuteTosInterrupt:
	move.l	4(sp),a0
	move.w	a0,-(sp)				// fake format/offset
	pea		.eti0(pc)				// fake pc
	move.w	sr,-(sp)				// fake sr
	move.l	(a0),-(sp)				// vector address
	rts								// jump to vector
.eti0:
	rts


//--------------------------------------------------------------
// vectors
//--------------------------------------------------------------

StackFormatTable:
	dc.w	4*2			// Four-word stack frame, format 0x0
	dc.w	4*2			// Throwaway four-word stack frame, format 0x1
	dc.w	6*2			// Six-word stack frame, format 0x2
	dc.w	6*2			// MC68040 floating-point post-instruction stack frame, format 0x3
	dc.w	8*2			// MC68EC040 and MC68LC040 floating-point unimplemented stack frame, format 0x4
	dc.w	4*2			// Format 0x5
	dc.w	4*2			// Format 0x6
	dc.w	30*2		// MC68040 access error stack frame, Format 0x7
	dc.w	29*2		// MC68010 bus and address error stack frame, format 0x8
	dc.w	10*2		// MC68020 and MC68030 coprocessor mid-instruction stack frame, format 0x9
	dc.w	16*2		//  MC68020 and MC68030 short bus cycle stack frame, format 0xa
	dc.w	46*2		// MC68020 and MC68030 long bus cycle stack frame, format 0xb
	dc.w	12*2		// CPU32 bus error for prefetches and operands stack frame, format 0xc
	dc.w	4*2			// Format 0xd
	dc.w	4*2			// Format 0xe
	dc.w	4*2			// Format 0xf

_VecRts:
	rts

_VecRte:
	rte

_VecNoHbl:
	move.w	#0x2300,(sp)
	rte

_VecMacLineA:
	move.l	0x28.w,-(sp)
	rts

_VecAcia:
	MFP_SR_FIX
	movem.l	d0-d2/a0-a2,-(sp)
	jsr		_VecAciaC
	movem.l	(sp)+,d0-d2/a0-a2
	bclr	#6,0xFFFFFA11.w		    		// Interrupt not in service
	rte

_VecTosTimerB:
	MFP_SR_FIX
	movem.l	d0-d2/a0-a2,-(sp)				// save registers
	jsr		_IntTimerBTos					// and call the C handler
	movem.l	(sp)+,d0-d2/a0-a2				// restore registers
	rte

_VecMacTimerB:
	MFP_SR_FIX
	movem.l	d0-d2/a0-a2,-(sp)				// save registers
	jsr		_IntTimerBMac					// and call the C handler
	movem.l	(sp)+,d0-d2/a0-a2				// restore registers
	rte

_VecTosTimerC:
	MFP_SR_FIX
	movem.l	d0-d2/a0-a2,-(sp)				// save registers
	jsr		_IntTimerCTos					// and call the C handler
	movem.l	(sp)+,d0-d2/a0-a2				// restore registers
	rte

_VecMacTimerC:
	MFP_SR_FIX
	movem.l	d0-d2/a0-a2,-(sp)				// save registers
	jsr		_IntTimerCMac					// and call the C handler
	movem.l	(sp)+,d0-d2/a0-a2				// restore registers
	rte

_VecTosIrq:
	MFP_SR_FIX
//	cmp.w	#2,_currentZeroPage				// in tos already?
//	beq		_VecOriginal					// just jump to original handler
	movem.l	d0-d2/a0-a2,-(sp)				// save registers
	move.l	#2,-(sp)						// switch to tos
	jsr		_SetZeroPage
	move.w	24+4+6(sp),d0					// build fake frame
	and.l	#0x00000FFF,d0	
	move.w	d0,-(sp)						// format/offset
	pea		.vti0(pc)						// return address
	move.w	sr,-(sp)						// return sr
	move.l	(0,d0),-(sp)					// vector address
	rts										// jump to tos vector
.vti0:
	move.l	#1,-(sp)						// back to mac
	jsr		_SetZeroPage
	addq.l	#8,sp
	movem.l	(sp)+,d0-d2/a0-a2				// restore registers
	rte

_VecMacFpuException:
	move.l	d0,-(sp)						// save registers
	fmove.l	fpcr,d0
	and.w	#0x00ff,d0						// disable FPU exceptions
	fmove.l	d0,fpcr
	move.l	(sp)+,d0						// restore registers
	rte

_VecMacException:
_VecMacIllegalInstruction:
	//move.w	#0x2700,sr
	movem.l	d0-d7/a0-a7,-(sp)				// push all registers, these can be modified by _VecMacExceptionC
	move.l	sp,d0
	move.l	d0,-(sp)						// push sp (which is now our trap_regs*)
	add.l	#64+8,d0
	move.l	d0,64(sp)						// correct pushed a7
	jsr		_VecMacExceptionC
	cmp.w	#0,d0
	beq.s	.mvh0
	addq.l	#4,sp							// restore stack after c call
	movem.l	(sp)+,d0-d7/a0-a6				// pop all registers
	addq.l	#4,sp							// but ignore potentially modified sp
	rte
.mvh0:
	move.w	68+6(sp),d0						// get format/offset word from exception frame
	and.l	#0x00000FFF,d0					// mask offset bits
	move.l	(0,d0),(sp)						// store jump address (value at 0+offset)
	movem.l	4(sp),d0-d7/a0-a6				// restore registers except a7
	move.l	(sp),64(sp)						// make jump addres the return address
	add.l	#64,sp							// get rid of stuff in stack
	rts										// and jump
