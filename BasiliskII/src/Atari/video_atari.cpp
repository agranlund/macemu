/*
 *  video_atari.cpp - Video/graphics emulation, Atari specific stuff
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

#include "sysdeps.h"
#include "cpu_emulation.h"
#include "main.h"
#include "adb.h"
#include "prefs.h"
#include "user_strings.h"
#include "video.h"
#include "video_defs.h"
#include "zeropage.h"
#include "macos_util.h"
#include "input_atari.h"
#include "mint/cookie.h"

#define DEBUG 1
#include "debug.h"


#define FALCON_CONSIDER_STPAL	0
#define SUPPORT_8ON16BIT		1
#define ENABLE_VIDEODEBUG		1

#define DEBUG_MMU				0

extern uint32* pmmu_68030_map(uint32 logaddr, uint32 physaddr, uint32 size, uint32 flag);
extern "C" uint32* pmmu_68040_get_pagetable(uint32 logaddr);

extern "C" {
void c2p1x1_8_from_8(uint8* dst, uint8* src, uint32 size, uint8* pal);			// Apple 8bit -> Atari 8bit
void c2p1x1_8_from_8_halfx(uint8*dst, uint8* src, uint32 size, uint8* pal);		// Apple 8bit -> Atari 8bit, half width
void c2p1x1_8_from_8_halfx_060(uint8*dst, uint8* src, uint32 size, uint8* pal);	// Apple 8bit -> Atari 8bit, half width (68060)
void c2p1x1_4_from_8(uint8* dst, uint8* src, uint32 size, uint8* pal);			// Apple 8bit -> Atari 4bit
void c2p1x1_4_from_8_halfx(uint8* dst, uint8* src, uint32 size, uint8* pal);	// Apple 8bit -> Atari 4bit, half width
void c2p1x1_4_from_4(uint8* dst, uint8* src, uint32 size, uint8* pal);			// Apple 4bit -> Atari 4bit
};

void rgb555le_from_rgb555be(uint8* dst, uint8* src, uint32 size, uint8* pal)
{
	uint16* s = (uint16*)src;
	uint16* d = (uint16*)dst;
	while (size >= 16)
	{
		uint16 c;
		c = *s++; *d++ = ((c << 8) | (c >> 8)); c = *s++; *d++ = ((c << 8) | (c >> 8));
		c = *s++; *d++ = ((c << 8) | (c >> 8)); c = *s++; *d++ = ((c << 8) | (c >> 8));
		c = *s++; *d++ = ((c << 8) | (c >> 8)); c = *s++; *d++ = ((c << 8) | (c >> 8));
		c = *s++; *d++ = ((c << 8) | (c >> 8)); c = *s++; *d++ = ((c << 8) | (c >> 8));
		size -= 16;
	}
}

void rgb565le_from_rgb555be(uint8* dst, uint8* src, uint32 size, uint8* pal)
{
	uint16* s = (uint16*)src;
	uint16* d = (uint16*)dst;
	const uint16 m2 = 0xC000;
	const uint16 m1 = 0x1F00;
	while (size >= 16)
	{
		uint16 c,c0,c1,c2;
		c = *s++; c0 = c >> 7; c1 = c << 8; c2 = (c1 << 1) & m2; c1 &= m1; *d++ = (c0|c1|c2);
		c = *s++; c0 = c >> 7; c1 = c << 8; c2 = (c1 << 1) & m2; c1 &= m1; *d++ = (c0|c1|c2);
		c = *s++; c0 = c >> 7; c1 = c << 8; c2 = (c1 << 1) & m2; c1 &= m1; *d++ = (c0|c1|c2);
		c = *s++; c0 = c >> 7; c1 = c << 8; c2 = (c1 << 1) & m2; c1 &= m1; *d++ = (c0|c1|c2);
		c = *s++; c0 = c >> 7; c1 = c << 8; c2 = (c1 << 1) & m2; c1 &= m1; *d++ = (c0|c1|c2);
		c = *s++; c0 = c >> 7; c1 = c << 8; c2 = (c1 << 1) & m2; c1 &= m1; *d++ = (c0|c1|c2);
		c = *s++; c0 = c >> 7; c1 = c << 8; c2 = (c1 << 1) & m2; c1 &= m1; *d++ = (c0|c1|c2);
		c = *s++; c0 = c >> 7; c1 = c << 8; c2 = (c1 << 1) & m2; c1 &= m1; *d++ = (c0|c1|c2);
		size -= 16;
	}
}

void rgb565be_from_rgb555be(uint8* dst, uint8* src, uint32 size, uint8* pal)
{
	const uint32 m0 = 0x001F001F;
	const uint32 m1 = 0xFFC0FFC0;
	uint32* s = (uint32*)src;
	uint32* d = (uint32*)dst;
	while (size >= 16)
	{
		uint32 c;
		c = *s++; *d++ = (((c << 1) & m1) | (c & m0));
		c = *s++; *d++ = (((c << 1) & m1) | (c & m0));
		c = *s++; *d++ = (((c << 1) & m1) | (c & m0));
		c = *s++; *d++ = (((c << 1) & m1) | (c & m0));
		size -= 16;
	}
}

void rgbxxxxx_from_rgb555be(uint8* dst, uint8* src, uint32 size, uint8* pal)
{
	uint16* s = (uint16*)src;
	uint16* d = (uint16*)dst;
	uint16* p = (uint16*)pal;
	while (size >= 16)
	{
		uint16 c;
		c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c];
		c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c];
		size -= 16;
	}	
}

void rgb565be_from_8bit(uint8* dst, uint8* src, uint32 size, uint8* pal)
{
	uint8* s = (uint8*)src;
	uint16* d = (uint16*)dst;
	uint16* p = (uint16*)pal;
	while (size >= 16)
	{
		uint16 c;
		c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c];
		c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c];
		c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c];
		c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c]; c = *s++; *d++ = p[c];
		size -= 16;
	}
}


typedef void (*blit_func)(uint8* src, uint8* dst, uint32 size, uint8* pal);

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
#define MODE_SHIFTER_16			(PF_BITS_16 | PF_PL_ATARI | PF_ENDIAN_BE | PF_FMT_565)

#define MODE_RGB565_BE			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_565)
#define MODE_RGB565_LE			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_LE | PF_FMT_565)
#define MODE_RGB555_BE			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_555)
#define MODE_RGB555_LE			(PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_LE | PF_FMT_555)
#define MODE_RGB888_BE			(PF_BITS_32 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_888)
#define MODE_RGB888_LE			(PF_BITS_32 | PF_PL_CHUNKY | PF_ENDIAN_LE | PF_FMT_888)




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
	uint8	vdiPen[256];	// palette -> pen color map
};

class MonitorDesc : public monitor_desc
{
public:
	MonitorDesc(const vector<video_mode> &available_modes, video_depth default_depth, uint32 default_id)
		:  monitor_desc(available_modes, default_depth, default_id) { };
	~MonitorDesc() { };

	virtual void switch_to_current_mode(void);
	virtual void set_palette(uint8 *pal, int num);
	virtual void set_gamma(uint8* gamma, int num);
	virtual int16 driver_control(uint16 code, uint32 param, uint32 dce);
};


class VideoDriver
{
public:
	VideoDriver();
	virtual ~VideoDriver();

	virtual MonitorDesc* Init(ScreenDesc& scr);			// called once on init
	virtual void Release();								// called once on shutdown

	virtual bool Setup();								// setup mode, called by MacOS
	virtual void SetPalette(uint8 *pal, int num);		// set palette, called by MacOS
	virtual void GrayPage();							// clear screen, called by MacOS

	virtual void Update();								// called once per frame

	bool Inited() { return init_ok; }

protected:
	virtual void UpdatePalette();						// update palette
	virtual void UpdateScreen();						// update screen

	bool init_ok;										// Initialization succeeded
	ScreenDesc screen;									// Atari screen descriptor

	int16 tosVdiPal[256*3];								// original vdi palette
	uint32 tosHwPal[256];								// original hardware palette
	uint8 macPalette[256*3];							// mac palette
	uint16 macPaletteDirty;								// number of dirty mac palette entries

	bool shouldUpdateVdiPalette;
	bool shouldUpdateHardwarePalette;
	bool shouldUpdateSoftwarePalette;

	bool supportNonPerfectModes;
	bool supportEmulatedModes;

	// emulation related
	int16 frameSkip;
	bool fullRedraw;
	uint8* emulatedScreen;
	uint8 softPalette[256*4];
	blit_func blitFunc;
	uint8* blitSrc;
	uint8* blitDst;
	uint32*	pageTable;
	uint16 pageSize;
	uint16 pageShift;
	uint32* hashBuffer;
	uint16 hashSize;
	bool lowRes;
	uint8* lowResOffs[480];
#if ENABLE_VIDEODEBUG
	int32 videoDebug;
#endif

	void AddModes(uint32 width, uint32 height, video_depth depth);
	void AddMode(uint32 width, uint32 height, uint32 resolution_id, uint32 bytes_per_row, video_depth depth);

	inline uint16 ScaleColorValueForVdi(uint16& c)
	{
		uint16 out = c;	   // x << 0     x*1
		c <<= 2; out += c; // x << 2	+x*4
		c <<= 1; out += c; // x << 3	+x*8
		c <<= 1; out += c; // x << 4	+x*16
		c <<= 1; out += c; // x << 5	+x*32
		c <<= 1; out += c; // x << 6	+x*64 = (x*125) == (x*(1000/8))
		out >>= 5;	// out = ((x*125)/32) == ((x*1000)/256)
		return out;
	}
};



extern int16 aesVersion;
static vector<video_mode> videoModes;
static VideoDriver* drv = (VideoDriver*)NULL;
static MonitorDesc* monitor = (MonitorDesc*)NULL;
static int16 wndScreen = -1;
static ScreenDesc vdiScreen;
static bool vdiScreenValid = false;


// Query Atari display details
bool QueryScreen(ScreenDesc& scr)
{
	if (vdiScreenValid)
	{
		scr = vdiScreen;
		return true;
	}

	int16 work_in[16];
	int16 work_out[16+257];
	memset(work_out, 0, sizeof(work_out));
	memset(work_in, 0, sizeof(work_in));
	work_in[0] = 1;				// current device ID. // Getrez() + 2
	for(uint16 i = 1; i < 10; i++);
		work_in[i] = 1;			// default crap
	work_in[10] = 2;			// raster coordinates

	memset(&scr, 0, sizeof(ScreenDesc));

	// get vdo cookie
	Getcookie('_VDO', &scr.vdo);

	// get basic info through openvwk
	int16 vdi_handle = 0;
	v_opnvwk(work_in, &vdi_handle, work_out);
	if (vdi_handle == 0)
	{
		D(bug(" Failed to get VDI handle\n"));
		return false;
	}

	scr.width = work_out[0] + 1;
	scr.height = work_out[1] + 1;
	scr.pf = 0;

	// get detailed screen information
	uint32 cookie;
	if (Getcookie('EdDI', cookie) == C_FOUND)
	{
		log(" Query screen (EdDI exists)\n");
		vq_scrninfo(vdi_handle, work_out);

		scr.bpp = work_out[2];
		if (scr.bpp <= 8)
		{
			uint16 numcols = work_out[4];
			scr.pf = scr.bpp | PF_ENDIAN_BE | PF_FMT_IDX;
			if ((work_out[0] == 2) || (scr.bpp <= 1))
			{
				scr.pf |= PF_PL_CHUNKY;
				scr.planes = 1;
			}
			else if (work_out[0] == 1)
			{
				scr.pf |= PF_PL_AMIGA;
				scr.planes = scr.bpp;
			}
			else
			{
				scr.pf |= PF_PL_ATARI;
				scr.planes = scr.bpp;
			}

			for (uint16 i=0; i<numcols; i++)
				scr.vdiPen[work_out[16+i]] = i;
		}
		else
		{
			scr.planes = 1;
			scr.pf |= PF_PL_CHUNKY;

			uint16 totalbits = work_out[8] + work_out[9] + work_out[10] + work_out[11] + work_out[12] + work_out[13];
			uint16 colorbits = work_out[8] + work_out[9] + work_out[10];
			uint16 bitorder = work_out[14];

			if (bitorder & (1 << 7))
				scr.pf |= PF_ENDIAN_LE;
			else
				scr.pf |= PF_ENDIAN_BE;

			switch (scr.bpp)
			{
				case 15:
				case 16:
				{
					scr.bpp = 16;
					uint16 colorbits = work_out[8] + work_out[9] + work_out[10];
					if (colorbits == 16)
						scr.pf |= PF_FMT_565;
					else if (bitorder & (1 << 1))
						scr.pf |= PF_FMT_565;	// treat falcon HiColor as RGB565
					else
						scr.pf |= PF_FMT_555;
				}
				break;
				case 24:
				case 32:
				{
					scr.pf |= PF_FMT_888;
					if (totalbits < 32)
						scr.bpp = 24;
				}
				break;
			}
			scr.pf |= scr.bpp;
		}
	}
	else
	{
		// there is no EdDI cookie, make shit up as best we can
		log(" Query screen (no EdDI)\n");
		uint16 clutsize = work_out[13];
		vq_extnd(vdi_handle, 1, work_out);
		scr.bpp	= work_out[4];
		if (scr.bpp > 8)
		{
			scr.planes = 1;
			switch(scr.bpp)
			{
				case 15:
					scr.pf = PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_555;
					scr.bpp = 16;
					break;
				case 16:
					scr.pf = PF_BITS_16 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_565;
					break;
				case 24:
					scr.pf = PF_BITS_24 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_888;
					break;
				case 32:
					scr.pf = PF_BITS_32 | PF_PL_CHUNKY | PF_ENDIAN_BE | PF_FMT_888;
					break;
			}
		}
		else
		{
			scr.planes = scr.bpp;
			scr.pf = scr.bpp | PF_ENDIAN_BE | PF_FMT_IDX;
			if (scr.planes <= 1)
				scr.pf |= PF_PL_CHUNKY;
			else
				scr.pf |= PF_PL_ATARI;
			// generate default vdi pen -> palette lookup
			static const int16 default_vdi_color_map[] = {
				0, 15, 1, 2, 4, 6, 3, 5, 7, 8, 9, 10, 12, 14, 11, 13
			};
			memset(&work_out[16+0], 0, 256*2);
			for (uint16 i=0; ((i<clutsize) && (i<256)); i++)
				work_out[16+i] = default_vdi_color_map[i];
			work_out[16+1] = clutsize-1;
			if (clutsize > 16)
			{
				for (uint16 i=16; i<clutsize-1; i++)
					work_out[16+i] = i;
				work_out[16+clutsize-1] = 15;
			}
			// generate palette -> vdi lookup
			for (uint16 i=0; i<clutsize; i++)
				scr.vdiPen[work_out[16+i]] = i;
		}
	}

	if (scr.bytesPerLine == 0)
		scr.bytesPerLine = (scr.width * scr.bpp) / 8;

	if (scr.addr == 0)
		scr.addr = Physbase();

/*
	// todo: detect Hatari? or file a bug report.
	// It sets Physbase to 0xfe800000 in Extended VDI mode but its screen is actually at Logbase()
	if (scr.addr == 0xfe800000)
		scr.addr = Logbase();
*/
	scr.handle = vdi_handle;
	vdiScreen = scr;
	vdiScreenValid = true;
	return true;
}


