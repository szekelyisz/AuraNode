/*
    SerialSysLog - logger class with serial and syslog output
    Copyright (C) 2018 - 2019  Szabolcs Szekelyi

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

#include <SerialSysLog.h>
#include <Arduino.h>

SerialSysLog logger;

SerialSysLog::SerialSysLog()
: mSerialEnabled(false)
, mSyslogEnabled(false)
, mDefaultPriority(LOG_KERN | INFO)
, mSyslogSocket()
, mSyslog(mSyslogSocket) {
}

SerialSysLog::~SerialSysLog() {
}

void SerialSysLog::serialOn(uint32_t baud) {
    Serial.begin(baud);
//	while (!Serial);
    mSerialEnabled = true;
}

void SerialSysLog::serialOff() {
    mSerialEnabled = false;
    Serial.end();
}

void SerialSysLog::syslogOn(const char *server, uint16_t port, const char *host,
        const char* appname) {
    mSyslog.server(server, port);
    mSyslog.deviceHostname(host);
    mSyslog.appName(appname);
    mSyslog.defaultPriority(mDefaultPriority);
    mSyslogEnabled = true;
}

void SerialSysLog::syslogOff() {
    mSyslogEnabled = false;
}

void SerialSysLog::logv(Severity prio, const char *format, va_list args) {
    if(mSerialEnabled) {
        va_list args_serial;
        va_copy(args_serial, args);
        serial_vsnprintf(format, args_serial);
        va_end(args_serial);
    }
    if(mSyslogEnabled) mSyslog.vlogf(prio | LOG_KERN, format, args);
    flush();
}

void SerialSysLog::logf(Severity prio, const char *format, ...) {
    va_list args;
    va_start(args, format);
    logv(prio, format, args);
    va_end(args);
}

void SerialSysLog::logf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    logv(Severity::INFO, format, args);
    va_end(args);
}

void SerialSysLog::log(Severity prio, const char *message) {
    if(mSerialEnabled) Serial.println(message);
    if(mSyslogEnabled) mSyslog.log(prio, message);
    flush();
}

void SerialSysLog::log(const char *message) {
    if(mSerialEnabled) Serial.println(message);
    if(mSyslogEnabled) mSyslog.log(message);
    flush();
}

void SerialSysLog::logfe(const char *format, ...) {
    va_list args;
    va_start(args, format);
    logv(Severity::ERR, format, args);
    va_end(args);
}

void SerialSysLog::logfw(const char *format, ...) {
    va_list args;
    va_start(args, format);
    logv(Severity::WARN, format, args);
    va_end(args);
}

void SerialSysLog::flush() {
    if(mSerialEnabled) Serial.flush();
    if(mSyslogEnabled) mSyslogSocket.flush();
}

int SerialSysLog::serial_vsnprintf(const char *format, va_list args) {
    /* Exact copy of Print::printf since there's no variadic version
     * of it (Print::vprintf for example)
     */
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, args);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
                return 0;
        }
        vsnprintf(buffer, len + 1, format, args);
    }
    len = Serial.write((const uint8_t*) buffer, len);
    if (buffer != temp) {
        delete[] buffer;
    }
        Serial.println();
        return len;
}
