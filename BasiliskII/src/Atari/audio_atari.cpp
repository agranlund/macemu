/*
 *  audio_atari.cpp - Audio support, Atari implementation
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
#include "prefs.h"
#include "audio.h"
#include "audio_defs.h"
#include "zeropage.h"
#include "mint/mintbind.h"
#include "mint/sysbind.h"
#include "mint/cookie.h"

#define DEBUG 0
#include "debug.h"


#define SND_DRV_DUMMY		0
#define SND_DRV_DMA			1
#define SND_DRV_YM			2
#define SND_DRV_COVOX		3
#define SND_DRV_MV16		4
#define SND_DRV_REPLAY8		5
#define SND_DRV_REPLAY8S	6
#define SND_DRV_REPLAY16	7

#define ENABLE_DYNAMIC_REFCONF	0

class sndDriver
{
public:
	sndDriver() {}
	virtual ~sndDriver() {}

	virtual bool Init() = 0;
	virtual void Release() = 0;
	virtual void Configure() = 0;
	virtual void StartStream() = 0;
	virtual void StopStream() = 0;
	virtual uint32 GetPos() = 0;
	virtual void Fill(uint8* dst, uint8* src, uint32 len) = 0;
};

class sndDriverDummy : public sndDriver
{
public:
	sndDriverDummy() {}
	~sndDriverDummy() {}
	bool Init();
	void Release() {}
	void Configure() {}
	void StartStream() {}
	void StopStream() {}
	uint32 GetPos();
	void Fill(uint8* dst, uint8* src, uint32 len) {}
};

class sndDriverDMA : public sndDriver
{
public:
	sndDriverDMA() {}
	~sndDriverDMA() {}
	bool Init();
	void Release();
	void Configure();
	void StartStream();
	void StopStream();
	uint32 GetPos();
	void Fill(uint8* dst, uint8* src, uint32 len);
};

class sndDriverST : public sndDriver
{
public:
	sndDriverST(uint16 _drv)
	{
		drv = _drv; ctrl = 0;
		irqInstalled = false;
		streamStarted = false;
	}
	~sndDriverST() {}
	bool Init();
	void Release();
	void Configure();
	void StartStream();
	void StopStream();
	uint32 GetPos();
	void Fill(uint8* dst, uint8* src, uint32 len);
	static volatile uint16 pos;
private:
	void InstallIrq();
	void RemoveIrq();
	uint16 drv;
	uint16 ctrl;
	bool irqInstalled;
	bool streamStarted;
};


#define MAC_MAX_VOLUME 0x0100

// The currently selected audio parameters (indices in audio_sample_rates[] etc. vectors)
static int audio_sample_rate_index = 0;
static int audio_sample_size_index = 0;
static int audio_channel_count_index = 0;

// Global variables
static bool main_mute = false;
static bool speaker_mute = false;

static int main_volume = MAC_MAX_VOLUME;
static int speaker_volume = MAC_MAX_VOLUME;

sndDriver* snd = NULL;
uint8* snd_buffer = NULL;
uint32 snd_buffer_size = 0;
uint32 snd_buffer_halfsize = 0;
uint32 snd_buffer_pos = 0;
uint32 snd_buffer_maxsize = (2048 * 2 * 2 * 2);
bool snd_stream_on = false;

extern bool TosIrqSafe;
extern bool isFalcon;

// Set AudioStatus to reflect current audio stream format
static void set_audio_status_format(void)
{
	AudioStatus.sample_rate = audio_sample_rates[audio_sample_rate_index];
	AudioStatus.sample_size = audio_sample_sizes[audio_sample_size_index];
	AudioStatus.channels = audio_channel_counts[audio_channel_count_index];
}

// reconfigure buffers and driver
void audio_configure()
{
	if (!audio_open)
		return;

	uint32 newsize = (AudioStatus.sample_size >> 3) * AudioStatus.channels * audio_frames_per_block * 2;
	if (newsize > snd_buffer_maxsize)
		newsize = snd_buffer_maxsize;

	uint16 sr = DisableInterrupts();
	snd_buffer_size = newsize;
	snd_buffer_halfsize = snd_buffer_size >> 1;
	snd_buffer_pos = 0;
	if (snd)
		snd->Configure();
	SetSR(sr);
}

bool audio_set_sample_rate_byval(uint32 value)
{
	bool changed = (AudioStatus.sample_rate != value);
	if(changed)
	{
		uint16 idx = 0;
		uint32 v0 = value >> 16;
		uint32 diff = 0xFFFFFFFF;
		for (uint16 i=0; i<audio_sample_rates.size(); i++)
		{
			uint32 v1 = audio_sample_rates[i] >> 16;
			uint32 d = (v1 > v0) ? (v1 - v0) : (v0 - v1);
			if (d < diff)
			{
				diff = d;
				idx = i;
			}
		}
		if (idx != audio_sample_rate_index)
		{
			audio_sample_rate_index = idx;
			AudioStatus.sample_rate = audio_sample_rates[idx];
			return true;
		}
	}
	return false;
}

bool audio_set_sample_size_byval(uint32 value)
{
	bool changed = (AudioStatus.sample_size != value);
	if(changed)
	{
		uint16 idx = 0;
		uint32 diff = 0xFFFFFFFF;
		for (uint16 i=0; i<audio_sample_sizes.size(); i++)
		{
			uint32 v = audio_sample_sizes[i];
			uint32 d = (v > value) ? (v - value) : (value - v);
			if (d < diff)
			{
				diff = d;
				idx = i;
			}
		}
		if (idx != audio_sample_size_index)
		{
			audio_sample_size_index = idx;
			AudioStatus.sample_size = audio_sample_sizes[idx];
			return true;
		}
	}
	return false;
}

bool audio_set_channels_byval(uint32 value)
{
	bool changed = (AudioStatus.channels != value);
	if(changed)
	{
		uint16 idx = 0;
		uint32 diff = 0xFFFFFFFF;
		for (uint16 i=0; i<audio_channel_counts.size(); i++)
		{
			uint32 v = audio_channel_counts[i];
			uint32 d = (v > value) ? (v - value) : (value - v);
			if (d < diff)
			{
				diff = d;
				idx = i;
			}
		}
		if (idx != audio_channel_count_index)
		{
			audio_channel_count_index = idx;
			AudioStatus.channels = audio_channel_counts[idx];
			return true;
		}
	}
	return false;
}

static bool open_audio(void)
{
	set_audio_status_format();
	audio_open = true;
	return audio_open;
}

static void close_audio(void)
{
	audio_open = false;
}


void AudioInit(void)
{
	log("AudioInit\n");

	audio_open = false;
	AudioStatus.mixer = 0;
	AudioStatus.num_sources = 0;

	audio_component_flags = cmpWantsRegisterMessage | kStereoOut | k16BitOut;
	audio_frames_per_block = 2048;

	audio_sample_rates.push_back(44100 << 16);
	audio_sample_rates.push_back(22050 << 16);
	audio_sample_rates.push_back(11025 << 16);
	audio_sample_sizes.push_back(16);
	audio_sample_sizes.push_back(8);
	audio_channel_counts.push_back(2);
	audio_channel_counts.push_back(1);
	audio_sample_rate_index = 1;		// 22050hz
	audio_sample_size_index = 1;		// 8bit
	audio_channel_count_index = 1;		// mono

	set_audio_status_format();

	// Sound disabled in prefs? Then do nothing
	if (PrefsFindBool("nosound"))
	{
		log("AudioInit : nosound\n");
		return;
	}

	uint32 cookie = 0;
	Getcookie('_SND', &cookie);
	bool haveYM = cookie & 1;
	bool haveDMA = cookie & 2;
	uint16 driver = (uint16) PrefsFindInt32("sound_driver");
	if ((driver == SND_DRV_DMA) && !haveDMA)
		driver = SND_DRV_YM;
	if ((driver == SND_DRV_YM) && !haveYM)
		driver = SND_DRV_DUMMY;

	// create buffer
	snd_buffer_maxsize = audio_frames_per_block * 2 * 2 * 2;	// 16 bit stereo * 2 buffers
	snd_buffer = Mxalloc((snd_buffer_maxsize<<1), (driver == SND_DRV_DMA) ? 0 : 3) + snd_buffer_maxsize;
	if (!snd_buffer)
	{
		D(bug(" Err: failed allocating sound buffer\n"));
		return;
	}
	memset(snd_buffer, 0, snd_buffer_maxsize);

	// create sound driver
	switch (driver)
	{
		case SND_DRV_DMA:
			snd = new sndDriverDMA();
			break;
		case SND_DRV_YM:
		case SND_DRV_COVOX:
		case SND_DRV_MV16:
		case SND_DRV_REPLAY8:
		case SND_DRV_REPLAY8S:
		case SND_DRV_REPLAY16:
			snd = new sndDriverST(driver);
			break;
		default:
			snd = new sndDriverDummy();
			break;
	}

	if (!snd) {
		D(bug(" Err: failed creating sound driver\n"));
		return;
	}

	// init sound driver
	if (!snd->Init()) {
		D(bug(" Err: failed initing sound driver\n"));
		delete snd;
		snd = NULL;
		return;
	}


	uint32 default_freq 	= ((uint32)PrefsFindInt32("sound_freq")) << 16;
	uint32 default_channels = (uint32)PrefsFindInt32("sound_channels");
	uint32 default_bits 	= (uint32)PrefsFindInt32("sound_bits");
	if (default_freq == 0)		default_freq = (22050 << 16);
	if (default_channels == 0)	default_channels = 1;
	if (default_bits == 0)		default_bits = 8;
	log(" req hz:%u sz:%d ch:%d\n", default_freq>>16, default_bits, default_channels);

	// reconfigure
	audio_frames_per_block = (snd_buffer_maxsize >> 3);
	set_audio_status_format();
	audio_set_sample_rate_byval(default_freq);
	audio_set_sample_size_byval(default_bits);
	audio_set_channels_byval(default_channels);
	log(" got hz:%u sz:%d ch:%d\n", AudioStatus.sample_rate>>16, AudioStatus.sample_size, AudioStatus.channels);
	open_audio();
	audio_configure();
	log(" drv=%d, %08x, buf=%08x, len=%d\n", driver, (uint32)snd, (uint32)snd_buffer, snd_buffer_halfsize);
}

void AudioExit(void)
{
	D(bug("AudioExit\n"));
	if (audio_open)
	{
		close_audio();
		if (snd)
		{
			snd->Release();
			delete snd;
			snd = NULL;
		}
		if (snd_buffer)
		{
			Mfree(snd_buffer);
			snd_buffer = NULL;
		}

		// save settings
		if (!PrefsFindBool("nosound") && (PrefsFindInt32("sound_driver") != SND_DRV_DUMMY))
		{
			PrefsReplaceInt32("sound_freq", AudioStatus.sample_rate >> 16);
			PrefsReplaceInt32("sound_channels", AudioStatus.channels);
			PrefsReplaceInt32("sound_bits", AudioStatus.sample_size);
			SavePrefs();
		}
	}
}


/*
 *  First source added, start audio stream
 */

