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

#ifndef SRC_LOG_H_
#define SRC_LOG_H_

#include <Syslog.h>
#include <WiFiUdp.h>

class SerialSysLog {
public:
	SerialSysLog();
	virtual ~SerialSysLog();

	typedef enum {
		EMERGENCY = LOG_EMERG, /* system is unusable */
		ALERT = LOG_ALERT, /* action must be taken immediately */
		CRIT = LOG_CRIT, /* critical conditions */
		ERR = LOG_ERR, /* error conditions */
		WARN = LOG_WARNING, /* warning conditions */
		NOTICE = LOG_NOTICE, /* normal but significant condition */
		INFO = LOG_INFO, /* informational */
		DEBUG = LOG_DEBUG /* debug-level messages */
	} Severity;

	void serialOn(uint32_t baud);
	void serialOff();
	void syslogOn(const char *server, uint16_t port, const char *host,
			const char *appname);
	void syslogOff();

	void logf(Severity prio, const char* format, ...)
		__attribute__((format(printf, 3, 4)));
	void logf(const char* format, ...)
		__attribute__((format(printf, 2, 3)));
	void log(Severity prio, const char *message);
	void log(const char *message);

	void flush();

private:
	bool mSerialEnabled;
	bool mSyslogEnabled;
	uint16_t mDefaultPriority;
	WiFiUDP mSyslogSocket;
	Syslog mSyslog;

	int serial_vsnprintf(const char *format, va_list args);
};

#endif /* SRC_LOG_H_ */
