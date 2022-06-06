/*
 *  input_atari.cpp
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
#include "main.h"
#include "adb.h"
#include "zeropage.h"
#include "input_atari.h"
#include "asm_support.h"
#include "prefs.h"

#define DEBUG 1
#include "debug.h"

#define IKBD_STATE_RESET    0
#define IKBD_STATE_FAULT    1
#define IKBD_STATE_KEYB     2
#define IKBD_STATE_MOUSE    3
#define IKBD_STATE_MOUSEABS 4
#define IKBD_STATE_JOY      5
#define IKBD_STATE_CLOCK    6

#define ACIA_REG_KEYCTRL    ((volatile uint8*)0x00FFFC00L)
#define ACIA_REG_KEYDATA    ((volatile uint8*)0x00FFFC02L)
#define ACIA_REG_MIDICTRL   ((volatile uint8*)0x00FFFC04L)
#define ACIA_REG_MIDIDATA   ((volatile uint8*)0x00FFFC06L)

#define MFP_REG_GPIP        ((volatile uint8*)0x00FFFA01)

#define ACIA_FLAG_IRQ       (1 << 7)    // interrupt
#define ACIA_FLAG_PERR      (1 << 6)    // parity error
#define ACIA_FLAG_OVERRUN   (1 << 5)    // overrun error
#define ACIA_FLAG_FERR      (1 << 4)    // framing error
#define ACIA_FLAG_CTS       (1 << 3)    //
#define ACIA_FLAG_DCD       (1 << 2)    //
#define ACIA_FLAG_TX_RDY    (1 << 1)    // transmit buffer empty
#define ACIA_FLAG_RX_RDY    (1 << 0)    // recieve buffer full

#define VRES_SHIFT  3
#define VRES_TO_PRES(x) ((x)>>VRES_SHIFT)
#define PRES_TO_VRES(x) ((x)<<VRES_SHIFT)



struct acia_packet
{
    uint8   buf[7];
    uint8   len;
    uint8   remain;
    uint8   state;    
};

struct mouse_state
{
    int16   mx;
    int16   my;
    uint8   ms;
};

acia_packet cur_packet;
mouse_state mouse_old;
mouse_state mouse;
uint8       keys[128];
bool        input_inited = false;
uint16      mouse_max_x = 400;
uint16      mouse_max_y = 640;
uint16      mouse_speed = 8;
bool        mouse_requested = false;

void ikbd_write(uint8 data)
{
    while(1)
    {
        uint8 keyb_ctrl = *ACIA_REG_KEYCTRL;
        if (keyb_ctrl & ACIA_FLAG_TX_RDY)
        {
            *ACIA_REG_KEYDATA = data;
            return;
        }
    }
}

volatile uint8 midi_data;
volatile uint8 keyb_data;

extern "C" void VecAciaC()
{
    #define SETSTATE(s,l) { cur_packet.state = s; cur_packet.len = 0; cur_packet.remain = l; }

    if (!input_inited)
        return;

    while (1)
    {
        // throw away midi
        uint8 midi_ctrl = *ACIA_REG_MIDICTRL;
        if (midi_ctrl & ACIA_FLAG_IRQ)
            midi_data = *ACIA_REG_MIDIDATA;

        // get keyb state
        uint8 keyb_ctrl = *ACIA_REG_KEYCTRL;
        if (((keyb_ctrl|midi_ctrl)&ACIA_FLAG_IRQ)==0)
        {
            return;
        }

        if (keyb_ctrl & ACIA_FLAG_OVERRUN)
        {
            //if (cur_packet.state != IKBD_STATE_RESET)
            {
                keyb_data = *ACIA_REG_KEYDATA;
                SETSTATE(IKBD_STATE_FAULT, 1)
                return;
            }
        }

        if (keyb_ctrl & ACIA_FLAG_RX_RDY)
        {
            keyb_data = *ACIA_REG_KEYDATA;

            if (cur_packet.state == IKBD_STATE_FAULT)
                return;

            cur_packet.buf[cur_packet.len++] = keyb_data;
            cur_packet.remain--;

            if (cur_packet.remain == 0)
            {
                switch (cur_packet.state)
                {
                    case IKBD_STATE_RESET:
                    {
                        if (keyb_data != 0xF1) {
                            SETSTATE(IKBD_STATE_RESET, 1);
                        } else {
                            SETSTATE(IKBD_STATE_KEYB, 1);
                        }
                    }
                    break;

                    case IKBD_STATE_KEYB:
                    {
                        switch (keyb_data)
                        {
                            case 0xF1:  // self test done
                                SETSTATE(IKBD_STATE_KEYB, 1);
                                break;
                            case 0xF7:  // mouse abs
                                SETSTATE(IKBD_STATE_MOUSEABS, 5);
                                break;
                            case 0xF8:  // mouse rel
                            case 0xF9:
                            case 0xFA:
                            case 0xFB:
                                SETSTATE(IKBD_STATE_MOUSE, 3);
                                cur_packet.buf[cur_packet.len++] = keyb_data & 3;
                                cur_packet.remain--;
                                break;
                            case 0xFC:  // clock
                                SETSTATE(IKBD_STATE_CLOCK, 6);
                                break;
                            case 0xFE:  // joystick
                            case 0xFF:
                                SETSTATE(IKBD_STATE_JOY, 2);
                                cur_packet.buf[cur_packet.len++] = keyb_data & 1;
                                cur_packet.remain--;
                                break;
                            default:
                            {
                                keys[keyb_data & 0x7F] = (keyb_data & 0x80) ? 0x00 : 0xFF;
                                SETSTATE(IKBD_STATE_KEYB, 1);
                            }
                            break;
                        }
                    }
                    break;

                    case IKBD_STATE_MOUSEABS:
                    {
                        mouse.ms = cur_packet.buf[0];
                        mouse.mx = (cur_packet.buf[1] << 8) | cur_packet.buf[2];
                        mouse.my = (cur_packet.buf[3] << 8) | cur_packet.buf[4];
                        SETSTATE(IKBD_STATE_KEYB, 1);
                    }
                    break;

                    case IKBD_STATE_MOUSE:
                    {
                        mouse.ms  = cur_packet.buf[0];
                        mouse.mx += (int16)((int8)cur_packet.buf[1]);
                        mouse.my += (int16)((int8)cur_packet.buf[2]);
                        SETSTATE(IKBD_STATE_KEYB, 1);
                    }
                    break;

                    case IKBD_STATE_JOY:
                    {
                        // todo: process joystick
                        SETSTATE(IKBD_STATE_KEYB, 1);
                    }
                    break;

                    case IKBD_STATE_CLOCK:
                    {
                        SETSTATE(IKBD_STATE_KEYB, 1);
                    }
                    break;

                    default:
                    {
                        SETSTATE(IKBD_STATE_KEYB, 1);
                    }
                    break;
                }
            }
        }
    }
    return;
}

void InitInput(uint16 resx, uint16 resy, uint16 sensx, uint16 sensy)
{
    D(bug("InitInput %d, %d\n", resx, resy));
    if (!input_inited)
    {
        // mouse prefs
        mouse_speed = PrefsFindInt32("mouse_speed");
        if (mouse_speed < 0)
            mouse_speed = 1;
        if (mouse_speed > 255)
            mouse_speed = 255;

        // install ikbd irq handler
        SetTosVector(0x118, VecAcia);
        SetMacVector(0x118, VecAcia);

        // start in keyboard mode
        cur_packet.state = IKBD_STATE_KEYB;
        cur_packet.remain = 1;
        cur_packet.len = 0;

        // send reset command
        //ikbd_write(0x80);
        //ikbd_write(0x01);

        // no joystick
        ikbd_write(0x1A);
    }

    mouse_max_x = resx - 1;
    mouse_max_y = resy - 1;

    // absolute mouse mode
    uint16 vresx = (resx-1) << VRES_SHIFT;
    uint16 vresy = (resy-1) << VRES_SHIFT;
    ikbd_write(0x09);
    ikbd_write(vresx >> 8);
    ikbd_write(vresx & 0xFF);
    ikbd_write(vresy >> 8);
    ikbd_write(vresy & 0xFF);

    // set absolute mouse scaling
    uint16 mouse_speed_x = (mouse_speed * sensx) >> 8;
    uint16 mouse_speed_y = (mouse_speed * sensy) >> 8;
    ikbd_write(0x0C);
    ikbd_write(mouse_speed_x);
    ikbd_write(mouse_speed_y);

    // update absolute mouse position
    if ((VRES_TO_PRES(mouse_old.mx) >= resx) || (VRES_TO_PRES(mouse_old.my) >= resy))
    {
        mouse_old.mx = PRES_TO_VRES(16);
        mouse_old.my = PRES_TO_VRES(16);
    }
    ikbd_write(0x0E);
    ikbd_write(0x00);
    ikbd_write(mouse_old.mx >> 8);
    ikbd_write(mouse_old.mx & 0xFF);
    ikbd_write(mouse_old.my >> 8);
    ikbd_write(mouse_old.my & 0xFF);

    input_inited = true;
}

void RestoreInput()
{
    if (!input_inited)
        return;

    input_inited = false;

    // joystick event reporting mode
    ikbd_write(0x14);

    // relative mouse mode
    ikbd_write(0x08);
}


static const uint8 AtariToMacScancodes[0x80] =
{
    0xff,       // 00 ---
    0x35,       // 01 esc
    0x12,       // 02 1
    0x13,       // 03 2
    0x14,       // 04 3
    0x15,       // 05 4
    0x17,       // 06 5
    0x16,       // 07 6
    0x1a,       // 08 7
    0x1c,       // 09 8
    0x19,       // 0a 9
    0x1d,       // 0b 0
    0x1b,       // 0c -
    0x18,       // 0d =
    0x33,       // 0e backspace
    0x30,       // 0f tab
    0x0c,       // 10 q
    0x0d,       // 11 w
    0x0e,       // 12 e
    0x0f,       // 13 r
    0x11,       // 14 t
    0x10,       // 15 y
    0x20,       // 16 u
    0x22,       // 17 i
    0x1f,       // 18 o
    0x23,       // 19 p
    0x21,       // 1a [
    0x1e,       // 1b ]
    0x24,       // 1c return
    0x3b,       // 1d ctrl
    0x00,       // 1e a
    0x01,       // 1f s
    0x02,       // 20 d
    0x03,       // 21 f
    0x05,       // 22 g
    0x04,       // 23 h
    0x26,       // 24 j
    0x28,       // 25 k
    0x25,       // 26 l
    0x29,       // 27 ;
    0x27,       // 28 ´
    0x32,       // 29 ¨
    0x38,       // 2a shift (left)
    0x2a,       // 2b 
    0x06,       // 2c z
    0x07,       // 2d x
    0x08,       // 2e c
    0x09,       // 2f v
    0x0b,       // 30 b
    0x2d,       // 31 n
    0x2e,       // 32 m
    0x2b,       // 33 ,
    0x2f,       // 34 .
    0x2c,       // 35 /
    0x38,       // 36 shift (right)
    0xff,       // 37 ----
    0x37,       // 38 alt   (APPLE)
    0x31,       // 39 space
    0x39,       // 3a capslock
    0x7a,       // 3b f1
    0x78,       // 3c f2
    0x63,       // 3d f3
    0x76,       // 3e f4
    0x60,       // 3f f5
    0x61,       // 40 f6
    0x62,       // 41 f7
    0x64,       // 42 f8
    0x65,       // 43 f9
    0x6d,       // 44 f10
    0xff,       // 45 ----
    0xff,       // 46 ----
    0x73,       // 47 home
    0x3e,       // 48 arrow (up)
    0xff,       // 49 ----
    0x4e,       // 4a numpad -
    0x3b,       // 4b arrow (left)
    0xff,       // 4c ----
    0x3c,       // 4d arrow (right)
    0x45,       // 4e numpad +
    0xff,       // 4f ----
    0x3d,       // 50 arrow (down)
    0xff,       // 51 ----
    0xff,       // 52 insert            ** unused **
    0x75,       // 53 delete
    0xff,       // 54 ----
    0xff,       // 55 ----
    0xff,       // 56 ----
    0xff,       // 57 ----
    0xff,       // 58 ----
    0xff,       // 59 ----
    0xff,       // 5a ----
    0xff,       // 5b ----
    0xff,       // 5c ----
    0xff,       // 5d ----
    0xff,       // 5e ----
    0xff,       // 5f ----
    0xff,       // 60 iso key?
    0xff,       // 61 undo              ** unused **
    0x72,       // 62 help
    0x47,       // 63 numpad (
    0x51,       // 64 numpad )
    0x4b,       // 65 numpad /
    0x43,       // 66 numpad *
    0x59,       // 67 numpad 7
    0x5b,       // 68 numpad 8
    0x5c,       // 69 numpad 9
    0x56,       // 6a numpad 4
    0x57,       // 6b numpad 5
    0x58,       // 6c numpad 6
    0x53,       // 6d numpad 1
    0x54,       // 6e numpad 2
    0x55,       // 6f numpad 3
    0x52,       // 70 numpad 0
    0x41,       // 71 numpad .
    0x4c,       // 72 enter
    0xff,       // 73 ---
    0xff,       // 74 ---
    0xff,       // 75 ---
    0xff,       // 76 ---
    0xff,       // 77 ---
    0xff,       // 78 ---
    0xff,       // 79 ---
    0xff,       // 7a ---
    0xff,       // 7b ---
    0xff,       // 7c ---
    0xff,       // 7d ---
    0xff,       // 7e ---
    0xff,       // 7f ---
};

//-------------------------------------
// atari           mac
//-------------------------------------
//  control         control
//  alternate       cmd (apple)
//                  option
//                  page up
//                  page down
//                  f11-f15
//  undo
//  insert
//-------------------------------------

void UpdateInput()
{
    if (!input_inited)
        return;

    // reset input if/when ikbd overruns...
    if (cur_packet.state == IKBD_STATE_FAULT)
    {
        // flush ikbd
        __asm__ volatile ( \
            ".flushkb0:\n"\
            "  move.b   0xFFFFFC00.w,d0\n" \
            "  btst     #0,d0\n" \
            "  beq.s	.flushkb1\n" \
            "  move.b	0xFFFFFC02.w,d0\n" \
            "  bra.s	.flushkb0\n" \
            ".flushkb1:\n" \
            "  move.b	#0,0xFFFFFC02.w\n" \
		: : : "d0", "cc");

        // send mouseup
        mouse.ms = 2 | 8;

        // clear key status
        memset(keys, 0, 128);

        // reset ikbd
        //ikbd_write(0x80);
        //ikbd_write(0x81);

        cur_packet.remain = 1;
        cur_packet.len = 0;
        cur_packet.state = IKBD_STATE_KEYB;
    }


    // keyboard
    static uint8 oldKeyboardData[128];
    for (uint8 i=0; i<128; i++)
    {
        uint8& oldstatus = oldKeyboardData[i];
        uint8 newstatus = keys[i];
        if (newstatus != oldstatus)
        {
            uint8 code = AtariToMacScancodes[i];
            if (code != 0xff)
            {
                if (newstatus)
                    ADBKeyDown(code);
                else
                    ADBKeyUp(code);
            }
        }
        oldstatus = newstatus;
    }

    // mouse
    if (mouse_requested)
    {
        if (mouse.mx != mouse_old.mx || mouse.my != mouse_old.my)
        {
            mouse_old.mx = mouse.mx;
            mouse_old.my = mouse.my;
            ADBMouseMoved(VRES_TO_PRES(mouse.mx), VRES_TO_PRES(mouse.my));
        }
        if (mouse.ms & 1)   ADBMouseDown(1);
        if (mouse.ms & 2)   ADBMouseUp(1);
        if (mouse.ms & 4)   ADBMouseDown(0);
        if (mouse.ms & 8)   ADBMouseUp(0);
        mouse_old.ms = mouse.ms;
        mouse_requested = false;
    }
}

void RequestInput()
{
    // ask for new mouse data
    if (input_inited && !mouse_requested)
    {
        mouse_requested = true;
        ikbd_write(0x0D);
    }
}