void audio_enter_stream()
{
	//D(bug("audio_enter_stream\n"));
	if (snd)
		snd->StartStream();
	snd_stream_on = true;
}


/*
 *  Last source removed, stop audio stream
 */

void audio_exit_stream()
{
	//D(bug("audio_exit_stream\n"));
	//if (snd)
	//	snd->StopStream();
	snd_stream_on = false;
}


/*
 *  MacOS audio interrupt, read next data block
 */

void AudioInterrupt(void)
{
	if (!snd || !snd_buffer)
		return;

	uint32 old_snd_buffer_pos = snd_buffer_pos;
	snd_buffer_pos = snd->GetPos() & (snd_buffer_size - 1);
	uint8* refill_ptr = 0;
	if ((snd_buffer_pos < snd_buffer_halfsize) && (old_snd_buffer_pos >= snd_buffer_halfsize))
		refill_ptr = snd_buffer + snd_buffer_halfsize;
	else if ((snd_buffer_pos >= snd_buffer_halfsize) && (old_snd_buffer_pos < snd_buffer_halfsize))
		refill_ptr = snd_buffer;
	else
		return;

	// delayed stream off
	const int16 offdelay = 8;
	static int16 offtimer = 0;
	if (!snd_stream_on)
	{
		if (offtimer < offdelay)
		{
			memset(refill_ptr, 0, snd_buffer_halfsize);
			offtimer++;
			if (offtimer == offdelay)
				snd->StopStream();
		}
		return;
	}

	// play
	offtimer = 0;
	uint32 work_size = 0;
	bool muted = main_mute;
	if (audio_open && audio_data && AudioStatus.num_sources)
	{
		uint16 zp = SetZeroPage(ZEROPAGE_MAC);
		if (AudioStatus.mixer)
		{
			M68kRegisters r;
			r.a[0] = audio_data + adatStreamInfo;
			r.a[1] = AudioStatus.mixer;
			Execute68k(audio_data + adatGetSourceData, &r);
		}
		else
		{
			WriteMacInt32(audio_data + adatStreamInfo, 0);
		}
	
		uint32 apple_stream_info = ReadMacInt32(audio_data + adatStreamInfo);
		if (apple_stream_info && !muted)
		{
			int32 sample_count = ReadMacInt32(apple_stream_info + scd_sampleCount);
			//D(bug("sample count = %d\n", sample_count));
			
		#if ENABLE_DYNAMIC_REFCONF
			if(sample_count != 0)
			{
				bool changed = false;
				uint32 sample_rate = ReadMacInt32(apple_stream_info + scd_sampleRate);
				if(sample_rate != AudioStatus.sample_rate) {
					changed |= audio_set_sample_rate_byval(sample_rate);
				}
				uint32 num_channels = ReadMacInt16(apple_stream_info + scd_numChannels);
				if(num_channels != AudioStatus.channels) {
					changed |= audio_set_channels_byval(num_channels);
				}
				uint32 sample_size = ReadMacInt16(apple_stream_info + scd_sampleSize);
				if(sample_size != AudioStatus.sample_size) {
					changed |= audio_set_sample_size_byval(sample_size);
				}
				if (changed) {
					D(bug("audio_mid_play_configure hz:%d sz:%d ch:%d\n", sample_rate >> 16, sample_size, num_channels));
					uint16 sr = DisableInterrupts();
					audio_configure();
					memset(snd_buffer, 0, snd_buffer_halfsize);
					refill_ptr = snd_buffer + snd_buffer_halfsize;
					snd_buffer_pos = 0;
					SetSR(sr);
				}
			}
		#endif

			work_size = sample_count * (AudioStatus.sample_size >> 3) * AudioStatus.channels;
			if (work_size > 0)
			{
				uint32 apple_buffer = ReadMacInt32(apple_stream_info + scd_buffer);
				D(bug("apple buffer = %x\n", apple_buffer));
				if (work_size > snd_buffer_halfsize)
					work_size = snd_buffer_halfsize;

				snd->Fill(refill_ptr, apple_buffer, work_size);
				if (work_size < snd_buffer_halfsize)
					memset(refill_ptr + work_size, 0, snd_buffer_halfsize - work_size);
			}
		}
		SetZeroPage(zp);
	}

	if (work_size == 0)
	{
		memset(refill_ptr, 0, snd_buffer_halfsize);
	}
}


