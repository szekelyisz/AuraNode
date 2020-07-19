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

#ifndef SRC_MODEL_MODULE_HTU21DMODULE_H_
#define SRC_MODEL_MODULE_HTU21DMODULE_H_

#include <Model/Module/I2CModule.h>
#include "SparkFunHTU21D.h"

namespace AuraNode {
namespace Module {

class HTU21DModule: public I2CModule {
public:
    HTU21DModule();
    virtual ~HTU21DModule();

    float readTemperature();
    float readHumidity();

private:
    HTU21D mDevice;
};

} /* namespace Interface */
} /* namespace AuraNode */

#endif /* SRC_MODEL_MODULE_HTU21DMODULE_H_ */