/******************************************************************************
RGB.h
RGB Library Header File
CJ Vaughter & Kevin Thomas
Original Creation Date: April 7, 2015
******************************************************************************/

#ifndef RGB_h
#define RGB_h
#include <Arduino.h>

#define RON	 (PORTC |= 0x01)
#define ROFF (PORTC &= 0xFE)
#define GON	 (PORTC |= 0x02)
#define GOFF (PORTC &= 0xFD)
#define BON	 (PORTC |= 0x04)
#define BOFF (PORTC &= 0xFB)

const float frequency = 120.0f;

const long OCR2A_HZ = F_CPU / 8L / frequency / 256L - 1;
const long OCR2B_HZ = (OCR2A_HZ + 1) / 2L - 1;

enum COLORS : byte
{
	OFF,
	RED,
	ORANGE,
	YELLOW,
	LIME,
	GREEN,
	SPRING,
	CYAN,
	AZURE,
	BLUE,
	VIOLET,
	MAGENTA,
	ROSE,
	WHITE,
	DIMYELLOW,
	DIMCYAN,
	DIMBLUE,
};


class RGBClass
{
public:
	void init();
	void start(bool temp = false);
	void stop(bool temp = false);
	void setColor(byte COLOR, bool _rainbow = false);
	void setColor(byte COLOR1, byte COLOR2, float _rate, bool _fade = false);
	void isr();
	volatile bool wakeup;
private:
	volatile short r, g, b;
	volatile byte rc, gc, bc;
	volatile byte count, blinkcount, fadecount, rainbowcolor, rainbowcount;
	volatile bool blinkstate, rf, gf, bf;
	byte r1, r2, g1, g2, b1, b2;
	short rs, gs, bs;
	float rate;
	bool fade, rainbow, started;
};

extern RGBClass RGB;

#endif