/*
 *  Set sampling parameters
 *  "index" is an index into the audio_sample_rates[] etc. vectors
 *  It is guaranteed that AudioStatus.num_sources == 0
 */

bool audio_set_sample_rate(int index)
{
	audio_sample_rate_index = index;
	set_audio_status_format();
	audio_configure();
	return true;
}

bool audio_set_sample_size(int index)
{
	audio_sample_size_index = index;
	set_audio_status_format();
	audio_configure();
	return true;
}

bool audio_set_channels(int index)
{
	audio_channel_count_index = index;
	set_audio_status_format();
	audio_configure();
	return true;
}

/*
 *  Get/set volume con::trols (volume values received/returned have the left channel
 *  volume in the upper 16 bits and the right channel volume in the lower 16 bits;
 *  both volumes are 8.8 fixed point values with 0x0100 meaning "maximum volume"))
 */

bool audio_get_main_mute(void)
{
	D(bug("audio_get_main_mute:  mute=%ld\n", main_mute));
	return main_mute;
}

uint32 audio_get_main_volume(void)
{
	D(bug("audio_get_main_volume\n"));
	uint32 volume = main_volume;
	D(bug("audio_get_main_volume: volume=%04lx\n", volume));
	return (volume << 16) + volume;
}

bool audio_get_speaker_mute(void)
{
	D(bug("audio_get_speaker_mute:  mute=%ld\n", speaker_mute));
	return speaker_mute;
}

uint32 audio_get_speaker_volume(void)
{
	D(bug("audio_get_speaker_volume: \n"));
	if (audio_open)
	{
		uint32 volume = speaker_volume;
		D(bug("audio_get_speaker_volume: volume=%04lx\n", volume));
		return (volume << 16) + volume;
	}
	return 0x01000100;
}

void audio_set_main_mute(bool mute)
{
	D(bug("audio_set_main_mute: mute=%ld\n", mute));
	main_mute = mute;
}

void audio_set_main_volume(uint32 vol)
{
	D(bug("audio_set_main_volume: vol=%08lx\n", vol));
	if (audio_open)
	{
		uint32 volume = vol >> 16;
		D(bug("audio_set_main_volume: volume=%08lx\n", volume));
		main_volume = volume;
	}
}

void audio_set_speaker_mute(bool mute)
{
	speaker_mute = mute;
}

void audio_set_speaker_volume(uint32 vol)
{
	D(bug("audio_set_speaker_volume: vol=%08lx\n", vol));
	if (audio_open)
	{
		uint32 volume = vol >> 16;
		D(bug("audio_set_speaker_volume: volume=%08lx\n", volume));
		speaker_volume = volume;
	}
}

