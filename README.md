# ArduinoOSC

OSC subscriber / publisher for Arduino


#### NOTE : BREAKING API CHANGES (v0.3.x or later)

- almost all apis has have changed and got much simpler
- dropped support for `OscSerial` (recommend to use [MsgPacketizer](https://github.com/hideakitai/MsgPacketizer) for much smaller packet size)


## Feature

- simple usage
    - flexible callback registration with lambda
    - directly binding osc packet to values
    - osc packet sending in one-line
    - publishing osc packet in one-line
- support basic OSC types based on [oscpkt](http://gruntthepeon.free.fr/oscpkt/html/)
    - TF (`bool`: true, false)
    - i (`int32_t`)
    - h (`int64_t`)
    - f (`float`)
    - d (`double`)
    - s (`string`)
    - b (`bundle`)
- support pattern-matching (wildcards)
- does NOT support timestamp values.


## Usage

Following examples use `OscWiFi`.
To use with `Ethernet`, please change `OscWiFi` to `OscEther`.

### One-Line Subscriber / Publisher

```C++
#include <ArduinoOSC.h>

int i; float f; String s;

void setup() {
    // WiFi stuff
    WiFi.begin(ssid, pwd);
    WiFi.config(ip, gateway, subnet);

    // subscribe osc packet and directly bind to variable
    OscWiFi.subscribe(bind_port, "/bind/values", i, f, s);

    // publish osc packet in 30 times/sec (default)
    OscWiFi.publish(host, publish_port, "/publish/value", i, f, s);
    // function can also be published
    OscWiFi.publish(host, publish_port, "/publish/func", &millis, &micros)
        ->setFrameRate(1); // and publish it once per second
}

void loop() {
    OscWiFi.update(); // should be called to subscribe + publish osc
}
```

### Bind OSC to Lambda Arguments and One-Line Send

``` C++
void setup() {
    // WiFi stuff
    // ...

    OscWiFi.subscribe(bind_port, "/lambda/bind/args",
        [&](int& i, float& f, String& s) {
            Serial.print("/lambda/bind/args ");
            Serial.print(i); Serial.print(" ");
            Serial.print(f); Serial.print(" ");
            Serial.print(s); Serial.println();

            // One-Line Send Back
            OscWiFi.send(host, send_port, "/reply", i, f, s);
        }
    );
}

void loop() {
    OscWiFi.update(); // should be called
}
```

### Other Way to Subscribe

``` C++
// OscMessage as lambda argument
OscWiFi.subscribe(recv_port, "/lambda/msg",
    [](const OscMessage& m) {
        Serial.print(m.remoteIP()); Serial.print(" ");
        Serial.print(m.remotePort()); Serial.print(" ");
        Serial.print(m.size()); Serial.print(" ");
        Serial.print(m.address()); Serial.print(" ");
        Serial.print(m.arg<int>(0)); Serial.print(" ");
        Serial.print(m.arg<float>(1)); Serial.print(" ");
        Serial.print(m.arg<String>(2)); Serial.println();
    }
);

// wildcard address pattern matching
OscWiFi.subscribe(recv_port, "/wildcard/*/test",
    [](const OscMessage& m) {
        Serial.print(m.remoteIP()); Serial.print(" ");
        Serial.print(m.remotePort()); Serial.print(" ");
        Serial.print(m.size()); Serial.print(" ");
        Serial.print(m.address()); Serial.print(" ");
        Serial.print(m.arg<int>(0)); Serial.println();
    }
);

// no arguments
OscWiFi.subscribe(recv_port, "/need/reply", []() {
    OscWiFi.send(host, send_port, "/reply", i, f, s);
});

// pre-defined callback
OscWiFi.subscribe(recv_port, "/callback", onOscReceived);
```


## Supported Platform

This library currently supports following platforms and interfaces.
Please feel free to send PR or request for more board support!

#### WiFi

- ESP32
- ESP8266
- Arduino Uno WiFi Rev2
- Arduino MKR VIDOR 4000
- Arduino MKR WiFi 1010
- Arduino MKR WiFi 1000
- Arduino Nano 33 IoT

#### Ethernet

- ESP32
- ESP8266
- Almost all platforms without WiFi


## Limitation and Options for NO-STL Boards

STL is used to handle packet data by default, but for following boards/architectures, [ArxContainer](https://github.com/hideakitai/ArxContainer) is used to store the packet data because STL can not be used for such boards.
The storage size of such boards for packets, queue of packets, max packet binary size, callbacks are limited.

- AVR
- megaAVR


### Usage Recommendation for Arduino Uno (and other boards with tiny memory size)

For the boards which has tiny memory size (e.g. Arduino Uno), I reccomend not to use publisher and subscriber.
Though you can use them on such boards, such rich functions requires more memory.
The reccomended way is to use `send` and `parse` manually.
The example is shown in `examples/arduino/OscEtherUno`, so please consider to use it.

``` C++
#include <ArduinoOSC.h>

// required to use manual packet parsing
OscEtherServer server(recv_port);
OscEtherClient client;
// OscEtherClient client(local_port);  // set the local port of client manually (default: 9)


void setup() {
    Ethernet.begin(mac, ip);
}

void loop() {
    // manual sending instead of publishers
    static uint32_t prev_func_ms = millis();
    if (millis() > prev_func_ms + 500) {
        client.send(host, publish_port, "/publish/func", millis(), micros());
        prev_func_ms = millis();
    }

    // manual parsing instead of subscribers
    if (server.parse()) {
        const OscMessage* msg = server.message();

        if (msg->address() == "/need/reply") {
            Serial.println("/need/reply");
            int i = millis();
            float f = (float)micros() / 1000.f;
            String s = F("hello");
            client.send(host, send_port, "/reply", i, f, s);
        }
    }
}
```


### Memory Management (only for NO-STL Boards)

As mentioned above, for such boards like Arduino Uno, the storage sizes are limited.
And of course you can manage them by defining following macros.
But these default values are optimized for such boards, please be careful not to excess your boards storage/memory.

``` C++
#define ARDUINOOSC_MAX_MSG_ARGUMENT_SIZE 8
#define ARDUINOOSC_MAX_MSG_BYTE_SIZE 128
#define ARDUINOOSC_MAX_MSG_QUEUE_SIZE 1
#define ARDUINOOSC_MAX_PUBLISH_DESTINATION 4
#define ARDUINOOSC_MAX_SUBSCRIBE_ADDRESS_PER_PORT 4
#define ARDUINOOSC_MAX_SUBSCRIBE_PORTS 2
```

### Enable Bundle for NO-STL Boards

OSC bundle option is disabled for such boards.
If you want to use that, please use this macro and handle packets manually.
`ArduinoOSC` does not use bundle by default.

``` C++
#define ARDUINOOSC_ENABLE_BUNDLE
#define ARDUINOOSC_MAX_MSG_BUNDLE_SIZE 128
```


### Enable Debug Logger

You can see the debug log when you insert following line before include `ArduinoOSC`.

``` C++
#define ARDUINOOSC_DEBUGLOG_ENABLE
#include <ArduinoOSC.h>
```


## Embedded Libraries

- [ArxTypeTraits v0.2.1](https://github.com/hideakitai/ArxTypeTraits)
- [ArxContainer v0.3.14](https://github.com/hideakitai/ArxContainer)
- [ArxSmartPtr v0.2.1](https://github.com/hideakitai/ArxSmartPtr)
- [DebugLog v0.4.0](https://github.com/hideakitai/DebugLog)
- [TeensyDirtySTLErrorSolution v0.1.0](https://github.com/hideakitai/TeensyDirtySTLErrorSolution)


## Special Thanks

- [ofxPubSubOsc](https://github.com/2bbb/ofxPubSubOsc)


## License

MIT
