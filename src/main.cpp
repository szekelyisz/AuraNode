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

#include <map>
#include <functional>

#include "../lib/Skeleton/SkeletonClass.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <EEPROM.h>
#include <Model/Interface/Actuator.h>
#include <Model/Interface/Interface.h>
#include <Model/Interface/Sensor.h>
#include <Model/Interface/StreamingSensor.h>
#include <Model/IO/FloatOut.h>
#include <Model/Module/Module.h>
#include <SerialSysLog.h>

#ifdef WITH_ADS1115
#include <Wire.h>
#include <Model/Module/ADS1115Module.h>
#endif
#ifdef WITH_PCA9685
#include <Wire.h>
#include <Model/Module/PCA9685Module.h>
#endif
#ifdef WITH_PCF8574
#include <Wire.h>
#include <Model/Module/PCF8574Module.h>
#endif

#ifdef WITH_DIGITALIN
#include <Model/Interface/DigitalIn.h>
#include <Model/IO/DigitalIn.h>
#include <Model/IO/BaseDigitalIn.h>
#ifdef WITH_PCF8574
#include <Model/IO/PCF8574In.h>
#endif
#endif

#ifdef WITH_DIGITALOUT
#include <Model/Interface/DigitalOut.h>
#include <Model/IO/DigitalOut.h>
#include <Model/IO/BaseDigitalOut.h>
#ifdef WITH_PCF8574
#include <Model/IO/PCF8574Out.h>
#endif
#endif

#ifdef WITH_ANALOGIN
#include <Model/Interface/AnalogIn.h>
#include <Model/IO/AnalogIn.h>
#include <Model/IO/BaseAnalogIn.h>
#ifdef WITH_ADS1115
#include <Model/IO/ADS1115In.h>
#endif
#endif

#ifdef WITH_PWM
#include <Model/Interface/PWMOut.h>
#include <Model/IO/PWMOutput.h>
#include <Model/IO/BasePWM.h>
#ifdef WITH_PCA9685
#include <Model/IO/PCA9685PWM.h>
#endif
#endif

#ifdef WITH_LEDSTRIP
#include <Model/Interface/LEDStripOut.h>
#include <Model/IO/BaseLEDStrip.h>
#endif

#ifdef WITH_IR_IN
#include <Model/Interface/IRCodeIn.h>
#include <Model/IO/BaseIRCodeIn.h>
#endif

#ifdef WITH_SERVO
#include <Model/Interface/ServoOut.h>
#include <Model/IO/PWMOutput.h>
#include <Model/IO/BasePWM.h>
#ifdef WITH_PCA9685
#include <Model/IO/PCA9685PWM.h>
#endif
#endif

#ifdef WITH_RGBA
#include <Model/Interface/RGBAOut.h>
#include <Model/IO/PWMOutput.h>
#include <Model/IO/BasePWM.h>
#ifdef WITH_PCA9685
#include <Model/IO/PCA9685PWM.h>
#endif
#endif

class InterfacesConfig : public Configuration {
public:
    persistentStringVar(url, "");
    persistentStringVar(json, "");
};

class Config : public SkeletonClass::SkeletonConfig {
public:
    subconfig(InterfacesConfig, interfaces);
};

Config config;

#ifndef JSON_BUFFER_SIZE
#error JSON_BUFFER_SIZE not defined
#define JSON_BUFFER_SIZE 4096
#endif

using namespace AuraNode;

IPAddress osc_destination_host;
uint16_t osc_port_remote = 5000;
const uint16_t osc_port_local = 5000; // fixed
WiFiUDP osc_socket;

uint32_t receive_time;

std::map<String, Module::Module*> allmodules;
std::map<String, Interface::Sensor*> allsensors;
std::map<String, Interface::StreamingSensor*> allstreams;
std::map<String, Interface::Actuator*> allactuators;

auto sensor_it = allsensors.begin();

#ifdef TwoWire_h
static bool wire_started = false;

void start_wire() {
    if(!wire_started) {
        Wire.begin();
        wire_started = true;
    }
}
#endif

