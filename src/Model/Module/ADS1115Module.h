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

#ifndef AURANODE_MODULE_ADS1115MODULE_H
#define AURANODE_MODULE_ADS1115MODULE_H

#include <Model/Module/I2CModule.h>

#include <Adafruit_ADS1015.h>

namespace AuraNode {
namespace Module {

class ADS1115Module: public I2CModule {
public:

	ADS1115Module(uint8_t address, uint8_t gain = 0);
	virtual ~ADS1115Module();

	uint16_t read(uint8_t pin);

private:
	Adafruit_ADS1115 mDevice;

};

}
}

#endif
