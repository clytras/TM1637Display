
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

extern "C" {
  #include <stdlib.h>
  #include <string.h>
  #include <inttypes.h>
}

#include <TM1637Display.h>
#include <Arduino.h>
#include <math.h>
#include "ZKRTypes.h"

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
const uint8_t digitToSegment[] = {
 // XGFEDCBA
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // b
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001,    // F
  0b01000000     // -
};


TM1637Display::TM1637Display(uint8_t pinClk, uint8_t pinDIO) :
m_modeModule(true), m_modeSN74HC595(false)
{
	// Copy the pin numbers
	m_pinClk = pinClk;
	m_pinDIO = pinDIO;
	
	// Set the pin direction and default value.
	// Both pins are set as inputs, allowing the pull-up resistors to pull them up
    pinMode(m_pinClk, INPUT);
    pinMode(m_pinDIO,INPUT);
	digitalWrite(m_pinClk, LOW);
	digitalWrite(m_pinDIO, LOW);
}

TM1637Display::TM1637Display(uint8_t pinClk, uint8_t pinLatch, uint8_t pinDIO,
	uint8_t pinD1, uint8_t pinD2, uint8_t pinD3, uint8_t pinD4) :
	m_modeModule(false), m_modeSN74HC595(true), m_digitScan(0)
{
	m_pinClk = pinClk;
	m_pinDIO = pinDIO;
	m_pinLatch = pinLatch;

	m_pinD[0] = pinD1;
	m_pinD[1] = pinD2;
	m_pinD[2] = pinD3;
	m_pinD[3] = pinD4;

	for (int i = 0; i < 4; i++) {
		pinMode(m_pinD[i], OUTPUT);
		digitalWrite(m_pinD[i], HIGH);
	}
}

void TM1637Display::setBrightness(uint8_t brightness)
{
	m_brightness = brightness;
}

void TM1637Display::clear(bool colon /*= false*/,
						  bool dp1 /*= false*/,
						  bool dp2 /*= false*/,
						  bool dp3 /*= false*/,
						  uint8_t length /*= 4*/)
{
	// (index 0) -0-- 3rd rtl digit decimal point
	// (index 1) :    colon
	// (index 2) --0- 2nd rtl digit decimal point
	// (index 3) ---0 1st rtl digit decimal point

	uint8_t digits[4] = {
		dp3 ? SEG_DP : DIGIT_OFF,
		colon ? SEG_COLON : DIGIT_OFF,
		dp2 ? SEG_DP : DIGIT_OFF,
		dp1 ? SEG_DP : DIGIT_OFF
	};

	setSegments(digits + (4 - length), length, 0);
}

void TM1637Display::setPaternAll(uint8_t patern,
								 bool dig1 /*= true*/,
								 bool dig2 /*= true*/,
								 bool dig3 /*= true*/,
								 bool dig4 /*= true*/)
{
	uint8_t digits[4] = {
		dig4 ? patern : DIGIT_OFF,
		dig3 ? patern : DIGIT_OFF,
		dig2 ? patern : DIGIT_OFF,
		dig1 ? patern : DIGIT_OFF
	};

	setSegments(digits, 4, 0);
}

void TM1637Display::setPatern(uint8_t dig1Patern,
							  uint8_t dig2Patern,
							  uint8_t dig3Patern,
							  uint8_t dig4Patern)
{
	uint8_t digits[4] = {
		dig4Patern,
		dig3Patern,
		dig2Patern,
		dig1Patern
	};

	setSegments(digits, 4, 0);
}

void TM1637Display::setSegments(const uint8_t segments[], uint8_t length, uint8_t pos)
{
	if(m_modeModule) {
		// Write COMM1
		start();
		writeByte(TM1637_I2C_COMM1);
		stop();

		// Write COMM2 + first digit address
		start();
		writeByte(TM1637_I2C_COMM2 + (pos & 0x03));

		// Write the data bytes
		for (uint8_t k = 0; k < length; k++)
			writeByte(segments[k]);

		stop();

		// Write COMM3 + brightness

		start();
		writeByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
		stop();
	}
	else if(m_modeSN74HC595) {
		for (uint8_t k = 0; k < length; k++)
			m_digits[k] = segments[k];
	}
}

