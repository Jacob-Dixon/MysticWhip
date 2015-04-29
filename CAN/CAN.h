/******************************************************************************
CAN.h
CAN Library Header File

Frank Kienast
Original Creation Date: November, 2010
https://github.com/franksmicro/Arduino/tree/master/libraries/MCP2515

Modified by CJ Vaughter & Jonathan Kropfinger: March 23, 2015
******************************************************************************/

#ifndef CAN_h
#define CAN_h
#include <Arduino.h>
#include <SPI.h>

#define SLAVESELECT SS

enum CANRegisters : byte
{
	RXF0SIDH	= 0x00,
	RXF0SIDL	= 0x01,
	RXF0EID8	= 0x02,
	RXF0EID0	= 0x03,
	RXF1SIDH	= 0x04,
	RXF1SIDL	= 0x05,
	RXF1EID8	= 0x06,
	RXF1EID0	= 0x07,
	RXF2SIDH	= 0x08,
	RXF2SIDL	= 0x09,
	RXF2EID8	= 0x0A,
	RXF2EID0	= 0x0B,
	BFPCTRL		= 0x0C,
	TXRTSCTRL	= 0x0D,
	CANSTAT		= 0x0E,
	CANCTRL		= 0x0F,
	RXF3SIDH	= 0x10,
	RXF3SIDL	= 0x11,
	RXF3EID8	= 0x12,
	RXF3EID0	= 0x13,
	RXF4SIDH	= 0x14,
	RXF4SIDL	= 0x15,
	RXF4EID8	= 0x16,
	RXF4EID0	= 0x17,
	RXF5SIDH	= 0x18,
	RXF5SIDL	= 0x19,
	RXF5EID8	= 0x1A,
	RXF5EID0	= 0x1B,
	TEC			= 0x1C,
	REC			= 0x1D,
	RXM0SIDH	= 0x20,
	RXM0SIDL	= 0x21,
	RXM0EID8	= 0x22,
	RXM0EID0	= 0x23,
	RXM1SIDH	= 0x24,
	RXM1SIDL	= 0x25,
	RXM1EID8	= 0x26,
	RXM1EID0	= 0x27,
	CNF3		= 0x28,
	CNF2		= 0x29,
	CNF1		= 0x2A,
	CANINTE		= 0x2B,
	  MERRE	= 7,
	  WAKIE	= 6,
	  ERRIE	= 5,
	  TX2IE	= 4,
	  TX1IE	= 3,
	  TX0IE	= 2,
	  RX1IE	= 1,
	  RX0IE	= 0,
	CANINTF		= 0x2C,
	  MERRF	= 7,
	  WAKIF	= 6,
	  ERRIF	= 5,
	  TX2IF	= 4,
	  TX1IF	= 3,
	  TX0IF	= 2,
	  RX1IF	= 1,
	  RX0IF	= 0,
	EFLG		= 0x2D,
	TXB0CTRL	= 0x30,
	  TXREQ	= 3,
	TXB0SIDH	= 0x31,
	TXB0SIDL	= 0x32,
	  EXIDE	= 3,
	TXB0EID8	= 0x33,
	TXB0EID0	= 0x34,
	TXB0DLC		= 0x35,
	  TXRTR	= 6,
	TXB0D0		= 0x36,
	RXB0CTRL 	= 0x60,
	  RXM1	= 6,
	  RXM0	= 5,
	  RXRTR	= 3,
	RXB0SIDH	= 0x61,
	RXB0SIDL	= 0x62,
	RXB0EID8	= 0x63,
	RXB0EID0	= 0x64,
	RXB0DLC		= 0x65,
	RXB0D0		= 0x66,
};

enum CANCmd : byte
{
	RESET			= 0xC0,
	READ			= 0x03,
	READ_RX_BUFFER	= 0x90,
	WRITE			= 0x02,
	LOAD_TX_BUFFER	= 0x40,
	RTS				= 0x80,
	READ_STATUS		= 0xA0,
	RX_STATUS		= 0xB0,
	BIT_MODIFY		= 0x05,
};

enum CANBaud : byte
{
	CAN_BAUD_10K 	= 1,
	CAN_BAUD_50K	= 2,
	CAN_BAUD_100K	= 3,
	CAN_BAUD_125K	= 4,
	CAN_BAUD_250K	= 5,
	CAN_BAUD_500K	= 6,
	CAN_BAUD_MAGE	= 7,
};

typedef struct
{
	boolean isExtendedAdrs = false;
	unsigned long adrsValue;
	boolean rtr = false;
	byte dataLength;
	byte data[8];
} CANMSG;


class CANCtrl
{
  public:
    static boolean init(byte baudConst = CAN_BAUD_MAGE);
    static boolean setNormalMode(boolean singleShot = false);
	static boolean setSleepMode();
	static boolean shutdownTransceiver();
    static boolean setReceiveOnlyMode();
    static boolean rx(CANMSG &msg, unsigned long timeout = 1000L);
    static boolean tx(CANMSG &msg, unsigned long timeout = 1000L);
    static byte getTxErrCnt();
    static byte getRxErrCnt();
	
	private:
    static boolean setBaud(byte baudConst);
    static void command(byte cmd);
	static void writeReg(byte regno, byte val);
    static void writeRegBit(byte regno, byte bitno, byte val);
    static byte readReg(byte regno);
};

extern CANCtrl CAN;

#endif