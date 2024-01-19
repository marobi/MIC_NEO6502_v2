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
#include <PicoDVI.h>

#include "config.h"
#include "version.h"
#include "memory.h"
#include "vdu.h"
#include "palette.h"
#include "sprite.h"
#include "tile.h"

#ifdef USE_NEO6502
// Pico HDMI for Olimex Neo6502 
static const struct dvi_serialiser_cfg pico_neo6502_cfg = {
  .pio = DVI_DEFAULT_PIO_INST,
  .sm_tmds = {0, 1, 2},
  .pins_tmds = {14, 18, 16},
  .pins_clk = 12,
  .invert_diffpairs = true
};
#endif

// Here's how an 320x240 256 colors graphics display is declared.
#ifndef USE_NEO6502
DVIGFX8 display(DVI_RES_320x240p60, true, pico_sock_cfg);
#else
#ifdef USE_DOUBLE_BUFFERING
DVIGFX8 display(DVI_RES_320x240p60, true, pico_neo6502_cfg);
#else
DVIGFX8 display(DVI_RES_320x240p60, false, pico_neo6502_cfg);
#endif
#endif

boolean        statusCursor = true;
uint8_t        currentColor;
uint8_t        currentColorIndex = 0;
boolean        autoUpdate = true;
boolean        autoScroll = true;
uint8_t        currentTextColor = DEFAULT_TEXT_COLOR;
uint32_t       hasDisplayUpdate = 0;

boolean        traceOn = false;

/// <summary>
/// 
/// </summary>
/// <param name="color_id"></param>
/// <returns></returns>
inline __attribute__((always_inline))
uint16_t getColor(const uint8_t color_id) {
  return display.getColor(color_id);
}

/// <summary>
/// 
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <returns></returns>
uint8_t getPixel(const uint16_t x, const uint16_t y) {
  return display.getPixel(x, y);
}

/// <summary>
/// 
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="color"></param>
void drawPixel(const uint16_t x, const uint16_t y, const uint16_t color) {
  display.drawPixel(x, y, color);
}

/// <summary>
/// 
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="bm"></param>
/// <param name="color"></param>
void drawBitmap(const uint16_t x, const uint16_t y, const uint8_t* bm, const uint8_t color) {
  display.drawBitmap(x, y, bm, 8, 8, color);
}

/// <summary>
/// control visibility of cursor
/// </summary>
/// <param name="vSet"></param>
inline __attribute__((always_inline))
void showCursor(const boolean vSet) {
  statusCursor = vSet;
}

/// <summary>
/// set current text color (by pallette index)
/// </summary>
/// <param name="vColor"></param>
//inline __attribute__((always_inline))
void setColor(const uint8_t vColor) {
  currentColor = vColor;
  display.setTextColor(vColor);
}

/// <summary>
/// 
/// </summary>
inline __attribute__((always_inline))
void setCursor() {
  int16_t cursor_x, cursor_y;
  // remove optional cursor
  if (statusCursor) {
    cursor_x = display.getCursorX();
    cursor_y = display.getCursorY();
    display.fillRect(cursor_x, cursor_y + FONT_CHAR_HEIGHT - 2, FONT_CHAR_WIDTH, 2, currentColor); // show cursor
  }
}

/// <summary>
/// 
/// </summary>
inline __attribute__((always_inline))
void removeCursor() {
  int16_t cursor_x, cursor_y;
  // remove optional cursor
  if (statusCursor) {
    cursor_x = display.getCursorX();
    cursor_y = display.getCursorY();
    display.fillRect(cursor_x, cursor_y, FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT, 0); // remove cursor
  }
}

//---------------------------------------------------------------------------------------------------------------------------
int16_t cx, cy, ex, ey;
uint16_t cw, ch, cr;

/// <summary>
/// clear display
/// </summary>
void cmdClearDisplay() {
  display.fillScreen(0);
  display.setCursor(0, 0);

  hasDisplayUpdate++;
}

/// <summary>
/// move cursor
/// </summary>
void cmdMoveCursor() {
  cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
  cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;
  display.setCursor(cx, cy);

  hasDisplayUpdate++;
}

/// <summary>
/// 
/// </summary>
void cmdSetColor() {
  setColor(mem[VDU_COL]);
}

