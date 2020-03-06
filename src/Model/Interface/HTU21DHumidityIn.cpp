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

#include <Model/Interface/HTU21DHumidityIn.h>

namespace AuraNode {
namespace Interface {

HTU21DHumidityIn::HTU21DHumidityIn(::AuraNode::Module::HTU21DModule* module)
: mModule(module)
, prevValue(NAN)
{
}

HTU21DHumidityIn::~HTU21DHumidityIn() {
}

bool HTU21DHumidityIn::read(OSCMessage& msg) {
    float value = mModule->readHumidity();
    if(value == prevValue) return false;
    msg.add(value);
    prevValue = value;
    return true;
}

bool HTU21DHumidityIn::forceRead(OSCMessage& msg) {
    float value = mModule->readHumidity();
    msg.add(value);
    prevValue = value;
    return true;
}

} /* namespace Interface */
} /* namespace AuraNode */
