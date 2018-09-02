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
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include <ErriezBH1750.h>
#include <ErriezDS3231.h>
#include <ErriezDHT22.h>

#include "Config.h"
#include "Flash.h"
#include "Sensors.h"
#include "Ubidots.h"

// Sensor samples
SensorData sensorData;

// -----------------------------------------------------------------------------
// Number of retries after a read error
#define DHT22_MAX_READ_RETRIES    2

// Number of temperature and humidity samples for average calculation
#define DHT22_NUM_SAMPLES         10

// Create DHT22 sensor objects
DHT22 dht22Sensor[2] = { DHT22(DHT22_SENSOR1_PIN), DHT22(DHT22_SENSOR2_PIN) };

// -----------------------------------------------------------------------------
// ADDR line LOW/open:  I2C address 0x23 (0x46 including R/W bit) [default]
// ADDR line HIGH:      I2C address 0x5C (0xB8 including R/W bit)
BH1750 bh1750Sensor(LOW);

// -----------------------------------------------------------------------------
// BMP280 I2C address
// 0x76: pin 6 SDO/SAO: LOW (default)
// 0x77: pin 6 SDO/SAO: HIGH
#define BMP280_I2C_ADDRESS        0x77

// GY-91 pins: CSE NCS SDD/SAC SDA SCL GND 3V3 VIN
Adafruit_BMP280 bmp280Sensor; // I2C

// -----------------------------------------------------------------------------
extern DS3231 rtc;

// -----------------------------------------------------------------------------
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature owSensors(&oneWire);

// DS1820 device address
DeviceAddress ds1820DeviceAddress;

#define DS1820_RESOLUTION   12

// -----------------------------------------------------------------------------

static void printTemperature(int16_t temperature)
{
    // Check valid temperature value
    if (temperature == ~0) {
        // Temperature error (Check hardware connection)
        Serial.println(F("Error"));
    } else {
        // Print temperature
        Serial.print(temperature / 10);
        Serial.print(F("."));
        Serial.print(temperature % 10);

        // Print degree Celsius symbols
        // Choose if (1) for normal or if (0) for extended ASCII degree symbol
        if (1) {
            // Print *C characters which are displayed correctly in the serial
            // terminal of the Arduino IDE
            Serial.println(F(" *C"));
        } else {
            // Note: Extended characters are not supported in the Arduino IDE
            // and displays ?C. This is displayed correctly with other serial
            // terminals such as Tera Term.
            // Degree symbol is ASCII code 248 (decimal).
            char buf[4];
            snprintf_P(buf, sizeof(buf), PSTR(" %cC"), 248);
            Serial.println(buf);
        }
    }
}

static void printHumidity(int16_t humidity)
{
    // Check valid humidity value
    if (humidity == ~0) {
        // Humidity error (Check hardware connection)
        Serial.println(F("Error"));
    } else {
        // Print humidity
        Serial.print(humidity / 10);
        Serial.print(F("."));
        Serial.print(humidity % 10);
        Serial.println(F(" %"));
    }
}

static void dht22Read(uint8_t sensorId)
{
    int16_t temperature;
    int16_t humidity;

    // Read temperature
    temperature = dht22Sensor[sensorId].readTemperature();

    // Read humidity
    humidity = dht22Sensor[sensorId].readHumidity();

    sensorData.dht22Temperature[sensorId] =
        (temperature / 10.0) + ((temperature % 10) / 10);
    sensorData.dht22Humidity[sensorId] =
        (humidity / 10.0) + ((humidity % 10) / 10);

    // Print temperature
    Serial.print(F("  DHT22 "));
    Serial.print(sensorId);
    Serial.print(F(" "));
    printTemperature(temperature);

    // Print humidity
    Serial.print(F("  DHT22 "));
    Serial.print(sensorId);
    Serial.print(F(" "));
    printHumidity(humidity);
}

