//  Author: avishorp@gmail.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __TM1637DISPLAY__
#define __TM1637DISPLAY__

#include <inttypes.h>

#define DIGIT_OFF	0b00000000
#define DIGIT_ZERO	0b00111111    // 0
#define DIGIT_ONE	0b00000110    // 1
#define DIGIT_TWO	0b01011011    // 2
#define DIGIT_THREE	0b01001111    // 3
#define DIGIT_FOUR	0b01100110    // 4
#define DIGIT_FIVE	0b01101101    // 5
#define DIGIT_SIX	0b01111101    // 6
#define DIGIT_SEVEN	0b00000111    // 7
#define DIGIT_EIGHT	0b01111111    // 8
#define DIGIT_NINE	0b01101111    // 9
#define DIGIT_A		0b01110111    // A
#define DIGIT_b		0b01111100    // b
#define DIGIT_C		0b00111001    // C
#define DIGIT_d		0b01011110    // d
#define DIGIT_E		0b01111001    // E
#define DIGIT_F		0b01110001    // F
#define DIGIT_H		0b01110110    // H
#define DIGIT_r		0b01010000    // r
#define DIGIT_i		0b00010000    // i

#define DIGIT_MINUS	0b01000000    // -

#define LIGHT_DARK		0x8
#define LIGHT_NORMAL	0x9
#define LIGHT_BRIGHT	0xa
#define LIGHT_BRIGHTEST	0xb

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D

#define SEG_A		0b00000001
#define SEG_B		0b00000010
#define SEG_C		0b00000100
#define SEG_D		0b00001000
#define SEG_E		0b00010000
#define SEG_F		0b00100000
#define SEG_G		0b01000000
#define SEG_DP		0b10000000    // : set on digit 1
#define SEG_COLON	0b10000000    // : set on digits 0, 2, 3

#define PATERN_DASHTOP		SEG_A
#define PATERN_DASHMIDDLE	SEG_G
#define PATERN_DASHBOTTOM	SEG_D
#define PATERN_COLUMNLEFT	SEG_F | SEG_E
#define PATERN_COLUMNRIGHT	SEG_B | SEG_C

class TM1637Display {

public:
  //! Initialize a TM1637Display object, setting the clock and
  //! data pins.
  //!
  //! @param pinClk - The number of the digital pin connected to the clock pin of the module
  //! @param pinDIO - The number of the digital pin connected to the DIO pin of the module
  TM1637Display(uint8_t pinClk, uint8_t pinDIO);
  TM1637Display(uint8_t pinClk, uint8_t pinLatch, uint8_t pinDIO, 
	  uint8_t pinD1, uint8_t pinD2, uint8_t pinD3, uint8_t pinD4);
  
  //! Sets the brightness of the display.
  //!
  //! The setting takes effect when a command is given to change the data being
  //! displayed.
  //!
  //! @param brightness A number from 0 (lowes brightness) to 7 (highest brightness)
  void setBrightness(uint8_t brightness);
  
  //! Display arbitrary data on the module
  //!
  //! This function receives raw segment values as input and displays them. The segment data
  //! is given as a byte array, each byte corresponding to a single digit. Within each byte,
  //! bit 0 is segment A, bit 1 is segment B etc.
  //! The function may either set the entire display or any desirable part on its own. The first
  //! digit is given by the @ref pos argument with 0 being the leftmost digit. The @ref length
  //! argument is the number of digits to be set. Other digits are not affected.
  //!
  //! @param segments An array of size @ref length containing the raw segment values
  //! @param length The number of digits to be modified
  //! @param pos The position from which to start the modification (0 - leftmost, 3 - rightmost)
  void setSegments(const uint8_t segments[], uint8_t length = 4, uint8_t pos = 0);
  void setPaternAll(uint8_t patern, bool dig1 = true, bool dig2 = true, bool dig3 = true, bool dig4 = true);
  void setPatern(uint8_t dig1Patern, uint8_t dig2Patern, uint8_t dig3Patern, uint8_t dig4Patern);
  
  //! Displayes a decimal number
  //!
  //! Dispalyes the given argument as a decimal number
  //!
  //! @param num The number to be shown
  //! @param leading_zero When true, leading zeros are displayed. Otherwise unnecessary digits are
  //!        blank
  //! @param length The number of digits to set. The user must ensure that the number to be shown
  //!        fits to the number of digits requested (for example, if two digits are to be displayed,
  //!        the number must be between 0 to 99)
  //! @param pos The position least significant digit (0 - leftmost, 3 - rightmost)
  void showNumberDec(int num, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);
  void showNumberFloat(float num, int prec = -1, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);
  void showNumberInt(int num, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);
  void showString(char *str);

  void showTime(int left, int right, bool colon, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);

  void clear(bool colon = false, bool dp1 = false, bool dp2 = false, bool dp3 = false, uint8_t length = 4);

  //! Translate a single digit into 7 segment code
  //!
  //! The method accepts a number between 0 - 15 and converts it to the
  //! code required to display the number on a 7 segment display.
  //! Numbers between 10-15 are converted to hexadecimal digits (A-F)
  //!
  //! @param digit A number between 0 to 15
  //! @return A code representing the 7 segment image of the digit (LSB - segment A;
  //!         bit 6 - segment G; bit 7 - always zero)
  uint8_t encodeDigit(uint8_t digit);
  uint8_t selectDigit(char digit);
  void update();

protected:
   void bitDelay();
   
   void start();
   
   void stop();
   
   bool writeByte(uint8_t b);
   
private:
	bool m_modeModule;
	bool m_modeSN74HC595;
	uint8_t m_pinClk;
	uint8_t m_pinDIO;
	uint8_t m_pinLatch;
	uint8_t m_brightness;

	uint8_t m_digits[4];
	uint8_t m_pinD[4];
	uint8_t m_digitScan;
};

#endif // __TM1637DISPLAY__