/// <summary>
/// 
/// </summary>
void cmdSetPixel() {
  cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
  cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;

  drawPixel(cx, cy, currentColor);

  hasDisplayUpdate++;
}

/// <summary>
/// 
/// </summary>
void cmdDrawLine() {
  cx = display.getCursorX();
  cy = display.getCursorY();
  ex = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
  ey = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;

  display.drawLine(cx, cy, ex, ey, currentColor);
  display.setCursor(ex, ey);

  hasDisplayUpdate++;
}

/// <summary>
/// 
/// </summary>
void cmdDrawRect() {
  cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
  cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;
  cw = (((uint16_t)mem[VDU_WH] * 256) + mem[VDU_WL]) % WIDTH;
  ch = (((uint16_t)mem[VDU_HH] * 256) + mem[VDU_HL]) % HEIGHT;

  //    Serial.printf("RECT %04x %04x %04x %04x\n", cx, cy, cw, ch);
  switch (mem[VDU_DMOD]) {
  case 0:
    display.drawRect(cx, cy, cw, ch, currentColor);
    break;
  default:
    display.fillRect(cx, cy, cw, ch, currentColor);
    break;
  }

  hasDisplayUpdate++;
}


/// <summary>
/// 
/// </summary>
void cmdDrawCircle() {
  cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
  cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;
  cr = (((uint16_t)mem[VDU_WH] * 256) + mem[VDU_WL]) % WIDTH;

  //    Serial.printf("CIRC %04x %04x %04x\n", cx, cy, cr);
  switch (mem[VDU_DMOD]) {
  case 0:
    display.drawCircle(cx, cy, cr, currentColor);
    break;
  default:
    display.fillCircle(cx, cy, cr, currentColor);
    break;
  }

  hasDisplayUpdate++;
}

/// <summary>
/// 
/// </summary>
void cmdDrawTriangle() {
  cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
  cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;
  ex = (((uint16_t)mem[VDU_WH] * 256) + mem[VDU_WL]) % WIDTH;
  ey = (((uint16_t)mem[VDU_HH] * 256) + mem[VDU_HL]) % WIDTH;

  //    Serial.printf("TRI %04x %04x %04x %04x\n", cx, cy, ex, ey);
  switch (mem[VDU_DMOD]) {
  case 0:
    display.drawTriangle(display.getCursorX(), display.getCursorY(), cx, cy, ex, ey, currentColor);
    break;
  default:
    display.fillTriangle(display.getCursorX(), display.getCursorY(), cx, cy, ex, ey, currentColor);
    break;
  }

  hasDisplayUpdate++;
}

/// <summary>
/// 
/// </summary>
void cmdSetPalette() {
  display.setColor(mem[VDU_PAL], ((uint16_t)mem[VDU_BCOL] * 256) + mem[VDU_COL]);
  display.swap(false, true);
}

/// <summary>
/// 
/// </summary>
void cmdSetVDU() {
  switch (mem[VDU_MOD] & 0x01) {
  case 1:
    showCursor(false);
    break;
  default:
    showCursor(true);
    break;
  }

  autoUpdate = !(mem[VDU_MOD] >> 7);
  autoScroll = !((mem[VDU_MOD] >> 1) & 0x01);
}

/// <summary>
/// 
/// </summary>
void cmdRefresh() {
  swapDisplay();
}

/// <summary>
/// 
/// </summary>
void cmdDefSprite() {
  initSprite();
}

/// <summary>
/// 
/// </summary>
void cmdDrawSprite() {
  drawSprite();
}

/// <summary>
/// 
/// </summary>
void cmdMoveSprite() {
  moveSprite();
}

/// <summary>
/// 
/// </summary>
void cmdCollSprite() {
  collSprite();
}

/// <summary>
/// 
/// </summary>
void cmdDefTile() {
  drawTile();
}

/// <summary>
/// 
/// </summary>
void cmdVoid() {

}

/// <summary>
/// 
/// </summary>
void cmdIllegal() {
  Serial.printf("ERR: illegal VDU command %02x\n", mem[VDU_CMD]);
}

//---------------------------------------------------------------------------------------------------------------------
/// <summary>
/// 
/// </summary>
typedef void (*cmdFunction)(void);

