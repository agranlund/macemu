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

	.text
	.globl	_FlushCache040
	.globl	_FlushATC040
	.globl	_GetMMU040
	.globl	_SetMMU040
	.globl	_GetCACR040
	.globl	_SetCACR040

_FlushCache040:
	nop
	cpusha	bc						// flush caches
	nop
	rts

_FlushATC040:
	nop
	pflusha
	nop
	rts

_GetMMU040:
	movem.l	d0-d1/a0,-(sp)
	move.w	sr,d1					// disable interrupts
	move.w	#0x2700,sr
	move.l	12+4(sp),a0				// a0 = MMURegs040*
	movec	srp,d0
	move.l	d0,(a0)+
	movec	urp,d0
	move.l	d0,(a0)+
	movec	itt0,d0
	move.l	d0,(a0)+
	movec	dtt0,d0
	move.l	d0,(a0)+
	movec	itt1,d0
	move.l	d0,(a0)+
	movec	dtt1,d0
	move.l	d0,(a0)+
	movec	tc,d0
	move.l	d0,(a0)+
	move.w	d1,sr					// restore interrupts
	movem.l	(sp)+,d0-d1/a0
	rts

_SetMMU040:
	movem.l	d0-d2/a0,-(sp)
	move.w	sr,d1					// disable interrupts
	move.w	#0x2700,sr
	movec	cacr,d2					// save cache settings
	moveq   #0,d0
	nop
	cpusha	bc						// flush caches
	nop
	movec   d0,cacr					// disable caches
	nop
	movec   d0,tc					// disable mmu
	movec   d0,itt0
	movec   d0,dtt0
	movec   d0,itt1
	movec   d0,dtt1
	nop
	pflusha							// flush atc
	nop
	move.l	16+4(sp),a0				// a0 = MMURegs040*
	move.l	0(a0),d0
	movec	d0,srp
	move.l	4(a0),d0
	movec	d0,urp
	move.l	8(a0),d0
	movec	d0,itt0
	move.l	12(a0),d0
	movec	d0,dtt0
	move.l	16(a0),d0
	movec	d0,itt1
	move.l	20(a0),d0
	movec	d0,dtt1
	move.l	24(a0),d0
	nop
	cpusha	bc						// probably unecessary
	nop
	movec	d2,cacr					// restore cache setting
	movec   d0,tc					// enable mmu
.SetMMU040done:
	nop
	pflusha							// flush atc
	nop
	move.w	d1,sr					// restore interrupts
	movem.l	(sp)+,d0-d2/a0
	rts

_GetCACR040:
	movec	cacr,d0
	rts

_SetCACR040:
	move.l	d0,-(sp)
	move.l	8(sp),d0
	nop
	cpusha	bc
	nop
	movec	d0,cacr
	move.l	(sp)+,d0
	rts
