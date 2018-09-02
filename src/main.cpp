/*
 * MIT License
 *
 * Copyright (c) 2018 Erriez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*!
 * \brief ESP8166 low power example
 * \details
 *    This example configures the ESP8266 in low power. Default power is ~70mA.
 *
 *    Bootloader takes about 350ms before setup() is reached.
 *
 *    Connect pin D0 to RST to wakeup. Disconnect this pin before flashing.
 */

#include <Wire.h>
#include <ESP8266WiFi.h>

extern "C" {
#include "user_interface.h"
}

#include <ErriezDS3231.h>
#include "Config.h"
#include "Diagnostics.h"
#include "Flash.h"
#include "NTP.h"
#include "RTCRAM.h"
#include "RTC.h"
#include "Sensors.h"
#include "WiFi.h"

// RTC RAM data
extern RTCRamData ramData;

// Current RTC date/time
static DS3231_DateTime dt;

// Set ADC mode to read VCC
ADC_MODE(ADC_VCC);

static void initializeDevices();
static void powerOnReset();
static void awake();
static void powerDown();


void setup()
{
    rst_info *rstInfo;

    // Initialize devices
    initializeDevices();

#if 0
    sensorsInit();
    while (1) {
        sensorsStartConversion();
        delay(2000);
        sensorsRead();
    }
#endif

    Serial.print(F("Reset reason: "));

    // Get reset info
    rstInfo = ESP.getResetInfoPtr();

    // Disable or enable WiFi at startup which takes stome time
    switch (rstInfo->reason) {
        case REASON_DEFAULT_RST:
            Serial.println(F("DEFAULT_RST"));
            break;
        case REASON_EXT_SYS_RST:
            Serial.println(F("EXT_SYS_RST"));
            powerOnReset();
            break;
        case REASON_DEEP_SLEEP_AWAKE:
            Serial.println(F("DEEP_SLEEP_AWAKE"));
            awake();
            break;
        case REASON_SOFT_WDT_RST:
            Serial.println(F("WATCHDOG RESET"));
            break;
        default:
            Serial.println(F("UNKNOWN"));
            break;
    }

    // Set in power down
    powerDown();
}

void loop()
{
    // This line will not be reached
}

static void initializeDevices()
{
    // Turn LED on
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize serial port
    Serial.begin(115200);
    Serial.println();

    // Initialize TWI
    Wire.begin(TWI_SDA, TWI_SCL);
    Wire.setClock(100000);

    // Initialize RTC
    rtcInitialize();

    // Initialize flash
    flashInit();

    // Initialize sensors
    sensorsInit();
}

static void powerOnReset()
{
    uint32_t epoch;

    // Initialize WiFi
    Serial.println(F("Initialize WiFi..."));
    wifiInit();
    wifiConnect();

    // Get NTP epoch time
    Serial.print(F("Get NTP epoch..."));
    epoch = getNTPEpoch();
    if (epoch > 0) {
        Serial.println(epoch);
    } else {
        Serial.println(F("FAILED"));
    }

    // Program external RTC
    Serial.println(F("Program RTC data/time from epoch..."));
    rtcSetGMT(epoch);
    Serial.println(F("Program RTC alarms..."));
    rtcSetAlarms();
    // Get and print RTC date time
    Serial.print(F("RTC date/time: "));
    rtcGetDateTime(&dt);
    rtcPrintDateTimeShort(&dt);

    // Print diagnostics
    //printDiagnostics();

    // Upload sensor data from flash storage
    if (getNumFiles()) {
        Serial.println(F("Upload sensors data..."));
        sensorsUpload();
    }

    // Reset count
    ramData.wakeCount = 0;
}

static void awake()
{
    // Read
    Serial.println(F("Read RTC RAM..."));
    readRTCRam();

    // Increment reset count
    ramData.wakeCount++;

    // Print reset count
    Serial.print(F("Wake count: "));
    Serial.println(ramData.wakeCount);

    // Get and print RTC date time
    Serial.print(F("RTC date/time: "));
    rtcGetDateTime(&dt);
    rtcPrintDateTimeShort(&dt);

    // Start sensor conversions
    if (((dt.minute % 4) == 0) && (dt.second == 58)) {
        // Start sensors conversion
        Serial.println(F("Start sensor conversions..."));
        sensorsStartConversion();
    }

    // Read sensor data
    if (((dt.minute % 5) == 0) && (dt.second == 0)) {
        // Read sensors
        Serial.println(F("Read sensors..."));
        sensorsRead();
    }

    // Uplaod sensor data
    if (((dt.minute % 30) == 0) && (dt.second == 0)) {
        if (getNumFiles()) {
            // Initialize WiFi
            Serial.println(F("Initialize WiFi..."));
            wifiInit();
            wifiConnect();

            // Check if already connected
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println(F("Upload sensors data..."));
                sensorsUpload();
            } else {
                Serial.println(F("WiFi connect failed"));
            }
        }
    }
}

static void powerDown()
{
    // Save variables to RTC RAM
    Serial.println(F("Write RTC RAM..."));
    writeRTCRam();

    // Deep sleep generates external reset after wake
    Serial.print(F("Deep sleep "));
    if (((dt.minute % 29) == 0) && (dt.second == 58)) {
        Serial.println(F("(RF default)..."));
        ESP.deepSleep(0 * 1000000UL, WAKE_RF_DEFAULT);
    } else {
        Serial.println(F("(RF disabled)..."));
        //wifiDisable();
        ESP.deepSleep(0 * 1000000UL, WAKE_RF_DISABLED);
    }
    Serial.println("");
}
