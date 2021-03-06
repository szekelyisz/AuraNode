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

#include <Model/Interface/DigitalIn.h>
#include <Model/IO/DigitalIn.h>

namespace AuraNode {
namespace Interface {

DigitalIn::DigitalIn(::AuraNode::IO::DigitalIn* pin)
: mPin(pin)
{
	mPin->subscribe(this);
	prevValue = false;
}

bool DigitalIn::read(OSCMessage& msg) {
	bool value = mPin->readValue();
	if(value == prevValue) return false;
	msg.add(value ? 1 : 0);
	prevValue = value;
	return true;
}
bool DigitalIn::forceRead(OSCMessage& msg) {
	uint8_t value = mPin->readValue();
	msg.add((bool)value);
	prevValue = value;
	return true;
}

void notify() {
	// TODO implement this
}

}
}
