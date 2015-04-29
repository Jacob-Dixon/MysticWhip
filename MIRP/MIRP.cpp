/******************************************************************************
MIRP.cpp
MIRP Library Source File
CJ Vaughter & Prateek Prakash
Original Creation Date: March 16, 2015
******************************************************************************/

#include "MIRP.h"
#include <avr/power.h>

MIRPClass MIRP;

void MIRPClass::init()
{
	power_timer1_enable();
	DDRB |= 0x02;					//Set pin as output
	PORTB &= ~0x02;					//Set pin low
	TCCR1A = 0x00;					//CTC
	TCCR1B = 0x09;					//CTC & no prescale
	OCR1A =  OCR1A_56K; 			//56338 Hz
	OCR1B =  OCR1B_56K; 			//50% duty
	mirp_packet[0] = m_start;
	mirp_packet[3] = m_stop;
	initialized = true;
	power_timer1_disable();
}

void MIRPClass::send(byte length, byte _number)
{
	if (!mirp_busy && initialized) //Must not be busy, and must be initialized
	{
		power_timer1_enable();
		mirp_busy = true;
		mirp_packet[1] = length;
		mirp_packet[2] = length;
		mirp_state = LOW;
		mirp_cycle_count = 0;
		packet_count = 0;
		number = _number;
		mirp_TQ = 0;
		TCNT1  = 0; //Reset counter
		TIMSK1 |= B00000010; //Enable interrupt on match
	}
}

void MIRPClass::isr()
{
	if (mirp_cycle_count < mirp_packet[mirp_TQ]) PORTB ^= 0x02;

	mirp_state = !mirp_state;
	if(!mirp_state) mirp_cycle_count++;

	if (mirp_cycle_count == m_stop)
	{
		mirp_cycle_count = 0;
		if (++mirp_TQ > 3) //Finished
		{
			if(++packet_count < number)
			{
				mirp_state = LOW;
				mirp_TQ = 0;
				TCNT1  = 0; //Reset counter
			}
			else
			{
				TIMSK1 &= B11111101; //Disable interrupt on match
				mirp_busy = false;
				power_timer1_disable();
			}
		}
	}
}

ISR(TIMER1_COMPA_vect)
{
	MIRP.isr();
}