bool AtariScreenInfo(int32 mode, bool& native, uint32& mem)
{
	bool emu = false;
	native = false;
	mem = 0;
	uint32 w = 640;
	uint32 h = 480;

	if (mode == 0)
	{
		ScreenDesc scr;
		if (!QueryScreen(scr))
			return false;
		switch (scr.pf)
		{
			case MODE_APPLE_1:
			case MODE_APPLE_2:
			case MODE_APPLE_4:
			case MODE_APPLE_8:
			case MODE_APPLE_16:
			case MODE_APPLE_32:
				native = true;
				break;

			case MODE_SHIFTER_4:
			case MODE_SHIFTER_8:
				emu = true;
				break;
		}
		if (scr.bpp == 16)
			emu = true;
		
		w = scr.width;
		h = scr.height;
	}
	else
	{
		if (mode == 1)
		{
			native = true;
		}
		if (mode > 1)
		{
			emu = true;
		}
	}

	if (emu)
	{
		if (w < 640) w = 640;
		if (h < 480) h = 480;
		mem = w * h * 2;		// 16bpp mac framebuffer
		mem += (mem / 32);		// compare buffer
		mem += 256 * 1024;		// alignment, padding, pagetables, etc..
	}

	return (native || emu);
}


//-------------------------------------------------------------
//
// Basilisk interface
//
//-------------------------------------------------------------
bool VideoInit(bool classic)
{
	log("VideoInit\n");
	ScreenDesc screen;
	if (!QueryScreen(screen))
	{
		ErrorAlert("Screen query failed");
		return false;
	}
/*
	// todo: switch video mode

	int32 mode = PrefsFindInt32("video_mode");
	if (mode != 0)
	{
		uint16 sr = DisableInterrupts();
		uint16 zp = SetZeroPage(ZEROPAGE_OLD);
		SetSR(0x2100);
		bool tt = (screen.vdo >> 16) == 2;
		bool falc = (screen.vdo >> 16) == 3;
		Vsync();
		if (tt)
		{
			switch(mode)
			{
				case 4:
					EsetShift(4 << 8);	// tt-medium
					break;
				case 8:
					EsetShift(7 << 7);	// tt-low
					break;
				default:
					EsetShift(2 << 8);	// st-high
					break;
			}
		}
		else if (falc)
		{
			//uint32 om = VsetMode(-1);
			//uint32 a = -1;
			//uint32 nm = om;
			//VSetscreen(a, a, nm);
		}
		else
		{
			switch (mode)
			{
				case 4:
					Setscreen(-1, -1, 0);	// st-low
					break;
				default:
					Setscreen(-1, -1, 2);	// st-high
					break;
			}
		}
		Vsync();
		Vsync();
		vdiScreenValid = false;
		QueryScreen(screen);
		DisableInterrupts();
		SetZeroPage(zp);
		SetSR(sr);
	}
	*/

	log(" VDI: %dx%dx%d (%s)\n", screen.width, screen.height, screen.bpp, screen.bpp == 1 ? "mono" : screen.planes > 1 ? "planar" : "chunky");
	log("      %d planes, %d bytes per line\n", screen.planes, screen.bytesPerLine);
	log(" VDO: %08x\n", screen.vdo);
	log(" FMT: %08x\n", screen.pf);
	log(" ScreenPtr: %08x\n", screen.addr);
	log(" Logbase:   %08x\n", Logbase());
	log(" Physbase:  %08x\n", Physbase());


	// create video driver
	drv = new VideoDriver();
	monitor = drv->Init(screen);
	if (!monitor)
	{
		log(" Err: Failed to init video driver\n");
		return false;
	}

	// create a dummy window so MiNT/Magic restores the desktop on shutdown
	if (aesVersion > 0)
	{
		wndScreen = wind_create(0, 0, 0, screen.width, screen.height);
		if (wndScreen)
			wind_open(wndScreen, 0, 0, screen.width, screen.height);
	}

	// assign monitor and setup driver
	VideoMonitors.push_back(monitor);
	drv->Setup();
	return true;
}

