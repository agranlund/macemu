/*
 *  serial_atari.cpp - Serial device driver, Atari implementation
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
#include "macos_util.h"
#include "prefs.h"
#include "serial.h"
#include "serial_defs.h"

#define DEBUG 0
#include "debug.h"


//----------------------------------------------------------------------
// Base port
//----------------------------------------------------------------------
class ASERDPort : public SERDPort {
public:
	ASERDPort(const char *dev) {
		device_name = (char *)dev;
	}
	virtual ~ASERDPort() {
	}
	virtual int16 open(uint16 config);
	virtual int16 prime_in(uint32 pb, uint32 dce);
	virtual int16 prime_out(uint32 pb, uint32 dce);
	virtual int16 control(uint32 pb, uint32 dce, uint16 code);
	virtual int16 status(uint32 pb, uint32 dce, uint16 code);
	virtual int16 close(void);
	virtual int16 update(void);
private:
	char *device_name;			// Device name
};



//----------------------------------------------------------------------
//
//	Helpers
//
//----------------------------------------------------------------------

SERDPort* CreateSerialPort(const char* dev)
{
	// todo: create device based on "dev" type
	return new ASERDPort(dev);
}


//----------------------------------------------------------------------
//
//	Basilisk interface
//
//----------------------------------------------------------------------

void SerialInit(void)
{
	the_serd_port[0] = CreateSerialPort(PrefsFindString("seriala"));
	the_serd_port[1] = CreateSerialPort(PrefsFindString("serialb"));
}

void SerialExit(void)
{
	delete (ASERDPort*)the_serd_port[0];
	delete (ASERDPort*)the_serd_port[1];
}

void SerialUpdate(void)
{
	((ASERDPort*)the_serd_port[0])->update();
	((ASERDPort*)the_serd_port[1])->update();
}

//----------------------------------------------------------------------
//
//	Dummy device
//
//----------------------------------------------------------------------

int16 ASERDPort::open(uint16 config)
{
	D(bug("ASERDPort::open %04x\n", config));
	return openErr;
}

int16 ASERDPort::prime_in(uint32 pb, uint32 dce)
{
	D(bug("ASERDPort::prime_in %08x, %08x\n", pb, dce));
	return readErr;
}

int16 ASERDPort::prime_out(uint32 pb, uint32 dce)
{
	D(bug("ASERDPort::prime_in %08x, %08x\n", pb, dce));
	return writErr;
}
 
int16 ASERDPort::control(uint32 pb, uint32 dce, uint16 code)
{
	D(bug("ASERDPort::control %08x, %08x, %04x\n", pb, dce, code));
	return controlErr;
}

int16 ASERDPort::status(uint32 pb, uint32 dce, uint16 code)
{
	D(bug("ASERDPort::status %08x, %08x, %04x\n", pb, dce, code));
	return statusErr;
}

int16 ASERDPort::close()
{
	D(bug("ASERDPort::close\n"));
	return noErr;
}

int16 ASERDPort::update()
{
	return 0;
}
