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

#ifndef SETTINGS_H_
#define SETTINGS_H_

// WiFi connection
#define WLAN_SSID           "****"
#define WLAN_PASS           "****"

// Ubidots token
#define UBIDOTS_TOKEN       "A1E-.............................."

// Connect RTC DS3231 SQW pin via 1uF capacitor to ESP8266 RST pin

// TWI pins
#define TWI_SCL             D1  // Same as 5 = GPIO5
#define TWI_SDA             D2  // Same as 4 = GPIO4

// LED pin
#define LED_PIN             D4  // Same as 2 = GPIO2

// DHT22 pins
#define DHT22_SENSOR1_PIN   D5 // Same as GPIO14
#define DHT22_SENSOR2_PIN   D6 // Same as GPIO12

// One wire pin
#define ONE_WIRE_BUS        D7

// Wake every minute
#define SLEEP_TIME_SEC      10

#endif // SETTINGS_H_
