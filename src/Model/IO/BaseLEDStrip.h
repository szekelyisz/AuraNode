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

#ifndef AURANODE_IO_BASELEDSTRIP_H
#define AURANODE_IO_BASELEDSTRIP_H

#include <Model/IO/BLOBOut.h>
#include <SerialSysLog.h>
#include "NeoPixelBus.h"

extern SerialSysLog logger;

namespace AuraNode {
namespace IO {

template<class FEATURE, class METHOD, class PIXEL, int CHANNELS>
class BaseLEDStrip: public BLOBOut {
private:
	NeoPixelBus<FEATURE, METHOD> strip;

public:

	BaseLEDStrip(uint8_t pin, uint16_t length)
	: strip(NeoPixelBus<FEATURE, METHOD>(length)) {
		strip.Begin();
		strip.Show();
		logger.logf("BaseLEDStrip: length = %d", length);
	}

	virtual ~BaseLEDStrip() {};

	void writeValue(const uint8_t* /*in*/value, uint16_t /*in*/length) {
		PIXEL* leds = (PIXEL*)value;
		length /= CHANNELS;
		uint16_t n = (strip.PixelCount() < length)
				? strip.PixelCount() : length;
		for(int c = 0; c != n; c++) strip.SetPixelColor(c, leds[c]);
		if(strip.CanShow()) {
//			logger.logf(SerialSysLog::DEBUG, "BaseLEDStrip: %d", length);
//			for(uint8_t c = 0; c != length; c++) {
//				logger.logf(SerialSysLog::DEBUG, "%02x", value[c]);
//			}
			strip.Show();
		}
	}


};

}
}

#endif