void VideoExit(void)
{
	D(bug("VideoExit\n"));
	if (drv)
	{
		drv->Release();
		delete drv;
		drv = NULL;
	}

	if (wndScreen)
	{
		wind_close(wndScreen);
		wndScreen = -1;
	}
}

void VideoQuitFullScreen(void)
{
}


void VideoInterrupt(void)
{
}


void AtariScreenUpdate()
{
	static bool busy = false;
	if (!busy)
	{
		busy = true;
		if (drv && drv->Inited())
		{
			uint16 sr = GetSR();
			SetSR(0x2500);
			drv->Update();
			SetSR(sr);
		}
		busy = false;
	}
}

//-------------------------------------------------------------
//
// Monitor interface
//
//-------------------------------------------------------------
int16 MonitorDesc::driver_control(uint16 code, uint32 param, uint32 dce)
{
	uint16 sr = DisableInterrupts();
	int16 ret = monitor_desc::driver_control(code, param, dce);
	if (ret != noErr)
	{
		SetSR(sr);
		return ret;
	}

	if (drv && drv->Inited())
	{
		if (code == cscGrayPage)
		{
			drv->GrayPage();
		}
	}
	SetSR(sr);
	return noErr;
}

void MonitorDesc::set_palette(uint8 *pal, int num)
{
	uint16 sr = DisableInterrupts();
	if (drv && drv->Inited())
		drv->SetPalette(pal, num);
	SetSR(sr);
}

void MonitorDesc::set_gamma(uint8 *gamma, int num)
{
}

