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

#include "DS18X20In.h"
#include "string.h"
#include "SerialSysLog.h"
#include "Arduino.h"

extern SerialSysLog logger;

namespace AuraNode {
namespace Interface {

DS18X20In::DS18X20In(uint8_t pin, uint8_t* addr)
: mDevice(pin)
, mConversionStart(0)
{
    memset(mAddress, 0, 8);
    if(addr == nullptr) {
        mDevice.reset_search();
        if(mDevice.search(mAddress)) {
            if(OneWire::crc8(mAddress, 7) != mAddress[7]) {
                logger.log("DS18X20: crc error on discovery");
                return;
            }
            const char* type;
            switch(mAddress[0]) {
            case 0x10: type = "DS18S20"; break;
            case 0x28: type = "DS18B20"; break;
            case 0x22: type = "DS1822"; break;
            default:
                logger.log(SerialSysLog::ERR, "DS18X20: unknown device found");
                mAddress[0] = 0;
                return;
            }
            logger.logf("DS18X20: %s at %02x%02x%02x%02x%02x%02x%02x%02x",
                    type,
                    mAddress[0], mAddress[1], mAddress[2], mAddress[3],
                    mAddress[4], mAddress[5], mAddress[6], mAddress[7]);
        } else {
            logger.log(SerialSysLog::ERR, "DS18X20: no device found");
            mAddress[0] = 0;
            return;
        }
    } else {
        memcpy(mAddress, addr, 8);
    }

    startConversion();
}

DS18X20In::~DS18X20In() {
}

float DS18X20In::readRaw() {
    uint8_t buffer[12];
    mDevice.reset();
    mDevice.select(mAddress);
    mDevice.write(0xbe);
    for (uint8_t i = 0; i < 9; i++)
        buffer[i] = mDevice.read();
    /* from OneWire example code */
    int16_t raw = (buffer[1] << 8) | buffer[0];
    if (mAddress[0] == 0x10) {
        raw = raw << 3; // 9 bit resolution default
        if (buffer[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - buffer[6];
        }
    } else {
        byte cfg = (buffer[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
            raw = raw & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
            raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
            raw = raw & ~1; // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
    }
    return (float)raw / 16.0;
}

bool DS18X20In::read(OSCMessage& msg) {

    if(mAddress[0] && millis() > mConversionStart + 1000) {
        // time to read the result
        float result;
        result = readRaw();
        bool change = result != mPrevValue;

        if(change) msg.add(result);

        // and start the next conversion
        startConversion();

        return change;
    }

    return false;
}

bool DS18X20In::forceRead(OSCMessage& msg) {

    if(!mAddress[0]) return false;

    startConversion();
    delay(1000);
    msg.add(readRaw());
    startConversion();
    return true;
}

void DS18X20In::startConversion() {
    mDevice.reset();
    mDevice.select(mAddress);
    mDevice.write(0x44, 1);
    mConversionStart = millis();
}

} /* namespace Interface */
} /* namespace AuraNode */