/// <summary>
/// 
/// </summary>
cmdFunction cmdCommands[32] = {
  cmdVoid, // 0
  cmdClearDisplay, // 1
  cmdMoveCursor,   // 2
  cmdSetColor, // 3
  cmdSetPixel, // 4
  cmdDrawLine, // 5
  cmdDrawRect, // 6
  cmdDrawCircle, // 7
  cmdSetPalette, // 8
  cmdSetVDU, // 9
  cmdRefresh, // A
  cmdDrawTriangle, // B
  cmdIllegal, // C
  cmdIllegal, // D
  cmdIllegal, // E
  cmdIllegal, // F
  cmdDefSprite, // 10
  cmdIllegal, // 11
  cmdDrawSprite, // 12
  cmdIllegal, // 13
  cmdMoveSprite, // 14
  cmdCollSprite, // 15
  cmdDefTile, // 16
};

/// <summary>
/// performa a action on the display
/// </summary>
/// <param name="vCmd"></param>
inline __attribute__((always_inline))
void setVDU(const uint8_t vCmd) {
  if (vCmd == CMD_SANE) {
    resetDisplay();
    return;
  }

  cmdFunction f = cmdCommands[vCmd];
  f();

#if 0
  int16_t cx, cy, ex, ey;
  uint16_t cw, ch, cr;

//  Serial.printf("*VDU: %02x\n", vCmd);

//  while (!dma_done);

  switch (vCmd) {
  case CMD_CLS: // clearscreen
    cmdClearDisplay();
    break;

  case CMD_MOVE: // set cursor
    cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
    cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;
    display.setCursor(cx, cy);
    hasDisplayUpdate++;
    break;

  case CMD_COLOR: // set color
    setColor(mem[VDU_COL]);
    break;

  case CMD_PIXEL: // set pixel (ignore screen mode)
    cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
    cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;

    display.drawPixel(cx, cy, currentColor);
    hasDisplayUpdate++;
    break;

  case CMD_DRAW: // draw line
    cx = display.getCursorX();
    cy = display.getCursorY();
    ex = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
    ey = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;

    display.drawLine(cx, cy, ex, ey, currentColor);
    display.setCursor(ex, ey);
    hasDisplayUpdate++;
    break;

  case CMD_RECT:
    cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
    cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;
    cw = (((uint16_t)mem[VDU_WH] * 256) + mem[VDU_WL]) % WIDTH;
    ch = (((uint16_t)mem[VDU_HH] * 256) + mem[VDU_HL]) % HEIGHT;

    //    Serial.printf("RECT %04x %04x %04x %04x\n", cx, cy, cw, ch);
    switch (mem[VDU_DMOD]) {
    case 0:
      display.drawRect(cx, cy, cw, ch, currentColor);
      break;
    default:
      display.fillRect(cx, cy, cw, ch, currentColor);
      break;
    }

    hasDisplayUpdate++;
    break;

  case CMD_CIRC:
    cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
    cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;
    cr = (((uint16_t)mem[VDU_WH] * 256) + mem[VDU_WL]) % WIDTH;

    //    Serial.printf("CIRC %04x %04x %04x\n", cx, cy, cr);
    switch (mem[VDU_DMOD]) {
    case 0:
      display.drawCircle(cx, cy, cr, currentColor);
      break;
    default:
      display.fillCircle(cx, cy, cr, currentColor);
      break;
    }

    hasDisplayUpdate++;
    break;

  case CMD_TRI:
    cx = (((uint16_t)mem[VDU_XH] * 256) + mem[VDU_XL]) % WIDTH;
    cy = (((uint16_t)mem[VDU_YH] * 256) + mem[VDU_YL]) % HEIGHT;
    ex = (((uint16_t)mem[VDU_WH] * 256) + mem[VDU_WL]) % WIDTH;
    ey = (((uint16_t)mem[VDU_HH] * 256) + mem[VDU_HL]) % WIDTH;

    //    Serial.printf("TRI %04x %04x %04x %04x\n", cx, cy, ex, ey);
    switch (mem[VDU_DMOD]) {
    case 0:
      display.drawTriangle(display.getCursorX(), display.getCursorY(), cx, cy, ex, ey, currentColor);
      break;
    default:
      display.fillTriangle(display.getCursorX(), display.getCursorY(), cx, cy, ex, ey, currentColor);
      break;
    }

    hasDisplayUpdate++;
    break;

  case CMD_PAL:
    display.setColor(mem[VDU_PAL], ((uint16_t)mem[VDU_BCOL] * 256) + mem[VDU_COL]);
    display.swap(false, true);
    break;

  case CMD_VDU: //  VDU control
    switch (mem[VDU_MOD] & 0x01) {
    case 1:
      showCursor(false);
      break;
    default:
      showCursor(true);
      break;
    }

    autoUpdate = !(mem[VDU_MOD] >> 7);
    autoScroll = !((mem[VDU_MOD] >> 1) & 0x01);
    break;

  case CMD_REFR: // REFRESH
    swapDisplay();
    break;

  case CMD_SANE:
    resetDisplay();
    break;

  case CMD_SPRITE:
    initSprite();
    break;

  case CMD_SDRAW:
    drawSprite();
    break;

  case CMD_SMOVE:
    moveSprite();
    break;

  case CMD_COLL:
    collSprite();
    break;

  case CMD_TILE:
    drawTile();
    break;
  }
#endif
}

