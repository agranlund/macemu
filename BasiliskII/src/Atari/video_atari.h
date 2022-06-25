/*
 *  input_atari.h
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

#ifndef ATARI_VIDEO_H
#define ATARI_VIDEO_H

#include "sysdeps.h"

struct ScreenDesc
{
	int16	handle;			// vdi handle
	uint16	width;			// width of screen in pixels
	uint16	height;			// height of screen in pixels
	uint8	bpp;			// bits per pixel
	uint8	planes;			// number of bitplanes
	uint16	bytesPerLine;	// number of bytes per line
	uint16	pf;				// pixel format
	uint32	addr;			// screen address
	uint32	vdo;			// vdo cookie
	uint16	hw;				// video hardware
	uint8	vdiPen[256];	// palette -> pen color map
};

bool QueryScreen(ScreenDesc& scr);
bool AtariScreenInfo(int32 mode, bool& native, uint32& mem);
void AtariScreenUpdate();
void AtariVideoScaleKeyPressed();

// standard hardware
#define HW_SHIFTER_ST			0
#define HW_SHIFTER_STE			1
#define HW_SHIFTER_TT			2
#define HW_VIDEL				3
// graphics cards
#define HW_GFXCARD				4
#define HW_SUPERVIDEL			5


#define PF_BITS_1		( 1 << 0)
#define PF_BITS_2		( 2 << 0)
#define PF_BITS_4		( 4 << 0)
#define PF_BITS_8		( 8 << 0)
#define PF_BITS_16		(16 << 0)
#define PF_BITS_24		(24 << 0)
#define PF_BITS_32		(32 << 0)
#define PF_MASK_BITS	(31 << 0)

#define PF_PL_CHUNKY	(0 << 6)		// chunky
#define PF_PL_ATARI		(1 << 6)		// interleaved planes
#define PF_PL_AMIGA		(3 << 6)		// separate planes
#define PF_MASK_PL		(3 << 6)

#define PF_ENDIAN_BE	(0 << 8)		// big endian
#define PF_ENDIAN_LE	(1 << 8)		// little endian
#define PF_MASK_ENDIAN	(1 << 8)

#define PF_FMT_IDX		(0 << 9)		// index mode
#define PF_FMT_555		(1 << 9)		// 16 bit 555
#define PF_FMT_565		(2 << 9)		// 16 bit 565
#define PF_FMT_888		(3 << 9)		// 24 bit 888
#define PF_MASK_FMT		(3 << 9)

// some helper defines
#define MODE_APPLE_1			(PF_BITS_1 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_IDX)
#define MODE_APPLE_2			(PF_BITS_2 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_IDX)
#define MODE_APPLE_4			(PF_BITS_4 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_IDX)
#define MODE_APPLE_8			(PF_BITS_8 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_IDX)
#define MODE_APPLE_16			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_555)
#define MODE_APPLE_32			(PF_BITS_32 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_888)

#define MODE_SHIFTER_1			(PF_BITS_1 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_IDX)
#define MODE_SHIFTER_2			(PF_BITS_2 | PF_PL_ATARI | PF_ENDIAN_BE | PF_FMT_IDX)
#define MODE_SHIFTER_4			(PF_BITS_4 | PF_PL_ATARI | PF_ENDIAN_BE | PF_FMT_IDX)
#define MODE_SHIFTER_8			(PF_BITS_8 | PF_PL_ATARI | PF_ENDIAN_BE | PF_FMT_IDX)
#define MODE_SHIFTER_16			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_565)

#define MODE_RGB565_BE			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_565)
#define MODE_RGB565_LE			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_LE | PF_FMT_565)
#define MODE_RGB555_BE			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_555)
#define MODE_RGB555_LE			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_LE | PF_FMT_555)
#define MODE_RGB888_BE			(PF_BITS_32 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_888)
#define MODE_RGB888_LE			(PF_BITS_32 | PF_PL_CHUNKY | PF_ENDIAN_LE | PF_FMT_888)


#endif // ATARI_VIDEO_H
