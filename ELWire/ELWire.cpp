/******************************************************************************
ELWire.cpp
ELWire Library Source File
CJ Vaughter & Kevin Thomas
Original Creation Date: March 11, 2015
******************************************************************************/

#include "ELWire.h"

MAX14521E ELWire;

void MAX14521E::init(byte addr, void (*handler)(void))
{
	I2C.begin();
	address = addr;
	isr = handler;
	
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	freq = 0x3F;
	bc_freq = 0x0F;
	send_freq = true;

	if(isr)
	{
		TCCR2A = 0x02;		//CTC
		TCCR2B = 0x07;		//CTC & 1024 prescale
		OCR2A =  OCR2A_60;	//60 Hz
		OCR2B =  OCR2B_60;	//50% duty
	}
}

void MAX14521E::setColor(byte r, byte g, byte b)
{
	color[0] = 0;
	color[1] = r;
	color[2] = g;
	color[3] = b;
	if(!isr) I2C.writeRegs(address, EL_1, &color[0], (byte)4);
}
/*
void MAX14521E::setColor(byte &c[3])
{
	//color[0] = 0;
	//color[1] = c[0];
	//color[2] = c[1];
	//color[3] = c[2];
	//rate = 0;
	//if(!isr) I2C.writeRegs(address, EL_1, &color[0], (byte)4);
}

void setColor(byte &_c1[3], byte &_c2[3], float _rate, bool _fade)
{
	//c1[0] = _c1[0]; c1[1] = _c1[1]; c1[2] = _c1[2];
	//c2[0] = _c2[0]; c2[1] = _c2[1]; c2[2] = _c2[2];
	//rate = 1 / _rate * 30;
	//fade = _fade;
}
*/
void MAX14521E::setFreq(byte _freq)
{
	freq = _freq;
	send_freq = true;
	if(!isr) I2C.writeReg(address, EL_FREQ, freq);
}

void MAX14521E::start()
{
	if(isr)
	{
		TCNT2  = 0;		//Reset counter
		TIMSK2 |= 0x02;	//Enable interrupt on match
	}
	I2C.writeReg(address, PWR_MODE, 0x01);
	I2C.writeReg(address, BC_FREQ, bc_freq);
}

void MAX14521E::stop(bool disable)
{
	if(isr) TIMSK2 &= 0xFD;	//Disable interrupt on match
	if(disable) I2C.writeReg(address, PWR_MODE, 0x00);
}

void MAX14521E::send()
{
	if(send_freq)
	{
		send_freq = false;
		I2C.writeReg(address, EL_FREQ, freq);
	}
	I2C.writeRegs(address, EL_1, &color[0], (byte)4);
}

ISR(TIMER2_COMPA_vect)
{
	ELWire.isr(); //color modification code
	ELWire.send(); 
}