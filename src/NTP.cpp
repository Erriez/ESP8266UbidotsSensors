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

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Config.h"
#include "NTP.h"
#include "WiFi.h"

// local port to listen for UDP packets
#define UDP_LOCAL_PORT    2390

// Don't hardwire the IP address or we won't get the benefits of the pool.
//  Lookup the IP address for the host name instead
// IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

// time.nist.gov NTP server address
static const char *ntpServerName = "time.nist.gov";

// NTP time stamp is in the first 48 bytes of the message
#define NTP_PACKET_SIZE   48

// Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
#define SEVENTY_YEARS     2208988800UL

// Buffer to hold incoming and outgoing packets
static byte packetBuffer[NTP_PACKET_SIZE];

// A UDP instance to let us send and receive packets over UDP
static WiFiUDP udp;


// send an NTP request to the time server at the given address
static void sendNTPpacket(IPAddress& address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);

    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

uint32_t getNTPEpoch()
{
    IPAddress timeServerIP;
    unsigned long highWord;
    unsigned long lowWord;
    unsigned long secsSince1900;

    // Connect with WiFi
    if (WiFi.status() != WL_CONNECTED) {
        return 0;
    }

    // Start UDP request
    udp.begin(UDP_LOCAL_PORT);

    // DNS lookup
    WiFi.hostByName(ntpServerName, timeServerIP);

    // Send an NTP packet to a time server
    sendNTPpacket(timeServerIP);

    // Wait to see if a reply is available
    for (int i = 0; i < 200; i++) {
        if (udp.available() >= NTP_PACKET_SIZE) {
            break;
        }
        delay(10);
    }

    if (!udp.parsePacket()) {
        return 0;
    }

    // We've received a packet, read the data from it
    if (udp.read(packetBuffer, NTP_PACKET_SIZE) < NTP_PACKET_SIZE) {
        return 0;
    }

    // The timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    highWord = word(packetBuffer[40], packetBuffer[41]);
    lowWord = word(packetBuffer[42], packetBuffer[43]);

    // Combine the four bytes (two words) into a long integer this is NTP time
    // (seconds since Jan 1 1900):
    secsSince1900 = highWord << 16 | lowWord;

    // Subtract seventy years to get epoch
    return secsSince1900 - SEVENTY_YEARS;
}

void printEpoch(uint32_t epoch)
{
    // print Unix time:
    Serial.print("Epoch: ");
    Serial.println(epoch);

    // Print UTC time at Greenwich Meridian (GMT): hour, minute and second
    Serial.print("The UTC time is ");
    // Print the hour (86400 equals secs per day)
    Serial.print((epoch  % 86400L) / 3600);
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
    }

    // Print the minute (3600 equals secs per minute)
    Serial.print((epoch  % 3600) / 60);
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
}
