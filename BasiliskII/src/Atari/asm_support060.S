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
	.globl	_GetPCR060
    .globl  _SetPCR060
	.globl	_FlushCache060
	.globl	_SetCACR060

_GetPCR060:
	movec	pcr,d0
	rts

_SetPCR060:
	move.l	4(sp),d0
	movec	d0,pcr
	rts

_FlushCache060:
	nop
	cpusha	bc						// flush data + instruction caches
	nop
//	movec	cacr,d0
//	or.l	0x00400000,d0			// flush branch cache
//	movec	d0,cacr
	rts

_SetCACR060:
	move.l	d0,-(sp)
	move.l	8(sp),d0
	nop
	cpusha	bc
	nop
	movec	d0,cacr
	move.l	(sp)+,d0
	rts


//
// Motorola 68060 support package to emulate missing '040 instructions
//

#include "ext/060sp/os.S"
#include "ext/060sp/iskeletn.S"
#include "ext/060sp/isp.S"
#include "ext/060sp/fskeletn.S"
#include "ext/060sp/fpsp.S"
