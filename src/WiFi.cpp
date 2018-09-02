#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "WiFi.h"
#include "Config.h"

#define NUM_WIFI_CONNECT_RETRIES        300


bool wifiConnect()
{
    uint8_t retries;

    // Check if already connected
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    Serial.print(F("Connecting to "));
    Serial.print(WLAN_SSID);
    Serial.print(F("..."));

    // Power up
    //WiFi.forceSleepWake();
    //delay(100);

    // Set WiFi powerOnReset has no effect?
    //WiFi.setOutputPower(0.0);

    // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi
    // settings in the flash memory.
    //WiFi.persistent(false);

    // Set mode station
    WiFi.mode(WIFI_STA);

    // Connect
    WiFi.begin(WLAN_SSID, WLAN_PASS);

    // Wait for connection
    for (retries = 0; retries < NUM_WIFI_CONNECT_RETRIES; retries++) {
        uint8_t wifiStatus = WiFi.status();

        if (wifiStatus == WL_CONNECTED) {
            break;
        } else if (wifiStatus == WL_NO_SSID_AVAIL) {
            return false;
        }

        delay(10);
        //Serial.print(".");
        //Serial.println(WiFi.status());
        //WiFi.printDiag(Serial);
    }

    if (retries >= NUM_WIFI_CONNECT_RETRIES) {
        Serial.println("Timeout");
        return false;
    }

    Serial.println(WiFi.localIP());

    return true;
}

void wifiDisable()
{
    // Disconnect WiFi gracefully
    WiFi.disconnect();
    delay(1);

    // Turn WiFi off
    WiFi.mode(WIFI_OFF);

    // Enter sleep mode
    WiFi.forceSleepBegin();
    delay(1);
}

void wifiInit()
{
    //ESP.eraseConfig();
}
