/*
 *  CTYPE.C	Character classification and conversion
 */

/* Modified by Guido Flohr <guido@freemint.de>:
 * - Characters > 128 are control characters.
 * - iscntrl(EOF) should return false, argh, stupid but that's the
 *   the opinion of the majority.
 */

#define __NO_CTYPE

#include <ctype.h>
#include <limits.h>
#include "ctypeint.h"

#define _CTb _ISblank
#define _CTg _ISgraph

unsigned char const __libc_ctype2[UCHAR_MAX + 1] =
{
	/* 00 */ 0,
	/* 01 */ 0,
	/* 02 */ 0,
	/* 03 */ 0,
	/* 04 */ 0,
	/* 05 */ 0,
	/* 06 */ 0,
	/* 07 */ 0,
	/* 08 */ 0,
	/* 09 */ _CTb,
	/* 0A */ 0,
	/* 0B */ 0,
	/* 0C */ 0,
	/* 0D */ 0,
	/* 0E */ 0,
	/* 0F */ 0,

	/* 10 */ 0,
	/* 11 */ 0,
	/* 12 */ 0,
	/* 13 */ 0,
	/* 14 */ 0,
	/* 15 */ 0,
	/* 16 */ 0,
	/* 17 */ 0,
	/* 18 */ 0,
	/* 19 */ 0,
	/* 1A */ 0,
	/* 1B */ 0,
	/* 1C */ 0,
	/* 1D */ 0,
	/* 1E */ 0,
	/* 1F */ 0,

	/* 20 */ _CTb,
	/* 21 */ _CTg,
	/* 22 */ _CTg,
	/* 23 */ _CTg,
	/* 24 */ _CTg,
	/* 25 */ _CTg,
	/* 26 */ _CTg,
	/* 27 */ _CTg,
	/* 28 */ _CTg,
	/* 29 */ _CTg,
	/* 2A */ _CTg,
	/* 2B */ _CTg,
	/* 2C */ _CTg,
	/* 2D */ _CTg,
	/* 2E */ _CTg,
	/* 2F */ _CTg,

	/* 30 */ _CTg,
	/* 31 */ _CTg,
	/* 32 */ _CTg,
	/* 33 */ _CTg,
	/* 34 */ _CTg,
	/* 35 */ _CTg,
	/* 36 */ _CTg,
	/* 37 */ _CTg,
	/* 38 */ _CTg,
	/* 39 */ _CTg,
	/* 3A */ _CTg,
	/* 3B */ _CTg,
	/* 3C */ _CTg,
	/* 3D */ _CTg,
	/* 3E */ _CTg,
	/* 3F */ _CTg,

	/* 40 */ _CTg,
	/* 41 */ _CTg,
	/* 42 */ _CTg,
	/* 43 */ _CTg,
	/* 44 */ _CTg,
	/* 45 */ _CTg,
	/* 46 */ _CTg,
	/* 47 */ _CTg,
	/* 48 */ _CTg,
	/* 49 */ _CTg,
	/* 4A */ _CTg,
	/* 4B */ _CTg,
	/* 4C */ _CTg,
	/* 4D */ _CTg,
	/* 4E */ _CTg,
	/* 4F */ _CTg,

	/* 50 */ _CTg,
	/* 51 */ _CTg,
	/* 52 */ _CTg,
	/* 53 */ _CTg,
	/* 54 */ _CTg,
	/* 55 */ _CTg,
	/* 56 */ _CTg,
	/* 57 */ _CTg,
	/* 58 */ _CTg,
	/* 59 */ _CTg,
	/* 5A */ _CTg,
	/* 5B */ _CTg,
	/* 5C */ _CTg,
	/* 5D */ _CTg,
	/* 5E */ _CTg,
	/* 5F */ _CTg,

	/* 60 */ _CTg,
	/* 61 */ _CTg,
	/* 62 */ _CTg,
	/* 63 */ _CTg,
	/* 64 */ _CTg,
	/* 65 */ _CTg,
	/* 66 */ _CTg,
	/* 67 */ _CTg,
	/* 68 */ _CTg,
	/* 69 */ _CTg,
	/* 6A */ _CTg,
	/* 6B */ _CTg,
	/* 6C */ _CTg,
	/* 6D */ _CTg,
	/* 6E */ _CTg,
	/* 6F */ _CTg,

	/* 70 */ _CTg,
	/* 71 */ _CTg,
	/* 72 */ _CTg,
	/* 73 */ _CTg,
	/* 74 */ _CTg,
	/* 75 */ _CTg,
	/* 76 */ _CTg,
	/* 77 */ _CTg,
	/* 78 */ _CTg,
	/* 79 */ _CTg,
	/* 7A */ _CTg,
	/* 7B */ _CTg,
	/* 7C */ _CTg,
	/* 7D */ _CTg,
	/* 7E */ _CTg,
	/* 7F */ 0,

	/* 80 */ 0,
	/* 81 */ 0,
	/* 82 */ 0,
	/* 83 */ 0,
	/* 84 */ 0,
	/* 85 */ 0,
	/* 86 */ 0,
	/* 87 */ 0,
	/* 88 */ 0,
	/* 89 */ 0,
	/* 8A */ 0,
	/* 8B */ 0,
	/* 8C */ 0,
	/* 8D */ 0,
	/* 8E */ 0,
	/* 8F */ 0,
	/* 90 */ 0,
	/* 91 */ 0,
	/* 92 */ 0,
	/* 93 */ 0,
	/* 94 */ 0,
	/* 95 */ 0,
	/* 96 */ 0,
	/* 97 */ 0,
	/* 98 */ 0,
	/* 99 */ 0,
	/* 9A */ 0,
	/* 9B */ 0,
	/* 9C */ 0,
	/* 9D */ 0,
	/* 9E */ 0,
	/* 9F */ 0,
	/* A0 */ 0,
	/* A1 */ 0,
	/* A2 */ 0,
	/* A3 */ 0,
	/* A4 */ 0,
	/* A5 */ 0,
	/* A6 */ 0,
	/* A7 */ 0,
	/* A8 */ 0,
	/* A9 */ 0,
	/* AA */ 0,
	/* AB */ 0,
	/* AC */ 0,
	/* AD */ 0,
	/* AE */ 0,
	/* AF */ 0,
	/* B0 */ 0,
	/* B1 */ 0,
	/* B2 */ 0,
	/* B3 */ 0,
	/* B4 */ 0,
	/* B5 */ 0,
	/* B6 */ 0,
	/* B7 */ 0,
	/* B8 */ 0,
	/* B9 */ 0,
	/* BA */ 0,
	/* BB */ 0,
	/* BC */ 0,
	/* BD */ 0,
	/* BE */ 0,
	/* BF */ 0,
	/* C0 */ 0,
	/* C1 */ 0,
	/* C2 */ 0,
	/* C3 */ 0,
	/* C4 */ 0,
	/* C5 */ 0,
	/* C6 */ 0,
	/* C7 */ 0,
	/* C8 */ 0,
	/* C9 */ 0,
	/* CA */ 0,
	/* CB */ 0,
	/* CC */ 0,
	/* CD */ 0,
	/* CE */ 0,
	/* CF */ 0,
	/* D0 */ 0,
	/* D1 */ 0,
	/* D2 */ 0,
	/* D3 */ 0,
	/* D4 */ 0,
	/* D5 */ 0,
	/* D6 */ 0,
	/* D7 */ 0,
	/* D8 */ 0,
	/* D9 */ 0,
	/* DA */ 0,
	/* DB */ 0,
	/* DC */ 0,
	/* DD */ 0,
	/* DE */ 0,
	/* DF */ 0,
	/* E0 */ 0,
	/* E1 */ 0,
	/* E2 */ 0,
	/* E3 */ 0,
	/* E4 */ 0,
	/* E5 */ 0,
	/* E6 */ 0,
	/* E7 */ 0,
	/* E8 */ 0,
	/* E9 */ 0,
	/* EA */ 0,
	/* EB */ 0,
	/* EC */ 0,
	/* ED */ 0,
	/* EE */ 0,
	/* EF */ 0,
	/* F0 */ 0,
	/* F1 */ 0,
	/* F2 */ 0,
	/* F3 */ 0,
	/* F4 */ 0,
	/* F5 */ 0,
	/* F6 */ 0,
	/* F7 */ 0,
	/* F8 */ 0,
	/* F9 */ 0,
	/* FA */ 0,
	/* FB */ 0,
	/* FC */ 0,
	/* FD */ 0,
	/* FE */ 0,
	/* FF */ 0
};