void MonitorDesc::switch_to_current_mode()
{
	if (drv && drv->Inited())
	{
		D(bug("init input %d, %d\n", get_current_mode().x, get_current_mode().y));
		uint16 sr = DisableInterrupts();
		drv->Setup();
		SetSR(sr);
		D(bug("init input %d, %d\n", get_current_mode().x, get_current_mode().y));
	}
}



//-------------------------------------------------------------
//
// Base video driver
//
//-------------------------------------------------------------

VideoDriver::VideoDriver()
	: init_ok(false)
	, macPaletteDirty(false)
	, shouldUpdateVdiPalette(true)
	, shouldUpdateHardwarePalette(true)
	, shouldUpdateSoftwarePalette(true)
	, supportNonPerfectModes(true)
	, supportEmulatedModes(true)
	, frameSkip(0)
	, fullRedraw(true)
	, emulatedScreen(NULL)
	, blitFunc(NULL)
	, blitSrc(NULL)
	, blitDst(NULL)
	, pageTable(NULL)
	, pageSize(0)
	, pageShift(0)
	, hashBuffer(NULL)
	, hashSize(0)
	, lowRes(false)
#if ENABLE_VIDEODEBUG
	, videoDebug(0)
#endif	
{
	supportEmulatedModes = PrefsFindBool("video_emu");
}

VideoDriver::~VideoDriver()
{
	Release();
}

MonitorDesc* VideoDriver::Init(ScreenDesc& scr)
{
	if (init_ok)
		return monitor;

	screen = scr;

	if (screen.bpp <= 8)
	{
		uint16 count = (1 << screen.bpp);
		log (" Backing up vdi palette\n");

		if (shouldUpdateVdiPalette)
		{
			// backup existing vdi palette
			memset(tosVdiPal, 0, 512*3);
			for (int16 i=0; i<count; i++) {
				vq_color(screen.handle, i, 1, &tosVdiPal[i*3]);
			}
		}

		// backup existing hw palette
		if (shouldUpdateHardwarePalette)
		{
			log (" Backing up hw palette\n");
			switch (screen.vdo >> 16)
			{
				case 0:
				case 1:
				{
					volatile uint16* p = (volatile uint16*)0xFF8240;
					for (uint16 i=0; (i<count) && (i < 16); i++)
						tosHwPal[i] = p[i];
				}
				break;
				case 2:
				{
					volatile uint16* p = (volatile uint16*)0xFF8400;
					for (uint16 i=0; (i<count) && (i < 256); i++)
						tosHwPal[i] = p[i];
				}
				break;
				case 3:
				{
					#if FALCON_CONSIDER_STPAL
					if (screen.bpp == 4)
					{
						volatile uint16* pst = (volatile uint16*)0xFF8240;
						for (uint16 i=0; (i<count) && (i < 16); i++)
							tosHwPal[i+16] = pst[i];
					}
					#endif
					volatile uint32* p = (volatile uint32*)0xFF9800;
					for (uint16 i=0; (i<count) && (i < 256); i++)
						tosHwPal[i] = p[i];
				}
				break;
			}
		}
	}

	bool hasEmulatedModes = false;

	log(" Creating video modes\n");

	videoModes.clear();
	switch (screen.pf)
	{
		case MODE_APPLE_1:
			AddModes(screen.width, screen.height, VDEPTH_1BIT);
			break;
		case MODE_APPLE_2:
			AddModes(screen.width, screen.height, VDEPTH_2BIT);
			break;
		case MODE_APPLE_4:
			AddModes(screen.width, screen.height, VDEPTH_4BIT);
			break;
		case MODE_APPLE_8:
			AddModes(screen.width, screen.height, VDEPTH_8BIT);
			break;
		case MODE_APPLE_16:
			AddModes(screen.width, screen.height, VDEPTH_16BIT);
			break;
		case MODE_APPLE_32:
			AddModes(screen.width, screen.height, VDEPTH_32BIT);
			break;
	}

	if (!videoModes.empty())
	{
		log(" Video mode is Apple native\n");
	}
	else
	{
		if (supportEmulatedModes)
		{
			if (screen.width == 320)
			{
				// special treatment for st/tt low resolution
				if ((screen.pf == MODE_SHIFTER_4) || (screen.pf == MODE_SHIFTER_8))
				{
					lowRes = true;
					shouldUpdateVdiPalette = false;
					AddMode(640, 480, 0x80, 640, VDEPTH_8BIT);
				}
			}
			else if (screen.pf == MODE_SHIFTER_4)
			{
				AddModes(screen.width, screen.height, VDEPTH_8BIT);
				AddModes(screen.width, screen.height, VDEPTH_4BIT);
			}
			else if (screen.pf == MODE_SHIFTER_8)
			{
				AddModes(screen.width, screen.height, VDEPTH_8BIT);
			}
			else if (screen.bpp == 16)
			{
				AddModes(screen.width, screen.height, VDEPTH_16BIT);

				// additional 8-on-16bit mode for Falcon
				#if SUPPORT_8ON16BIT
				if ((screen.pf == MODE_RGB565_BE) && ((screen.vdo >> 16) == 3))
					AddModes(screen.width, screen.height, VDEPTH_8BIT);
				#endif
			}
			if (!videoModes.empty())
			{
				hasEmulatedModes = true;
				log(" Video mode is emulated\n");
			}
		}
		else if (supportNonPerfectModes)
		{
			if (screen.bpp == 16)
			{
				AddModes(screen.width, screen.height, VDEPTH_16BIT);
			}
			else if (screen.bpp == 32)
			{
				AddModes(screen.width, screen.height, VDEPTH_32BIT);
			}
			if (!videoModes.empty())
			{
				log(" Video mode is semi-native\n");
			}
		}
	}

	if (videoModes.empty())
	{
		ErrorAlert("Unsupported video mode");
		return NULL;
	}

	if (hasEmulatedModes)
	{
		// todo: don't just assume the first mode is the largest one. it is right now, but may not always be...
		video_mode& largestMode = videoModes[0];
		const uint32 alignSize = (4 * 1024);
		const uint32 alignMask = (alignSize - 1);

		const uint32 screenBufSize = ((((largestMode.y * largestMode.bytes_per_row) + alignMask) & ~alignMask) + (alignSize << 1));
		const uint32 hashBufSize = (screenBufSize / 32) << 2;
		const uint32 totalSize = alignSize + screenBufSize + alignSize + hashBufSize + alignSize;

		log(" Allocating %d kb for emulation buffers\n", totalSize / 1024);

		uint32 memptr = Mxalloc(totalSize, 3);
		if (memptr == 0)
		{
			ErrorAlert("Not enough memory|for video emulation");
			return NULL;
		}

		memset((void*)memptr, 0, totalSize);
		emulatedScreen = (uint8*) ((memptr + alignMask) & ~alignMask);
		hashBuffer = (uint32*) (emulatedScreen + screenBufSize + alignSize);

		log(" emulatedScreen: %08x (%d, %d, %d)\n", emulatedScreen, largestMode.x, largestMode.y, largestMode.depth);
		log(" compareBuffer:  %08x\n", hashBuffer);

		uint16 sr = DisableInterrupts();
		EnterSection(SECTION_DEBUG);

		if (PrefsFindBool("video_mmu") == false)
		{
			pageTable = 0;
			pageSize = 0;
			hashSize = 256;
		}
		else
		{
			if (HostCPUType >= 4)
			{
				pageTable = pmmu_68040_get_pagetable((uint32)emulatedScreen);
				if (pageTable)
				{
					pageSize = MMU040_PAGESIZE;
					pageShift = MMU040_PAGESHIFT;
					hashSize = 32;

					for (uint16 i=0; i<(screenBufSize / pageSize); i++)
					{
						uint32 desc = pageTable[i];
						pageTable[i] = ((desc & ~0x60L) | 0x40);	// cache-inhibit (c_precise)
					}
					FlushATC040();
					FlushCache040();
				}
			}
			else
			{
				pageTable = pmmu_68030_map((uint32)emulatedScreen, (uint32)emulatedScreen, screenBufSize, 0x40);	// cache-inhibit
				pageSize = MMU030_PAGESIZE;
				pageShift = MMU030_PAGESHIFT;
			#if 0 //MMU030_PAGESIZE < 1024
				hashSize = 0;
			#else
				hashSize = 32;
			#endif
				FlushATC030();
				FlushCache030();
				ZPState[ZEROPAGE_MAC].mmu.m030.ttr0 |=  (1<<9);	// Transparent reads
				ZPState[ZEROPAGE_MAC].mmu.m030.ttr0 &= ~(1<<8);	// RWM = 0
				FlushATC030();
				FlushCache030();
			}
		}
		if (PrefsFindBool("video_cmp") == false)
			hashSize = 0;

		ExitSection(SECTION_DEBUG);
		SetSR(sr);
		log(" pageTable:      %08x (%d %d %d)\n", pageTable, pageSize, pageShift, hashSize);
	}

#if ENABLE_VIDEODEBUG
	videoDebug = PrefsFindInt32("video_debug");
	log(" video_debug: %d\n", videoDebug);
#endif

	init_ok = true;
	MonitorDesc* m = new MonitorDesc(videoModes, videoModes[0].depth, videoModes[0].resolution_id);
	return m;
}

