/*
* The MIT License (MIT)
*
* Copyright (c) JEMRF
* Copyright (c) 2020 Bastian Maerkisch
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
*
********************************************************************************
*
* Module         : SevSeg_MAX7219.cpp
* Original Author: Jonathan Evans
* Description    : MAX7219 LED Display Driver
*
* The MAX7219/MAX7221 are compact, serial input/output common-cathode display 
* drivers that interface microprocessors (µPs) to 7-segment numeric LED displays
* of up to 8 digits, bar-graph displays, or 64  individual LEDs
* Datasheet  : https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
*
* Library Description
*
*  - This library implements the 7-segment numeric LED display of 8 digits
*  - The host communicates with the MAX7219 using three signals: CLK, CS, DIN.
*  - Pins can be configured in the constructor
*  - The MAX7219 is a SPI interface
*  - This library uses the bitbang method for communication with the MAX7219
*
* Usage
*
* Three methods are exposed for use:
*
*  1. begin
*  This method initializes communication, takes the display out of test mode, clears the screen and sets intensity.
*  Intensity is set at maximum but can be configured in max7219.h
*
*  2. displayChar(Digit, Value, DP)
*  This method displays a single value (character) in position DIGIT (0=right most digit, 7=left most digit)
*
*  3. displayText(Text, Justify)
*  This method displays a text string (Text) either right justified (Justify=1) or left justified (Justify=0)
*/

#include <avr/pgmspace.h> 
#include "SevSeg_MAX7219.h"

//MAX7219
const byte MAX7219_REG_NOOP          = 0x00;
const byte MAX7219_REG_DECODE        = 0x09;
const byte MAX7219_REG_INTENSITY     = 0x0a;
const byte MAX7219_REG_SCAN_LIMIT    = 0x0b;
const byte MAX7219_REG_SHUTDOWN      = 0x0c;
const byte MAX7219_REG_DISPLAY_TEST  = 0x0f;

#define INTENSITY_MIN     0x00
#define INTENSITY_MAX     0x0f


SevSeg_MAX7219::SevSeg_MAX7219(byte _dinPin, byte _clkPin, byte _csPin) :
  dinPin(_dinPin), clkPin(_clkPin), csPin(_csPin),
  digits(4), pos(0), autoscrolling(false)
{
  pinMode(dinPin, OUTPUT);
  pinMode(csPin, OUTPUT);
  pinMode(clkPin, OUTPUT);
}

void SevSeg_MAX7219::begin(byte digits)
{
  digitalWrite(csPin, HIGH);

  if (digits < 4) digits = 4;
  writeSPI(MAX7219_REG_SCAN_LIMIT, digits);

  // Turn BCD decoding off for all digits.
  writeSPI(MAX7219_REG_DECODE, 0x00);

  clear();
  noTestMode();

  brightness(INTENSITY_MAX);

  // Turn on display last.
  display();
}

void SevSeg_MAX7219::clear(void) {
  for (int i = 0; i < 8; i++) {
    buf[i] = 0x00;
    writeSPI(i + 1, 0x00);
  }
  pos = 0;
}

void SevSeg_MAX7219::display(void)
{
  // normal operation
  writeSPI(MAX7219_REG_SHUTDOWN, 1);
}

void SevSeg_MAX7219::noDisplay(void)
{
  // shutdown mode
  writeSPI(MAX7219_REG_SHUTDOWN, 0);
}

void SevSeg_MAX7219::autoScroll(void)
{
  autoscrolling = true;
}

void SevSeg_MAX7219::noAutoScroll(void)
{
  autoscrolling = false;
}

void SevSeg_MAX7219::testMode(void)
{
  writeSPI(MAX7219_REG_DISPLAY_TEST, 1);
}

void SevSeg_MAX7219::noTestMode(void)
{
  writeSPI(MAX7219_REG_DISPLAY_TEST, 0);
}

void SevSeg_MAX7219::brightness(byte brightness)
{
  brightness &= 0x0f;
  writeSPI(MAX7219_REG_INTENSITY, brightness);
}

void SevSeg_MAX7219::home(void)
{
  pos = 0;
}

void SevSeg_MAX7219::setCursor(byte x, byte y)
{
  pos = x;
}