void TM1637Display::update()
{
	if(m_modeSN74HC595) {
		byte k;

		for (k = 0; k < 4; k++)
		{
			digitalWrite(m_pinD[k], LOW);
		}

		digitalWrite(m_pinLatch, LOW);
		shiftOut(m_pinDIO, m_pinClk, MSBFIRST, B11111111);
		digitalWrite(m_pinLatch, HIGH);
		delayMicroseconds(50);
		digitalWrite(m_pinD[m_digitScan], HIGH);

		//for (k = 0; k < 4; k++) {
		//	digitalWrite(m_pinD[k], LOW);
			digitalWrite(m_pinLatch, LOW);
			shiftOut(m_pinDIO, m_pinClk, MSBFIRST, m_digits[k]);
			//Serial.println(m_digits[k]);
			digitalWrite(m_pinLatch, HIGH);
			//digitalWrite(m_pinD[k], HIGH);
		//}

		if (++m_digitScan > 3) m_digitScan = 0;
	}
}
 
void TM1637Display::showNumberDec(int num, bool leading_zero, uint8_t length, uint8_t pos)
{
	uint8_t digits[4];
	const static int divisors[] = { 1, 10, 100, 1000 };
	bool leading = true;
	
	for(int8_t k = 0; k < 4; k++) {
	    int divisor = divisors[4 - 1 - k];
		int d = num / divisor;
		
		if (d == 0) {
		  if (leading_zero || !leading || (k == 3))
		    digits[k] = encodeDigit(d);
	      else
		    digits[k] = 0;
		}
		else {
			digits[k] = encodeDigit(d);
			num -= d * divisor;
			leading = false;
		}
	}
	
	setSegments(digits + (4 - length), length, pos);
}

void TM1637Display::showNumberInt(int num, bool leading_zero, uint8_t length, uint8_t pos)
{
	uint8_t digits[4] = { 0, 0, 0, 0 };
	int dec = num, num_digits;
	if ((num > 9999) || (num < -999))return;
	if (num < 0)
	{
		num_digits = 3;

		if (num >= -9)
		{
			Serial.println(" >= -9");
			digits[2] = DIGIT_MINUS;
			num_digits = 1;
		}
		else if (num >= -99)
		{
			Serial.println(" >= -99");
			digits[1] = DIGIT_MINUS;
			num_digits = 2;
		}
		else
		{
			Serial.println(" else");
			digits[0] = DIGIT_MINUS;
			num_digits = 3;
		}

		//temp[0] = INDEX_NEGATIVE_SIGN;
		dec = abs(num);

		if (num_digits >= 3)
			digits[1] = encodeDigit(dec / 100);

		dec %= 100;

		if (num_digits >= 2)
			digits[2] = encodeDigit(dec / 10);

		if (num_digits >= 1)
			digits[3] = encodeDigit(dec % 10);

		/*if (BlankingFlag)
		{
			if (temp[1] == 0)
			{
				temp[1] = INDEX_BLANK;
				if (temp[2] == 0) temp[2] = INDEX_BLANK;
			}
		}*/
	}
	else
	{
		digits[0] = encodeDigit(dec / 1000);
		dec %= 1000;
		digits[1] = encodeDigit(dec / 100);
		dec %= 100;
		digits[2] = encodeDigit(dec / 10);
		digits[3] = encodeDigit(dec % 10);

		if (!leading_zero)
		{
			if (digits[0] == DIGIT_ZERO)
			{
				digits[0] = 0;
				if (digits[1] == DIGIT_ZERO)
				{
					digits[1] = 0;
					if (digits[2] == DIGIT_ZERO) digits[2] = 0;
				}
			}
		}
	}

	setSegments(digits + (4 - length), length, pos);
}

void TM1637Display::showNumberFloat(float num,
									int prec /*= -1*/,
									bool leading_zero /*= false*/,
									uint8_t length /*= 4*/,
									uint8_t pos /*= 0*/)
{
	String str = String(num);
	bool isLessZero = num < 0;
	num = abs(num);

	if (prec == -1)
	{
		// right trim 0
		while (str.endsWith("0"))
			str.remove(str.length() - 1);
	}
	else
		str = String(num, prec);
	
	if (isLessZero) str = "-" + str;
	showString((char*)str.c_str());
}

