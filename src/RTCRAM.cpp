#include <Arduino.h>

#include "CRC32.h"
#include "RTCRAM.h"

RTCRamData ramData;


void readRTCRam()
{
    uint32_t crc;

    ESP.rtcUserMemoryRead(0, (uint32_t *)&crc, sizeof(crc));
    ESP.rtcUserMemoryRead(4, (uint32_t *)&ramData, sizeof(ramData));

    if (crc != calculateCRC32((uint8_t *)&ramData, sizeof(ramData))) {
        Serial.println(F("Error: CRC RTC data incorrect"));
        memset(&ramData, 0, sizeof(ramData));
    }
}

void writeRTCRam()
{
    uint32_t crc;

    crc = calculateCRC32((uint8_t *)&ramData, sizeof(ramData));

    ESP.rtcUserMemoryWrite(0, (uint32_t *)&crc, sizeof(crc));
    ESP.rtcUserMemoryWrite(4, (uint32_t *)&ramData, sizeof(ramData));
}