//------------------------------------------------------
//
// Dummy sound driver
//
//------------------------------------------------------
bool sndDriverDummy::Init()
{
	audio_sample_rates.clear();
	audio_sample_rates.push_back(44100 << 16);
	audio_sample_rates.push_back(22050 << 16);
	audio_sample_rates.push_back(11025 << 16);
	audio_sample_sizes.clear();
	audio_sample_sizes.push_back(16);
	audio_sample_sizes.push_back(8);
	audio_channel_counts.clear();
	audio_channel_counts.push_back(2);
	audio_channel_counts.push_back(1);
	audio_sample_rate_index = 1;		// 22050hz
	audio_sample_size_index = 1;		// 8bit
	audio_channel_count_index = 1;		// mono
	return true;
}

uint32 sndDriverDummy::GetPos()
{
	uint32 bytesPerSecond = (AudioStatus.sample_rate >> 16) * (AudioStatus.sample_size >> 3) * AudioStatus.channels;
	uint32 p = snd_buffer_pos + (bytesPerSecond >> 6);
	return p;
}


//------------------------------------------------------
//
// DMA sound driver
//
//------------------------------------------------------
#ifndef Devconnect
#define Devconnect(a,b,c,d,e) (long)trap_14_wwwwww(0x8b,(short)(a),(short)(b),(short)(c),(short)(d),(short)(e))
#endif
#ifndef Soundcmd
#define Soundcmd(a,b) (long)trap_14_www(0x82,(short)(a),(short)(b))
#endif
#ifndef Setmode
#define Setmode(a) (long)trap_14_ww(0x84,(short)(a))
#endif

bool sndDriverDMA::Init()
{
	if (isFalcon)
	{
		// Make STE/TT compatible
		Devconnect(0,8,0,0,1);	// src = DMA, dst = D/A, clk=internal, prescale=STE compatible, handshake=off
		Setmode(0);				// 8bit stereo
		Soundcmd(2, 64);		// left gain
		Soundcmd(3, 64);		// right gain
		Soundcmd(6, 3);			// STE compatible. Prescale 160
		Soundcmd(4, 3);			// adder input: A/D & matrix
		Soundcmd(5, 3);			// A/D input: Left+Right		
	}

	audio_sample_rates.clear();
	audio_sample_rates.push_back(50066 << 16);
	audio_sample_rates.push_back(25033 << 16);
	audio_sample_rates.push_back(12517 << 16);
	if (!isFalcon)
		audio_sample_rates.push_back(6258 << 16);
	
	audio_sample_sizes.clear();
	if (isFalcon)
		audio_sample_sizes.push_back(16);
	audio_sample_sizes.push_back(8);

	audio_channel_counts.clear();
	audio_channel_counts.push_back(2);
	audio_channel_counts.push_back(1);		

	audio_sample_rate_index = 1;
	audio_sample_size_index = isFalcon ? 1 : 0;
	audio_channel_count_index = 1;

	uint32 en = (uint32)(snd_buffer + snd_buffer_size);
	uint32 st = (uint32)(snd_buffer);
	*((volatile uint8*)0x00FF8901) = 0;						// off
	*((volatile uint8*)0x00FF890F) = ((en >> 16) & 0xFF);	// end
	*((volatile uint8*)0x00FF8911) = ((en >>  8) & 0xFF);
	*((volatile uint8*)0x00FF8913) = ((en >>  0) & 0xFF);
	*((volatile uint8*)0x00FF8903) = ((st >> 16) & 0xFF);	// start
	*((volatile uint8*)0x00FF8905) = ((st >>  8) & 0xFF);
	*((volatile uint8*)0x00FF8907) = ((st >>  0) & 0xFF);
	*((volatile uint8*)0x00FF8901) = (1 << 1) | (1 << 0);	// loop mode
	return true;
}

void sndDriverDMA::Release()
{
	*((volatile uint8*)0x00FF8901) = 0;	// off
}

void sndDriverDMA::Configure()
{
	uint32 rateFlag = 0;
	uint8 bitFlag = AudioStatus.sample_size == 16 ? 1 : 0;
	uint8 stereoFlag = AudioStatus.channels == 1 ? 1 : 0;
	uint32 rate = AudioStatus.sample_rate >> 16;
	if (rate >= 50066)
		rateFlag = 3;
	else if (rate >= 25033)
		rateFlag = 2;
	else if (rate >= 12517)
		rateFlag = 1;

	D(bug("rate = %d, idx = %d\n", rate, rateFlag));

	uint8 wasEnabled = *((volatile uint8*)0x00FF8901) & 1;
	*((volatile uint8*)0x00FF8901) &= ~1;
	uint8 d = (stereoFlag << 7) | (bitFlag << 6) | (rateFlag & 3);
	*((volatile uint8*)0x00FF8921) = d;
	uint32 en = (uint32)(snd_buffer + snd_buffer_size);
	*((volatile uint8*)0x00FF890F) = ((en >> 16) & 0xFF);
	*((volatile uint8*)0x00FF8911) = ((en >>  8) & 0xFF);
	*((volatile uint8*)0x00FF8913) = ((en >>  0) & 0xFF);
	uint32 st = (uint32)(snd_buffer);
	*((volatile uint8*)0x00FF8903) = ((st >> 16) & 0xFF);
	*((volatile uint8*)0x00FF8905) = ((st >>  8) & 0xFF);
	*((volatile uint8*)0x00FF8907) = ((st >>  0) & 0xFF);
	*((volatile uint8*)0x00FF8901) |= wasEnabled;
}

void sndDriverDMA::StartStream()
{
	//*((volatile uint8*)0x00FF8901) |= 1;	// enable
}

void sndDriverDMA::StopStream()
{
	//*((volatile uint8*)0x00FF8901) &= ~1;	// disable
}

uint32 sndDriverDMA::GetPos()
{
	uint32 addr = *((volatile uint8*)0x00FF8909);
	addr <<= 8;
	addr |= *((volatile uint8*)0x00FF890B);
	addr <<= 8;
	addr |= *((volatile uint8*)0x00FF890D);
	addr -= (uint32)snd_buffer;
	return addr;
}