#ifdef WITH_PWM
IO::PWMOutput* parsePWM(JsonObject& js, JsonDocument& doc) {
    bool invert;
    if(js.containsKey("invert")
            && js["invert"].is<bool>()
            && js["invert"].as<bool>())
        invert = true;
    if(js["module"].as<String>() == "base")
        return new IO::BasePWM(js["pin"].as<uint8_t>(), invert);
    auto module = allmodules.find(js["module"].as<String>());
    if(module == allmodules.end()) {
        logger.log("module not found");
        return NULL;
    }
    String type = doc["modules"][js["module"].as<String>()]["type"].as<String>();
#ifdef WITH_PCA9685
    if(type == "pca9685" || type == "i2c_pwm")
        return new IO::PCA9685PWM(
                (Module::PCA9685Module*)allmodules[js["module"].as<String>()],
                js["pin"].as<uint8>(),
                invert);
#endif
    logger.log("unsupported module type");
    return NULL;
}
#endif

void parseTarget(JsonObject& js, AuraNode::Interface::Sensor* thing) {
    if(js.containsKey("destination") && js["destination"].is<char*>()) {
        const char * d = js["destination"].as<char*>();
        char* port_delimiter = strchr(d, ':');
        IPAddress ip;
        uint16_t port = 0;
        if(port_delimiter) {
            if(!(port = atoi(port_delimiter + 1))) {
                logger.log("Invalid OSC destination port");
                return;
            }
            *port_delimiter = 0; // ugly but... yeah :)
        }
        if(!ip.fromString(d)) {
            logger.log("Invalid OSC destination host");
            return;
        }
        thing->mTargetIP = ip;
        thing->mTargetPort = port;
        logger.logf("destination: %s:%d", ip.toString().c_str(), port);
        return;
    }
    // fall back to default
    thing->mTargetIP = osc_destination_host;
    thing->mTargetPort = osc_port_remote;
}

int parseInteger(JsonVariantConst v) {
    if(v.is<char*>()) return strtoul(v.as<char*>(), nullptr, 0);
    return v.as<int>();
}

