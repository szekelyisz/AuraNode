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

#ifndef AURANODE_INTERFACE_RGBAOUT_H
#define AURANODE_INTERFACE_RGBAOUT_H

#include <Model/Interface/Actuator.h>
#include <Model/IO/PWMOutput.h>

namespace AuraNode {
namespace Interface {

class RGBAOut: public Actuator {
public:

	RGBAOut(::AuraNode::IO::PWMOutput* r_io,
			::AuraNode::IO::PWMOutput* g_io,
			::AuraNode::IO::PWMOutput* b_io,
			::AuraNode::IO::PWMOutput* a_io = NULL);
	void write(OSCMessage& msg);

private:

	::AuraNode::IO::PWMOutput* mRPin;
	::AuraNode::IO::PWMOutput* mGPin;
	::AuraNode::IO::PWMOutput* mBPin;
	::AuraNode::IO::PWMOutput* mAPin;

};

}
}

#endif
