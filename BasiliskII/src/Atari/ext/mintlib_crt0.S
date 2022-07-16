	.globl	__base
	.globl	___has_no_ssystem

//	.extern	__checkcpu
	.extern	__crtinit

	.text
	.even
	.globl	_start
_start:
	subl	%a6, %a6		// clear a6 for debuggers
	movel	%sp@(4), %a0	// get basepage
	movel	%a0, __base		// save it
	movel	%a0@(4), %d0	// get _base->p_hitpa
	bclr	#0, %d0			// round off
	movel	%d0, %sp		// set stack (temporarily)
	subl	#64, %sp		// see note in mintlib/setstack.S

	clrl	%sp@-
	clrl	%sp@-
	movew	#-1,%sp@-		// Check Ssystem() availability
	movew	#0x154,%sp@-	// Ssystem()
	trap	#1
	lea		%sp@(12),%sp
	movel	%d0,___has_no_ssystem

//	jsr		_checkcpu		// check for correct CPU or exit
	jmp		__crtinit		// in crtinit.c

	.globl 	_monstartup
	.globl	_moncontrol
	.globl	__mcleanup
_monstartup:
_moncontrol:
__mcleanup:
	rts

