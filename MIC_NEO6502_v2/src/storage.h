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

 Name:		storage.h
 Author:	Rien Matthijsse
*/
#pragma once

// FIO register definitions
#define FIO_CMD    0xD050  // CMD register
#define FIO_STATUS 0xD051  // IO Status (readonly)
#define FIO_DEVICE 0xD052  // device id
#define FIO_FILE   0xD053  // Filename (pointer to buffer)
#define FIO_FNLEN  0xD055  // Length of filename
#define FIO_SADDR  0xD056  // Load address
#define FIO_EADDR  0xD058  // Load address
#define FIO_OFFSET 0xD05A  // Offset in loadfile image, normally 0x0000

#define FIO_LOAD   0x01  // LOAD
#define FIO_ROM    0x02  // LOAD ROM cartridge
#define FIO_SAVE   0x03  // SAVE
#define FIO_DIR    0x04  // SHOW directory
#define FIO_CD     0x05  // CHANGE directory

extern char gCurrentDirectory[];  // default = "/"

bool initStorage();
void changeDirectory(const uint8_t device, const char* directory);
void listDirectory(const uint8_t device, const char* directory);
uint8_t readFile(const uint8_t device, const char* fileName, const uint16_t loadAddress, const uint16_t offset);
uint8_t writeFile(const uint8_t device, const char* fileName, const uint16_t startAddress, const uint32_t size);

void scanFIO();
