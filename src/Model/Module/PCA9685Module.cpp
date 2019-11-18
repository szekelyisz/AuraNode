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

#include <Model/Module/PCA9685Module.h>
#include <Wire.h>
#include <SerialSysLog.h>

extern SerialSysLog logger;

namespace AuraNode {
namespace Module {

PCA9685Module::PCA9685Module(uint8_t address, bool stagger)
: mAddress(address)
, mStagger(stagger)
{
	writeReg(0x0, (uint8_t)0x21); //  RESTART | AUTOINCR | ALLADRR
//	if(stagger) for(uint8_t c = 0; c != 16; c++) writeOnTime(c, c << 8);
	for(uint8_t c = 0; c != 16; c++) {
		writeOffTime(c, 4095);
		write(c, 0);
	}
	logger.logf("Created PCA9685 module at address %d", mAddress);
}

void PCA9685Module::write(uint8_t /*in*/pin, uint16_t /*in*/value) {
	writeOnTime(pin, 4095 - (value & 0x0fff));
}

void PCA9685Module::setFrequency(uint16_t frequency) {
	// TODO
}

void PCA9685Module::writeOnTime(uint8_t pin, uint16_t time) {
	writeReg(0x06 + (pin << 2), (uint16_t)time);
}

void PCA9685Module::writeOffTime(uint8_t pin, uint16_t time) {
	writeReg(0x08 + (pin << 2), (uint16_t)time);
}

void PCA9685Module::writeReg(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(mAddress);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void PCA9685Module::writeReg(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(mAddress);
    Wire.write(reg);
    Wire.write(value & 0xff);
    Wire.write(value >> 8);
    Wire.endTransmission();
}

}
}
