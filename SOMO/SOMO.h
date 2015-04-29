/******************************************************************************
SOMO.h
SOMO Library Header File
Jacob Dixon & CJ Vaughter
Original Creation Date: March 22, 2015
******************************************************************************/

#ifndef SOMO_h
#define SOMO_h
#include <Arduino.h>

//#define DEBUGSOMO

const byte cmdBegin = 0x7E;
const byte cmdEnd = 0xEF;
const byte VOLUME_MAX = 30;

enum SOMOCommands : byte
{
	VOL		= 0x06,
	EQ		= 0x07,
	PLAY	= 0x0D,
	PAUSE	= 0x0E,
	TRACK	= 0x0F,
	STOP	= 0x16,
};

enum SOMOEQ : byte
{
	EQ_NORMAL	 = 0,
	EQ_POP		 = 1,
	EQ_ROCK		 = 2,
	EQ_JAZZ		 = 3,
	EQ_CLASSICAL = 4,
	EQ_BASS		 = 5,
};

class SOMOClass
{
public:
	void init();
	void setVolume(byte vol);
	void setEQ(byte eq);
	void play(byte folder = 0, byte track = 0, bool wait = true);
	void pause();
	void stop();
private:
	void send(byte cmd, byte feedback = 0, byte para1 = 0, byte para2 = 0, bool off = true);
	byte packet[8];
};

extern SOMOClass SOMO;

#endif