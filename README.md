# AuraNode

[![Build Status](https://travis-ci.com/szekelyisz/AuraNode.svg?branch=master)](https://travis-ci.com/szekelyisz/AuraNode)

AuraNode is a firmware for the ESP8266 MCU that enables reading values from sensors and controlling actuators through WiFi. Each instance (node) has a configuration file that contains information about attached sensors and actuators (collectively called *interfaces*). All network communication uses the OSC protocol which enables easy integration with a wide range of software, enabling them to access the sensors and actuators thus interact with the physical world.

Application areas include artistic multimedia installations, home automation, industrial monitoring and control, among other fields.

Other features:

* Completely dynamic configuration: all WiFi credentials, network settings, and interface configuration are stored in EEPROM, separately from firmware code. This eliminates the need to recompile the code for every node individually.

* Web-based configuration: a built-in web server is used to configure the node, including WiFi and network settings.

* AP mode: a node that has no WiFi credentials set or fails to join the network will enter AP mode allowing initial configuration or fixes.

* OTA (Over-The-Air) firmware upgrades: new firmware can be uploaded over WiFi without need for a USB or serial connection to nodes.

* mDNS/DNS-SD support: nodes are discoverable on the network via multicast DNS and are also capable of finding and downloading their own configuration.

* Network logging: besides serial logging, diagnostic and error messages can be also sent out over the network with standard syslog protocol.

* Add-on module support: in addition to the capabilities of bare ESP8266 boards, AuraNode supports a number of external peripheral controllers via I2C. This enables building far more complex and capable nodes with a virtually unlimited number of interfaces.

* Multi-level sensor and actuator abstraction: nodes present sensors and actuators to the network as high-level abstract entities. An RGB LED, for example, expects the color data as a single standard 32-bit RGBW color value instead of 3 or 4 individual 8-bit values. I/O lines are also abstracted away from the module that provides the functionality; an RGB LED that requires 3 channels of PWM signals can use outputs on different types of modules.

* Compile-time customizability: specific module and interface handling code can be disabled to save code space.

## Supported hardware

### Devices

All ESP8266 and ESP8285-based board should be supported. Tested on Wemos D1 Mini Pro and Lite, ESP-01, and NodeMCU.

### Sensors and actuators

#### Currently supported

* Digital input and output (for Passive Infrared (PIR) sensor, photogate, tilt sensor, relay, etc)
* Analog input and output (for knobs, sliders, thermistors, photoresistors, etc)
* PWM output (digital servo motor and RGB/RGBW LED available as an abstraction)
* Infrared remote reader
* addressable LED strip
* DS18X20-based temperature sensor
* LM75-like temperature sensor

#### Support planned

* Analog output (for analog servo motors)
* Stepper motors (unipolar, bipolar, direct attach and via driver IC)
* Distance sensors (ultrasonic and laser)
* RGB sensor
* Acceleration and orientation sensors
* Environmental sensors: temperature, humidity, flame, etc.

### Addon modules

 * ADS1115 analog to digital converter
 * PCA9685 PWM controller
 * PCF8574 GPIO expander

## Usage

## Quick start

Download the source code, [compile and flash with PlatformIO](https://docs.platformio.org/en/latest/quickstart.html#process-project). Since the node has no WiFi credentials configured at the first startup, it will enter AP mode. The default SSID is OSC_XXXXXX where XXXXXX is the chip's unique ID. Connect to this SSID with the default password `AuraNode`. Point your browser to `http://192.168.4.1/`. You should see the configuration interface that allows the setting of Wi-Fi credentials, network, and interfaces. Make sure you set the node name on the Network tab so you can find your node after the WiFi connection is established. Once everything is configured, click *Apply and reboot*.

### Configuration

Modules and interfaces attached to a given node are described using JSON. It can be given in a textual format on the web interface, uploaded as a file, or downloaded from a preconfigured or automatically discovered HTTP server elsewhere on the network when the node starts up.

The top-level object described by the JSON file can have can have the following members. Ones in bold are mandatory, others are optional:

Member | Type | Description
------ | ---- | -----------
`loghost` | String | Syslog server address in `<host>:<port>` format. Port is optional, and defaults to the standard syslog port 514.
**`destination`** | String | Default OSC target to send sensor values to in `<host>:<port>` format. Can be overridden on the sensor level.
`modules` | Object | See below
**`interfaces`** | Object | See below

The `modules` object's attribute names are arbitrary and unique strings used to reference each module in later definitions. Each attribute is an object with members specific to the type of the module. For now all modules use the same attributes, detailed below:

Member | Type | Description
------ | ---- | -----------
**`type`** | String | Type of the module. See below for valid values.
**`address`** | String | I2C address of the module in hexadecimal notation ("0xXX")

Modules might have type-specific attributes in the future.

The module named `base` is defined by default and the name is reserved. This module represents the ESP8266 chip itself, and its I/O pins.

The `interfaces` object's attributes are interface definitions. Keys are strings representing OSC addresses. In case of sensors, this is used as the OSC address when readings are sent out to the destination. With actuators, this is the OSC address the node expects data sent to for the given actuator. It must begin with a `/` character otherwise the definition is ignored. Each of them should have a mandatory string member named `type` that determines the type of the interface and also further options, most commonly the definition of the pin(s) the interface is physically connected to. See the [example configuration file](AuraNode.example.json) in the source tree for examples of defining interfaces of different types.

## Internals

### Operation

When a node boots, it tries to connect to the WiFi network. If this fails or it is not configured, it enters AP mode as outlined above. As the network is not available yet, only serial logging is available during this process. Network settings (IP address/netmask/gateway/DNS) are applied as needed in case they are configured -- otherwise it defaults to DHCP. After the connection is established, the node tries to find the configuration file in different locations. Some steps try multiple filenames:

 1. `<hostname>.json`
 1. `AuraNode.<hostname>.json`
 1. `AuraNode.<chip_id>.json`
 1. `AuraNode.json`

The locations of these files are probed as follows. The first existing and valid file wins.

 1. If *URL* on the *Interfaces* page is not empty, it is probed. If it doesn't end in `/`, then it is downloaded directly, otherwise the series of filenames above are appended to it and probed.
 1. The network is queried to resolve the mDNS service named `_AuraNode-config._tcp.local`. If any responses are received, the first one is used as a configuration server. The root directory of the service as an HTTP server is probed with filenames above.
 1. If no configuration can be retreived from the network, local sources are probed. The root directory of the built-in SPIFFS filesystem is searched for the above filenames.
 1. Last, the contents of the *JSON* field on the `Interfaces` page is probed as a JSON document.

If no configuration is found, no further actions are performed. Otherwise the modules and interfaces are initialized according to the configuration file, and a send/receive loop starts that checks each sensor in a round-robin fashion and transmits the readings to the OSC host and address as per configuration. At the same time, if a valid OSC message is received from the network, it is forwarded to the interface determined by the target OSC address of the message if such an actuator is configured.

### Interface model

Internally AuraNode has three concepts that help abstracting interfaces from electrical signals:

 * [***Interface***](src/Model/Interface)s provide the highest level of abstraction. Ideally each type of (ie. functionally identical) sensor and actuator should have its own class. For example, a RGB LED appears as one entity although it is composed of three electrical signals. This means that from the network perspective, the LED can be sent a single color value encoded in a 32-bit integer (8 bits per channel) instead of three analog values. Thus interfaces define the message format they accept. In case of actuators, they also dissect the incomping OSC message to extract and transform contained information into values understood by IOs. In the RGB LED case, this means extracting the values of the three color components from the message, scalig them, and delivering to the recpective PWM outputs. On the other hand, sensors compose OSC messages based on the readings from inputs according to their own definition. Interfaces can also be thought of as composite devices that aggregate individual *IO*s to form a logical device.

 * [***IO***](src/Model/IO)s represent a single electrical signal. They also have a well-defined data format, for example, analog values are represented as 32-bit values normalized to 16 bits -- this provides a tradeoff between decent resolution and reduced computing power compared to floating point representation. The job of the IO is to communicate with the respective *Module* to output the appropriate signal or read a value. IOs are specific to both the provided functionality and the module they provide the functinality through. For example, PWM output on the base module and through a specific type of I2C IC are two different classes. They share the same interface but they talk to different types of modules. IOs can also be viewed as an abstraction and decomposition layer for addon boards.

 * [***Module***](src/Model/Module)s handle the low-level communication to the addon (I2C) boards. They are specific to the IC model as they implement the steps necessary to make the IC behave accordingly, such as output the requested signal on the given pin(s), read an input, process commands or provide data.

## Message format

Different types of interfaces accept/provide data in specific formats.

Type | Data format (OSC type tag)
---- | --------------------------
`analog_in` | `i`: reading value as a 16-bit integer encoded on 32 bits
`digital_in`, `digital_out` | `T`/`F`: boolean, true for high, false for low -- note that the type specifies the value; no bytes are allocated in the message data
`pwm` | `i`: 16-bit integer value encoded as a 32-bit integer *or*<br>`f`: 32-bit float value normalized to [0,1] interval
`ledstrip` | `b`: blob of raw pixel data as expected by the attached LED strip
`rgba` | `i`: 32 bits of RGBA color encoded as 0xRRGGBBAA *or*<br>`[i/f][i/f][i/f][i/f]`: each component encoded separately as 16-bit integers encoded as 32-bit integers or 32-bit float values normalized to [0,1] interval
`servo` | `i`: 16-bit integer value encoded as a 32-bit integer *or*<br>`f`: 32-bit float value normalized to [0,1] interval
`ir` | `i`: 32 bits of the code encoded as a 32-bit integer value

## Security

Be avdised that OSC provides no secutity features at all. This means there's no way to check that an incoming message originated from an authentic/trusted source, so any host on the same network can control actuators. The entire security of a network of nodes depends on security provided by your WiFi network. Therefore it is strongly recommended to use a dedicated, password-protected WiFi network with strong authentication such as WPA or WPA2.

## Contributing

Pull requests are more than welcome. If you find a bug or miss a feature, feel free to open an issue. In case you just want to make this project better, pick something to work on form the TODO list below or the [interfaces waiting to be implemented](#support-planned) above.

## TODO

 * Document module and interface configuration options. For now, please refer to the [example configuration file](AuraNode.example.json)
 * Add option to disable serial logging after startup or completely (in case there's an interface attached to the TX pin)
 * Split Skeleton library into a separate repository
 * Decouple configuration parsing from the main logic -- allow module and interface code to do the job
 * Port everything to an RTOS

## License

AuraNode is distributed unter the conditions of the GNU General Public Lisense version 3 (GPLv3). Check [the license file](LICENSE.txt) for details.
