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

#include <Model/Module/ADS1115Module.h>

namespace AuraNode {
namespace Module {

ADS1115Module::ADS1115Module(uint8_t /*in*/address, uint8_t gain)
: mDevice(address)
{
	adsGain_t g;
	mDevice.begin();
	switch(gain) {
	case 0: g = GAIN_TWOTHIRDS; break;
	case 1: g = GAIN_ONE; break;
	case 2: g = GAIN_TWO; break;
	case 4: g = GAIN_FOUR; break;
	case 8: g = GAIN_EIGHT; break;
	case 16: g = GAIN_SIXTEEN; break;
	default: g = GAIN_TWOTHIRDS; break;
	}
	mDevice.setGain(g);
}

ADS1115Module::~ADS1115Module() {
}

uint16_t ADS1115Module::read(uint8_t /*in*/pin) {
	return mDevice.readADC_SingleEnded(pin);
}

}
}
