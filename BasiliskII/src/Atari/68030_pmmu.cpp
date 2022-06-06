/*
 *  68030_pmmu.cpp
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
#include "zeropage.h"
#include "asm_support.h"
#include "debug_atari.h"

#define DEBUG 1
#include "debug.h"

static const uint32 MMU_TABLE  = 0x02;
static const uint32 MMU_PAGE   = 0x01;
static const uint32 MMU_CI     = 0x40;

static const uint32 tia_bits = 4;
static const uint32 tib_bits = 4;
static const uint32 tic_bits = 4;
#if MMU030_PAGESIZE == 1024
static const uint32 tid_bits = 10;
#elif MMU030_PAGESIZE == 256
static const uint32 tid_bits = 12;
#endif

static const uint32 pagesize = ((1 << (32 - (tia_bits + tib_bits + tic_bits))) / (1 << tid_bits));

static uint32* tia = 0;
static uint32* tib = 0;
static uint32* tic = 0;
static uint32* tid = 0;

#define TIA(entry, offset)   tia[(entry*(1<<tia_bits))+offset]
#define TIB(entry, offset)   tib[(entry*(1<<tib_bits))+offset]
#define TIC(entry, offset)   tic[(entry*(1<<tic_bits))+offset]
#define TID(entry, offset)   tid[(entry*(1<<tid_bits))+offset]

void pmmu_68030_init()
{
    D(bug(" pmmu_68030_init: Start\n"));
    const uint32 is_bits = 0;

    // prealloc all tia,tib,tic & one tid
    const uint32 num_tia_tables = 1;
    const uint32 num_tib_tables = (1 << tib_bits) * num_tia_tables;
    const uint32 num_tic_tables = (1 << tic_bits) * num_tib_tables;
    const uint32 num_tid_tables = 1;

    const uint32 tia_size = ((1 << tia_bits) * num_tia_tables);
    const uint32 tib_size = ((1 << tib_bits) * num_tib_tables);
    const uint32 tic_size = ((1 << tic_bits) * num_tic_tables);
    const uint32 tid_size = ((1 << tid_bits) * num_tid_tables);

    const uint32 table_size = 4 * (tia_size + tib_size + tic_size + tid_size);

    const uint32 zeroSize = (8 * 1024);
    const uint32 pageSize = MMU030_PAGESIZE;
    const uint32 memSize = 4096 + zeroSize + table_size;
    char* mem = (char*) Mxalloc(memSize, 3);
    memset(mem, 0, memSize);
    char* zeroPhys = (char*) ((((uint32)mem) + 4095) & ~4095L);

    //  0 : TIA : X.......
    tia = (uint32*) (zeroPhys + zeroSize);
    tib = tia + tia_size;
    tic = tib + tib_size;
    tid = tic + tic_size;

    D(bug("   mem  = 0x%08x (%d)\n", mem, memSize));
    D(bug("   zero = 0x%08x (%d)\n", zeroPhys, zeroSize));
    D(bug("   tia = 0x%08x (%d, %d)\n", tia, num_tia_tables, 4 * tia_size));
    D(bug("   tib = 0x%08x (%d, %d)\n", tib, num_tib_tables, 4 * tib_size));
    D(bug("   tic = 0x%08x (%d, %d)\n", tic, num_tic_tables, 4 * tic_size));
    D(bug("   tid = 0x%08x (%d, %d)\n", tid, num_tid_tables, 4 * tid_size));

    // default table
    for (uint16 i=0; i<(1<<tia_bits); i++)
    {
        TIA(0, i) = uint32(&TIB(i, 0)) | MMU_TABLE;
        for (uint16 j=0; j<(1<<tib_bits); j++)
        {
            uint16 ticidx = (i * (1 << tia_bits)) + j;
            TIB(i,j) = uint32(&TIC(ticidx, 0)) | MMU_TABLE;
            for (uint16 k=0; k<(1<<tic_bits); k++)
            {
                TIC(ticidx, k) = (i << (32-(tia_bits))) | (j << (32-(tia_bits+tib_bits))) | (k << (32-(tia_bits+tib_bits+tic_bits))) | MMU_PAGE;
            }
        }
    }

    TIA(0, 0) = (uint32)(&TIB(0,0)) | MMU_TABLE;
    TIA(0, 8) = 0x80000000UL  | MMU_PAGE | MMU_CI;
    TIA(0, 9) = 0x90000000UL  | MMU_PAGE | MMU_CI;
    TIA(0,10) = 0xA0000000UL  | MMU_PAGE | MMU_CI;
    TIA(0,11) = 0xB0000000UL  | MMU_PAGE | MMU_CI;
    TIA(0,12) = 0xC0000000UL  | MMU_PAGE | MMU_CI;
    TIA(0,13) = 0xD0000000UL  | MMU_PAGE | MMU_CI;
    TIA(0,14) = 0xE0000000UL  | MMU_PAGE | MMU_CI;
    TIA(0,15) = (uint32)(&TIB(15,0)) | MMU_TABLE;

    TIB(0, 0)  = (uint32)(&TIC(0,0)) | MMU_TABLE;   // 00...... -> 00x.....
    TIB(15,15) = (uint32)(&TIC(0,0)) | MMU_TABLE;   // FF...... -> 00x.....
    TIC(0, 0)  = (uint32)(&TID(0,0)) | MMU_TABLE;   // 000..... -> 000x....
    TIC(0, 1)  = TIC(0,1) | MMU_CI;     // 0x001xxxxx
    TIC(0, 2)  = TIC(0,2) | MMU_CI;     // 0x002xxxxx
    TIC(0, 3)  = TIC(0,3) | MMU_CI;     // 0x003xxxxx
    TIC(0, 4)  = TIC(0,4) | MMU_CI;     // 0x004xxxxx
    TIC(0, 5)  = TIC(0,5) | MMU_CI;     // 0x005xxxxx
    TIC(0, 6)  = TIC(0,6) | MMU_CI;     // 0x006xxxxx
    TIC(0, 7)  = TIC(0,7) | MMU_CI;     // 0x007xxxxx
    TIC(0, 8)  = TIC(0,8) | MMU_CI;     // 0x008xxxxx
    TIC(0, 9)  = TIC(0,9) | MMU_CI;     // 0x009xxxxx
    TIC(0, 10)  = TIC(0,10) | MMU_CI;   // 0x00Axxxxx
    TIC(0, 11)  = TIC(0,11) | MMU_CI;   // 0x00Bxxxxx
    TIC(0, 12)  = TIC(0,12) | MMU_CI;   // 0x00Cxxxxx
    TIC(0, 13)  = TIC(0,13) | MMU_CI;   // 0x00Dxxxxx
    //TIC(0, 14)  = TIC(0,14) | MMU_CI;   // 0x00Exxxxx
    TIC(0, 15)  = TIC(0,15) | MMU_CI;   // 0x00Fxxxxx

    // tid0
    uint32* tidptr = &TID(0,0);
    uint32 pageAddress = 0;
    for (uint32 i=0; i<(1<<tid_bits); i++)
    {
        if (pageAddress < zeroSize)
            tidptr[i] = ((uint32)zeroPhys + pageAddress) | MMU_PAGE | MMU_CI;
        else
            tidptr[i] = pageAddress | MMU_PAGE | MMU_CI;
        pageAddress += pageSize;
    }

    ZPState[ZEROPAGE_MAC].mmu.m030.crp[0] = 0x80000002;  //
    ZPState[ZEROPAGE_MAC].mmu.m030.crp[1] = tia;         //
    ZPState[ZEROPAGE_MAC].mmu.m030.ttr0 = 0x017e8107;    // 0x01000000-0x7FFFFFFF
    ZPState[ZEROPAGE_MAC].mmu.m030.ttr1 = 0x807e8507;    // 0x80000000-0xFEFFFFFF (CI)
    const uint32 ps_bits = 32 - is_bits - tia_bits - tib_bits - tic_bits - tid_bits;
    ZPState[ZEROPAGE_MAC].mmu.m030.tc =
        (ps_bits  << 20) |
        (is_bits  << 16) |
        (tia_bits << 12) |
        (tib_bits <<  8) |
        (tic_bits <<  4) |
        (tid_bits <<  0) |
        0x80000000;

    D(bug(" pmmu_68030_init: Done\n"));
}

uint32* pmmu_68030_map(uint32 logaddr, uint32 physaddr, uint32 size, uint32 flag)
{
    // align to pagesize
    size = (size + (pagesize - 1)) & ~(pagesize - 1);
    uint32 st = logaddr / pagesize;
    uint32 en = (logaddr + size) / pagesize;
    uint16 numpages = en - st;
        
    logaddr = (uint32*) (logaddr & ~(pagesize - 1));
    physaddr = (uint32*) (physaddr & ~(pagesize - 1));

    D(bug(" pmmu_68030_map: %08x -> %08x (%d pages) (%x flag)\n", logaddr, physaddr, numpages, flag))

    const uint16 tia_shift = 32 - (tia_bits);
    const uint16 tib_shift = 32 - (tia_bits + tib_bits);
    const uint16 tic_shift = 32 - (tia_bits + tib_bits + tic_bits);
    const uint16 tid_shift = 32 - (tia_bits + tib_bits + tic_bits + tid_bits);

    uint32 logend = logaddr + (numpages * pagesize);
    uint16 numtics = (logend >> tic_shift) - (logaddr >> tic_shift);
    if (numtics == 0)
        numtics = 1;

    uint32 tidmemsize = (numtics * (1 << tid_bits)) << 2;
    uint32* tidmembase = (uint32*)((((uint32)Mxalloc(4096 + tidmemsize, 3)) + 4095) & ~4095L);
    uint32* tidmem = tidmembase;
    uint32* returnptr = &tidmem[(logaddr >> tid_shift) & ((1 << tid_bits)-1)];

    D(bug("   alloc %d bytes for tables at %08x\n", tidmemsize, tidmem));

    for (uint16 i = 0; i < numpages; i++)
    {
        uint16 tao = (logaddr >> tia_shift) & ((1 << tia_bits)-1);   // tia table offset
        uint16 tbo = (logaddr >> tib_shift) & ((1 << tib_bits)-1);   // tib table offset
        uint16 tco = (logaddr >> tic_shift) & ((1 << tic_bits)-1);   // tic table offset
        uint16 tdo = (logaddr >> tid_shift) & ((1 << tid_bits)-1);   // tid table offset

        uint16 tai = 0;                                     // only one tia table
        uint16 tbi = (tai * (1<<tia_shift)) + tao;          // tib table index
        uint16 tci = (tbi * (1<<tib_shift)) + tbo;          // tic table index

        D(bug("page %d : idx=%d,%d,%d. offset=%d,%d,%d,%d\n", i,tai,tbi,tbo,tao,tbo,tco,tdo));
/*
        if ((TIA(tai,tao) & MMU_TABLE) == 0)
            TIA(tai,tao) = (uint32)(&TIB(tbi,tbo)) | MMU_TABLE;

        if ((TIB(tbi,tbo) & MMU_TABLE) == 0)
            TIB(tbi,tbo) = (uint32)(&TIC(tci,tco)) | MMU_TABLE;
*/
        uint32 ticval = (uint32)TIC(tci,tco);

        D(bug("ticval = %08x\n", ticval));

        if ((ticval & MMU_TABLE) == 0)
        {
            // generate new table
            D(bug("  generate new table\n"));
            TIC(tci,tco) = uint32(tidmem) | MMU_TABLE;
            uint32 addr = logaddr & ~(0xFFFFFFFF >> (tia_bits + tib_bits + tic_bits));
            for (uint16 i=0; i<(1<<tid_bits); i++, addr += pagesize)
            {
                D(bug("  %d = (%08x) %08x\n", i, tidmem, addr));
                *tidmem++ = addr | MMU_PAGE;
            }
        }
        else
        {
            uint32 oldtidaddr = ticval & 0xFFFFFF00;
            D(bug("oldtidaddr = %08x, tidmembase = %08x, tidmem = %08x\n", oldtidaddr, tidmembase, tidmem));
            if ((oldtidaddr < uint32(tidmembase)) || (oldtidaddr > uint32(tidmembase)+tidmemsize))
            {
                // copy old table
                D(bug("  copy old table\n"));
                TIC(tci,tco) = uint32(tidmem) | MMU_TABLE;
                uint32* oldmem = (uint32*)oldtidaddr;
                for (uint16 i=0; i<(1<<tid_bits); i++)
                    *tidmem++ = *oldmem++;
            }
            else
            {
                // use existing table
                D(bug("  use existing table\n"));
            }
        }
        uint32* tidptr = (uint32*)(((uint32)TIC(tci,tco)) & 0xFFFFFF00);
        tidptr[tdo] = physaddr | MMU_PAGE | flag;
        D(bug("  wrote %08x\n", tidptr[tdo]));
        logaddr += pagesize;
        physaddr += pagesize;
    }

    for (uint16 i=0; i<numpages; i++)
    {
        D(bug("** %d = %08x : %08x\n", i, &tidmembase[i], tidmembase[i]));
    }

    return returnptr;
}
