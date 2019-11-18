/*
 * BleeperFSStorage.cpp
 *
 *  Created on: Feb 4, 2018
 *      Author: cc
 */

#include <BleeperFSStorage.h>
#include <FS.h>
#include <Bleeper.h>
#include <SerialSysLog.h>

extern SerialSysLog logger;

BleeperFSStorage::BleeperFSStorage(const String& dir)
: mDir(dir) {
	SPIFFS.begin();
}

BleeperFSStorage::BleeperFSStorage()
: mDir("/bleeper")
{
	SPIFFS.begin();
}

BleeperFSStorage::~BleeperFSStorage() {
}

void BleeperFSStorage::persist() {
	for(const auto& entry : Bleeper.configuration.getAsDictionary(true)) {
		String filename = entry.first;
		logger.logf("Saving '%s' = '%s'",
				entry.first.c_str(), entry.second.c_str());
		filename.replace('.', '/');
		File f = SPIFFS.open(mDir + "/" + filename, "w");
		if(f) {
			f.print(entry.second);
			f.flush();
			f.close();
		} else {
			logger.log(SerialSysLog::ERR, "FS error");
		}
	}
}

void BleeperFSStorage::load() {
	auto dict = Bleeper.configuration.getAsDictionary(true);
	for(auto& entry : dict) {
		String filename = entry.first;
		filename.replace('.', '/');
		File f = SPIFFS.open(mDir + "/" + filename, "r");
		if(f) {
			entry.second = f.readString();
			logger.logf("Loading '%s' = '%s'",
					entry.first.c_str(), entry.second.c_str());
			f.close();
		} else {
			logger.logf(SerialSysLog::WARN, "Loading '%s': file not found",
					entry.first.c_str());
		}
	}
	Bleeper.configuration.setFromDictionary(dict);
}
