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
#include <ErriezDS3231.h>

// Create DS3231 RTC object
DS3231 rtc;

#define RTC_NUM_RETRIES     10


void rtcSetAlarms()
{
    // Program alarms
    rtc.setAlarm1(Alarm1MatchSeconds, 0, 0, 0, 58);
    rtc.setAlarm2(Alarm2EveryMinute, 0, 0, 0);

    // Clear alarm 1 and 2 interrupts
    rtc.clearAlarmFlag(Alarm1);
    rtc.clearAlarmFlag(Alarm2);

    // Enable alarm interrupts
    rtc.alarmInterruptEnable(Alarm1, true);
    rtc.alarmInterruptEnable(Alarm2, true);
}

void rtcGetDateTime(DS3231_DateTime *dt)
{
   // Get date time from RTC
    if (rtc.getDateTime(dt)) {
        Serial.println(F("Error: Read date time failed"));
    }
}

void rtcPrintDateTimeShort(DS3231_DateTime *dt)
{
    char buf[10];

    // Clear alarm interrupts
    rtc.clearAlarmFlag(Alarm1);
    rtc.clearAlarmFlag(Alarm2);

    // Print day of the week
    Serial.print(dt->dayWeek);
    Serial.print(F(" "));

    // Print day month, month and year
    Serial.print(dt->dayMonth);
    Serial.print(F("-"));
    Serial.print(dt->month);
    Serial.print(F("-"));
    Serial.print(dt->year);
    Serial.print(F("  "));

    // Print time
    snprintf(buf, sizeof(buf), "%d:%02d:%02d", dt->hour, dt->minute, dt->second);
    Serial.println(buf);
}

void rtcSetGMT(uint32_t epoch)
{
    // Create and initialize date time object
    DS3231_DateTime newDateTime = {
        .second = 0,
        .minute = 0,
        .hour = 14,
        .dayWeek = 7, // 1 = Monday
        .dayMonth = 26,
        .month = 8,
        .year = 2018
    };

    // Set new RTC date/time
    //rtc.setDateTime(&newDateTime);
}

void rtcInitialize()
{
    int retries;

    // Initialize RTC
    for (retries = 0; retries < RTC_NUM_RETRIES; retries++) {
        if (rtc.begin() == false) {
            break;
        }
        delay(100);
    }

    if (retries >= RTC_NUM_RETRIES) {
        Serial.println(F("Error: Could not detect DS3231 RTC"));
    }

    // Check oscillator status
    if (rtc.isOscillatorStopped()) {
        Serial.println(F("Error: DS3231 RTC oscillator stopped"));
    }
}