static void bmp280Read()
{
    float altitude;

    sensorData.bmp280Temperature = bmp280Sensor.readTemperature();
    sensorData.bmp280Pressure = bmp280Sensor.readPressure() / 100;

    // This should be adjusted to your local forcase
    altitude = bmp280Sensor.readAltitude(1014.30);

    Serial.print(F("  BMP280: "));
    Serial.print(sensorData.bmp280Temperature);
    Serial.println(F(" *C"));

    Serial.print(F("  BMP280: "));
    Serial.print(sensorData.bmp280Pressure);
    Serial.println(F(" hPa"));

    Serial.print(F("  BMP280: "));
    Serial.print(altitude);
    Serial.println(F(" m"));
}

static void rtcRead()
{
    int8_t temperature;
    uint8_t fraction;
    DS3231_DateTime dt;

    // Read RTC date and time from RTC
    if (rtc.getDateTime(&dt)) {
        return;
    }

    // Read RTC temperature
    rtc.getTemperature(&temperature, &fraction);

    // Store in sensor data
    sensorData.timestamp = rtc.getEpochTime(&dt);
    sensorData.ds3231Temperature = temperature + (fraction / 100.0);

    // Print time and temperature
    Serial.print(F("  DS3231: "));
    Serial.print(dt.hour);
    Serial.print(F(":"));
    Serial.print(dt.minute);
    if (dt.minute < 10) {
        Serial.print(F("0"));
    }
    Serial.print(F(":"));
    if (dt.second < 10) {
        Serial.print(F("0"));
    }
    Serial.print(dt.second);
    Serial.print(F("  "));
    Serial.print(temperature);
    Serial.print(F("."));
    Serial.print(fraction);
    Serial.println(F(" *C"));
}

static void ds1820Read()
{
    float temperature = owSensors.getTempCByIndex(0);

    // Store in sensor data
    sensorData.ds1820Temperature = temperature;

    Serial.print("  DS1820: ");
    Serial.print(temperature);
    Serial.println(F(" *C"));
}

static void bh1750Read()
{
    uint16_t lux;

    // Read light
    lux = bh1750Sensor.read();

    // Store in sensor data
    sensorData.bh1750Light = (lux / 2) + ((lux % 10) / 10.0);

    // Print light
    Serial.print(F("  BH1750: "));
    Serial.print(lux / 2);
    Serial.print(F("."));
    Serial.print(lux % 10);
    Serial.println(F(" LUX"));

    //bh1750Sensor.powerDown();
}

static void printSensorData()
{
    Serial.println(F("Sensor data:"));

    Serial.print(F("  Timestamp: "));
    Serial.println(sensorData.timestamp);

    Serial.print(F("  dht22Temperature[0]: "));
    Serial.println(sensorData.dht22Temperature[0]);
    Serial.print(F("  dht22Temperature[1]: "));
    Serial.println(sensorData.dht22Temperature[1]);

    Serial.print(F("  dht22Humidity[0]: "));
    Serial.println(sensorData.dht22Humidity[0]);
    Serial.print(F("  dht22Humidity[1]: "));
    Serial.println(sensorData.dht22Humidity[1]);

    Serial.print(F("  bmp280Temperature: "));
    Serial.println(sensorData.bmp280Temperature);
    Serial.print(F("  bmp280Pressure: "));
    Serial.println(sensorData.bmp280Pressure);

    Serial.print(F("  ds1820Temperature: "));
    Serial.println(sensorData.ds1820Temperature);

    Serial.print(F("  ds3231Temperature: "));
    Serial.println(sensorData.ds3231Temperature);

    Serial.print(F("  bh1750Light: "));
    Serial.println(sensorData.bh1750Light);
}

