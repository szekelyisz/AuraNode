/*
 * WebConfig.cpp
 *
 *  Created on: Jan 7, 2018
 *      Author: cc
 */

#include <Bleeper.h>
#include <functional>
#include <ArduinoJson.h>
#include <BleeperCustomInterface.h>
#include <BleeperCustomInterface.js.h>
#include <functional>
#include <SerialSysLog.h>
#include <FS.h>


extern SerialSysLog logger;

BleeperCustomInterface::BleeperCustomInterface(uint16_t port)
: ESP8266WebServer(port) {

}

BleeperCustomInterface::~BleeperCustomInterface() {
    // TODO Auto-generated destructor stub
}

void BleeperCustomInterface::init() {
    addHandler(this);
}

bool BleeperCustomInterface::canHandle(HTTPMethod method, String uri) {
    return ((method == HTTP_GET || method == HTTP_POST) &&
            (uri.equals(mBaseUri)))
        || (method == HTTP_GET && uri.equals(mJSUri));
}

bool BleeperCustomInterface::handle(ESP8266WebServer& server,
        HTTPMethod method, String uri) {
    bool ret = false;
    if(method == HTTP_GET && uri.equals(mJSUri)) {
        send_P(200, "text/javascript", BleeperCustomInterface_js);
        return true;
    }
    if(!mPreRequestHook(method)) return false;
    switch(method) {
    case HTTP_GET:
        ret = handleGet();
        break;
    case HTTP_POST:
        ret = handlePost();
        break;
    default:
        break;
    }
    if(ret) mPostRequestHook(method);
    return ret;
}

bool BleeperCustomInterface::handleGet() {
    auto conn = client();
    DynamicJsonDocument doc(2048);
    std::vector<String> strings =
            Bleeper.configuration.getAsDictionary(false).toStrings();
    for(size_t i = 0; i < strings.size() - 1;) {
        doc[strings[i++]] = strings[i++];
    }

    setContentLength(measureJson(doc));
    sendHeader("Access-Control-Allow-Origin", "*");
    send(200, "text/json", nullptr);

    serializeJson(doc, conn);
    return true;
}

bool BleeperCustomInterface::handlePost() {
    ConfigurationDictionary d;
    for(int i = 0; i != args(); i++) {
        logger.logf("Updating '%s' = '%s'",
                argName(i).c_str(), arg(i).c_str());
        d[argName(i)] = arg(i);
        Bleeper.configuration.setFromDictionary(d);
    }
    return true;
}

 void BleeperCustomInterface::handle() {
     handleClient();
};
