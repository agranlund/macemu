	.globl _c2p1x1_4_from_4
	.globl _c2p1x1_4_from_4_060

_c2p1x1_4_from_4_060:
	nop
_c2p1x1_4_from_4:
	movem.l	d2-d7/a2-a6,-(sp)

	move.l	44+4(sp),a1				// a1 = planar
	move.l	44+8(sp),a0				// a0 = chunky start
	move.l	44+12(sp),a2			// 
	add.l	a0,a2					// a2 = chunky end
	move.l	_blit_CompareBuf,a3		// a3 = compare buffer

	move.b	#0x0000000f,d4
	move.l	#0xfffffff0,d5
	move.l	#0x55555555,d6

.start:
	move.l	(a0)+,d0			// test against compare buffer
	move.l	(a0)+,d1
	move.l	a3,a5
	cmp.l	(a5)+,d0
	bne.s	.go
	cmp.l	(a5)+,d1
	bne.s	.go
	addq.l	#8,a1				// ignore pixels
	move.l	a5,a3
	cmp.l	a0,a2
	bne.s	.start
	movem.l	(sp)+,d2-d7/a2-a6
	rts	
.go:
	move.l	d0,(a3)+			// update compare buffer
	move.l	d1,(a3)+

	moveq.l	#0,d0
	move.b	0-8(a0),d0		// 000000ab
	lsl.l	#4,d0			// 00000ab0
	move.b	2-8(a0),d0		// 00000aef
	lsl.l	#4,d0			// 0000aef0
	move.b	4-8(a0),d0		// 0000aeij
	lsl.l	#4,d0			// 000aeij0
	move.b	6-8(a0),d0		// 000aeimn
	lsl.l	#4,d0			// 00aeimn0
	move.b	1-8(a0),d0		// 00aeimcd
	lsl.l	#4,d0			// 0aeimcde
	move.b	3-8(a0),d0		// 0aeimcgh
	lsl.l	#4,d0			// aeimcgh0
	move.b	5-8(a0),d0		// aeimcgkl
	and.l	d5,d0			// aeimcgk0
	move.b	7-8(a0),d2		// d2 = op
	lsr.b	#4,d2			// d2 = 0o
	or.b	d2,d0			// aeimcgko

	moveq.l	#0,d1
	move.b	7-8(a0),d1		// 000000op
	ror.l	#4,d1			// p000000o
	move.b	5-8(a0),d1		// p00000kl
	ror.l	#4,d1			// lp00000k
	move.b	3-8(a0),d1		// lp0000gh
	ror.l	#4,d1			// hlp0000g
	move.b	1-8(a0),d1		// hlp000cd
	ror.l	#4,d1			// dhlp000c
	move.b	6-8(a0),d1		// dhlp00mn
	ror.l	#4,d1			// ndhlp00m
	move.b	4-8(a0),d1		// ndhlp0ij
	ror.l	#4,d1			// jndhlp0i
	move.b	2-8(a0),d1		// jndhlpef
	ror.l	#4,d1			// fjndhlpe
	and.l	d5,d1			// fjndhlp0
	move.b	0-8(a0),d2		// d2 = ab
	and.b	d4,d2			// d2 = 0b
	or.b	d2,d1			// fjndhlpb
	ror.l	#4,d1			// bfjndhlp

	not.l	d0				// invert palette
	not.l	d1

	// a3a2a1a0e3e2e1e0 i3i2i1i0m3m2m1m0 c3c2c1c0g3g2g1g0 k3k2k1k0o3o2o1o0
	// b3b2b1b0f3f2f1f0 j3j2j1j0n3n2n1n0 d3d2d1d0h3h2h1h0 l3l2l1l0p3p2p1p0

	move.l	d1,d7
	lsr.l	#1,d7
	eor.l	d0,d7
	and.l	d6,d7
	eor.l	d7,d0
	add.l	d7,d7
	eor.l	d7,d1

	// a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1
	// a2b2a0b0e2f2e0f0 i2j2i0j0m2n2m0n0 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.w	d1,d7
	move.w	d0,d1
	swap	d1
	move.w	d1,d0
	move.w	d7,d1

	// a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 a2b2a0b0e2f2e0f0 i2j2i0j0m2n2m0n0
	// c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.l	d1,d7
	lsr.l	#2,d7
	eor.l	d0,d7
	and.l	#0x33333333,d7
	eor.l	d7,d0
	lsl.l	#2,d7
	eor.l	d7,d1

	// a3b3c3d3e3f3g3h3 i3j3k3l3m3n3o3p3 a2b2c2d2e2f2g2h2 i2j2k2l2m2n2o2p2
	// a1b1c1d1e1f1g1h1 i1j1k1l1m1n1o1p1 a0b0c0d0e0f0g0h0 i0j0k0l0m0n0o0p0

	swap	d1
	move.l	d1,(a1)+
	swap	d0
	move.l	d0,(a1)+

	cmp.l	a0,a2
	bne		.start

	movem.l	(sp)+,d2-d7/a2-a6
	rts