template<class T>
uint8_t parseConfig(const T& json) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    logger.logf("Buffer size: %d", JSON_BUFFER_SIZE);

    DeserializationError err = deserializeJson(doc, (T&)json);
    if(err) {
        logger.logf("Error parsing config: %s", err.c_str());
        return 1;
    }

    // NODE CONFIG

    if(doc.containsKey("destination") && doc["destination"].is<char*>()) {
        const char * d = doc["destination"].as<char*>();
        char* port_delimiter = strchr(d, ':');
        if(port_delimiter) {
            if(!(osc_port_remote = atoi(port_delimiter + 1))) {
                logger.log("Invalid OSC destination port");
                return 1;
            }
            *port_delimiter = 0; // ugly but... yeah :)
        }
        if(!osc_destination_host.fromString(d)) {
            logger.log("Invalid OSC destination host");
            return 1;
        }
    } else {
        logger.logw("no OSC destination");
        return 1;
    }

    // MODULES

    if(doc.containsKey("modules") && doc["modules"].is<JsonObject>()) {
        long address;
        for (JsonObject::iterator it = doc["modules"].as<JsonObject>().begin();
                it != doc["modules"].as<JsonObject>().end(); ++it) {
            JsonObject module = it->value();
            delay(100);

            // make a copy of the module name
            String* module_name = new String(it->key().c_str());
            String type = module["type"].as<String>();
            logger.logf("Adding module '%s' type '%s'",
                    module_name->c_str(), type.c_str());
#ifdef WITH_ADS1115
            if(type == "ads1115" || type == "i2c_adc") {
                if(module.containsKey("address")
                        && module["address"].is<char*>()) {
                    address = parseInteger(module["address"]);
                    if(address == 0 || address > 128) {
                        logger.logf("invalid address: %s",
                                module["address"].as<char*>());
                        return 1;
                    } else {
                        start_wire();
                        allmodules[*module_name] = new
                                Module::ADS1115Module(address);
                        logger.logf("module created: address = 0x%02lx",
                                address);
                    }
                } else {
                    logger.log("no address");
                    return 1;
                }
                continue;
            }
#endif
#ifdef WITH_PCF8574
            if(type == "pcf8574" || type == "i2c_gpio") {
                if(module.containsKey("address")
                        && module["address"].is<char*>()) {
                    address = parseInteger(module["address"]);
                    if(address == 0 || address > 128) {
                        logger.logf("invalid address: %s",
                                module["address"].as<char*>());
                        return 1;
                    } else {
                        start_wire();
                        allmodules[*module_name] = new
                                Module::PCF8574Module(address);
                        logger.logf("module created: address = 0x%02lx",
                                address);
                    }
                } else {
                    logger.log("no address");
                    return 1;
                }
                continue;
            }
#endif
#ifdef WITH_PCA9685
            if(type == "pca9685" || type == "i2c_pwm") {
                if(module.containsKey("address")
                        && module["address"].is<char*>()) {
                    address = parseInteger(module["address"]);
                    if(address == 0 || address > 128) {
                        logger.logf("invalid address: %s",
                                module["address"].as<char*>());
                        return 1;
                    } else {
                        start_wire();
                        allmodules[*module_name] = new
                                Module::PCA9685Module(address);
                        logger.logf("module created: address = 0x%02lx",
                                address);
                    }
                } else {
                    logger.log("no address");
                    return 1;
                }
                continue;
            }
#endif
            logger.logw("unknown type, ignored");
        }
    }


    // INTERFACES

    if(!doc.containsKey("interfaces") || !doc["interfaces"].is<JsonObject>()) {
        logger.log("No interfaces configured");
        return 1;
    }

    for(JsonObject::iterator it = doc["interfaces"].as<JsonObject>().begin();
            it != doc["interfaces"].as<JsonObject>().end(); ++it) {
        delay(100);
        JsonObject intf = it->value();
        // make a copy of the OSC path
        String* osc_path = new String(it->key().c_str());
        String intf_type = intf["type"].as<String>();
        logger.logf("Interface '%s' type '%s'",
                osc_path->c_str(), intf_type.c_str());
        if(osc_path[0][0] != '/') {
            logger.logw("Definition ignored");
            continue;
        }
#ifdef WITH_ANALOGIN
        if(intf_type == "analog_in") {
            // ANALOG_IN
            if(!intf.containsKey("on") || !intf["on"].is<JsonObject>()) {
                logger.log("no IO");
                continue;
            }
            JsonObject on = intf["on"].as<JsonObject>();
            if(!on.containsKey("module") || !on["module"].is<char*>()) {
                logger.log("no module");
                continue;
            }
            if(!on.containsKey("pin") || !on["pin"].is<uint8_t>()) {
                logger.log("no pin");
                continue;
            }
            IO::AnalogIn* io;
            if(on["module"].as<String>() == "base") {
                io = new IO::BaseAnalogIn(on["pin"].as<uint8_t>());
            } else if(allmodules.find(on["module"].as<char*>()) !=
                    allmodules.end()) {
                String type = doc["modules"][on["module"].
                                             as<char*>()]["type"].as<String>();
#ifdef WITH_ADS1115
                if(type == "ads1115" || type == "i2c_adc") {
                    io = new IO::ADS1115In(
                            (Module::ADS1115Module*)allmodules[on["module"].
                                                               as<char*>()],
                            on["pin"].as<uint8_t>());
                } else
#endif
                {
                    logger.log("wrong module");
                    continue;
                }
            } else {
                logger.log("module not found");
                continue;
            }
            Interface::AnalogIn* t = new Interface::AnalogIn(io);
            parseTarget(intf, t);
            allsensors[*osc_path] = t;
            allstreams[*osc_path] = t;
            goto thing_added;
        }
#endif
#ifdef WITH_DIGITALIN
        if(intf_type == "digital_in") {
            // DIGITAL_IN
            if(!intf.containsKey("on") || !intf["on"].is<JsonObject>()) {
                logger.log("no IO");
                continue;
            }
            JsonObject on = intf["on"].as<JsonObject>();
            if(!on.containsKey("module") || !on["module"].is<char*>()) {
                logger.log("no module");
                continue;
            }
            if(!on.containsKey("pin") || !on["pin"].is<uint8_t>()) {
                logger.log("no pin");
                continue;
            }
            IO::DigitalIn* io;
            if(on["module"].as<String>() == "base") {
                io = new IO::BaseDigitalIn(on["pin"].as<uint8_t>());
            } else if(allmodules.find(on["module"].as<char*>()) !=
                    allmodules.end()) {
                String type = doc["modules"][on["module"].
                                             as<char*>()]["type"].as<String>();
#ifdef WITH_PCF8574
                if(type == "pcf8574" || type == "i2c_gpio") {
                    io = new IO::PCF8574In(
                            (Module::PCF8574Module*)allmodules[on["module"].
                                                               as<char*>()],
                            on["pin"].as<uint8_t>());
                } else
#endif
                {
                    logger.log("wrong module");
                    continue;
                }
            } else {
                logger.log("module not found");
                continue;
            }
            allsensors[*osc_path] = new Interface::DigitalIn(io);
            parseTarget(intf, allsensors[*osc_path]);
            goto thing_added;
        }
#endif
#ifdef WITH_DIGITALOUT
        if(intf_type == "digital_out") {
            // DIGITAL_OUT
            if(!intf.containsKey("on") || !intf["on"].is<JsonObject>()) {
                logger.log("no IO");
                continue;
            }
            JsonObject on = intf["on"].as<JsonObject>();
            if(!on.containsKey("module") || !on["module"].is<char*>()) {
                logger.log("no module");
                continue;
            }
            if(!on.containsKey("pin") || !on["pin"].is<uint8_t>()) {
                logger.log("no pin");
                continue;
            }
            IO::DigitalOut* io;
            if(on["module"].as<String>() == "base") {
                io = new IO::BaseDigitalOut(on["pin"].as<uint8_t>());
            } else if(allmodules.find(on["module"].as<char*>()) !=
                    allmodules.end()) {
                String type = doc["modules"][on["module"].
                                             as<char*>()]["type"].as<String>();
#ifdef WITH_PCF8574
                if(type == "pcf8574" || type == "i2c_gpio") {
                    io = new IO::PCF8574Out(
                            (Module::PCF8574Module*)allmodules[on["module"].
                                                               as<char*>()],
                            on["pin"].as<uint8_t>());
                } else
#endif
                {
                    logger.loge("wrong module");
                    continue;
                }
            } else {
                logger.loge("module not found");
                continue;
            }
            allactuators[*osc_path] = new Interface::DigitalOut(io);
            parseTarget(intf, allsensors[*osc_path]);
            goto thing_added;
        }
#endif
#ifdef WITH_PWM
        if(intf_type == "pwm" || intf_type == "servo") {
            // PWM / SERVO
            if(!intf.containsKey("on") || !intf["on"].is<JsonObject>()) {
                logger.loge("no IO");
                continue;
            }
            JsonObject on = intf["on"].as<JsonObject>();
            if(!on.containsKey("module") || !on["module"].is<char*>()) {
                logger.loge("no module");
                continue;
            }
            if(!on.containsKey("pin") || !on["pin"].is<uint8_t>()) {
                logger.loge("no pin");
                continue;
            }
            IO::PWMOutput* io = parsePWM(on, doc);
            if(io) {
                allactuators[*osc_path] =
                    (intf_type == "pwm") ?
                            (Interface::Actuator*)new Interface::PWMOut(io) :
                            (Interface::Actuator*)new Interface::ServoOut(io);
                goto thing_added;
            } else {
                continue;
            }
        }
#endif
#ifdef WITH_LEDSTRIP
        if(intf_type == "ledstrip") {
            // LEDSTRIP
            if(!intf.containsKey("length") ||
                    !intf["length"].is<uint16_t>()) {
                logger.loge("no length");
                continue;
            }
            if(!intf.containsKey("on") || !intf["on"].is<JsonObject>()) {
                logger.loge("no IO");
                continue;
            }
            JsonObject on = intf["on"].as<JsonObject>();
            if(!on.containsKey("module") || !on["module"].is<char*>()) {
                logger.loge("no module");
                continue;
            }
            if(!on.containsKey("pin") || !on["pin"].is<uint8_t>()) {
                logger.loge("no pin");
                continue;
            }
            if(on["module"].as<String>() != "base") {
                logger.loge("wrong module");
                continue;
            }
            uint16_t ch = 3;
            if(intf.containsKey("channels")) {
                if(!intf["channels"].is<uint16_t>()) {
                    logger.loge("channels must be integer");
                    continue;
                }
                ch = intf["channels"].as<uint16_t>();
            }
            if(ch == 3) {
                auto io = new IO::BaseLEDStrip<
                        NeoGrbFeature,
                        Neo800KbpsMethod,
                        RgbColor, 3>(on["pin"].as<uint8_t>(),
                                intf["length"].as<uint16_t>());
                allactuators[*osc_path] = new Interface::LEDStripOut(io);
                goto thing_added;
            } else if(ch == 4) {
                auto io = new IO::BaseLEDStrip<
                        NeoGrbwFeature,
                        Neo800KbpsMethod,
                        RgbwColor, 4>(on["pin"].as<uint8_t>(),
                                intf["length"].as<uint16_t>());
                allactuators[*osc_path] = new Interface::LEDStripOut(io);
                goto thing_added;
            } else {
                logger.log("channels must be 3 or 4");
                continue;
            }
        }
#endif
#ifdef WITH_RGBA
        if(intf_type == "rgba") {
            // RGBA
            bool has_a = false;
            if(!intf.containsKey("r") || !intf["r"].is<JsonObject>()) {
                logger.loge("no red IO");
                continue;
            }
            if(!intf.containsKey("g") || !intf["g"].is<JsonObject>()) {
                logger.loge("no green IO");
                continue;
            }
            if(!intf.containsKey("b") || !intf["b"].is<JsonObject>()) {
                logger.loge("no blue IO");
                continue;
            }
            if(intf.containsKey("a")) {
                if(!intf["a"].is<JsonObject>()) {
                    logger.loge("error in amber pin");
                    continue;
                }
                has_a = true;
            }

            JsonObject r_on = intf["r"].as<JsonObject>();
            if(!r_on.containsKey("module") || !r_on["module"].is<char*>()) {
                logger.loge("no module for red");
                continue;
            }
            if(!r_on.containsKey("pin") || !r_on["pin"].is<uint8_t>()) {
                logger.loge("no pin for red");
                continue;
            }
            JsonObject g_on = intf["g"].as<JsonObject>();
            if(!g_on.containsKey("module") || !g_on["module"].is<char*>()) {
                logger.loge("no module for green");
                continue;
            }
            if(!g_on.containsKey("pin") || !g_on["pin"].is<uint8_t>()) {
                logger.loge("no pin for green");
                continue;
            }
            JsonObject b_on = intf["b"].as<JsonObject>();
            if(!b_on.containsKey("module") || !b_on["module"].is<char*>()) {
                logger.loge("no module for blue");
                continue;
            }
            if(!b_on.containsKey("pin") || !b_on["pin"].is<uint8_t>()) {
                logger.loge("no pin for blue");
                continue;
            }
            JsonObject a_on = intf["a"].as<JsonObject>();
            if(has_a) {
                if(!a_on.containsKey("module") ||
                        !a_on["module"].is<char*>()) {
                    logger.loge("no module for amber");
                    continue;
                }
                if(!a_on.containsKey("pin") || !a_on["pin"].is<uint8_t>()) {
                    logger.loge("no pin for amber");
                    continue;
                }
            }

            IO::PWMOutput* r_io = parsePWM(r_on, doc);
            if(!r_io) continue;
            IO::PWMOutput* g_io = parsePWM(g_on, doc);
            if(!g_io) {
                delete r_io;
                continue;
            }
            IO::PWMOutput* b_io = parsePWM(b_on, doc);
            if(!b_io) {
                delete r_io;
                delete g_io;
                continue;
            }
            IO::PWMOutput* a_io = NULL;
            if(has_a) {
                a_io = parsePWM(a_on, doc);
                if(!a_io) {
                    delete r_io;
                    delete g_io;
                    delete b_io;
                    continue;
                }
            }
            allactuators[*osc_path] = (Interface::Actuator*)new
                    Interface::RGBAOut(r_io, g_io, b_io, a_io);
            goto thing_added;
        }
#endif
#ifdef WITH_IR_IN
        if(intf_type == "ir_in") {
            if(!intf.containsKey("on") || !intf["on"].is<JsonObject>()) {
                logger.loge("no IO");
                continue;
            }
            JsonObject on = intf["on"].as<JsonObject>();
            if(!on.containsKey("module") || !on["module"].is<char*>()) {
                logger.loge("no module");
                continue;
            }
            if(!on.containsKey("pin") || !on["pin"].is<uint8_t>()) {
                logger.loge("no pin");
                continue;
            }
            if(on["module"].as<String>() != "base") {
                logger.loge("IR input is supported only on base module");
                continue;
            }
            if(!on.containsKey("pin") || !on["pin"].is<uint8_t>()) {
                logger.log("no pin");
                continue;
            }

            IO::BaseIRCodeIn* io = new IO::BaseIRCodeIn(
                    on["pin"].as<uint8_t>());
            allsensors[*osc_path] = new Interface::IRCodeIn(io);
            parseTarget(intf, allsensors[*osc_path]);
            goto thing_added;
        }
#endif
        logger.loge("unknown type");
        continue;
thing_added:
        logger.logf("'%s' added", osc_path->c_str());
    }

    logger.log("Config parsed successfully");
    return 0;
}

