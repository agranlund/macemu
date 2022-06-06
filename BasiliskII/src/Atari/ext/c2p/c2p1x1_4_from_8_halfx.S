	.globl _c2p1x1_4_from_8_halfx
	.globl _c2p1x1_4_from_8_060_halfx

_c2p1x1_4_from_8_halfx:
_c2p1x1_4_from_8_060_halfx:
	movem.l	d2-d7/a2-a6,-(sp)
	move.l	44+4(sp),a1							// a1 = planar destination
	move.l	44+8(sp),a0							// a0 = chunky start
	move.l	44+12(sp),a2						//
	add.l	a0,a2								// a2 = chunky end
	move.l	44+16(sp),a3						// a3 = colormap table

	move.l	#0x00ff00ff,d5						// mask
	move.l	#0x55555555,d6						// mask
	moveq.l	#0,d4								// color reduction lookup

.start:
    // read pixels 0-3 and reduce to 16 colors
    move.b  (a0),d4
	addq.l	#2,a0
    move.b  (a3,d4.w),d0
    lsl.l   #8,d0
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d0
    lsl.l   #8,d0
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d0
    lsl.l   #8,d0
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d0
	// read pixels 4-7 and reduce to 16 colors
    move.b  (a0),d4
	addq.l	#2,a0
    move.b  (a3,d4.w),d2
    lsl.l   #8,d2
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d2
    lsl.l   #8,d2
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d2
    lsl.l   #8,d2
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d2
    // read pixels 8-11 and reduce to 16 colors
    move.b  (a0),d4
	addq.l	#2,a0
    move.b  (a3,d4.w),d1
    lsl.l   #8,d1
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d1
    lsl.l   #8,d1
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d1
    lsl.l   #8,d1
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d1
    // read pixels 12-15 and reduce to 16 colors
    move.b  (a0),d4
	addq.l	#2,a0
    move.b  (a3,d4.w),d3
    lsl.l   #8,d3
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d3
    lsl.l   #8,d3
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d3
    lsl.l   #8,d3
    move.b  (a0),d4
	addq.l	#2,a0
    or.b    (a3,d4.w),d3

	lsl.l	#4,d0								// shift into high nibble
	lsl.l	#4,d1								// shift into high nibble
	or.l	d2,d0								// combine nibbles of pixels 0-7
	or.l	d3,d1								// combine nibbles of pixles 8-15

	// a3a2a1a0e3e2e1e0 b3b2b1b0f3f2f1f0 c3c2c1c0g3g2g1g0 d3d2d1d0h3h2h1h0
	// i3i2i1i0m3m2m1m0 j3j2j1j0n3n2n1n0 k3k2k1k0o3o2o1o0 l3l2l1l0p3p2p1p0

	move.l	d1,d7								// pixels 8-15
	lsr.l	#8,d7								// pixels 8-13
	eor.l	d0,d7
	and.l	d5,d7
	eor.l	d7,d0
	lsl.l	#8,d7
	eor.l	d7,d1

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

	swap	d0
	swap	d1

	move.l	d1,(a1)+							// store planes 0-1
	move.l	d0,(a1)+							// store planes 2-3

	cmp.l	a0,a2
	bne.w	.start

	movem.l	(sp)+,d2-d7/a2-a6
	rts
