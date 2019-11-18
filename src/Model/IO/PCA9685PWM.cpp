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

#include <Model/IO/PCA9685PWM.h>
#include <SerialSysLog.h>

extern SerialSysLog logger;

namespace AuraNode {
namespace IO {

PCA9685PWM::PCA9685PWM(::AuraNode::Module::PCA9685Module* module, uint8_t pin,
		bool invert)
: PWMOutput(invert)
, mModule(module)
, mPinNumber(pin)
{

}

void PCA9685PWM::writeValue(uint32_t value) {
	// scale down 16bit -> 12 bit native
//	logger.logf("[%02d] = %d", mPinNumber, value);

	value >>= 4;
	mModule->write(mPinNumber, mInvert ? (0x0fff - value) : value);
	// syslog.logf("IO::PCA9685PWM 0x%04x", value >> 4);
}


void PCA9685PWM::setFrequency(uint16_t frequency) {
	mModule->setFrequency(frequency);
}

}
}
