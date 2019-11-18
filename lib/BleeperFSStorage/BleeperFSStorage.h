/*
 * BleeperFSStorage.h
 *
 *  Created on: Feb 4, 2018
 *      Author: cc
 */

#ifndef BLEEPERFSSTORAGE_H_
#define BLEEPERFSSTORAGE_H_

#include <Storage/Storage.h>
#include <WString.h>

class BleeperFSStorage: public Storage {
public:
	BleeperFSStorage();
	BleeperFSStorage(const String& dir);
	virtual ~BleeperFSStorage();

	void init() { };

	void persist();
	void load();

private:
	String mDir;
};

#endif /* BLEEPERFSSTORAGE_H_ */
