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

#ifndef AURANODE_INTERFACE_ANALOGIN_H
#define AURANODE_INTERFACE_ANALOGIN_H

#include <Model/Interface/Sensor.h>
#include <Model/IO/AnalogIn.h>

namespace AuraNode {
namespace Interface {

class AnalogIn: public Sensor {
protected:

	uint32_t prevValue;

public:

	AnalogIn(::AuraNode::IO::AnalogIn *io);
	virtual ~AnalogIn();

	bool read(OSCMessage& msg);
	bool forceRead(OSCMessage& msg);

private:

	::AuraNode::IO::AnalogIn* mPin;

};

}
}

#endif
