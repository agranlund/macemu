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
	.globl	_FlushCache030
	.globl	_FlushATC030
	.globl	_InitMMU030
	.globl	_GetMMU030
	.globl	_SetMMU030
	.globl	_GetCACR030
	.globl	_SetCACR030

_FlushCache030:
	move.l	d0,-(sp)
	nop
	movec	cacr,d0
	ori.w	#0x0808,d0
	movec	d0,cacr
	nop
	move.l	(sp)+,d0
	rts

_FlushATC030:
	nop
	pflusha
	nop
	rts

_GetMMU030:
	move.l	d0,-(sp)
	move.w	sr,d0			// disable interrupts
	or.w	#0x0700,sr
	move.l	8(sp),a0		// a0 = MMURegs030*
	//pmove	srp,0(a0)
	pmove	crp,8(a0)
	pmove	tt0,16(a0)
	pmove	tt1,20(a0)
	pmove	tc,24(a0)
	move.w	d0,sr			// restore interrupts
	move.l	(sp)+,d0
	rts

_SetMMU030:
	movem.l	d0-d1/a0,-(sp)
	move.w	sr,d0			// disable interrupts
	or.w	#0x700,sr
	move.l	#0,-(sp)		// disable mmu
	pmove	(sp),tc
	addq.l	#4,sp
	move.l	12+4(sp),a0		// a0 = MMURegs030*
	move.l	24(a0),d1
	beq		.SetMMU030done
	//pmove	0(a0),srp
	pmove	8(a0),crp
	pmove	16(a0),tt0
	pmove	20(a0),tt1
	pmove	24(a0),tc
.SetMMU030done:
	nop
	pflusha					// flush mmu cache
	nop
	move.w	d0,sr			// restore interrupts
	movem.l	(sp)+,d0-d1/a0
	rts

_GetCACR030:
	movec	cacr,d0
	rts

_SetCACR030:
	move.l	4(sp),d0		// get cacr
	ori.w	#0x0808,d0		// clear data + instruction cache
	movec	d0,cacr
	rts