void TM1637Display::showString(char *str)
{
	const uint8_t length = 4;
	uint8_t digits[length] = { 0, 0, 0, 0 },
		strsize = sizeofsz(str);
	int	digit = 3;
	char c;

	//for (int digit = length - strsize, c = 0; digit < length; digit++, c++)
	for (int i = strsize - 1; i >= 0; i--)
	{
		c = str[i];
		if (c == '.')
		{
			if (digit == 1)
				digits[0] |= SEG_DP;
			else if (digit == 3 || digit == 2)
				digits[digit] |= SEG_DP;

			continue;
		}

		digits[digit] |= selectDigit(c);
		digit--;

		if (digit < 0) break;
	}

	setSegments(digits, length, 0);
}



void TM1637Display::showTime(int left,
							 int right, 
							 bool colon, 
							 bool leading_zero /*= false*/,
							 uint8_t length /*= 4*/,
							 uint8_t pos /*= 0*/)
{
	uint8_t digits[4];

	if (left > 99) left = 99;
	//else if (left < 0) left = 0;
	if (right > 99) right = 99;
	//else if (right < 0) right = 0;

	if (left == -1) {
		digits[0] = DIGIT_OFF;
		digits[1] = DIGIT_OFF;
	}
	else {
		digits[0] = encodeDigit(left / 10);
		digits[1] = encodeDigit(left % 10);
	}

	if (right == -1) {
		digits[2] = DIGIT_OFF;
		digits[3] = DIGIT_OFF;
	} else {
		digits[2] = encodeDigit(right / 10);
		digits[3] = encodeDigit(right % 10);
	}

	if (colon)
		digits[1] |= SEG_COLON;

	if (!leading_zero && left <= 9)
		digits[0] = 0;

	setSegments(digits + (4 - length), length, pos);
}

void TM1637Display::bitDelay()
{
	delayMicroseconds(50);
}
   
void TM1637Display::start()
{
  pinMode(m_pinDIO, OUTPUT);
  bitDelay();
}
   
void TM1637Display::stop()
{
	pinMode(m_pinDIO, OUTPUT);
	bitDelay();
	pinMode(m_pinClk, INPUT);
	bitDelay();
	pinMode(m_pinDIO, INPUT);
	bitDelay();
}
  
bool TM1637Display::writeByte(uint8_t b)
{
  uint8_t data = b;

  // 8 Data Bits
  for(uint8_t i = 0; i < 8; i++) {
    // CLK low
    pinMode(m_pinClk, OUTPUT);
    bitDelay();
    
	// Set data bit
    if (data & 0x01)
      pinMode(m_pinDIO, INPUT);
    else
      pinMode(m_pinDIO, OUTPUT);
    
    bitDelay();
	
	// CLK high
    pinMode(m_pinClk, INPUT);
    bitDelay();
    data = data >> 1;
  }
  
  // Wait for acknowledge
  // CLK to zero
  pinMode(m_pinClk, OUTPUT);
  pinMode(m_pinDIO, INPUT);
  bitDelay();
  
  // CLK to high
  pinMode(m_pinClk, INPUT);
  bitDelay();
  uint8_t ack = digitalRead(m_pinDIO);
  if (ack == 0)
    pinMode(m_pinDIO, OUTPUT);
	
	
  bitDelay();
  pinMode(m_pinClk, OUTPUT);
  bitDelay();
  
  return ack;
}

uint8_t TM1637Display::encodeDigit(uint8_t digit)
{
	return digitToSegment[digit /*& 0x0f*/];
}

uint8_t TM1637Display::selectDigit(char digit)
{
	char strDigit[] = { digit, 0 };

	switch (digit)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case 'a':
	case 'A':
	case 'b':
	case 'B':
	case 'c':
	case 'C':
	case 'd':
	case 'D':
	case 'e':
	case 'E':
	case 'f':
	case 'F':
		return encodeDigit((uint8_t)strtol((const char*)&strDigit, NULL, 16));
	case 'H':
		return DIGIT_H;
	case 'r':
		return DIGIT_r;
	case 'i':
		return DIGIT_i;
	case '-':
		return encodeDigit(16);
	default:
		return DIGIT_OFF;
	}
}