/// <summary>
/// 
/// </summary>
void scanVDU() {
  uint8_t cmd = mem[VDU_CMD];

  if (cmd != 0x00) {
    setVDU(cmd);
    mem[VDU_CMD] = 0x00;
  }
}

/// <summary>
/// 
/// </summary>
/// <param name="c"></param>
/// <returns></returns>
inline __attribute__((always_inline))
void displayWrite(const uint8_t c) {
  int16_t cursor_x, cursor_y;
//  while (!dma_done);

  cursor_x = display.getCursorX();
  cursor_y = display.getCursorY();

  removeCursor();
  if (c == '\n') { // Carriage return
    cursor_x = 0;
  }
  else if ((c == '\r') || (cursor_x >= WIDTH)) { // Newline OR right edge
    cursor_x = 0;
    if (cursor_y >= (HEIGHT - 9)) { // Vert scroll?
      if (autoScroll) {
        memmove(display.getBuffer(), display.getBuffer() + WIDTH * (FONT_CHAR_HEIGHT + 1), WIDTH * (HEIGHT - (FONT_CHAR_HEIGHT + 1)));
        display.fillRect(0, HEIGHT - (FONT_CHAR_HEIGHT + 1), WIDTH, FONT_CHAR_HEIGHT + 1, 0);
        //        display.drawFastHLine(0, HEIGHT - 9, WIDTH, 0); // Clear bottom line
        cursor_y = HEIGHT - (FONT_CHAR_HEIGHT + 1);
      }
    }
    else {
      cursor_y += FONT_CHAR_HEIGHT + 1;
    }

    display.setCursor(cursor_x, cursor_y);
  }

  switch (c) {
  case 0:
  case '\r':
  case '\n':
    break;
  case 0x08:
  case 0x7F:
    if (cursor_x > 0) {
      display.fillRect(cursor_x - FONT_CHAR_WIDTH, cursor_y, FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT, 0);
      display.setCursor(cursor_x - FONT_CHAR_WIDTH, cursor_y);
    }
    break;
  case 0x20:
    display.fillRect(cursor_x, cursor_y, FONT_CHAR_WIDTH, FONT_CHAR_HEIGHT, 0);
    display.setCursor(cursor_x + FONT_CHAR_WIDTH, cursor_y);
    break;
  default:
    display.setTextColor(currentColor);
    display.write(c);
    break;
  }
  setCursor();

  hasDisplayUpdate++;
}

/// <summary>
///  scan for a char to output DVI output
/// </summary>
/// <param name="vChar"></param>
void scanChar() {
  uint8_t ch = mem[DSP];

  if (ch != 0x00) {
    if (traceOn) {
      Serial.printf("OUT [%02X]\n", ch);
    }

    displayWrite(ch);
//    hasDisplayUpdate++;

    mem[DSP] = 0x00;
  }
}

/// <summary>
/// 
/// </summary>
void swapDisplay() {
  if (hasDisplayUpdate > 0) {
#ifdef USE_DOUBLE_BUFFERING
    display.swap(true, false);
#endif
    hasDisplayUpdate = 0;
  }
}

