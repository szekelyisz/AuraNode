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

#ifndef AURANODE_INTERFACE_SENSOR_H
#define AURANODE_INTERFACE_SENSOR_H

#include <Model/Interface/Interface.h>
#include <WiFiUdp.h>

namespace AuraNode {
namespace Interface {

class Sensor: public Interface {
public:

	IPAddress mTargetIP;
	uint16_t mTargetPort;
	uint32_t mInterval = 0;
	uint32_t nextTime = 0;

	virtual bool read(OSCMessage& msg) = 0;
	virtual bool forceRead(OSCMessage& msg) = 0;

};

}
}

#endif
