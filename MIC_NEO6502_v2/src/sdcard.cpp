/*
This software is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 Name:		sdcard.cpp
 Author:	Rien Matthijsse
*/

#include "arduino.h"

#include "config.h"

#include "pins.h"

// include the SD library:
#include <SPI.h>
#include <SD.h>

#include "memory.h"
#include "vdu.h"

#ifdef USE_SDCARD

bool gSdInitialized = false;
bool gSdDirIsOpen = false;
File gDir;

/// <summary>
/// 
/// </summary>
/// <returns></returns>
bool sdInitialise() {
  SPI1.setRX(_MISO);
  SPI1.setTX(_MOSI);
  SPI1.setSCK(_SCK);
  gSdInitialized = SD.begin(_CS, SD_SPI);

  if (!gSdInitialized) {
    Serial.println(F("SDCard initialization failed"));
  }
  else {
    Serial.println(F("SDCard initialised"));
  }

  return gSdInitialized;
}

/// <summary>
/// open a directory
/// </summary>
/// <param name="directory"></param>
/// <returns></returns>
bool sdOpendir(const char *directory) {
  if (gSdInitialized) {
    gDir = SD.open(directory);
    gSdDirIsOpen = true;
    return true;
  }

  return false;
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
bool sdCloseDir() {
  if (gSdDirIsOpen) {
    gDir.close();
    gSdDirIsOpen = false;
    return true;
  }
  return false;
}

/// <summary>
/// show next directory entry
/// </summary>
/// <returns></returns>
bool sdNextFile() {
  if (gSdDirIsOpen) {
    File entry = gDir.openNextFile();

    if (!entry) {
      return false;
    }

    if (entry.isDirectory()) {
      log("<%s>\n", entry.fullName());
    }
    else {
      log("%14s   %d\n", entry.name(), entry.size());
    }

    return true;
  }

  return false;
}

/// <summary>
/// read a file info mem
/// </summary>
/// <param name="fileName"></param>
/// <param name="loadAddress"></param>
/// <param name="offset"></param>
/// <returns></returns>
uint8_t sdReadFile(const char* fileName, const uint16_t loadAddress, const uint16_t offset) {
  if (gSdInitialized) {
    File fp = SD.open(fileName, FILE_READ);

    fp.read(mem + loadAddress, fp.size());
    fp.close();
    return 0;
  }
  return 1;
}

/// <summary>
/// write a file from mem
/// </summary>
/// <param name="fileName"></param>
/// <param name="startAddress"></param>
/// <param name="size"></param>
/// <returns></returns>
uint8_t sdWriteFile(const char* fileName, const uint16_t startAddress, const uint32_t size) {
  if (gSdInitialized) {
    File fp = SD.open(fileName, FILE_WRITE);

    fp.write(mem + startAddress, size);
    fp.close();
    return 0;
  }

  return 1;
}

#endif
