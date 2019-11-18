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

#ifndef SKELETONCLASS_H_
#define SKELETONCLASS_H_

#include "SerialSysLog.h"

#include <Bleeper.h>
#include <BleeperCustomInterface.h>
#include <BleeperFSStorage.h>

class SkeletonClass {
public:
	SkeletonClass();
	virtual ~SkeletonClass();

	static SkeletonClass* shared();

	class WifiConfig : public Configuration {
	public:
		persistentStringVar(ssid, "");
		persistentStringVar(passwd, "");
		stringVar(passwd_new, "");
		stringVar(ssid_list, "");
	};

	class IPConfig : public Configuration {
	public:
		persistentStringVar(hostname, "");
		persistentStringVar(address, "");
		persistentStringVar(netmask, "");
		persistentStringVar(gateway, "");
		persistentStringVar(dns1, "");
		persistentStringVar(dns2, "");
		persistentStringVar(loghost, "");
	};

	class SkeletonConfig : public RootConfiguration {
	public:
		subconfig(WifiConfig, wifi);
		subconfig(IPConfig, ip);
	};

	void init(SkeletonConfig* config, const char* appname);
	void loop();
	const char* getHostname() { return hostname; }

private:
	static SkeletonClass* sharedInstance;

	BleeperCustomInterface bleeperInterface;
	BleeperFSStorage bleeperStorage;

	SkeletonConfig* config;

	bool bleeperPreRequestHook(HTTPMethod m);
	void bleeperPostRequestHook(HTTPMethod m);
	void updateBleeperWifiList();

	char hostname[16];

	void setupLogging(const char* appname);
	void setupNetwork();
	bool ConnectAP();
	void logIPAddress();
	void setupMDNS();
	void setupOTA();

};

#define Skeleton (*SkeletonClass::shared())

extern SerialSysLog logger;

#endif
