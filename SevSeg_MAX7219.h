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
 *********************************************************************************
 *
 * Module         : SevSeg_MAX7219.h
 * Original Author: Jonathan Evans
 * Description    : MAX7219 LED Display Driver
 *
 ********************************************************************************
 */

#ifndef SevSeg_MAX7219_h
#define SevSeg_MAX7219_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/*
*********************************************************************************************************
* LED Segments:         a
*                     ----
*                   f|    |b
*                    |  g |
*                     ----
*                   e|    |c
*                    |    |
*                     ----  o dp
*                       d
*   Register bits:
*      bit:  7  6  5  4  3  2  1  0
*           dp  a  b  c  d  e  f  g
*********************************************************************************************************
* Example : The letter 'I' is represented by illuminating LED's 'b' and 'c' (refer above diagram)
*           Therefore the binary representation of 'I' is as follows
*
*           abcdefg
*           0110000
*
*           The table below contains all the binary values for the desired font. New font characters
*           can be added or altered as required.
*
*           The DP bit is used to switch on the decimal place LED. DP is not included in the below table
*           but is added in the register within the library depending on the content being displayed.
*********************************************************************************************************

*/

#include <Print.h>


class SevSeg_MAX7219 : public Print
{
public:

  SevSeg_MAX7219(byte _dinPin, byte _clkPin, byte _csPin);

  void begin(byte ndigits = 4);
  void clear(void);

  void brightness(byte brightness);
  void display(void);
  void noDisplay(void);

  void home(void);
  void setCursor(byte x, byte y = -1);
  // void print();
  void autoScroll(void);
  void noAutoScroll(void);

  void displayChar(char digit, char character, bool dp);
  void displayText(const char * text, bool rightjustify = false);

  void testMode(void);
  void noTestMode(void); 

  // Print class support
  virtual size_t write(uint8_t);

protected:

  byte dinPin;
  byte clkPin;
  byte csPin;

  byte digits;        // number of digits (starting at 0, max 8)
  byte pos;           // virtual cursor position
  bool autoscrolling; // automatically scroll at the end of the display
  bool justify;       // right justify text?
  char buf[8];        // current 7 segment contents

  void writeSPI(byte opcode, byte data);
  byte lookup(char c, bool dp);

};

#endif
