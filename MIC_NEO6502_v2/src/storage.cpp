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

 Name:		storage.cpp
 Author:	Rien Matthijsse
*/

#include "arduino.h"
#include "config.h"
#include "sdcard.h"

#include "vdu.h"
#include "storage.h"
#include "memory.h"

char gCurrentDirectory[64];
char gFilename[64];

/// <summary>
/// 
/// </summary>
void dumpFIO() {
  Serial.printf("FIO: %0x04x= ", FIO_CMD);
  for (uint16_t c = FIO_CMD; c < FIO_OFFSET + 1; c++) {
    Serial.printf(" %02x", mem[c]);
  }
  Serial.println();
}

/// <summary>
/// 
/// </summary>
void cpFilename() {
  uint8_t len = mem[FIO_FNLEN];
  uint16_t fn = mem[FIO_FILE] + ((uint16_t)mem[FIO_FILE+1] << 8);

  for (uint8_t ch = 0; ch < len; ch++) {
    gFilename[ch] = mem[fn + ch];
  }

  gFilename[len] = '\0';
}

/// <summary>
/// initialise sdcard
/// </summary>
/// <returns></returns>
bool initStorage() {
  strcpy(gCurrentDirectory, "/");

  mem[FIO_CMD] = 0x00;
  mem[FIO_STATUS] = 0x00;
  mem[FIO_DEVICE] - 0x00;
  mem[FIO_FNLEN] = 0x00;
  mem[FIO_OFFSET] = 0x0000;

#ifdef USE_SDCARD
  return sdInitialise();
#endif
}

/// <summary>
/// list directories in directory
/// </summary>
/// <param name="directory"></param>
void listDirectory(const uint8_t device, const char *directory) {
  strcpy(gCurrentDirectory, directory);

#ifdef USE_SDCARD
  sdOpendir(directory);

  while (sdNextFile());

  sdCloseDir();
#endif

}

/// <summary>
/// change directory
/// </summary>
/// <param name="directory"></param>

void changeDirectory(const uint8_t device, const char* directory) {
  strcpy(gCurrentDirectory, directory);
}

/// <summary>
/// read a file from storage
/// </summary>
/// <param name="fileName"></param>
/// <param name="loadAddress"></param>
/// <param name="offset"></param>
/// <returns></returns>
uint8_t readFile(const uint8_t device, const char* fileName, const uint16_t loadAddress, const uint16_t offset) {
#ifdef USE_SDCARD
  return sdReadFile(fileName, loadAddress, offset);
#endif
}

/// <summary>
/// write a file to storage
/// </summary>
/// <param name="fileName"></param>
/// <param name="startAddress"></param>
/// <param name="size"></param>
/// <returns></returns>
uint8_t writeFile(const uint8_t device, const char* fileName, const uint16_t startAddress, const uint32_t size) {
#ifdef USE_SDCARD
  return sdWriteFile(fileName, startAddress, size);
#endif
}

/// <summary>
/// 
/// </summary>
void scanFIO()
{
  uint8_t cmd = mem[FIO_CMD];
  uint16_t saddr = mem[FIO_SADDR] + ((uint16_t)mem[FIO_SADDR + 1] << 8);
  uint16_t eaddr = mem[FIO_EADDR] + ((uint16_t)mem[FIO_EADDR + 1] << 8);

  if (cmd != 0x00) {
    dumpFIO();

    switch (cmd) {
    case FIO_LOAD:
      cpFilename();
Serial.printf("***D: FIO_LOAD: %s on %d @0x%04x\n", gFilename, mem[FIO_DEVICE], saddr);
      readFile(mem[FIO_DEVICE], gFilename, saddr, 0);
      break;
    
    case FIO_ROM:
      break;
    
    case FIO_SAVE:
      cpFilename();
Serial.printf("***D: FIO_SAVE: %s on %d @0x%04x:@0x%04x\n", gFilename, mem[FIO_DEVICE], saddr, eaddr);
      writeFile(mem[FIO_DEVICE], gFilename, saddr, eaddr - saddr);
      break;
    
    case FIO_DIR:
      cpFilename();
Serial.printf("***D: FIO_DIR: %s on %d\n", gFilename, mem[FIO_DEVICE]);
      if (strcmp(gFilename, "") == 0)
        listDirectory(mem[FIO_DEVICE], gCurrentDirectory);
      else
        listDirectory(mem[FIO_DEVICE], gFilename);
      
      break;
    
    case FIO_CD:
      cpFilename();
Serial.printf("***D: FIO_CD: %s on %d\n", gFilename, mem[FIO_DEVICE]);
      if (strcmp(gFilename, "") == 0)
        changeDirectory(mem[FIO_DEVICE], "/");
      else
        changeDirectory(mem[FIO_DEVICE], gFilename);
      break;

    default: // illegal command
      Serial.println("***E: Illegal FIO command");
      break;
    }

    // reset command register
    mem[FIO_CMD] = 0x00;
  }
}