void sndDriverDMA::Fill(uint8* dst, uint8* src, uint32 len)
{
	if (audio_sample_sizes[audio_sample_size_index] == 16)
	{
		// unsigned 16bit samples
		uint32 *s = (uint32 *)src;
		uint32 *d = (uint32 *)dst;
		uint16 l = (uint16) (len >> 2);
		while (l--)
			*d++ = *s++;
	}
	else
	{
		// unsigned -> signed 8bit samples
		uint32 *s = (uint32 *)src;
		uint32 *d = (uint32 *)dst;
		uint16 l = (uint16) (len >> 2);
		while (l--)
			*d++ = *s++ ^ 0x80808080;
	}
}

//------------------------------------------------------
//
// Software sound driver (YM/Car/Printer)
//
//------------------------------------------------------
volatile uint16 sndDriverST::pos = 0;
extern const uint16 _soundTable_U8[];

#define YM2149_REGSELECT	((volatile uint8*)0xFFFF8800)
#define YM2149_REGDATA		((volatile uint8*)0xFFFF8802)

/*
#define YM2149_WR(r,d) {				\
	uint16 v = (r << 8) | d; 			\
	uint8* a = (uint8*) 0xFFFF8800;		\
	__asm__ volatile					\
	(									\
		"movep.w	%0,0(%1)\n\t"		\
	: : "d"(v), "a"(a) : "memory", "cc"	\
	); \
}
*/
#define YM2149_WR(r,d) { *YM2149_REGSELECT = (uint8) (r); *YM2149_REGDATA = d; }
#define YM2149_RD(r,d) { *YM2149_REGSELECT = (uint8) (r); d = *YM2149_REGSELECT; }
#define YM2149_WR_MASKED(r,d,m) { uint8 b = 0; YM2149_RD(r, b); b &= ~(m); b |= ((d)&(m)); YM2149_WR(r, b); }

static void __attribute__ ((interrupt)) timerA_YM(void)
{
	volatile uint8* sample = &snd_buffer[sndDriverST::pos];
	__asm__ volatile							\
	(											\
		"moveq.l	#0,d3\n\t"					\
		"move.b		(%0),d3\n\t"				\
		"lsl.w		#3,d3\n\t"					\
		"move.l		0(%1,d3.w),d5\n\t"			\
		"move.l		4(%1,d3.w),d4\n\t"			\
		"lea.l		0x00ff8800,a4\n\t"			\
		"movep.l	d5,0(a4)\n\t"				\
		"movep.w	d4,0(a4)\n\t"				\
	: 											\
	: "a"(sample), "a"(_soundTable_U8)			\
	: "d3", "d4", "d5", "a4", "memory", "cc"	\
	);
	sndDriverST::pos = (sndDriverST::pos + 1) & (snd_buffer_size - 1);
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}

static void __attribute__ ((interrupt)) timerA_YM_Falcon(void)
{
	volatile uint8* sample = &snd_buffer[sndDriverST::pos];
	__asm__ volatile							\
	(											\
		"moveq.l	#0,d3\n\t"					\
		"move.b		(%0),d3\n\t"				\
		"lsl.w		#3,d3\n\t"					\
		"move.l		0(%1,d3.w),d5\n\t"			\
		"move.l		4(%1,d3.w),d4\n\t"			\
		"lea.l		0x00ff8800,a4\n\t"			\
		"movep.w	d5,0(a4)\n\t"				\
		"swap		d5\n\t"						\
		"movep.w	d5,0(a4)\n\t"				\
		"movep.w	d4,0(a4)\n\t"				\
	: 											\
	: "a"(sample), "a"(_soundTable_U8)			\
	: "d3", "d4", "d5", "a4", "memory", "cc"	\
	);
	sndDriverST::pos = (sndDriverST::pos + 1) & (snd_buffer_size - 1);
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}


static void __attribute__ ((interrupt)) timerA_YM_040(void)
{
	volatile uint8* sample = &snd_buffer[sndDriverST::pos];
	__asm__ volatile							\
	(											\
		"moveq.l	#0,d3\n\t"					\
		"move.b		(%0),d3\n\t"				\
		"lsl.w		#3,d3\n\t"					\
		"add.l		d3,%1\n\t"					\
		"move.b		(%1)+,0x00ff8800\n\t"		\
		"move.b		(%1)+,0x00ff8802\n\t"		\
		"move.b		(%1)+,0x00ff8800\n\t"		\
		"move.b		(%1)+,0x00ff8802\n\t"		\
		"move.b		(%1)+,0x00ff8800\n\t"		\
		"move.b		(%1)+,0x00ff8802\n\t"		\
	: 											\
	: "a"(sample), "a"(_soundTable_U8)			\
	: "d3", "memory", "cc"	\
	);
	sndDriverST::pos = (sndDriverST::pos + 1) & (snd_buffer_size - 1);
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}

static void __attribute__ ((interrupt)) timerA_NULL(void)
{
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}

static void __attribute__ ((interrupt)) timerA_COVOX(void)
{
	uint8 sample = snd_buffer[sndDriverST::pos];
	YM2149_WR(15, sample);
	sndDriverST::pos = (sndDriverST::pos + 1) & (snd_buffer_size - 1);
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}

volatile uint8 cart_read_to_write;
static void __attribute__ ((interrupt)) timerA_MV16(void)
{
	uint16 sample = snd_buffer[sndDriverST::pos] << 4;
	cart_read_to_write = *((volatile uint8*)(0x00fa0000 + sample));
	sndDriverST::pos = (sndDriverST::pos + 1) & (snd_buffer_size - 1);
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}

static void __attribute__ ((interrupt)) timerA_Replay8(void)
{
	uint16 sample = snd_buffer[sndDriverST::pos] << 1;
	cart_read_to_write = *((volatile uint8*)(0x00fa0000 + sample));
	sndDriverST::pos = (sndDriverST::pos + 1) & (snd_buffer_size - 1);
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}

static void __attribute__ ((interrupt)) timerA_Replay8S(void)
{
	uint16 sample = *((uint16*)(&snd_buffer[sndDriverST::pos]));
	cart_read_to_write = *((volatile uint8*)(0x00fa0000 + ((sample << 1) & 0x1FE)));
	cart_read_to_write = *((volatile uint8*)(0x00fa0200 + ((sample >> 7) & 0x1FE)));
	sndDriverST::pos = (sndDriverST::pos + 2) & (snd_buffer_size - 1);
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}

