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

*/

#pragma once
// 
// Author: Rien Matthijsse
// 

#include "config.h"

// 65c02 connections
//
#ifdef USE_NEO6502
// NEO6502 board v1.0
#define uP_RESET  28 // RESB(40) <-- UEXT pin 3
#define uP_CLOCK  21 // PHI2
#define uP_RW     11 // RW#

#define uP_BUZZ   20

#ifdef USE_SDCARD
#define SD_SPI SPI1
const int _MISO = 24; // AKA SPI RX
const int _MOSI = 27; // AKA SPI TX
const int _CS   = 25;
const int _SCK  = 26;
#endif

/*
BUS:
0 - 7
HDMI: pico_neo6502_cfg
15 - 18  27 - 30
*/
#else
// Rien's BB setup
#define uP_RESET  28 // RESB
#define uP_CLOCK  27 // PHI2
#define uP_RW     26 // RW#

#define uP_BUZZ   20
/*
BUS:
0 - 7
HDMI: pico_sock_cfg 
12 - 19
*/
#endif

// mux bus enable pins
//                                2         1         0
//                              21098765432109876543210   
#ifdef NEO6502
// 8 - 10
constexpr uint32_t en_MASK =  0b00000000000011100000000;
constexpr uint32_t en_NONE =  0b00000000000011100000000;
constexpr uint32_t en_A0_7 =  0b00000000000011000000000;
constexpr uint32_t en_A8_15 = 0b00000000000010100000000;
constexpr uint32_t en_D0_7 =  0b00000000000001100000000;
#else
// 20 - 22
constexpr uint32_t en_MASK =  0b11100000000000000000000;
constexpr uint32_t en_NONE =  0b11100000000000000000000;
constexpr uint32_t en_A0_7 =  0b01100000000000000000000;
constexpr uint32_t en_A8_15 = 0b10100000000000000000000;
constexpr uint32_t en_D0_7 =  0b11000000000000000000000;
#endif
