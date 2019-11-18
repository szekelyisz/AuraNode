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

#ifndef AURANODE_MODULE_PCF8574MODULE_H
#define AURANODE_MODULE_PCF8574MODULE_H

#include <Model/IO/PCF8574In.h>
#include <Model/Module/I2CModule.h>

#include "pcf8574_esp.h"

namespace AuraNode {
namespace Module {

class PCF8574Module: public I2CModule {
public:
	PCF8574Module(uint8_t address);
	void setPinMode(uint8_t pin, int mode);
	bool read(uint8_t pin);
	void write(uint8_t pin, bool value);
	void notify();
	void subscribe(::AuraNode::IO::PCF8574In* io, uint8_t pin);

private:
	PCF857x mModule;
	AuraNode::IO::DigitalIn* mSubscriptions[8];
	uint8_t mPinStates;

};

}
}

#endif
