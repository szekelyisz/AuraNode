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

#ifndef SRC_MODEL_INTERFACE_LM75_H_
#define SRC_MODEL_INTERFACE_LM75_H_

#include <Model/Interface/Sensor.h>
#include <Temperature_LM75_Derived.h>

namespace AuraNode {
namespace Interface {

template<typename T> class LM75: public Sensor {
public:
    LM75(TwoWire& bus, uint8_t address)
    : mSensor(&bus, address)
    , prevValue(0)
    {
    }
    virtual ~LM75() {};

    bool read(OSCMessage& msg) {
        float result = mSensor.readTemperatureC();
        if(result != prevValue) {
            msg.add(result);
            prevValue = result;
            return true;
        }
        return false;
    }

    bool forceRead(OSCMessage& msg) {
        float result = mSensor.readTemperatureC();
        msg.add(result);
        prevValue = result;
        return true;
    }

private:
    T mSensor;
    float prevValue;
};

} /* namespace Interface */
} /* namespace AuraNode */

#endif /* SRC_MODEL_INTERFACE_LM75_H_ */
