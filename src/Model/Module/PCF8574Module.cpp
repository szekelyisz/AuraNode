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

#include <Model/Module/PCF8574Module.h>

namespace AuraNode {
namespace Module {

PCF8574Module::PCF8574Module(uint8_t address)
: mModule(address, &Wire)
, mPinStates(0)
{
	Wire.setClock(100000L);
	mModule.begin();
	// InterruptManager.subscribe(interruptPin, this);
}

void PCF8574Module::setPinMode(uint8_t pin, int mode) {
	mModule.write(pin, (mode == INPUT) ? HIGH : LOW);
}


bool PCF8574Module::read(uint8_t pin) {
	return mModule.read(pin);
}

void PCF8574Module::write(uint8_t pin, bool value) {
	mModule.write(pin, value);
}

void PCF8574Module::notify() {
	// which pin changed?
	uint8_t newState = mModule.read8();
	mPinStates ^= newState;
	for(uint8_t c = 0; c != 8; c++) {
		if(mPinStates & (1 << c)) {
			if(mSubscriptions[c]) mSubscriptions[c]->notify();
		}
	}
	mPinStates = newState;
}

void PCF8574Module::subscribe(::AuraNode::IO::PCF8574In* io, uint8_t pin) {
	mSubscriptions[pin] = io;
}

}
}
