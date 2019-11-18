/*
	AuraNode - ESP8266 firmware for wireless access to sensors and actuators
    Copyright (C) 2016 - 2019  Szabolcs Szekelyi

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Model/Interface/RGBAOut.h>
#include <Model/IO/PWMOutput.h>
#include <SerialSysLog.h>

extern SerialSysLog logger;

namespace AuraNode {
namespace Interface {

RGBAOut::RGBAOut(::AuraNode::IO::PWMOutput* r_io,
		::AuraNode::IO::PWMOutput* g_io, ::AuraNode::IO::PWMOutput* b_io,
		 ::AuraNode::IO::PWMOutput* a_io)
: mRPin(r_io)
, mGPin(g_io)
, mBPin(b_io)
, mAPin(a_io)
{

}

void RGBAOut::write(OSCMessage& msg) {
	uint8_t nvalues = msg.size();
	uint16_t c[4];
	uint32_t i;
	switch(nvalues) {
	case 1:
		if(msg.getType(0) != 'i') break;
		// a single int32 -- interpret it as packed RGBA
		i = msg.getInt(0);
		// values must be scaled up 8 bits -> 16 bits
		c[0] = (i & 0xff000000) >> 16;
		c[1] = (i & 0x00ff0000) >> 8;
		c[2] = (i & 0x0000ff00);
		c[3] = mAPin ? (i & 0xff) << 8 : 0;
		break;
	case 3:
	case 4:
		// values in dedicated slots -- uint16 or 0..1 float
		for(i = 0; i != nvalues; i++) {
			switch(msg.getType(i)) {
			case 'i':
				c[i] = msg.getInt(i) << 8;
//				logger.logf("[%d] = %d", i, msg.getInt(i));
				break;
			case 'f':
				c[i] = msg.getFloat(i) * 0xffff;
				break;
			default:
				logger.logf(SerialSysLog::ERR, "unrecognized number format: %c", msg.getType(i));
				break;
			}
		}
		break;
	default:
		// unrecognized format
		logger.logf(SerialSysLog::ERR, "%s: unrecognized format string",
				msg.getAddress());
		return;
	}
	mRPin->writeValue(c[0]);
	mGPin->writeValue(c[1]);
	mBPin->writeValue(c[2]);
	if(mAPin) mAPin->writeValue(c[3]);

}

}
}
