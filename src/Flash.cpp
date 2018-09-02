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
#include "FS.h"

static Dir dir;


void flashInit()
{
    // Begin
    SPIFFS.begin();
}

int getNumFiles()
{
    int numFiles = 0;

    Dir dir = SPIFFS.openDir("");
    for (int i = 0; i < 1000; i++) {
        if (!dir.next()) {
            break;
        }
        numFiles++;
    }

    return numFiles;
}

void flashRemoveFiles()
{
    Dir dir = SPIFFS.openDir("");
    for (int i = 0; i < 1000; i++) {
        if (!dir.next()) {
            break;
        }
        Serial.print(F("Removing: "));
        Serial.println(dir.fileName());
        SPIFFS.remove(dir.fileName());
    }
}

bool flashWrite(const char *fileName, const void *buffer, size_t bufferLength)
{
    File f;

    // Open file for write
    f = SPIFFS.open(fileName, "w");
    if (!f) {
        return false;
    }

    // Write to file
    f.write((uint8_t *)buffer, bufferLength);

    // Close file
    f.close();

    return true;
}

size_t flashRead(const char *fileName, void *buffer, size_t bufferLength)
{
    size_t readSize;
    File f;

    // Open file for read
    f = SPIFFS.open(fileName, "r");
    if (!f) {
        return false;
    }

    // Read file
    readSize = f.read((uint8_t *)buffer, bufferLength);

    // Close file
    f.close();

    return readSize;
}

bool flashRemove(const char *fileName)
{
    // Remove file
    return SPIFFS.remove(fileName);
}

void flashCreateFileList()
{
    dir = SPIFFS.openDir("");
}

void getNextFile(String &fileName)
{
    if (dir.next()) {
        fileName = dir.fileName();
    } else {
        fileName = "";
    }
}
