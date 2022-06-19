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

#ifndef ATARI_INPUT_H
#define ATARI_INPUT_H

#include "sysdeps.h"
#ifndef __GNUC_INLINE__
#define __GNUC_INLINE__
#include "mint/linea.h"
#undef __GNUC_INLINE__
#else
#include "mint/linea.h"
#endif

void InitInput(uint16 resx, uint16 resy, uint16 sensx = 1<<8, uint16 sensy = 1<<8);
void RestoreInput();
void UpdateInput();
void RequestInput();

bool GetKeyStatus(uint8 key);
void GetMouse(int16& x, int16& y);


#endif // ATARI_INPUT_H
