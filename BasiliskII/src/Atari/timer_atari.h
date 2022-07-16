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

#ifndef ATARI_TIMER_H
#define ATARI_TIMER_H

#include "sysdeps.h"

#define SEPARATE_CLOCK_AND_TIMER	0


#if SEPARATE_CLOCK_AND_TIMER
extern tm_time_t globalTime;
#endif
extern tm_time_t globalTimerTime;
extern time_t secondsSinceDawnOfTime;
extern uint64 microsSinceBoot;

#if SEPARATE_CLOCK_AND_TIMER
inline void time_atari_tick(suseconds_t micros)
{
	globalTime.tv_usec += micros;
	if (globalTime.tv_usec >= 1000000) {
		globalTime.tv_sec++;
		globalTime.tv_usec -= 1000000;
	}
	microsSinceBoot += micros;
	secondsSinceDawnOfTime = globalTime.tv_sec;
}
#endif
inline void timer_atari_tick(suseconds_t micros)
{
	globalTimerTime.tv_usec += micros;
	if (globalTimerTime.tv_usec >= 1000000) {
		globalTimerTime.tv_sec++;
		globalTimerTime.tv_usec -= 1000000;
	}
#if !SEPARATE_CLOCK_AND_TIMER
	microsSinceBoot += micros;
	secondsSinceDawnOfTime = globalTimerTime.tv_sec;
#endif
}

/*
 *  Return microseconds since boot (64 bit)
 */

inline void Microseconds(uint32 &hi, uint32 &lo)
{
	lo = (microsSinceBoot & 0xFFFFFFFF);
	hi = (microsSinceBoot >> 32);
}

/*
 *  Get current time
 */

inline void timer_current_time(tm_time_t &t)
{
	t.tv_sec = globalTimerTime.tv_sec;
	t.tv_usec = globalTimerTime.tv_usec;
}
 
/*
 *  Add times
 */

inline void timer_add_time(tm_time_t &res, tm_time_t a, tm_time_t b)
{
	res.tv_sec = a.tv_sec + b.tv_sec;
	res.tv_usec = a.tv_usec + b.tv_usec;
	if (res.tv_usec >= 1000000)
	{
		res.tv_sec++;
		res.tv_usec -= 1000000;
	}
}

/*
 *  Subtract times
 */

inline void timer_sub_time(tm_time_t &res, tm_time_t a, tm_time_t b)
{
	res.tv_sec = a.tv_sec - b.tv_sec;
	res.tv_usec = a.tv_usec - b.tv_usec;
	if (res.tv_usec < 0)
	{
		res.tv_sec--;
		res.tv_usec += 1000000;
	}
}

/*
 *  Compare times (<0: a < b, =0: a = b, >0: a > b)
 */

inline int timer_cmp_time(tm_time_t a, tm_time_t b)
{
	if (a.tv_sec == b.tv_sec)
		return a.tv_usec - b.tv_usec;
	else
		return a.tv_sec - b.tv_sec;
}

/*
 *  Return local date/time in Mac format (seconds since 1.1.1904)
 */

extern uint32 TimeToMacTime(time_t t);

inline uint32 TimerDateTime(void)
{
	return TimeToMacTime(secondsSinceDawnOfTime);
}


/*
 *  Convert Mac time value (>0: microseconds, <0: microseconds) to tm_time_t
 */

inline void timer_mac2host_time(tm_time_t &res, int32 mactime)
{
	if (mactime > 0) {
		res.tv_sec = mactime / 1000;			// Time in milliseconds
		res.tv_usec = (mactime % 1000) * 1000;
	} else {
		res.tv_sec = -mactime / 1000000;		// Time in negative microseconds
		res.tv_usec = -mactime % 1000000;
	}
}


/*
 *  Convert positive tm_time_t to Mac time value (>0: microseconds, <0: microseconds)
 *  A negative input value for hosttime results in a zero return value
 *  As long as the microseconds value fits in 32 bit, it must not be converted to milliseconds!
 */

inline int32 timer_host2mac_time(tm_time_t hosttime)
{
	if (hosttime.tv_sec < 0)
		return 0;
	else {
		uint64 t = (uint64)hosttime.tv_sec * 1000000 + hosttime.tv_usec;
		if (t > 0x7fffffff)
			return t / 1000;	// Time in milliseconds
		else
			return -t;			// Time in negative microseconds
	}
}


#endif // ATARI_INPUT_H