/// <summary>
/// translate rgb to rgb565
/// </summary>
/// <param name="red"></param>
/// <param name="green"></param>
/// <param name="blue"></param>
/// <returns></returns>
inline __attribute__((always_inline))
uint16_t color565(const uint8_t red, const uint8_t green, const uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

/// <summary>
/// 
/// </summary>
void resetDisplay() {
  uint16_t colr, r, g, b;
  // load palette, convert from RGB888 to RGB565
  for (uint8_t c = 0; c <= 254; c++) {
    r = default_palette[c] >> 16;
    g = (default_palette[c] & 0x00FF00) >> 8;
    b = default_palette[c] & 0x0000FF;
//    colr = ((r & 0b11111000) << 8) + ((g & 0b11111100) << 3) + (b >> 3);
    colr = color565(r, g, b);
//    Serial.printf("PAL: %d %04x\n", c, colr);
    display.setColor(c,colr);
  }
#if 0
  display.setColor(0, 0x0000);   // Black
  display.setColor(1, 0XF800);   // Red
  display.setColor(2, 0x07e0);   // Green
  display.setColor(3, 0xffe0);   // Yellow
  display.setColor(4, 0x001f);   // Blue
  display.setColor(5, 0xFA80);   // Orange
  display.setColor(6, 0xF8F9);   // Magenta
#endif
  display.setColor(255, 0xFFFF); // Last palette entry = White
//  Serial.println("Default palette loaded");

  // Clear back framebuffer
  display.fillScreen(0);
  display.setFont();             // Use default font
  display.setCursor(0, 0);       // Initial cursor position
  display.setTextSize(1);        // Default size
  display.setTextWrap(false);
  display.swap(true, true);     // Duplicate same palette into front & back buffers

  mem[VDU_CMD] = 0x00;

  statusCursor = true;
  autoUpdate = true;
  autoScroll = true;
  setColor(currentTextColor);

  initSprites();
  initTiles(); // no tiles defined

  cmdClearDisplay();
}

///
void initDisplay() {
  if (!display.begin()) {
    Serial.println("ERROR: not enough RAM available");
    for (;;);
  }

  resetDisplay();
}


/// <summary>
/// 
/// </summary>
void helloDisplay() {
  // and we have lift off
  setColor(12); // BLUE
#if 0
  display.println("      N   N            666  5555   000   222");
  display.println("      N   N           6     5     0   0 2   2");
  display.println("      NN  N           6     5     0   0     2");
  display.println("      N N N EEEE  OO  6666  5555  0   0    2");
  display.println("      N  NN E    O  O 6   6     5 0   0   2");
  display.println("      N   N EEE  O  O 6   6     5 0   0  2");
  display.println("      N   N E    O  O 6   6 5   5 0   0 2");
  display.println("      N   N EEEE  OO   666   555   000  22222");
#endif
#if 1
  display.println("          N  N          66  555   00   22");
  display.println("          N  N         6    5    0  0 2  2");
  display.println("          NN N         6    5    0  0    2");
  display.println("          N NN EEE  O  666  555  0  0   2");
  display.println("          N NN E   O O 6  6    5 0  0  2");
  display.println("          N  N EE  O O 6  6    5 0  0 2");
  display.println("          N  N E   O O 6  6 5  5 0  0 2");
  display.println("          N  N EEE  O   66   55   00  2222");
#endif
//  display.print("NEO6502");
  setColor(255); // WHITE
  display.printf("\n                memulator %s\n\n", cVERSION);
  setColor(currentTextColor);

  display.swap(true, false);
}

/// <summary>
/// output to screen ala printf
/// </summary>
/// <param name="fmt"></param>
/// <param name="args"></param>
inline void vPrint(char const* fmt, va_list args) {
  // calculate required buffer length
  int msg_buf_size = vsnprintf(nullptr, 0, fmt, args) + 1; // add one for null terminator
  char msg_buf[msg_buf_size];

  vsnprintf(msg_buf, msg_buf_size, fmt, args);

#if 0
  Serial.print(msg_buf);
#else
  char *c = msg_buf;

  while (*c != 0x00) {
    displayWrite(*c);
    switch (*c++) {
    case '\n':
      displayWrite('\r');
      break;
    }
  }

  swapDisplay();
#endif
}

/// <summary>
/// 
/// </summary>
/// <param name="fmt"></param>
/// <param name=""></param>
void log(const char* fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vPrint(fmt, args);
  va_end(args);
}
