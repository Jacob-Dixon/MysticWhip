/******************************************************************************
RGB.cpp
RGB Library Source File
CJ Vaughter & Kevin Thomas
Original Creation Date: April 7, 2015
******************************************************************************/

#include "RGB.h"

RGBClass RGB;

void RGBClass::init()
{
	wakeup = false;
	rainbow = false;
	started = false;
	
	DDRC |= 0x07; //Set pins as output
	PORTC &= ~0x07; //Turn pins off

	TCCR2A = 0x02;		//CTC
	TCCR2B = 0x02;		//CTC & 8 prescale
	OCR2A = OCR2A_HZ;	//Counter value for desired frequency
	OCR2B = OCR2B_HZ;	//50% duty cycle
}

void RGBClass::start(bool temp)
{
	TCNT2 = 0;		//Reset counter
	TIMSK2 |= 0x02;	//Enable interrupt on match
	if(!temp) started = true;
}

void RGBClass::stop(bool temp)
{
	TIMSK2 &= ~0x02;	//Disable interrupt on match
	if(!temp) started = false;
}

void RGBClass::setColor(byte COLOR1, byte COLOR2, float _rate, bool _fade)
{
	if(started) stop(true);
	rainbow = false;

	switch (COLOR1)
	{
	case RED:     r1 = 255; g1 = 0;   b1 = 0;   break;
	case ORANGE:  r1 = 191; g1 = 63;  b1 = 0;   break;
	case YELLOW:  r1 = 127; g1 = 127; b1 = 0;   break;
	case DIMYELLOW:r1 = 63; g1 = 63;  b1 = 0;   break;
	case LIME:    r1 = 64;  g1 = 191; b1 = 0;   break;
	case GREEN:   r1 = 0;   g1 = 255; b1 = 0;   break;
	case SPRING:  r1 = 0;   g1 = 191; b1 = 63;  break;
	case CYAN:    r1 = 0;   g1 = 127; b1 = 127; break;
	case DIMCYAN: r1 = 0;   g1 = 63;  b1 = 63;  break;
	case AZURE:   r1 = 0;   g1 = 63;  b1 = 191; break;
	case BLUE:    r1 = 0;   g1 = 0;   b1 = 255; break;
	case DIMBLUE: r1 = 0;   g1 = 0;   b1 = 63;  break;
	case VIOLET:  r1 = 63;  g1 = 0;   b1 = 191; break;
	case MAGENTA: r1 = 127; g1 = 0;   b1 = 127; break;
	case ROSE:    r1 = 191; g1 = 0;   b1 = 63;  break;
	case WHITE:   r1 = 85;  g1 = 85;  b1 = 85;  break;
	case OFF:     r1 = 0;   g1 = 0;   b1 = 0;   break;
	}

	switch (COLOR2)
	{
	case RED:     r2 = 255; g2 = 0;   b2 = 0;   break;
	case ORANGE:  r2 = 191; g2 = 63;  b2 = 0;   break;
	case YELLOW:  r2 = 127; g2 = 127; b2 = 0;   break;
	case DIMYELLOW:r2 = 63; g2 = 63;  b2 = 0;   break;
	case LIME:    r2 = 63;  g2 = 191; b2 = 0;   break;
	case GREEN:   r2 = 0;   g2 = 255; b2 = 0;   break;
	case SPRING:  r2 = 0;   g2 = 191; b2 = 63;  break;
	case CYAN:    r2 = 0;   g2 = 127; b2 = 127; break;
	case DIMCYAN: r2 = 0;   g2 = 63;  b2 = 63;  break;
	case AZURE:   r2 = 0;   g2 = 63;  b2 = 191; break;
	case BLUE:    r2 = 0;   g2 = 0;   b2 = 255; break;
	case DIMBLUE: r2 = 0;   g2 = 0;   b2 = 63;  break;
	case VIOLET:  r2 = 63;  g2 = 0;   b2 = 191; break;
	case MAGENTA: r2 = 127; g2 = 0;   b2 = 127; break;
	case ROSE:    r2 = 191; g2 = 0;   b2 = 63;  break;
	case WHITE:   r2 = 85;  g2 = 85;  b2 = 85;  break;
	case OFF:     r2 = 0;   g2 = 0;   b2 = 0;   break;
	}

	r = r1; g = g1; b = b1;

	rate = (frequency / 2 / _rate);

	if (fade = _fade)
	{
		rs = abs(r1 - r2) / rate;
		gs = abs(g1 - g2) / rate;
		bs = abs(b1 - b2) / rate;
	}
	else
	{
		rs = gs = bs = 0;
	}

	rc = gc = bc = count = blinkcount = fadecount = 0;
	blinkstate = true;
	
	if(r1 > r2) rf = false; else rf = true;
	if(g1 > g2) gf = false; else gf = true;
	if(b1 > b2) bf = false; else bf = true;

	if(started) start(true);
}

