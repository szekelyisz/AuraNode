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

#ifndef AURANODE_MODULE_PWMMODULE_H
#define AURANODE_MODULE_PWMMODULE_H

#include <Model/Module/I2CModule.h>

namespace AuraNode {
namespace Module {

class PCA9685Module: public I2CModule {
public:

	PCA9685Module(uint8_t address, bool stagger = true);
	void write(uint8_t pin, uint16_t value);
	void setFrequency(uint16_t frequency);

private:

	uint8_t mAddress;
	bool mStagger;

	void writeOnTime(uint8_t pin, uint16_t time);
	void writeOffTime(uint8_t pin, uint16_t time);

	void writeReg(uint8_t reg, uint8_t value);
	void writeReg(uint8_t reg, uint16_t value);

};

}
}

#endif
