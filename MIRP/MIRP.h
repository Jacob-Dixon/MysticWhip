/******************************************************************************
MIRP.h
MIRP Library Header File
CJ Vaughter & Prateek Prakash
Original Creation Date: March 16, 2015
******************************************************************************/

#ifndef MIRP_h
#define MIRP_h
#include <Arduino.h>

//56338
const byte OCR1A_56K = (byte)(F_CPU / (56000L * 2L));
const byte OCR1B_56K = (byte)(OCR1A_56K / 2L);

//55944
//const byte OCR1A_56K = 142;
//const byte OCR1B_56K = 71;

enum mirp_packets : byte
{
  m_start	 = 10,
  m_damage	 = 20,
  m_heal	 = 30,
  m_stun	 = 40,
  m_stop	 = 150,
};

class MIRPClass
{
public:
	void init();
	void send(byte length, byte _number = 1);
	void isr();
private:
	bool initialized;
	volatile bool mirp_busy;
	volatile bool mirp_state;
	volatile short mirp_cycle_count;
	volatile byte mirp_TQ;
	volatile byte packet_count;
	byte number;
	short mirp_packet[4];
};

extern MIRPClass MIRP;

#endif