static void __attribute__ ((interrupt)) timerA_Replay16(void)
{
	uint16 sample = *((uint16*)(&snd_buffer[sndDriverST::pos])) >> 1;
	cart_read_to_write = *((volatile uint8*)(0x00fa0000 + sample));
	sndDriverST::pos = (sndDriverST::pos + 2) & (snd_buffer_size - 1);
	*TIMERA_REG_SERVICE = ~TIMERA_MASK_ENABLE;
}

bool sndDriverST::Init()
{
	audio_sample_rates.clear();
	audio_sample_rates.push_back(22050 << 16);
	audio_sample_rates.push_back(11025 << 16);
	audio_sample_rates.push_back(5512 << 16);
	
	audio_sample_sizes.clear();
	if (drv == SND_DRV_REPLAY16)
		audio_sample_sizes.push_back(16);
	else
		audio_sample_sizes.push_back(8);

	audio_channel_counts.clear();
	if (drv == SND_DRV_REPLAY8S)
		audio_channel_counts.push_back(2);
	else
		audio_channel_counts.push_back(1);

	audio_sample_rate_index = 0;
	audio_sample_size_index = 0;
	audio_channel_count_index = 0;

	InstallIrq();
	return true;
}

void sndDriverST::Release()
{
	RemoveIrq();
}

void sndDriverST::InstallIrq()
{
	const uint32 dividers[7] = {4, 10, 16, 50, 64, 100, 200};
	const uint32 baseclk = 2457600;

	int hz = (audio_sample_rates[audio_sample_rate_index] >> 16);
	int bestDiff = 0xFFFFFF;
	uint16 bestCtrl = 1;
	uint16 bestData = 1;

	for (int i=7; i!=0; i--)
	{
		uint32 val0 = baseclk / dividers[i];
		for (int j=1; j<256; j++)
		{
			int val = val0 / j;
			int diff = (hz > val) ? (hz - val) : (val - hz);
			if (diff < bestDiff)
			{
				bestDiff = diff;
				bestCtrl = (i+1);
				bestData = j;
			}
			if (val < hz)
				break;
		}
	}

	ctrl = bestCtrl;
	*TIMERA_REG_ENABLE 	&= ~TIMERA_MASK_ENABLE;
	//*TIMERA_REG_MASK 	&= ~TIMERA_MASK_ENABLE;
	*TIMERA_REG_MASK 	|= TIMERA_MASK_ENABLE;
	*TIMERA_REG_PENDING	&= ~TIMERA_MASK_ENABLE;
	*TIMERA_REG_SERVICE	&= ~TIMERA_MASK_ENABLE;
	*TIMERA_REG_DATA	= bestData;
	*TIMERA_REG_CTRL	= ((*TIMERA_REG_CTRL) & ~TIMERA_MASK_CTRL) | (bestCtrl << TIMERA_SHIFT_CTRL);

	*TIMERA_REG_SERVICE = 0;
	switch(drv)
	{
		case SND_DRV_YM:
		{
			YM2149_WR(0,0);
			YM2149_WR(1,0);
			YM2149_WR(2,0);
			YM2149_WR(3,0);
			YM2149_WR(4,0);
			YM2149_WR(5,0);
			YM2149_WR(6,0);
			YM2149_WR_MASKED(7, 0xFF, 0x3F);
			YM2149_WR(8,0);
			YM2149_WR(9,0);
			YM2149_WR(10,0);
			YM2149_WR(11,0);
			YM2149_WR(12,0);
			YM2149_WR(13,0);
			if (HostCPUType >= 4)
			{
				SetMacVector(TIMERA_VECTOR, timerA_YM_040);
				SetTosVector(TIMERA_VECTOR, timerA_YM_040);
			}
			else if (isFalcon)
			{
				SetMacVector(TIMERA_VECTOR, timerA_YM_Falcon);
				SetTosVector(TIMERA_VECTOR, timerA_YM_Falcon);
			}
			else
			{
				SetMacVector(TIMERA_VECTOR, timerA_YM);
				SetTosVector(TIMERA_VECTOR, timerA_YM);
			}
		}
		break;
		
		case SND_DRV_COVOX:
		{
			YM2149_WR_MASKED(7, 0x80, 0x80);
			SetMacVector(TIMERA_VECTOR, timerA_COVOX);
			SetTosVector(TIMERA_VECTOR, timerA_COVOX);
		}
		break;

		case SND_DRV_MV16:
		{
			SetMacVector(TIMERA_VECTOR, timerA_MV16);
			SetTosVector(TIMERA_VECTOR, timerA_MV16);
		}
		break;
		
		case SND_DRV_REPLAY8:
		{
			SetMacVector(TIMERA_VECTOR, timerA_Replay8);
			SetTosVector(TIMERA_VECTOR, timerA_Replay8);
		}
		break;
		
		case SND_DRV_REPLAY8S:
		{
			SetMacVector(TIMERA_VECTOR, timerA_Replay8S);
			SetTosVector(TIMERA_VECTOR, timerA_Replay8S);
		}
		break;
		
		case SND_DRV_REPLAY16:
		{
			SetMacVector(TIMERA_VECTOR, timerA_Replay16);
			SetTosVector(TIMERA_VECTOR, timerA_Replay16);
		}
		break;
	}
	if (TosIrqSafe)
		SetTosVector(TIMERA_VECTOR, timerA_NULL);
	

	irqInstalled = true;
}

void sndDriverST::RemoveIrq()
{
	if (!irqInstalled)
		return;

	*TIMERA_REG_ENABLE  = (~TIMERA_MASK_ENABLE) & (*TIMERA_REG_ENABLE);
	*TIMERA_REG_CTRL    = (~TIMERA_MASK_CTRL) & (*TIMERA_REG_CTRL);
	*TIMERA_REG_PENDING &= ~TIMERA_MASK_ENABLE;
	*TIMERA_REG_SERVICE &= ~TIMERA_MASK_ENABLE;
	//*TIMERA_REG_MASK    &= ~TIMERA_MASK_ENABLE;
	irqInstalled = false;
}

