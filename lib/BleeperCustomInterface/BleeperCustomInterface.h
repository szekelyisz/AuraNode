/*
 * WebConfig.h
 *
 *  Created on: Jan 7, 2018
 *      Author: cc
 *
 *  IDEA: HTML page has input fields tagged with specific class "bleeper"
 *  with names set to Bleeper variables. A JS routine finds all tagged
 *  elements and sets their value/innerHtml (depending on type) to values
 *  queried from the server one by one (or all at once?). The form is
 *  submitted as usual with all values, stored on flash and the device is
 *  rebooted.
 */

#ifndef SRC_BLEEPERCUSTOMINTERFACE_H_
#define SRC_BLEEPERCUSTOMINTERFACE_H_

#include <ConfigurationInterface/WebServer/BleeperWebServer.h>
#include <ESP8266WebServer.h>
#include <functional>

class BleeperCustomInterface :
		public ConfigurationInterface,
		public ESP8266WebServer,
		public RequestHandler {

private:
	bool handleGet();
	bool handlePost();

	std::function<bool(HTTPMethod)> mPreRequestHook;
	std::function<void(HTTPMethod)> mPostRequestHook;

	static constexpr const char* mBaseUri = "/bleeper";
	static constexpr const char* mJSUri = "/BleeperCustomInterface.js";

public:
	bool canHandle(HTTPMethod method, String uri) override;
	bool handle(ESP8266WebServer& server, HTTPMethod method, String uri) override;

	BleeperCustomInterface(uint16_t port = 80);
	virtual ~BleeperCustomInterface();

	void init();
	void setPreRequestHook(std::function<bool(HTTPMethod)> f) {
		mPreRequestHook = f;
	}
	void setPostRequestHook(std::function<void(HTTPMethod)> f) {
		mPostRequestHook = f;
	}
	void handle();
};

#endif /* SRC_BLEEPERCUSTOMINTERFACE_H_ */