String getConfigURL() {
    if(config.interfaces.url.length()) return config.interfaces.url;

    logger.log("Searching for config servers...");

    uint32_t answers = MDNS.queryService("_AuraNode-config", "tcp", 3000);
    if(!answers) {
        logger.log("No config server found");
        return "";
    }
    logger.logf("Config server found at %s:%d/", MDNS.hostname(0).c_str(),
            MDNS.port(0));
    return String("http://") +
            ((MDNS.hostname(0) != "")
                    ? MDNS.hostname(0)
                            : MDNS.IP(0).toString()) +
            ":" + MDNS.port(0) + "/";
}

bool localConfig(const String& filename) {
    if (SPIFFS.exists(filename)) {
        logger.logf("Found config file %s", filename.c_str());
        File f(SPIFFS.open(filename, "r"));
        return parseConfig<Stream>((const Stream&) (f));
        f.close();
    } else {
        logger.logf("%s not found", filename.c_str());
        return false;
    }
}

bool tryLocalConfigs(const std::vector<String>& filenames) {
    for(auto cfname : filenames) {
        if(localConfig(cfname)) return true;
    }
    logger.log("No local configuration found");
    return false;
}

bool httpConfig(const String& config_url) {
    HTTPClient configHttpClient;
    WiFiClient netClient;

    logger.logf("Downloading %s", config_url.c_str());

    configHttpClient.begin(netClient, config_url);
    int httpCode = configHttpClient.GET();

    if(httpCode < 0) {
        logger.loge("Download failed: connection error");
        configHttpClient.end();
        return false;
    } else {
        if(httpCode != HTTP_CODE_OK) {
            logger.logfw("Download failed: status %d (%s)", httpCode,
                    configHttpClient.errorToString(httpCode).c_str());
            configHttpClient.end();
            return false;
        }
    }

    logger.log("Download successful");

    if(parseConfig<Stream>(configHttpClient.getStream())) {
        logger.loge("Config parsing failed");
        configHttpClient.end();
        return false;
    }

    configHttpClient.end();
    return true;
}