void RGBClass::setColor(byte COLOR, bool _rainbow)
{
	if(started && !rainbow) stop(true);
	
	if(_rainbow)
	{
		rainbowcolor = COLOR;
		rainbowcolor++;
		if(rainbowcolor >= WHITE) rainbowcolor = RED;
		COLOR = rainbowcolor;
	}
	
	switch (COLOR)
	{
	case RED:     r = 255; g = 0;   b = 0;   break;
	case ORANGE:  r = 191; g = 63;  b = 0;   break;
	case YELLOW:  r = 127; g = 127; b = 0;   break;
	case DIMYELLOW:r = 63; g = 63;  b = 0;   break;
	case LIME:    r = 63;  g = 191; b = 0;   break;
	case GREEN:   r = 0;   g = 255; b = 0;   break;
	case SPRING:  r = 0;   g = 191; b = 63;  break;
	case CYAN:    r = 0;   g = 127; b = 127; break;
	case DIMCYAN: r = 0;   g = 63;  b = 63;  break;
	case AZURE:   r = 0;   g = 63;  b = 191; break;
	case BLUE:    r = 0;   g = 0;   b = 255; break;
	case DIMBLUE: r = 0;   g = 0;   b = 63;  break;
	case VIOLET:  r = 63;  g = 0;   b = 191; break;
	case MAGENTA: r = 127; g = 0;   b = 127; break;
	case ROSE:    r = 191; g = 0;   b = 63;  break;
	case WHITE:   r = 85;  g = 85;  b = 85;  break;
	case OFF:     r = 0;   g = 0;   b = 0;   break;
	}

	rc = gc = bc = count = 0;
	rate = 0.0f;
	fade = false;

	if(started && !rainbow) start(true);
	rainbow = _rainbow;
}

void RGBClass::isr()
{
	if (r == 0) ROFF;
	if (g == 0) GOFF;
	if (b == 0) BOFF;

	if (rc++ < r) { BOFF; GOFF; RON; }
	else if (gc++ < g) { ROFF; BOFF; GON; }
	else if (bc++ < b) { GOFF; ROFF; BON; }
	else { ROFF; GOFF; BOFF; }

	if (count++ == 255)
	{
		rc = gc = bc = 0;
		if(rainbow)
		{
			if(rainbowcount++ >= 4)
			{
				rainbowcount = 0;
				setColor(rainbowcolor, true);
			}
		}
		if (rate != 0.0f)
		{
			if (fade)
			{
				if (rf) r += rs; else r -= rs;
				if (gf) g += gs; else g -= gs;
				if (bf) b += bs; else b -= bs;
				
				if (r < 0) r = 0;
				if (g < 0) g = 0;
				if (b < 0) b = 0;
				
				if (fadecount++ >= rate) { fadecount = 0; rf = !rf; gf = !gf; bf = !bf; }
			}
			else
			{
				if (blinkcount++ >= rate)
				{
					blinkcount = 0;
					if (blinkstate = !blinkstate) { r = r1; g = g1; b = b1; }
					else { r = r2; g = g2; b = b2; }
				}
			}
		}
	}
	wakeup = true;
}

ISR(TIMER2_COMPA_vect)
{
	RGB.isr(); //color modification code
}