void sndDriverST::Configure()
{
	if (!irqInstalled)
		return;

	D(bug("- Configure -\n"));
	uint16 sr = DisableInterrupts();
	RemoveIrq();
	InstallIrq();
	sndDriverST::pos = 0;
	SetSR(sr);
	if (streamStarted)
		*TIMERA_REG_ENABLE |= TIMERA_MASK_ENABLE;
}

void sndDriverST::StartStream()
{
	if (!irqInstalled)
		return;

	if (!streamStarted)
	{
		D(bug("- StartStream -\n"));
		streamStarted = true;
		*TIMERA_REG_ENABLE |= TIMERA_MASK_ENABLE;
	}
}

void sndDriverST::StopStream()
{
	if (!irqInstalled)
		return;

	if (streamStarted)
	{
		D(bug("- StopStream -\n"));
		streamStarted = false;
		*TIMERA_REG_ENABLE &= ~TIMERA_MASK_ENABLE;
	}
}

uint32 sndDriverST::GetPos()
{
	return pos;
}

void sndDriverST::Fill(uint8* dst, uint8* src, uint32 len)
{
	uint32 *s = (uint32*)src;
	uint32 *d = (uint32*)dst;
	uint16 l = (uint16) (len >> 2);
	while (l--)
		*d++ = *s++;
}