bool tryHttpConfigs(const String& config_url,
        const std::vector<String>& filenames) {
    if(config_url.endsWith("/")) {
        for (auto cfname : filenames) {
            if(httpConfig(config_url + cfname)) return true;
        }
    } else {
        if(httpConfig(config_url)) return true;
    }
    return false;
}

bool loadConfig() {
    logger.log("Loading config");

    String fn_prefix("AuraNode.");
    String fn_postfix(".json");
    String config_url = getConfigURL();
    String hostname = Skeleton.getHostname();
    std::vector<String> filenames;
    char chip_id[8];

    snprintf(chip_id, 8, "%06X", ESP.getChipId());
    filenames.push_back(hostname + fn_postfix);
    filenames.push_back(fn_prefix + hostname + fn_postfix);
    filenames.push_back(fn_prefix + chip_id + fn_postfix);
    filenames.push_back("AuraNode.json");

    if(config_url.length()) {
        if (tryHttpConfigs(config_url, filenames)) return true;
        logger.log("Retrying with local config");
    } else {
        logger.log("Config URL not set, trying local file");
    }

    if(tryLocalConfigs(filenames)) return true;
    logger.log("Retrying with text configuration");
    if(config.interfaces.json.length())
        return parseConfig<String>(config.interfaces.json);
    logger.logw("No configuration found");
    return false;
}

