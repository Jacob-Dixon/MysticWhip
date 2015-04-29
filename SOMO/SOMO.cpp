/******************************************************************************
SOMO.cpp
SOMO Library Source File
Jacob Dixon & CJ Vaughter
Original Creation Date: March 22, 2015
******************************************************************************/

#include "SOMO.h"
#include <avr/power.h>

SOMOClass SOMO;

int soundlength[1][9] =
{
	//FAIL,	DAMAGE,	STUN,	HEAL,	READY,	STARTUP, NULL, NULL, ANNIHILATION
	{5174,	500,	1556,	2021,	1000,	1404, 	 0,	   0,    4800},
};

void SOMOClass::init()
{
	power_usart0_enable();
    Serial.begin(9600);
	setVolume(VOLUME_MAX);
    setEQ(EQ_ROCK);
}

void SOMOClass::setVolume(byte vol)
{
#ifndef DEBUGSOMO
	while(true)
	{
		send(VOL, 1, 0, vol, false);
		Serial.readBytes(packet, 8);
		if(packet[0] == 0x7E && packet[4] == 0x00 && packet[7] == 0xEF) break;
	}
#else
	send(VOL, 0, 0, vol, false);
#endif
}

void SOMOClass::setEQ(byte eq)
{
	send(EQ, 0, 0, eq);
}

void SOMOClass::play(byte folder, byte track, bool wait)
{
	if(folder == 0 || track == 0) send(PLAY);
	else
	{
		send(TRACK, 0, folder, track);
		if(wait) delay(soundlength[folder-1][track-1]);
	}
	
}

void SOMOClass::pause()
{
	send(PAUSE);
}

void SOMOClass::stop()
{
	send(STOP);
}

void SOMOClass::send(byte cmd, byte feedback, byte para1, byte para2, bool off)
{
	unsigned short chksum = 0xFFFF - (cmd + feedback + para1 + para2) + 1;
	
	packet[0] = cmdBegin;
	packet[1] = cmd;
	packet[2] = feedback;
	packet[3] = para1;
	packet[4] = para2;
	packet[5] = (byte)(chksum>>8);
	packet[6] = (byte)chksum;
	packet[7] = cmdEnd;
	
	power_usart0_enable();
	Serial.write(packet,8);
	Serial.flush();
	if(off) power_usart0_disable();
}