void VideoDriver::Release()
{
	if (!init_ok)
		return;

	if (screen.bpp <= 8)
	{
		uint16 count = (1 << screen.bpp);

		// restore original vdi palette
		if (shouldUpdateVdiPalette)
		{
			if (screen.handle)
			{
				for (int16 i=0; i<count; i++)
					vs_color(screen.handle, i, &tosVdiPal[i*3]);
			}
		}

		// restore original hw palette
		if (shouldUpdateHardwarePalette)
		{
			switch (screen.vdo >> 16)
			{
				case 0:
				case 1:
				{
					volatile uint16* p = (volatile uint16*)0xFF8240;
					for (uint16 i=0; (i < count) && (i < 16); i++)
						p[i] = tosHwPal[i];
				}
				break;
				case 2:
				{
					volatile uint16* p = (volatile uint16*)0xFF8400;
					for (uint16 i=0; (i < count) && (i < 256); i++)
						p[i] = tosHwPal[i];
				}
				break;
				case 3:
				{
					#if FALCON_CONSIDER_STPAL
					if (screen.bpp == 4)
					{
						volatile uint16* pst = (volatile uint16*)0xFF8240;
						for (uint16 i=0; (i < count) && (i < 16); i++)
							pst[i] = tosHwPal[i + 16];
					}
					#endif
					volatile uint32* p = (volatile uint32*)0xFF9800;
					for (uint16 i=0; (i < count) && (i < 256); i++)
						p[i] = tosHwPal[i];
				}
				break;
			}
		}
	}

	// todo: free memory
	init_ok = false;
}

bool VideoDriver::Setup()
{
	if (!init_ok)
		return false;

	const video_mode& mode = monitor->get_current_mode();
	uint16 sr = DisableInterrupts();
	uint16 zp = SetZeroPage(ZEROPAGE_TOS);

	blitFunc = NULL;
	shouldUpdateSoftwarePalette = false;

	if (supportEmulatedModes)
	{
		switch (mode.depth)
		{
			case VDEPTH_4BIT:
			{
				if (screen.pf != MODE_APPLE_4)
				{
					blitFunc = c2p1x1_4_from_4;
				}
			}
			break;

			case VDEPTH_8BIT:
			{
				if (screen.pf != MODE_APPLE_8)
				{
					switch (screen.bpp)
					{
						case 4:
						{
							blitFunc = lowRes ? c2p1x1_4_from_8_halfx : c2p1x1_4_from_8;
							macPaletteDirty = 256;
						}
						break;
						case 8:
						{
							if (HostCPUType >= 6)
								blitFunc = lowRes ? c2p1x1_8_from_8_halfx_060 : c2p1x1_8_from_8;
							else
								blitFunc = lowRes ? c2p1x1_8_from_8_halfx : c2p1x1_8_from_8;
						}
						break;
						case 16:
						{
							blitFunc = rgb565be_from_8bit;
							shouldUpdateSoftwarePalette = true;
							macPaletteDirty = 256;
						}
						break;
					}
				}
			}
			break;
			
			case VDEPTH_16BIT:
			{
				if (screen.pf != MODE_APPLE_16)
				{
					//uint16* p = (uint16*)softPalette;
					switch (screen.pf & (PF_MASK_FMT | PF_MASK_ENDIAN))
					{
						case PF_FMT_555 | PF_ENDIAN_LE:
						{
							blitFunc = rgb555le_from_rgb555be;
							/*
							blitFunc = rgbxxxxx_from_rgb555be;
							for (uint32 i=0; i<64*1024; i++) {
								uint16 c = i;
								p[i] = ((c << 8) | (c >> 8));
							}
							*/
						}
						break;
						case PF_FMT_565 | PF_ENDIAN_LE:
						{
							blitFunc = rgb565le_from_rgb555be;
							/*
							blitFunc = rgbxxxxx_from_rgb555be;
							for (uint32 i=0; i<64*1024; i++) {
								uint16 c = i;
								uint16 c0 = c >> 7;
								uint16 c1 = c << 8;
								uint16 c2 = (c1 << 1) & 0xC000;
								c1 &= 0x1F00;
								p[i] = (c0|c1|c2);
							}
							*/
						}
						break;
						case PF_FMT_565 | PF_ENDIAN_BE:
						{
							blitFunc = rgb565be_from_rgb555be;
							/*
							blitFunc = rgbxxxxx_from_rgb555be;
							for (uint32 i=0; i<64*1024; i++) {
								uint16 c = i;
								p[i] = (((c << 1) & 0xFFC0) | (c & 0x001F));
							}
							*/
						}
						break;
					}
				}
			}
			break;
		}
	}

	// initialize mac frame buffer base
	uint8* macFrameBase;
	if (blitFunc == NULL)
	{
		// direct mode
		macFrameBase = screen.addr;

		// vertical offset
		if (screen.height > mode.y)
			macFrameBase += ((screen.height - mode.y) >> 1) * screen.bytesPerLine;

		// horizontal offset
		if (screen.width > mode.x)
			macFrameBase += ((TrivialBytesPerRow(screen.width - mode.x, mode.depth) >> 1) & ~3L);
	}
	else
	{
		// emulated mode
		fullRedraw = true;
		macFrameBase = (uint8*)emulatedScreen;

		// horizontal offset
		if (!lowRes && (screen.width > mode.x))
			macFrameBase += ((TrivialBytesPerRow(screen.width - mode.x, mode.depth) >> 1) & ~3L);

		// blit source and destination
		blitSrc = emulatedScreen;
		blitDst = screen.addr;
		if (!lowRes && (screen.height > mode.y))
			blitDst += ((screen.height - mode.y) >> 1) * screen.bytesPerLine;

		// scale table
		if (lowRes)
		{
			uint32 offs = 0;
			uint32 step = (mode.y << 8) / screen.height;
			for (uint16 i=0; i<480; i++)
			{
				lowResOffs[i] = blitSrc + ((offs >> 8) * mode.bytes_per_row);
				offs += step;
			}
		}
	}


	SetZeroPage(zp);
	monitor->set_mac_frame_base((uint32)macFrameBase);

	// todo: show logo?
	uint8 clearColor = (screen.bpp > 8) ? 0xFF : 0x00;
	memset((void*)screen.addr, clearColor, screen.bytesPerLine * screen.height);

	SetSR(sr);

	const uint16 msx = (mode.x << 8) / screen.width;
	const uint16 msy = (mode.y << 8) / screen.height;
	InitInput(mode.x, mode.y, msx, msy);
	return true;
}