const uint16 _soundTable_U8[] =
{
	0x80E,0x90D,0xA0C,0,0x80F,0x903,0xA00,0,	0x80F,0x903,0xA00,0,0x80F,0x903,0xA00,0,
	0x80F,0x903,0xA00,0,0x80F,0x903,0xA00,0,	0x80F,0x903,0xA00,0,0x80E,0x90D,0xA0B,0,
	0x80E,0x90D,0xA0B,0,0x80E,0x90D,0xA0B,0,	0x80E,0x90D,0xA0B,0,0x80E,0x90D,0xA0B,0,
	0x80E,0x90D,0xA0B,0,0x80E,0x90D,0xA0B,0,	0x80E,0x90D,0xA0A,0,0x80E,0x90D,0xA0A,0,
	0x80E,0x90D,0xA0A,0,0x80E,0x90D,0xA0A,0,	0x80E,0x90C,0xA0C,0,0x80E,0x90D,0xA00,0,
	0x80D,0x90D,0xA0D,0,0x80D,0x90D,0xA0D,0,	0x80D,0x90D,0xA0D,0,0x80D,0x90D,0xA0D,0,
	0x80D,0x90D,0xA0D,0,0x80D,0x90D,0xA0D,0,	0x80E,0x90C,0xA0B,0,0x80E,0x90C,0xA0B,0,
	0x80E,0x90C,0xA0B,0,0x80E,0x90C,0xA0B,0,	0x80E,0x90C,0xA0B,0,0x80E,0x90C,0xA0B,0,
	0x80E,0x90C,0xA0B,0,0x80E,0x90C,0xA0B,0,	0x80E,0x90C,0xA0A,0,0x80E,0x90C,0xA0A,0,
	0x80E,0x90C,0xA0A,0,0x80E,0x90C,0xA0A,0,	0x80D,0x90D,0xA0C,0,0x80D,0x90D,0xA0C,0,
	0x80E,0x90C,0xA09,0,0x80E,0x90C,0xA09,0,	0x80E,0x90C,0xA05,0,0x80E,0x90C,0xA00,0,
	0x80E,0x90C,0xA00,0,0x80E,0x90B,0xA0B,0,	0x80E,0x90B,0xA0B,0,0x80E,0x90B,0xA0B,0,
	0x80E,0x90B,0xA0B,0,0x80E,0x90B,0xA0A,0,	0x80E,0x90B,0xA0A,0,0x80E,0x90B,0xA0A,0,
	0x80D,0x90D,0xA0B,0,0x80D,0x90D,0xA0B,0,	0x80D,0x90D,0xA0B,0,0x80E,0x90B,0xA09,0,
	0x80E,0x90B,0xA09,0,0x80E,0x90B,0xA09,0,	0x80D,0x90C,0xA0C,0,0x80D,0x90D,0xA0A,0,
	0x80E,0x90B,0xA07,0,0x80E,0x90B,0xA00,0,	0x80E,0x90B,0xA00,0,0x80D,0x90D,0xA09,0,
	0x80D,0x90D,0xA09,0,0x80E,0x90A,0xA09,0,	0x80D,0x90D,0xA08,0,0x80D,0x90D,0xA07,0,
	0x80D,0x90D,0xA04,0,0x80D,0x90D,0xA00,0,	0x80E,0x90A,0xA04,0,0x80E,0x909,0xA09,0,
	0x80E,0x909,0xA09,0,0x80D,0x90C,0xA0B,0,	0x80E,0x909,0xA08,0,0x80E,0x909,0xA08,0,
	0x80E,0x909,0xA07,0,0x80E,0x908,0xA08,0,	0x80E,0x909,0xA01,0,0x80C,0x90C,0xA0C,0,
	0x80D,0x90C,0xA0A,0,0x80E,0x908,0xA06,0,	0x80E,0x907,0xA07,0,0x80E,0x908,0xA00,0,
	0x80E,0x907,0xA05,0,0x80E,0x906,0xA06,0,	0x80D,0x90C,0xA09,0,0x80E,0x905,0xA05,0,
	0x80E,0x904,0xA04,0,0x80D,0x90C,0xA08,0,	0x80D,0x90B,0xA0B,0,0x80E,0x900,0xA00,0,
	0x80D,0x90C,0xA06,0,0x80D,0x90C,0xA05,0,	0x80D,0x90C,0xA02,0,0x80C,0x90C,0xA0B,0,
	0x80C,0x90C,0xA0B,0,0x80D,0x90B,0xA0A,0,	0x80D,0x90B,0xA0A,0,0x80D,0x90B,0xA0A,0,
	0x80D,0x90B,0xA0A,0,0x80C,0x90C,0xA0A,0,	0x80C,0x90C,0xA0A,0,0x80C,0x90C,0xA0A,0,
	0x80D,0x90B,0xA09,0,0x80D,0x90B,0xA09,0,	0x80D,0x90A,0xA0A,0,0x80D,0x90A,0xA0A,0,
	0x80D,0x90A,0xA0A,0,0x80C,0x90C,0xA09,0,	0x80C,0x90C,0xA09,0,0x80C,0x90C,0xA09,0,
	0x80D,0x90B,0xA06,0,0x80C,0x90B,0xA0B,0,	0x80C,0x90C,0xA08,0,0x80D,0x90B,0xA00,0,
	0x80D,0x90B,0xA00,0,0x80C,0x90C,0xA07,0,	0x80C,0x90C,0xA06,0,0x80C,0x90C,0xA05,0,
	0x80C,0x90C,0xA03,0,0x80C,0x90C,0xA01,0,	0x80C,0x90B,0xA0A,0,0x80D,0x90A,0xA05,0,
	0x80D,0x90A,0xA04,0,0x80D,0x90A,0xA02,0,	0x80D,0x909,0xA08,0,0x80D,0x909,0xA08,0,
	0x80C,0x90B,0xA09,0,0x80C,0x90B,0xA09,0,	0x80D,0x908,0xA08,0,0x80B,0x90B,0xA0B,0,
	0x80D,0x909,0xA05,0,0x80C,0x90B,0xA08,0,	0x80D,0x909,0xA02,0,0x80D,0x908,0xA06,0,
	0x80C,0x90B,0xA07,0,0x80D,0x907,0xA07,0,	0x80C,0x90B,0xA06,0,0x80C,0x90A,0xA09,0,
	0x80B,0x90B,0xA0A,0,0x80C,0x90B,0xA02,0,	0x80C,0x90B,0xA00,0,0x80C,0x90A,0xA08,0,
	0x80D,0x906,0xA04,0,0x80D,0x905,0xA05,0,	0x80D,0x905,0xA04,0,0x80C,0x909,0xA09,0,
	0x80D,0x904,0xA03,0,0x80B,0x90B,0xA09,0,	0x80C,0x90A,0xA05,0,0x80B,0x90A,0xA0A,0,
	0x80C,0x909,0xA08,0,0x80B,0x90B,0xA08,0,	0x80C,0x90A,0xA00,0,0x80C,0x90A,0xA00,0,
	0x80C,0x909,0xA07,0,0x80B,0x90B,0xA07,0,	0x80C,0x909,0xA06,0,0x80B,0x90B,0xA06,0,
	0x80B,0x90A,0xA09,0,0x80B,0x90B,0xA05,0,	0x80A,0x90A,0xA0A,0,0x80B,0x90B,0xA02,0,
	0x80B,0x90A,0xA08,0,0x80C,0x907,0xA07,0,	0x80C,0x908,0xA04,0,0x80C,0x907,0xA06,0,
	0x80B,0x909,0xA09,0,0x80C,0x906,0xA06,0,	0x80A,0x90A,0xA09,0,0x80C,0x907,0xA03,0,
	0x80B,0x90A,0xA05,0,0x80B,0x909,0xA08,0,	0x80B,0x90A,0xA03,0,0x80A,0x90A,0xA08,0,
	0x80B,0x90A,0xA00,0,0x80B,0x909,0xA07,0,	0x80B,0x908,0xA08,0,0x80A,0x90A,0xA07,0,
	0x80A,0x909,0xA09,0,0x80C,0x901,0xA01,0,	0x80A,0x90A,0xA06,0,0x80B,0x908,0xA07,0,
	0x80A,0x90A,0xA05,0,0x80A,0x909,0xA08,0,	0x80A,0x90A,0xA02,0,0x80A,0x90A,0xA01,0,
	0x80A,0x90A,0xA00,0,0x809,0x909,0xA09,0,	0x80A,0x908,0xA08,0,0x80B,0x908,0xA01,0,
	0x80A,0x909,0xA06,0,0x80B,0x907,0xA04,0,	0x80A,0x909,0xA05,0,0x809,0x909,0xA08,0,
	0x80A,0x909,0xA03,0,0x80A,0x908,0xA06,0,	0x80A,0x909,0xA00,0,0x809,0x909,0xA07,0,
	0x809,0x908,0xA08,0,0x80A,0x908,0xA04,0,	0x809,0x909,0xA06,0,0x80A,0x908,0xA01,0,
	0x809,0x909,0xA05,0,0x809,0x908,0xA07,0,	0x808,0x908,0xA08,0,0x809,0x909,0xA02,0,
	0x809,0x908,0xA06,0,0x809,0x909,0xA00,0,	0x809,0x907,0xA07,0,0x808,0x908,0xA07,0,
	0x809,0x907,0xA06,0,0x809,0x908,0xA02,0,	0x808,0x908,0xA06,0,0x809,0x906,0xA06,0,
	0x808,0x907,0xA07,0,0x808,0x908,0xA04,0,	0x808,0x907,0xA06,0,0x808,0x908,0xA02,0,
	0x807,0x907,0xA07,0,0x808,0x906,0xA06,0,	0x808,0x907,0xA04,0,0x807,0x907,0xA06,0,
	0x808,0x906,0xA05,0,0x808,0x906,0xA04,0,	0x807,0x906,0xA06,0,0x807,0x907,0xA04,0,
	0x808,0x905,0xA04,0,0x806,0x906,0xA06,0,	0x807,0x906,0xA04,0,0x807,0x905,0xA05,0,
	0x806,0x906,0xA05,0,0x806,0x906,0xA04,0,	0x806,0x905,0xA05,0,0x806,0x906,0xA02,0,
	0x806,0x905,0xA04,0,0x805,0x905,0xA05,0,	0x806,0x905,0xA02,0,0x805,0x905,0xA04,0,
	0x805,0x904,0xA04,0,0x805,0x905,0xA02,0,	0x804,0x904,0xA04,0,0x804,0x904,0xA03,0,
	0x804,0x904,0xA02,0,0x804,0x903,0xA03,0,	0x803,0x903,0xA03,0,0x803,0x903,0xA02,0,
	0x803,0x902,0xA02,0,0x802,0x902,0xA02,0,	0x802,0x902,0xA01,0,0x801,0x901,0xA01,0,
	0x802,0x901,0xA00,0,0x801,0x901,0xA00,0,	0x801,0x900,0xA00,0,0x800,0x900,0xA00,0,	
};