void setup(void) {
    Serial.begin(115200);
    Serial.println();
    Serial.println("============");
    Serial.println("= AuraNode =");
    Serial.println("============");
    Serial.flush();

    if(!Skeleton.init(&config, "AuraNode")) {
        Serial.println("Skeleton initialization failed; unable to continue");
        while(true) delay(1000);
    }

    logger.log("Skeleton init done");
    logger.flush();

    MDNS.addService("AuraNode", "udp", osc_port_local);

    loadConfig(); // failure is not an error -- configuration should still be
                  // possible

    sensor_it = allsensors.begin();
    osc_socket.begin(osc_port_local);
    logger.log(SerialSysLog::INFO, "Init done");
    logger.logf("Free heap size: %d", ESP.getFreeHeap());
    receive_time = millis() + 1000;
}

OSCMessage msg;

auto actuator = allactuators.begin();

uint16_t messages_in = 0, messages_out = 0, loop_count = 0;

void loop(void) {
    Skeleton.loop();

    if(receive_time < millis()) {
        logger.logf(SerialSysLog::DEBUG, "loop: %d, in: %d, out: %d, heap: %d",
                loop_count, messages_in, messages_out, ESP.getFreeHeap());
        loop_count = messages_in = messages_out = 0;
        receive_time = millis() + 1000;
        // digitalWrite(2, !digitalRead(2));
    }

    int osc_packet_size;
    while((osc_packet_size = osc_socket.parsePacket()) > 0) {
//		logger.logf(SerialSysLog::DEBUG, "got %d", osc_packet_size);
        uint8_t packet_buffer[128];
        while(osc_packet_size > 0) {
            int size = osc_socket.read(packet_buffer, 128);
            if(size < 0) {
                osc_socket.flush();
                break;
            }
            msg.fill(packet_buffer, size);
            osc_packet_size -= size;
        }

        if(!msg.hasError()) {
//			logger.logf(SerialSysLog::DEBUG, "recv on '%s'", msg.getAddress());
            actuator = allactuators.find(msg.getAddress());
            if(actuator != allactuators.end()) {
                actuator->second->write(msg);
                yield();
            }
            messages_in++;
        } else {
//			logger.logf(SerialSysLog::DEBUG, "OSC packet error: %d / %d",
//                  msg.getError(), msg.getOSCData(0)->error);
        }
        msg.empty();
    }

    if(allsensors.size() == 0) return;

    if(sensor_it == allsensors.end()) {
        sensor_it = allsensors.begin();
        if(sensor_it == allsensors.end()) {
            delay(5);
            return;
        }
    }

//	logger.logf("Reading %s", sensor_it->first.c_str());
    if(sensor_it->second->read(msg)) {
//		logger.logf(SerialSysLog::DEBUG, "event on %s",
//              sensor_it->first.c_str());
        osc_socket.beginPacket(sensor_it->second->mTargetIP,
                sensor_it->second->mTargetPort);
//		logger.logf(SerialSysLog::DEBUG, "sending to %s:%d",
//				sensor_it->second->mTargetIP.toString().c_str(),
//				sensor_it->second->mTargetPort);
        msg.setAddress(sensor_it->first.c_str());
        msg.send(osc_socket);
//		osc_socket.endPacket();
        yield();
        msg.empty();
        osc_socket.flush();
        messages_out++;
    }
    ++sensor_it;


//	osc_socket.flush();
//	delay(5);

    loop_count++;
}
