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

#ifndef SRC_MODEL_INTERFACE_DS18X20IN_H_
#define SRC_MODEL_INTERFACE_DS18X20IN_H_

#include <Model/Interface/Sensor.h>
#include <OneWire.h>

namespace AuraNode {
namespace Interface {

class DS18X20In : public Sensor {
public:
    DS18X20In(uint8_t pin, uint8_t* addr = nullptr);
    virtual ~DS18X20In();

    bool read(OSCMessage& msg);
    bool forceRead(OSCMessage& msg);

private:
    OneWire mDevice;
    uint8_t mAddress[8];
    uint32_t mConversionStart;
    float mPrevValue;

    void startConversion();
    float readRaw();
};

} /* namespace Interface */
} /* namespace AuraNode */

#endif /* SRC_MODEL_INTERFACE_DS18X20IN_H_ */