void VideoDriver::GrayPage()
{
	uint8 c = (screen.bpp > 8) ? 0x00 : 0xFF;
	memset((void*)screen.addr, c, screen.bytesPerLine * screen.height);
	if (emulatedScreen && monitor)
	{
		const video_mode& mode = monitor->get_current_mode();
		c = (mode.depth > VDEPTH_8BIT) ? 0x00 : 0xFF;
		memset(emulatedScreen, 0, mode.y * mode.bytes_per_row);
		fullRedraw = true;
	}
}

void VideoDriver::SetPalette(uint8 *pal, int num)
{
	if (num > 0)
	{
		if (num > 256)
			num = 256;

		memcpy(macPalette, pal, num+(num<<1));

		if (macPaletteDirty < num)
			macPaletteDirty = num;
	}
}

void VideoDriver::Update()
{
	static int16 frameSkipper = 0;
	if (blitFunc)
	{
		frameSkipper++;
		if (frameSkipper <= frameSkip)
			return;
		frameSkipper = 0;
	}

	if (macPaletteDirty)
	{
		UpdatePalette();
		macPaletteDirty = 0;
		if (fullRedraw)
			frameSkipper = -2;
	}

	UpdateScreen();
}

void VideoDriver::UpdateScreen()
{
	if (blitFunc == NULL)
		return;

#if ENABLE_VIDEODEBUG
	if (videoDebug & 1)
		hashSize = 0;
	if (videoDebug & 2)
		fullRedraw = true;
#endif

	uint8* src = blitSrc;
	uint8* dst = blitDst;

	const video_mode& mode = monitor->get_current_mode();
	const uint32 macScreenSize = mode.y * mode.bytes_per_row;

	uint16 srcBits;
	switch (mode.depth)
	{
		case VDEPTH_1BIT: srcBits = 1; break;
		case VDEPTH_2BIT: srcBits = 2; break;
		case VDEPTH_4BIT: srcBits = 4; break;
		case VDEPTH_8BIT: srcBits = 8; break;
		case VDEPTH_16BIT: srcBits = 16; break;
		case VDEPTH_32BIT: srcBits = 32; break;
	}

	const uint32 mmuModifiedFlag = (1 << 4);	// same bit on 030/040/060
	const uint16 pageCount = macScreenSize >> pageShift;

	if (lowRes)
	{
		if (fullRedraw || (pageTable == 0))
		{
			fullRedraw = false;
			for (uint16 y=0; y<screen.height; y++)
			{
				blitFunc(dst, lowResOffs[y], mode.bytes_per_row, softPalette);
				dst += screen.bytesPerLine;
			}
		}
		else
		{
			const uint32 dstInc = screen.bytesPerLine;
			uint16 pagesPerLine = (mode.bytes_per_row >> pageShift) + 1;
			if (pagesPerLine < 2)
				pagesPerLine = 2;

			for (uint16 y=0; y<screen.height; y++)
			{
				uint8* src = lowResOffs[y];
				uint16 page = (src - blitSrc) >> pageShift;
				uint16 lastpage = ((src + mode.bytes_per_row) - blitSrc) >> pageShift;
				uint32 desc = 0;
				while (page <= lastpage)
				{
					desc |= pageTable[page];
					page++;
				}
				if (desc & mmuModifiedFlag)
					blitFunc(dst, src, mode.bytes_per_row, softPalette);
			#if DEBUG_MMU
				else
					memset(d, 0x00, dstInc);
			#endif
				dst += dstInc;
			}

			for (uint16 i=0; i<pageCount+1; i++)
				pageTable[i] &= ~mmuModifiedFlag;

			FlushATC();
		}
		return;
	}


	if (fullRedraw)
	{
		// full redraw
		fullRedraw = false;
		blitFunc(dst, src, macScreenSize, softPalette);
	}
	else if ((pageTable == NULL) || (pageSize == 0))
	{
		// no mmu acceleration
		if ((hashBuffer == NULL) || (hashSize < 32))
		{
			// no compare acceleration
			blitFunc(dst, src, macScreenSize, softPalette);
		}
		else
		{
			// compare acceleration
			uint32 hashCount = macScreenSize / hashSize;
			uint16 hashSizeDst = ((uint32(hashSize) * screen.bpp) / srcBits) >> (lowRes ? 1 : 0);
			uint32* cmpDst = hashBuffer;
			uint16 chunkCount = hashSize >> 5;
			while (hashCount)
			{
				uint32* cmpSrc = (uint32*)src;
				uint32 oldHash = *cmpDst;
				uint32 hash = (uint32)cmpSrc;
				uint16 chunks = chunkCount;
				while (chunks)
				{
					hash += *cmpSrc++; hash += *cmpSrc++; hash += *cmpSrc++; hash += *cmpSrc++;
					hash += *cmpSrc++; hash += *cmpSrc++; hash += *cmpSrc++; hash += *cmpSrc++;
					chunks--;
				}
				if (hash != oldHash)
					blitFunc(dst, src, hashSize, softPalette);
			#if DEBUG_MMU
				else
					memset(dst, 0xFF, hashSizeDst);
			#endif
				src += hashSize;
				dst += hashSizeDst;
				*cmpDst++ = hash;
				hashCount--;
			}
		}
	}
	else
	{
		// mmu acceleration
		uint32* page = pageTable;
		const uint16 minNumDstPixelsPerC2P = 16;
		const uint16 remainBytes = (macScreenSize - (pageCount * pageSize)) & ~(minNumDstPixelsPerC2P-1);
		const uint32* lastPage = pageTable + pageCount;
		const uint32 pageSizeDst = ((uint32(pageSize) * screen.bpp) / srcBits) >> (lowRes ? 1 : 0);

		if ((hashSize == 0) || (hashBuffer == NULL))
		{
			// no compare acceleration
			while (page < lastPage)
			{
				uint32 desc = *page;
				if (desc & mmuModifiedFlag)
				{
					*page = (desc & ~mmuModifiedFlag);
					blitFunc(dst, src, pageSize, softPalette);
				}
			#if DEBUG_MMU
				else
					memset(dst, 0, pageSizeDst);
			#endif
				src += pageSize;
				dst += pageSizeDst;
				page++;
			}
		}
		else
		{
			// compare acceleration
			const uint16 hashCount = pageSize / hashSize;
			uint32* cmpDst = hashBuffer;
			uint16 hashSizeDst = ((uint32(hashSize) * screen.bpp) / srcBits) >> (lowRes ? 1 : 0);

			while (page < lastPage)
			{
				uint32 desc = *page;
				if ((desc & mmuModifiedFlag) == 0)
				{
				#if DEBUG_MMU
					memset(dst, 0x00, pageSizeDst);
				#endif
					src += pageSize;
					dst += pageSizeDst;
					cmpDst += hashCount;
				}
				else
				{
					*page = (desc & ~mmuModifiedFlag);
					uint16 count = hashCount;
					while (count)
					{
						uint32* cmpSrc = (uint32*)src;
						uint32 oldHash = *cmpDst;
						uint32 hash = (uint32)cmpSrc;
						uint16 dohash = (hashSize >> 5);
						while (dohash)
						{
							hash += *cmpSrc++; hash += *cmpSrc++; hash += *cmpSrc++; hash += *cmpSrc++;
							hash += *cmpSrc++; hash += *cmpSrc++; hash += *cmpSrc++; hash += *cmpSrc++;
							dohash--;
						}
						if (hash != oldHash)
							blitFunc(dst, src, hashSize, softPalette);
					#if DEBUG_MMU
						else
							memset(dst, 0xFF, hashSizeDst);
					#endif
						src += hashSize;
						dst += hashSizeDst;
						*cmpDst++ = hash;
						count--;
					}
				}
				page++;
			}
		}
		if (remainBytes && (*page & mmuModifiedFlag))
		{
			*page &= ~mmuModifiedFlag;
			blitFunc(dst, src, remainBytes, softPalette);
		}

		FlushATC();
	}
}


