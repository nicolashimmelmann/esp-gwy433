
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include <Ticker.h>

#include "GWY433.h"
#include "ESP_RX433.h"
#include "WiFiCredentials.h"

namespace std
{
    void __throw_bad_cast(void) {}
    void __throw_ios_failure(char const *) {}
    void __throw_runtime_error(char const *) {}
}

#define DEBUG

const char *_hostname = "HomeGateway";

// Wifi event handlers and methods
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
const float _reconnect_time = 10; // seconds
Ticker wifiReconnectTimer;
void wifi_connect(void);
void onWifiConnected(const WiFiEventStationModeGotIP &event);
void onWifiDisconnected(const WiFiEventStationModeDisconnected &event);
bool _wifi_connected;

const unsigned int udpPort = 49880;
GWY433 gwy(5); // Arduino Pin 5 = Wemos D1 pin 20 (D1)

// Function pre-definitions
void initOnConnect();

void wifi_connect(void)
{
#ifdef DEBUG
    Serial.println("Connecting to Wi-Fi...");
#endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(_WIFI_SSID, _WIFI_PSK);

    // Start timer to retry if nothing happens in the configured time
    wifiReconnectTimer.once(_reconnect_time, wifi_connect);
}
void onWifiConnected(const WiFiEventStationModeGotIP &event)
{
#ifdef DEBUG
    Serial.println("Connected to Wi-Fi.");
    Serial.println(WiFi.localIP());
#endif

    _wifi_connected = true;

    // Cancel running wifiReconnectTimer
    wifiReconnectTimer.detach();

    initOnConnect();
}
void onWifiDisconnected(const WiFiEventStationModeDisconnected &event)
{
#ifdef DEBUG
    Serial.println("Wi-Fi disconnected. Trying to reconnect!");
    Serial.println(WiFi.localIP());
#endif

    _wifi_connected = false;
    wifi_connect();
}

void setup()
{
    Serial.begin(115200);
    // Start WiFi Connection
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnected);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnected);
    wifi_connect();

    // Set receiver data pin to low (Wemos Pin D2 (19) = Arduino 4)
    pinMode(4, INPUT);
    digitalWrite(4, LOW);
}

void initOnConnect()
{
    // Start OTA server
    ArduinoOTA.setHostname(_hostname);
    ArduinoOTA.onError([](ota_error_t error)
                       {
        (void)error;
        ESP.restart(); });
    ArduinoOTA.begin();
#ifdef DEBUG
    Serial.println("[INIT] Started OTA Service.");
#endif

    // Start MDNS
    MDNS.begin(_hostname);
    MDNS.addService("ota", "tcp", 8266);
#ifdef DEBUG
    Serial.println("[INIT] Started MDNS.");
#endif

    // Start GWY433 gateway
    gwy.begin(udpPort);
#ifdef DEBUG
    Serial.println("[INIT] Started 433MHz Brennenstuhl Gateway.");
#endif
}

void loop()
{
    if (!_wifi_connected)
    {
        return;
    }
    gwy.listen();
    ArduinoOTA.handle();
}