size_t SevSeg_MAX7219::write(uint8_t ch)
{
  // special handling of dots/fullstops.
  if (ch == '.') {
    // add dp to previous symbol
    byte p = (pos > 0) ? pos - 1 : 0;
    buf[p] |= 0x80;
    writeSPI(p + 1, buf[p]);
    return 1;
  }
  if (autoscrolling && pos == digits) {
    for (byte i = 0; i < digits - 1; i++) {
      buf[i] = buf[i + 1];
      writeSPI(i + 1, buf[i]);
    }
    displayChar(digits - 1, ch, false);
  } else {
    displayChar(pos++, ch, false);
  }
  return 1;
}

void SevSeg_MAX7219::displayChar(char digit, char value, bool dp)
{
  byte code = lookup(value, dp);
  buf[int(digit)] = code;
  writeSPI(digit + 1, code);
}

void SevSeg_MAX7219::displayText(const char *text, bool rightjustify)
{
  bool decimal[16];
  char trimStr[16] = "";
  int x, y = 0;
  int s;

  s = strlen(text);
  if (s > 16) s = 16;
  for (x = 0; x < s; x++) {
    if (text[x] == '.') {
      decimal[y - 1] = true;
    } else {
      trimStr[y] = text[x];
      decimal[y] = false;
      y++;
    }
  }
  if (y > digits) y = digits;

  for (x = 0; x < y; x++) {
    if (!rightjustify)
      displayChar(x, trimStr[x], decimal[x]);
    else
      displayChar(digits - y + x, trimStr[x], decimal[x]);
  }
}

void SevSeg_MAX7219::writeSPI(byte opcode, byte data)
{
  // FIXME: optionally use hardware SPI
  digitalWrite(csPin, LOW);
  shiftOut(dinPin, clkPin, MSBFIRST, opcode);
  shiftOut(dinPin, clkPin, MSBFIRST, data);
  digitalWrite(csPin, HIGH);
}

byte SevSeg_MAX7219::lookup(char c, bool dp)
{
  byte pat;

  // hex encoded values:  MSB is segment A, LSB segment P
  const static byte pattern[94] PROGMEM = {
    0B00000000, 0B01100001, 0B01000100, 0B01101110, 0,           // space!"#$
    0,          0,          0B01000000, 0B10011100, 0B11110000,  // %&'()
    0,          0,          0,          0B00000010, 0B00000001,  // *+,-.
    0,                                                           // /
    0xfc, 0x60, 0xda, 0xf2, 0x66,  // 0-4
    0xb6, 0xbe, 0xe0, 0xfe, 0xf6,  // 5-9
    0,          0,          0,          0B00010010, 0,           // :;<=>
    0B11001011, 0B11111010,                                      // ?@
    0B11101110, 0B11111110, 0B10011100, 0B01111010, 0B10011110,  // A-E
    0B10001110, 0B10111100, 0B01101110, 0B01100000, 0B01110000,  // F-J
    0B10101110, 0B00011100, 0B10101000, 0B11101100, 0B11111100,  // K-O
    0B11001110, 0B11100110, 0B00001010, 0B10110110, 0B00011110,  // P-T
    0B01111100, 0,          0,          0,          0B01110110,  // U-Y
    0B11011010,                                                  // Z
    0B10011100, 0B00000100, 0B11110000, 0,          0B00010000,  // [\]^_
    0B01000000,                                                  // '
    0B11111010, 0B00111110, 0B00011010, 0B01111010, 0B11011110,  // a-e
    0B10001110, 0B11110110, 0B00101110, 0B00001000, 0B00110000,  // f-j
    0B10101110, 0B00001100, 0B10101000, 0B00101010, 0B00111010,  // k-o
    0B11001110, 0B11100110, 0B00001010, 0B10110110, 0B00011110,  // p-t
    0B00111000, 0,          0,          0,          0B01110110,  // u-y
    0B11011010,                                                  // z
    0B10011100, 0B00001100, 0B11110000                           // {|}
  };
/*
    case '°': // not ASCII
      pattern = 0B11000110;
      break;
*/
  // 0B01111000  // alternative capital J
  if (c >= ' ' && c <= '}') {
    // pat = pattern[(int) c];
    pat = pgm_read_byte_near(pattern + (int) c - ' ');
    pat = (pat >> 1) | (pat << 7);
  }
  if (dp) pat |= 0x80;
  return pat;
}
