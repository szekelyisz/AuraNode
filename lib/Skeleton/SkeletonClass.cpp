/*
	Skeleton - ESP8266 firmware skeleton
    Copyright (C) 2019  Szabolcs Szekelyi

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

#include "SkeletonClass.h"

SkeletonClass* SkeletonClass::sharedInstance = nullptr;

#include "FS.h"
#include "ArduinoOTA.h"
#include "ArduinoJson.h"
#include <ESP8266mDNS.h>

#ifndef SKELETON_WIFI_SOFTAP_PASSWD
#define SKELETON_WIFI_SOFTAP_PASSWD "theresnospoon"
#endif

#ifndef SKELETON_WIFI_CONNECT_RETRIES
#define SKELETON_WIFI_CONNECT_RETRIES 10
#endif

SkeletonClass::SkeletonClass() {
	// TODO Auto-generated constructor stub

}

SkeletonClass::~SkeletonClass() {
	// TODO Auto-generated destructor stub
}

SkeletonClass* SkeletonClass::shared() {
  return sharedInstance
		  ? sharedInstance
		  : sharedInstance = new SkeletonClass();
}

void SkeletonClass::init(SkeletonConfig* c, const char* appname) {
	logger.serialOn(115200);
	logger.log("Skeleton starting");

	// initialize filesystem
	logger.log("Initializing filesystem");
	if(!SPIFFS.begin()) logger.logf(SerialSysLog::Severity::CRIT,
			"Unable to mount filesystem");

	// setup config web interface
    bleeperInterface.serveStatic("/", SPIFFS,
    		"/webconfig/index.html");
    bleeperInterface.serveStatic("/style.css", SPIFFS,
    		"/webconfig/style.css");
    bleeperInterface.serveStatic("/index.js", SPIFFS,
    		"/webconfig/index.js");
    bleeperInterface.setPreRequestHook([this](HTTPMethod m) {
    	return this->bleeperPreRequestHook(m);
    });
	bleeperInterface.setPostRequestHook([this](HTTPMethod m) {
		return this->bleeperPostRequestHook(m);
	});
    // initialize Bleeper
    config = c;
    Bleeper.configuration.set(config);
    Bleeper.configurationInterface.add(&bleeperInterface);
    Bleeper.storage.set(&bleeperStorage);
    Bleeper.init();

    // initialize hostname
    if(config->ip.hostname.length())
    		snprintf(hostname, 16, "%s",
    				config->ip.hostname.c_str());
    else
    		snprintf(hostname, 16, "ESP_%06X", ESP.getChipId());

	setupNetwork();

	setupLogging(appname);

	setupMDNS();

    setupOTA();

    bleeperInterface.begin();

    logger.log("Skeleton initialized");
}

void SkeletonClass::loop() {
	ArduinoOTA.handle();
	Bleeper.handle();
	bleeperInterface.handle();
}

void SkeletonClass::setupNetwork() {
	logger.log("Setting up network");
	WiFi.hostname(hostname);
	if(config->wifi.ssid.length()) {
		if(ConnectAP()) {
			logIPAddress();
			return;
		} else {
			logger.log("Connection error");
		}
	}
	logger.log("Entering AP mode");
	WiFi.mode(WIFI_AP);
	WiFi.softAP(hostname, SKELETON_WIFI_SOFTAP_PASSWD);
}

void SkeletonClass::logIPAddress() {
	logger.logf("IP address: %s", WiFi.localIP().toString().c_str());
}

void SkeletonClass::updateBleeperWifiList() {
	uint8_t nets = WiFi.scanNetworks(false, true);

	DynamicJsonDocument doc(JSON_ARRAY_SIZE(nets) +
			nets * JSON_OBJECT_SIZE(2) + 32 * nets * 2);
	doc[""] = ""; // a null entry to disable wifi if desired
	for(uint8_t c = 0; c != nets; c++) {
		if(WiFi.SSID(c).length())
			doc[WiFi.SSID(c)] = WiFi.SSID(c);
	}
	serializeJson(doc, config->wifi.ssid_list);
}

void SkeletonClass::setupMDNS() {
	if (MDNS.begin(hostname)) {
		logger.log(SerialSysLog::INFO, "mDNS responder started");
	} else {
		logger.log(SerialSysLog::ERR, "mDNS responder startup error");
		return;
	}
}

bool SkeletonClass::ConnectAP() {
	// Connect to WiFi network
	uint8_t wifi_connect_retries = 0;

	if(config->ip.address.length()) {
		// static IP configuration
		IPAddress address, gateway, netmask, dns1, dns2;
		logger.logf("Setting address %s/%s gateway %s dns1 %s dns2 %s",
				config->ip.address.c_str(),
				config->ip.netmask.c_str(),
				config->ip.gateway.c_str(),
				config->ip.dns1.c_str(),
				config->ip.dns2.c_str());
		if(!address.fromString(config->ip.address)) {
			logger.log(SerialSysLog::ERR, "Invalid IP address");
			return false;
		}
		if(!netmask.fromString(config->ip.netmask)) {
			// try as prefix
			auto prefix_length = config->ip.netmask.toInt();
			if(prefix_length > 32 || prefix_length < 0) {
				logger.log(SerialSysLog::ERR, "Invalid netmask");
				return false;
			}
			netmask = (uint32_t)(~0 << (32 - prefix_length));
		}
		if(config->ip.gateway.length() &&
				!gateway.fromString(config->ip.gateway)) {
			logger.log(SerialSysLog::ERR, "Invalid gateway");
			return false;
		}
		if(config->ip.dns1.length() &&
				!dns1.fromString(config->ip.dns1)) {
			logger.logf(SerialSysLog::ERR, "Invalid dns1");
			return false;
		}
		if(config->ip.dns2.length() &&
				!dns2.fromString(config->ip.dns2)) {
			logger.log(SerialSysLog::ERR, "Invalid dns2");
			return false;
		}
		WiFi.config(address, gateway, netmask, dns1, dns2);
	}

	logger.logf("Connecting to %s", config->wifi.ssid.c_str());
	WiFi.mode(WIFI_STA);
	WiFi.setAutoReconnect(true);
	WiFi.begin(config->wifi.ssid.c_str(), config->wifi.passwd.c_str());


	do {
		wifi_connect_retries++;
		WiFi.begin(config->wifi.ssid.c_str(), config->wifi.passwd.c_str());
		if(WiFi.waitForConnectResult() == WL_CONNECTED) break;
		logger.log(SerialSysLog::INFO, "retrying");
		WiFi.disconnect(false);
		delay(2000);
	} while(wifi_connect_retries < SKELETON_WIFI_CONNECT_RETRIES);

	if(WiFi.status() != WL_CONNECTED) {
		logger.logf(SerialSysLog::WARN, "Connection failed %d times",
				wifi_connect_retries);
		return false;
	} else {
		logger.log("WiFi connected");
		return true;
	}
}

void SkeletonClass::setupOTA() {
    ArduinoOTA.onStart([this]() {
      logger.log(SerialSysLog::INFO, "OTA upload start");
    });
    ArduinoOTA.onEnd([this]() {
      logger.log(SerialSysLog::INFO, "OTA upload successful");
    });
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
      logger.logf(SerialSysLog::INFO, "OTA progress: %d/%d", progress, total);
    });
    ArduinoOTA.onError([this](ota_error_t error) {
      switch(error) {
      case OTA_AUTH_ERROR:
	  	  logger.log(SerialSysLog::ERR, "OTA auth failed");
	  	  break;
      case OTA_BEGIN_ERROR:
		  logger.log(SerialSysLog::ERR, "OTA begin failed");
		  break;
      case OTA_CONNECT_ERROR:
		  logger.log(SerialSysLog::ERR, "OTA connect failed");
		  break;
      case OTA_RECEIVE_ERROR:
    	  	  logger.log(SerialSysLog::ERR, "OTA receive failed");
    	  	  break;
      case OTA_END_ERROR:
		  logger.log(SerialSysLog::ERR, "OTA upload failed");
		  break;
      default:
    	  	  logger.log(SerialSysLog::ERR, "OTA unknown error");
    	  	  break;
      }
    });
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.begin();
    logger.log(SerialSysLog::INFO, "OTA handler started");
}

void SkeletonClass::setupLogging(const char* appname) {
	if (config->ip.loghost.length()) {
		logger.log("Switching to syslog");
		logger.syslogOn(config->ip.loghost.c_str(), 514, hostname, appname);
//		logger.serialOff();
	}
}

bool SkeletonClass::bleeperPreRequestHook(HTTPMethod m) {
	switch(m) {
	case HTTP_GET:
		// update networks list
		updateBleeperWifiList();
		return true;
		break;
	case HTTP_POST:
		// TODO validate configuration?
		return true;
		break;
	default:
		break;
	}
	return true;
}

void SkeletonClass::bleeperPostRequestHook(HTTPMethod m) {
	switch(m) {
	case HTTP_GET:
		// free up some memory by throwing away wifi ssid list
		config->wifi.ssid_list = "";
		break;
	case HTTP_POST:
		if(bleeperInterface.arg("wifi.passwd_new").length()) {
			config->wifi.passwd = config->wifi.passwd_new;
			config->wifi.passwd_new = "";
		}
		Bleeper.storage.persist();
		logger.log("Config saved; rebooting");
		ESP.restart();
		break;
	default:
		break;
	}
}