void sensorsUpload()
{
    String fileName;

    if (!getNumFiles()) {
        return;
    }

    // Ubidots begin
    ubidotsBegin();

    // Create file list
    flashCreateFileList();

    for (int i = 0; i < 1000; i++) {
        getNextFile(fileName);
        if (fileName != "") {
            size_t readSize;

            // Read sensor data from file
            readSize = flashRead(fileName.c_str(),
                                 &sensorData, sizeof(sensorData));

            // Print sensor data
            if (readSize == sizeof(sensorData)) {
                Serial.print(F("  Upload "));
                Serial.print(fileName);
                Serial.println(F("..."));

                // Print sensor data
                printSensorData();

                // Send sensor data
                if (sensorData.dht22Temperature[0] != ~0) {
                    ubidotsAddVariable("DHT22 1 Temperature",
                                        sensorData.dht22Temperature[0],
                                        sensorData.timestamp);
                }
                if (sensorData.dht22Temperature[1] != ~0) {
                    ubidotsAddVariable("DHT22 2 Temperature",
                                        sensorData.dht22Temperature[1],
                                        sensorData.timestamp);
                }
                if (sensorData.dht22Humidity[0] != ~0) {
                    ubidotsAddVariable("DHT22 1 Humidity",
                                        sensorData.dht22Humidity[0],
                                        sensorData.timestamp);
                }
                if (sensorData.dht22Humidity[1] != ~0) {
                    ubidotsAddVariable("DHT22 2 Humidity",
                                        sensorData.dht22Humidity[1],
                                        sensorData.timestamp);
                }
                if (sensorData.bmp280Temperature != -1) {
                    ubidotsAddVariable("BMP280 Temperature",
                                        sensorData.bmp280Temperature,
                                        sensorData.timestamp);
                }
                ubidotsSend();

                if (sensorData.bmp280Pressure > 0) {
                    ubidotsAddVariable("BMP280 Pressure",
                                        sensorData.bmp280Pressure,
                                        sensorData.timestamp);
                }
                ubidotsAddVariable("DS1820 Temperature",
                                    sensorData.ds1820Temperature,
                                    sensorData.timestamp);
                ubidotsAddVariable("DS3231 Temperature",
                                    sensorData.ds3231Temperature,
                                    sensorData.timestamp);
                ubidotsAddVariable("BH1750 Light",
                                    sensorData.bh1750Light,
                                    sensorData.timestamp);
                ubidotsSend();

                Serial.println();
            }

            // Remove file from flash
            flashRemove(fileName.c_str());
        } else {
            break;
        }
    }
}

static void saveSensorData()
{
    char fileName[16];

    // Create filename
    snprintf_P(fileName, sizeof(fileName), PSTR("%d"), sensorData.timestamp);

    Serial.print(F("Save sensor data "));
    Serial.println(fileName);

    // Write sensor data to file on flash
    flashWrite(fileName, &sensorData, sizeof(SensorData));
}

void sensorsRead()
{
    // Sample sensors
    dht22Read(0);
    dht22Read(1);
    bmp280Read();
    ds1820Read();
    rtcRead();
    bh1750Read();

    // Save sensor data to flash
    saveSensorData();
}

void sensorsStartConversion()
{
    bh1750Sensor.startConversion();
    owSensors.requestTemperatures();
    //rtc.startTemperatureConversion(); // Takes 64 seconds to complete
}

void sensorsInit()
{
    // Initialize DHT22 sensors
    dht22Sensor[0].begin(DHT22_MAX_READ_RETRIES, DHT22_NUM_SAMPLES);
    dht22Sensor[1].begin(DHT22_MAX_READ_RETRIES, DHT22_NUM_SAMPLES);

    // Initialize sensor in one-time mode, high 1 lx resolution
    bh1750Sensor.begin(ModeOneTime, ResolutionHigh);

    // Initialize BMP280
    if (!bmp280Sensor.begin(BMP280_I2C_ADDRESS)) {
        Serial.println("Error: Could not detect BMP280 sensor");
    }

    // Initialize one-wire DS1820 sensor
    owSensors.begin();

    owSensors.getAddress(ds1820DeviceAddress, 0);
    owSensors.setResolution(ds1820DeviceAddress, DS1820_RESOLUTION);

    owSensors.setWaitForConversion(false);
}
