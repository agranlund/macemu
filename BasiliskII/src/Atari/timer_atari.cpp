/*
 *  timer_atari.cpp - Time Manager emulation, Atari specific stuff
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
#include "timer.h"
#include "macos_util.h"
#include "zeropage.h"
#include "sys/time.h"
#include "time.h"

#define DEBUG 0
#include "debug.h"

uint64 microsSinceBoot = 0;
tm_time_t globalTime =  {0, 0};
tm_time_t globalTimerTime =  {0, 0};
time_t secondsSinceDawnOfTime = 0;

void timer_atari_set()
{
	unsigned short tos_time;
	unsigned short tos_date;
	struct tm now;

	uint16 zp = SetZeroPage(ZEROPAGE_TOS);
	tos_time = Tgettime();
	tos_date = Tgetdate();
	SetZeroPage(zp);

	now.tm_sec = (tos_time & 0x1f) * 2;
	now.tm_min = (tos_time >> 5) & 0x3f;
	now.tm_hour = tos_time >> 11;
	now.tm_mday = tos_date & 0x1f;
	now.tm_mon = ((tos_date >> 5) & 0xf) - 1;
	now.tm_year = (tos_date >> 9) + 80;
	now.tm_isdst = -1;

	globalTimerTime.tv_sec = mktime(&now);
	globalTimerTime.tv_usec = 0;
	secondsSinceDawnOfTime = globalTimerTime.tv_sec;
	microsSinceBoot = 0;
#if SEPARATE_CLOCK_AND_TIMER
	globalTime.tv_sec = globalTimerTime.tv_sec;
	globalTime.tv_usec = globalTimerTime.tv_usec;
#endif
}


/*
 *  Suspend emulator thread, virtual CPU in idle mode
 */

void idle_wait(void)
{
	// XXX if you implement this make sure to call idle_resume() from TriggerInterrupt()
}


/*
 *  Resume execution of emulator thread, events just arrived
 */

void idle_resume(void)
{
}
