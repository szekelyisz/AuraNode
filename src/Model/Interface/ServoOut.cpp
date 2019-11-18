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

#include <Model/Interface/ServoOut.h>
#include <Model/IO/PWMOutput.h>
#include <SerialSysLog.h>

extern SerialSysLog logger;

namespace AuraNode {
namespace Interface {

ServoOut::ServoOut(::AuraNode::IO::PWMOutput* pin)
: mPin(pin)
{
	mPin->setFrequency(50);
}

void ServoOut::write(OSCMessage& msg) {
	// syslog.logf(LOG_DEBUG, "PWMOut: 0x%08x", msg.getInt(0));
	switch(msg.getType(0)) {
	case 'i':
		// int32
		mPin->writeValue(3276 + msg.getInt(0) * 3277 / 65536);
		break;
	case 'f':
		// float
		mPin->writeValue((1 + msg.getFloat(0)) * 3276.8);
		break;
	default:
		logger.logf(SerialSysLog::ERR, "%s: unrecognized number format '%c'",
				msg.getAddress(), msg.getType(0));
		break;
	}
}

}
}
