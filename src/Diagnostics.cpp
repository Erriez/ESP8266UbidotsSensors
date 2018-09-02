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

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "Diagnostics.h"


void printDiagnostics()
{
    // Print reset reason
    Serial.print(F("Reset reason: "));
    Serial.println(ESP.getResetReason());

    // Print reset info
    Serial.print(F("Reset info: "));
    Serial.println(ESP.getResetInfo());

    // Print core version
    Serial.print(F("Core version: "));
    Serial.println(ESP.getCoreVersion());

    // Print SDK version
    Serial.print(F("SDK version: "));
    Serial.println(ESP.getSdkVersion());

    // Print CPU frequency
    Serial.print(F("CPU frequency: "));
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(F("MHz"));

    // Print CPU ID
    Serial.print(F("CPU ID: 0x"));
    Serial.println(ESP.getChipId(), HEX);

    // Print VCC
    Serial.print(F("getVcc: "));
    Serial.print(ESP.getVcc() / 1024.0);
    Serial.println(F("V"));

    // Print sketch size
    Serial.print(F("Sketch size: "));
    Serial.println(ESP.getSketchSize());

    Serial.print(F("Flash chip size: "));
    Serial.println(ESP.getFlashChipSize());

    Serial.print(F("Flash chip real size:"));
    Serial.println(ESP.getFlashChipRealSize());

    Serial.print(F("CPU flash chip speed: "));
    Serial.println(ESP.getFlashChipSpeed());

    // Print free heap size
    Serial.print(F("Free heap size: "));
    Serial.println(ESP.getFreeHeap());

    // Print WiFi status
    WiFi.printDiag(Serial);
}