void VideoDriver::UpdatePalette()
{
	uint8* colors = macPalette;
	uint16 numcolors = macPaletteDirty;

	if (screen.bpp <= 8)
	{
		// 8 -> 4 bit mapping
		if ((screen.bpp == 4) && (monitor->get_current_mode().depth == VDEPTH_8BIT))
		{
			#define MAKE_PC_COLOR(_r,_g,_b) _r,_g,_b
			const uint8 hard8to4Palette_color[] = 
			{
				MAKE_PC_COLOR(0,0,0),
				MAKE_PC_COLOR(68,36,52),
				MAKE_PC_COLOR(48,52,109),
				MAKE_PC_COLOR(78,74,78),
				MAKE_PC_COLOR(133,76,48),
				MAKE_PC_COLOR(52,101,36),
				MAKE_PC_COLOR(208,70,72),
				MAKE_PC_COLOR(117,113,97),
				MAKE_PC_COLOR(89,125,206),
				MAKE_PC_COLOR(210,125,44),
				MAKE_PC_COLOR(133,149,161),
				MAKE_PC_COLOR(109,170,44),
				MAKE_PC_COLOR(210,170,153),
				MAKE_PC_COLOR(109,194,202),
				MAKE_PC_COLOR(218,212,94),
				MAKE_PC_COLOR(255,255,255)
			};

			const uint8 hard8to4Palette_gray[] = 
			{
				MAKE_PC_COLOR(0,0,0),
				MAKE_PC_COLOR(16,16,16),
				MAKE_PC_COLOR(32,32,32),
				MAKE_PC_COLOR(48,48,48),
				MAKE_PC_COLOR(64,64,64),
				MAKE_PC_COLOR(96,96,96),
				MAKE_PC_COLOR(128,128,128),
				MAKE_PC_COLOR(144,144,144),
				MAKE_PC_COLOR(160,160,160),
				MAKE_PC_COLOR(176,176,176),
				MAKE_PC_COLOR(192,192,192),
				MAKE_PC_COLOR(208,208,208),
				MAKE_PC_COLOR(224,224,224),
				MAKE_PC_COLOR(240,240,240),
				MAKE_PC_COLOR(255,255,255),
				MAKE_PC_COLOR(255,255,255)
			};

			uint8* spal = colors;
			uint8* hardPalette = hard8to4Palette_gray;
			for(uint16 i=0; (i<numcolors) && (hardPalette == hard8to4Palette_gray); i++)
			{
				uint8 r = *spal++ >> 2; uint8 g = *spal++ >> 2; uint8 b = *spal++ >> 2;
				if ((r != g) || (r != b))
					hardPalette = hard8to4Palette_color;
			}

			spal = colors;
			uint16 diff = 0;
			for (uint16 i=0; i<numcolors; i++)
			{
				int r = *spal++ >> 2;
				int g = *spal++ >> 2;
				int b = *spal++ >> 2;
				uint8* dpal = (uint8*)hardPalette;
				uint8 bestitem = 0;
				uint32 bestsum = 0xFFFFFFFF;
				for (uint16 j=0; j<16; j++)
				{
					int c1,c2,sum;
					c1 = *dpal++ >> 2;	// red
					c2 = (c1 > r) ? c1 - r : r - c1;
					c2 = c2 * c2;
					sum = (c2 + (c2 << 1));
					c1 = *dpal++ >> 2;	// green
					c2 = (c1 > g) ? c1 - g : g - c1;
					c2 = c2 * c2;
					sum += /*((c2<<1) +*/ (c2<<2);
					c1 = *dpal++ >> 2;	// blue
					c2 = (c1 > b) ? c1 - b : b - c1;
					c2 = c2 * c2;
					sum += (c2<<1);
					if (sum < bestsum)
					{
						bestsum = sum;
						bestitem = j;
					}
				}
				if (softPalette[i] != bestitem)
				{
					softPalette[i] = bestitem;
					diff++;
				}
			}
			if (diff == 0)
				return;
			colors = (uint8*)hardPalette;
			numcolors = 16;
			fullRedraw = true;
			shouldUpdateVdiPalette = false;
			shouldUpdateHardwarePalette = true;
			shouldUpdateSoftwarePalette = false;
		}

		// update vdi palette
		if (shouldUpdateVdiPalette)
		{
			TOS_CONTEXT();
			uint8* pal = colors;
			for (uint16 i=0; i<numcolors; i++)
			{
				uint16 c;
				uint16 rgb[3];
				c = *pal++; rgb[0] = ScaleColorValueForVdi(c);
				c = *pal++; rgb[1] = ScaleColorValueForVdi(c);
				c = *pal++;	rgb[2] = ScaleColorValueForVdi(c);
				uint8 pen = screen.vdiPen[i];
				vs_color(screen.handle, pen, (const short*) rgb);
			}
		}

		// update hardware palette
		if (shouldUpdateHardwarePalette)
		{
			uint16 max = (1 << screen.bpp);
			if (numcolors > max)
				numcolors = max;

			switch (screen.vdo >> 16)
			{
				case 0:		// ST shifter
				case 1:		// STE shifter
				{
					uint8* src = colors;
					volatile uint16* dst = (volatile uint16*)0xFF8240;
					for (uint16 i=0; (i<numcolors) && (i < 16); i++)
					{
						uint16 r = *src++; uint16 g = *src++; uint16 b = *src++;
						uint16 rgb =((r & 0xE0) << 3) | ((r & 0x10) << 7) |
									((g & 0xE0) >> 1) | ((g & 0x10) << 3) |
									((b & 0xE0) >> 5) | ((b & 0x10) >> 1);
						*dst++ = rgb;
					}
				}
				break;

				case 2:		// TT shifter
				{
					*((volatile uint16*)0xFF8262) &= ~0x7;
					uint8* src = colors;
					volatile uint16* dst = (volatile uint16*)0xFF8400;
					for (uint16 i=0; (i<numcolors) && (i < 256); i++)
					{
						uint16 r = *src++ & 0xF0;
						uint16 g = *src++ & 0xF0;
						uint16 b = *src++ & 0xF0;
						uint16 rgb = (r << 4) | g | (b >> 4);
						*dst++ = rgb;
					}
				}
				break;

				case 3:		// Falcon Videl
				{
	 				#if FALCON_CONSIDER_STPAL
					if (screen.bpp == 4)
					{
						uint8* src = colors;
						volatile uint16* dst = (volatile uint16*)0xFF8240;
						for (uint16 i=0; (i<numcolors) && (i < 16); i++)
						{
							uint16 r = *src++; uint16 g = *src++; uint16 b = *src++;
							uint16 rgb =((r & 0xE0) << 3) | ((r & 0x10) << 7) |
										((g & 0xE0) >> 1) | ((g & 0x10) << 3) |
										((b & 0xE0) >> 5) | ((b & 0x10) >> 1);
							*dst++ = rgb;
						}
						memset((void*)0xFF9800, 0, 256*4);
					}
					#endif
					{
						uint8* src = colors;
						volatile uint32* dst = (volatile uint32*)0xFF9800;
						for (uint16 i=0; (i<numcolors) && (i < 256); i++)
						{
							uint32 r = *src++ & 0xFC;
							uint32 g = *src++ & 0xFC;
							uint32 b = *src++ & 0xFC;
							uint32 rgb = (r << 24) | (g << 16) | b;
							*dst++ = rgb;
						}
					}
				}

				case 4:		// Milan
				{
				}
				break;
			}
		}
	}
	else if (shouldUpdateSoftwarePalette)
	{
		uint8* pal = colors;
		switch (screen.pf & PF_MASK_BITS)
		{
			case PF_BITS_16:
			{
				fullRedraw = true;
				uint16* colorMap = (uint16*)softPalette;
				switch (screen.pf & (PF_MASK_ENDIAN | PF_MASK_FMT))
				{
					case (PF_ENDIAN_BE | PF_FMT_555):
					{
						for (uint16 i=0; i<numcolors; i++)
						{
							uint16 r = *pal++ & 0xF8;
							uint16 g = *pal++ & 0xF8;
							uint16 b = *pal++ & 0xF8;
							colorMap[i] = (r << 7) | (g << 2) | (b >> 3);
						}
					}
					break;
					case (PF_ENDIAN_LE | PF_FMT_555):
					{
						for (uint16 i=0; i<numcolors; i++)
						{
							uint16 r = *pal++ & 0xF8;
							uint16 g = *pal++ & 0xF8;
							uint16 b = *pal++ & 0xF8;
							colorMap[i] = (g << 10) | (b << 5) | (r >> 1) | (g >> 6);
						}
					}
					break;
					case (PF_ENDIAN_BE | PF_FMT_565):
					{
						for (uint16 i=0; i<numcolors; i++)
						{
							uint16 r = *pal++ & 0xF8;
							uint16 g = *pal++ & 0xF8;
							uint16 b = *pal++ & 0xF8;
							colorMap[i] = (r << 8) | (g << 3) | (b >> 3);
						}				
					}
					break;
					case (PF_ENDIAN_LE | PF_FMT_565):
					{
						for (uint16 i=0; i<numcolors; i++)
						{
							uint16 r = *pal++ & 0xF8;
							uint16 g = *pal++ & 0xF8;
							uint16 b = *pal++ & 0xF8;
							colorMap[i] = (g << 10) | (b << 5) | (r) | (g >> 5);
						}
					}
					break;
				}
			}
			break;

			case PF_BITS_32:
			{

			}
			break;
		}
	}
}

