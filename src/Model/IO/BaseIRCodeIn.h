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

#ifndef SRC_MODEL_IO_BASEIRCODEIN_H_
#define SRC_MODEL_IO_BASEIRCODEIN_H_

#include <Model/IO/IRCodeIn.h>

#include <IRrecv.h>
#include <IRremoteESP8266.h>

namespace AuraNode {
namespace IO {

class BaseIRCodeIn : public IRCodeIn {
public:
	BaseIRCodeIn(uint8_t pin);
	virtual ~BaseIRCodeIn();

	bool readValue(uint32_t* code);

private:
	uint8_t mPinNumber;
	IRrecv mIRRecv;
};

}
}

#endif
