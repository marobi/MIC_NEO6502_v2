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

 Name:		sdcard.h
 Author:	Rien Matthijsse
*/

#pragma once
#include "config.h"

#ifdef USE_SDCARD

extern bool gSdInitialized;

bool sdInitialise();
bool sdOpendir(const char* directory);
bool sdCloseDir();
bool sdNextFile();

uint8_t sdReadFile(const char* fileName, const uint16_t loadAddress, const uint16_t offset);
uint8_t sdWriteFile(const char* fileName, const uint16_t startAddress, const uint32_t size);

#endif

