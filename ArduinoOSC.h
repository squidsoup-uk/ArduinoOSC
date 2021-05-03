#ifndef ARDUINOOSC_H
#define ARDUINOOSC_H

#include "ArduinoOSC/util/ArxTypeTraits/ArxTypeTraits.h"
#include "ArduinoOSC/util/ArxSmartPtr/ArxSmartPtr.h"
#include "ArduinoOSC/util/ArxContainer/ArxContainer.h"
#include "ArduinoOSC/util/DebugLog/DebugLog.h"

#ifdef ARDUINOOSC_DEBUGLOG_ENABLE
#include "ArduinoOSC/util/DebugLog/DebugLogEnable.h"
#else
#include "ArduinoOSC/util/DebugLog/DebugLogDisable.h"
#endif

#if ARX_HAVE_LIBSTDCPLUSPLUS >= 201103L  // Have libstdc++11
// all features are available
#else
#ifndef ARDUINOOSC_ENABLE_BUNDLE
#define ARDUINOOSC_DISABLE_BUNDLE
#endif
#endif

#if defined(ESP_PLATFORM) || defined(ESP8266) || defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_MKRVIDOR4000) || defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_SAMD_NANO_33_IOT)
// #define ARDUINOOSC_ENABLE_WIFI
#endif

#if defined(ESP8266) || defined(ESP_PLATFORM) || !defined(ARDUINOOSC_ENABLE_WIFI)
// #define ARDUINOOSC_ENABLE_ETHER
#endif

#if !defined(ARDUINOOSC_ENABLE_WIFI) && !defined(ARDUINOOSC_ENABLE_ETHER)
// #error THIS PLATFORM HAS NO WIFI OR ETHERNET OR NOT SUPPORTED ARCHITECTURE. PLEASE LET ME KNOW!
#endif

#ifdef ARDUINOOSC_ENABLE_WIFI
#ifdef ESP_PLATFORM
#include <WiFi.h>
#include <WiFiUdp.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#elif defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_MKRVIDOR4000) || defined(ARDUINO_SAMD_NANO_33_IOT)
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#elif defined(ARDUINO_SAMD_MKR1000)
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#endif
#endif  // ARDUINOOSC_ENABLE_WIFI

#ifdef ARDUINOOSC_ENABLE_ETHER
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "ArduinoOSC/util/TeensyDirtySTLErrorSolution/TeensyDirtySTLErrorSolution.h"
#endif  // ARDUINOOSC_ENABLE_ETHER

#include "ArduinoOSC/OscUdpMap.h"
#include "ArduinoOSC/OSCServer.h"
#include "ArduinoOSC/OSCClient.h"


namespace arduino {
namespace osc {

    template <typename S>
    class Manager {
        Manager() {}
        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;

    public:
        // server

        static Manager<S>& getInstance() {
            static Manager<S> m;
            return m;
        }

        const OscUdpMap<S>& getUdpMap() const {
            return OscUdpMapManager<S>::getInstance().getUdpMap();
        }

        const OscServerMap<S>& getServerMap() const {
            return OscServerManager<S>::getInstance().getServerMap();
        }

        OscServer<S>& getServer(const uint16_t port) {
            return OscServerManager<S>::getInstance().getServer(port);
        }

        template <typename... Ts>
        void subscribe(const uint16_t port, const String& addr, Ts&&... ts) {
#if defined(ARDUINOOSC_ENABLE_WIFI) && defined(ESP_PLATFORM)
            if (WiFi.getMode() != WIFI_OFF)
                OscServerManager<S>::getInstance().getServer(port).subscribe(addr, std::forward<Ts>(ts)...);
            else
                LOG_ERROR(F("WiFi is not enabled. Subscribing OSC failed."));
#else
            OscServerManager<S>::getInstance().getServer(port).subscribe(addr, std::forward<Ts>(ts)...);
#endif
        }

        void parse() {
#if defined(ARDUINOOSC_ENABLE_WIFI) && defined(ESP_PLATFORM)
            if (WiFi.status() == WL_CONNECTED) {
                OscServerManager<S>::getInstance().parse();
            } else {
                LOG_ERROR(F("WiFi is not connected. Please connected to WiFi"));
            }
#else
            OscServerManager<S>::getInstance().parse();
#endif
        }

        // client

        OscClient<S>& getClient() {
            return OscClientManager<S>::getInstance().getClient();
        }

        template <typename... Ts>
        void send(const String& ip, const uint16_t port, const String& addr, Ts&&... ts) {
#if defined(ARDUINOOSC_ENABLE_WIFI) && defined(ESP_PLATFORM)
            if (WiFi.status() == WL_CONNECTED) {
                OscClientManager<S>::getInstance().send(ip, port, addr, std::forward<Ts>(ts)...);
            } else {
                LOG_ERROR(F("WiFi is not connected. Please connected to WiFi"));
            }
#else
            OscClientManager<S>::getInstance().send(ip, port, addr, std::forward<Ts>(ts)...);
#endif
        }

        void post() {
#if defined(ARDUINOOSC_ENABLE_WIFI) && defined(ESP_PLATFORM)
            if (WiFi.status() == WL_CONNECTED) {
                OscClientManager<S>::getInstance().post();
            } else {
                LOG_ERROR(F("WiFi is not connected. Please connected to WiFi"));
            }
#else
            OscClientManager<S>::getInstance().post();
#endif
        }

        template <typename... Ts>
        OscPublishElementRef publish(const String& ip, const uint16_t port, const String& addr, Ts&&... ts) {
#if defined(ARDUINOOSC_ENABLE_WIFI) && defined(ESP_PLATFORM)
            if (WiFi.getMode() != WIFI_OFF)
                return OscClientManager<S>::getInstance().publish(ip, port, addr, std::forward<Ts>(ts)...);
            else {
                LOG_ERROR(F("WiFi is not enabled. Publishing OSC failed."));
                return nullptr;
            }
#else
            return OscClientManager<S>::getInstance().publish(ip, port, addr, std::forward<Ts>(ts)...);
#endif
        }

        OscPublishElementRef getPublishElementRef(const String& ip, const uint16_t port, const String& addr) {
            return OscClientManager<S>::getInstance().getPublishElementRef(ip, port, addr);
        }

        // update both server and client

        void update() {
            parse();
            post();
        }
    };

}  // namespace osc
}  // namespace arduino

namespace ArduinoOSC = arduino::osc;

#ifdef ARDUINOOSC_ENABLE_WIFI
using OscWiFiManager = ArduinoOSC::Manager<WiFiUDP>;
#define OscWiFi OscWiFiManager::getInstance()
using OscWiFiServer = OscServer<WiFiUDP>;
using OscWiFiClient = OscClient<WiFiUDP>;
#endif  // ARDUINOOSC_ENABLE_WIFI

#ifdef ARDUINOOSC_ENABLE_ETHER
using OscEtherManager = ArduinoOSC::Manager<EthernetUDP>;
#define OscEther OscEtherManager::getInstance()
using OscEtherServer = OscServer<EthernetUDP>;
using OscEtherClient = OscClient<EthernetUDP>;
#endif  // ARDUINOOSC_ENABLE_ETHER

#include "ArduinoOSC/util/DebugLog/DebugLogRestoreState.h"

#endif  // ARDUINOOSC_H