void VideoDriver::AddMode(uint32 width, uint32 height, uint32 resolution_id, uint32 bytes_per_row, video_depth depth)
{
	video_mode mode;
	mode.x = width;
	mode.y = height;
	mode.resolution_id = resolution_id;
	mode.bytes_per_row = bytes_per_row;
	mode.depth = depth;
	D(bug("   Added video mode: w=%ld  h=%ld  d=%ld\n", width, height, depth));
	videoModes.push_back(mode);
}

void VideoDriver::AddModes(uint32 width, uint32 height, video_depth depth)
{
	// add requested mode
	uint32 id = 0x80;
	AddMode(width, height, id++, TrivialBytesPerRow(width, depth), depth);

	// add standard mac modes
	if (width > 1600 && height > 1200)
		AddMode(1600, 1200, id++, TrivialBytesPerRow(width, depth), depth);
	if (width > 1280 && height > 1024)
		AddMode(1280, 1024, id++, TrivialBytesPerRow(width, depth), depth);
	if (width > 1152 && height > 870)
		AddMode(1152, 870, id++, TrivialBytesPerRow(width, depth), depth);
	if (width > 1024 && height > 768)
		AddMode(1024, 768, id++, TrivialBytesPerRow(width, depth), depth);
	if (width > 800 && height > 600)
		AddMode(800, 600, id++, TrivialBytesPerRow(width, depth), depth);
	if (width > 640 && height > 480)
		AddMode(640, 480, id++, TrivialBytesPerRow(width, depth), depth);
	if (width > 512 && height > 384)
		AddMode(512, 384, id++, TrivialBytesPerRow(width, depth), depth);
	if (width > 512 && height > 342)
		AddMode(512, 342, id++, TrivialBytesPerRow(width, depth), depth);
}

