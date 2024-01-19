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
// 
// 
// 

#include "tile.h"
#include "vdu.h"
#include "memory.h"

defTile Tiles[MAX_TILES];

/// <summary>
/// 
/// </summary>
/// <param name="id"></param>
void dumpTile(uint8_t id) {
  defTile* tile = &Tiles[id];

  Serial.printf("TILE: %d\n", id);
  Serial.println(tile->cache.s, HEX);

    for (uint8_t i = 0; i < 8; i++) {
      Serial.printf("%02x ", tile->cache.b[i]);
    }
    Serial.println();
}


/// <summary>
/// 
/// </summary>
void initTile(const uint8_t id) {
  defTile* ltile = &Tiles[id];

  if (ltile->state == 0) {
    // 8x8 1 cpp tile
    for (uint8_t i = 0; i < 8; i++) {
      ltile->cache.b[i] = mem[TILE_BUFFER + id * 8 + i];
    }

    ltile->state = 1; //defined
  }
}

/// <summary>
/// 
/// </summary>
void drawTile() {
  uint8_t id = mem[VDU_SID];
  defTile* ltile = &Tiles[id];
  uint8_t mode = mem[VDU_DMOD];
  uint8_t color = mem[VDU_COL];
  uint16_t  cx, cy;

  initTile(id);
//  dumpTile(id);

  // get coords
  switch (mode) {
  case 0: // absolute coords
    cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
    cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;

    break;

  default:
  case 1: // raster coords 8*8
    cx = ((((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) * 8) % WIDTH;  // 40
    cy = ((((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) * 8) % HEIGHT; // 30
    break;
  }

  drawBitmap(cx, cy, ltile->cache.b, color);

  hasDisplayUpdate++;
}


/// <summary>
/// 
/// </summary>
void initTiles() {
  for (uint16_t i = 0; i < 256; i++)
    mem[TILE_BUFFER + i] = 0x00;

  for (uint8_t t = 0; t < MAX_TILES; t++)
    Tiles->state = 0; // undef
}
