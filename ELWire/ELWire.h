/******************************************************************************
ELWire.h
ELWire Library Header File
CJ Vaughter & Kevin Thomas
Original Creation Date: March 11, 2015
******************************************************************************/

#ifndef ELWire_h
#define ELWire_h
#include <Arduino.h>
#include "I2C.h"

const long OCR2A_60 = F_CPU / 1024L / 61L - 1;
const long OCR2B_60 = (OCR2A_60 + 1) / 2L - 1;

enum MAX14521E_Registers : byte
{
	DEV_ID		= 0x00,
	PWR_MODE	= 0x01,
	EL_FREQ 	= 0x02,
	EL_SHAPE	= 0x03,
	BC_FREQ		= 0x04,
	AUDIO		= 0x05,
	EL_1 		= 0x06,
	EL_2 		= 0x07,
	EL_3 		= 0x08,
	EL_4 		= 0x09,
};

class MAX14521E
{
public:
	void init(byte addr = 0x00, void (*handler)(void) = NULL);
	void setColor(byte r, byte g, byte b);
	//void setColor(byte* c);
	//void setColor(byte* _c1, byte* _c2, float _rate = 0.0f, bool _fade = true);
	void setFreq(byte _freq);
	void start();
	void stop(bool disable = false);
	void send();
	void (*isr)();
private:
	byte address;
	byte freq;
	byte bc_freq;
	byte color[4];
	volatile bool send_freq;
	volatile bool blink;
	volatile bool fade;
};

extern MAX14521E ELWire;

#endif