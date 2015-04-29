/******************************************************************************
CAN.cpp
CAN Library Source File

Frank Kienast
Original Creation Date: November, 2010
https://github.com/franksmicro/Arduino/tree/master/libraries/MCP2515

Modified by CJ Vaughter & Jonathan Kropfinger: March 23, 2015
******************************************************************************/

#include "CAN.h"
#include <avr/power.h>

CANCtrl CAN;

boolean CANCtrl::init(byte baudConst)
{
  power_spi_enable();
  SPI.begin();
  command(RESET); //Reset cmd
  //Read mode and make sure it is config
  delay(100);
  byte mode = readReg(CANSTAT) >> 5;
  if(mode != 0b100) 
    return false;
  digitalWrite(6,HIGH);
  byte status = setBaud(baudConst);
  setNormalMode(LOW);
  return status;
}

boolean CANCtrl::setBaud(byte baudConst)
{
  byte brp;

  //BRP<5:0> = 00h, so divisor (0+1)*2 for 125ns per quantum at 16MHz for 500K   
  //SJW<1:0> = 00h, Sync jump width = 1
  switch(baudConst)
  {
    case CAN_BAUD_500K: brp = 0; break;
    case CAN_BAUD_250K: brp = 1; break;
    case CAN_BAUD_125K: brp = 3; break;
    case CAN_BAUD_100K: brp = 4; break;
	case CAN_BAUD_MAGE: brp = 7; break;
    default: return false;
  }

  writeReg(CNF1, brp & 0b00111111);
  //PRSEG<2:0> = 0x01, 2 time quantum for prop
  //PHSEG<2:0> = 0x06, 7 time constants to PS1 sample
  //SAM = 0, just 1 sampling
  //BTLMODE = 1, PS2 determined by CNF3
  writeReg(CNF2, 0b10110001);
  //PHSEG2<2:0> = 5 for 6 time constants after sample 
  writeReg(CNF3, 0x05);
  //SyncSeg + PropSeg + PS1 + PS2 = 1 + 2 + 7 + 6 = 16
  power_spi_disable();
  return true;
}

boolean CANCtrl::setNormalMode(boolean singleShot)
{
  //REQOP2<2:0> = 000 for normal mode
  //ABAT = 0, do not abort pending transmission
  //OSM = 0, not one shot
  //CLKEN = 0, disable output clock
  //CLKPRE = 0b11, clk/8

  byte settings;
  byte mode;

  settings = 0b00000011 | (singleShot << 3);

  writeReg(CANCTRL,settings);
  //Read mode and make sure it is normal
  mode = readReg(CANSTAT) >> 5;
  if(mode != 0)
    return false;

  return true;
}

boolean CANCtrl::shutdownTransceiver()
{
  //B1BFS = 1, output 1 on
  //B0BFS = 1, output 0 on
  //B1BFE = 1, pin enabled
  //B0BFE = 1, pin enabled
  //B1BFM = 0, digital output mode
  //B0BFM = 0, digital output mode
  
  writeReg(BFPCTRL,0b00111100);
}

boolean CANCtrl::setSleepMode()
{
  //REQOP2<2:0> = 001 for sleep mode
  //ABAT = 0, do not abort pending transmission
  //OSM = 0, not one shot
  //CLKEN = 0, disable output clock
  //CLKPRE = 0b11, clk/8

  byte mode;
  
  writeReg(CANCTRL,0b00100011);
  //Read mode and make sure it is normal
  mode = readReg(CANSTAT) >> 5;
  if(mode != 1)
    return false;

  return true;
}

boolean CANCtrl::setReceiveOnlyMode()
{
  //REQOP2<2:0> = 011 for receive-only mode
  //ABAT = 0, do not abort pending transmission
  //OSM = 0, not one shot
  //CLKEN = 0, disable output clock
  //CLKPRE = 0b11, clk/8

  byte mode;

  writeReg(CANCTRL,0b01100011);
  //Read mode and make sure it is receive-only
  mode = readReg(CANSTAT) >> 5;
  if(mode != 3)
    return false;

  return true;
}

boolean CANCtrl::rx(CANMSG &msg, unsigned long timeout)
{
  unsigned long startTime, endTime;
  unsigned short standardID = 0;
  boolean gotMessage;
  byte val;
  int i;

  startTime = millis();
  endTime = startTime + timeout;
  gotMessage = false;
  while(millis() < endTime)
  {
    val = readReg(CANINTF);
    //If we have a message available, read it
    if(bitRead(val,RX0IF) == 1)
    {
      gotMessage = true;
      break;
    }
  }

  if(gotMessage)
  {
    val = readReg(RXB0CTRL);
    msg.rtr = ((bitRead(val,3) == 1) ? true : false);
    //Address received from
    val = readReg(RXB0SIDH);
    standardID |= (val << 3);
    val = readReg(RXB0SIDL);
    standardID |= (val >> 5);

    msg.adrsValue = long(standardID);
    msg.isExtendedAdrs = ((bitRead(val,EXIDE) == 1) ? true : false);
    if(msg.isExtendedAdrs)
    {
      msg.adrsValue = ((msg.adrsValue << 2) | (val & 0b11));
      val = readReg(RXB0EID8);
      msg.adrsValue = (msg.adrsValue << 8) | val;
      val = readReg(RXB0EID0);
      msg.adrsValue = (msg.adrsValue << 8) | val;
    }
    msg.adrsValue = 0b11111111111111111111111111111 & msg.adrsValue; // mask out extra bits
    //Read data bytes
    val = readReg(RXB0DLC);
    msg.dataLength = (val & 0xf);
	
	power_spi_enable();
    digitalWrite(SLAVESELECT,LOW);
    SPI.transfer(READ);
    SPI.transfer(RXB0D0);
    for(i = 0; i < msg.dataLength; i++)
    {
      msg.data[i] = SPI.transfer(0);
    }
    digitalWrite(SLAVESELECT,HIGH);
	power_spi_disable();
	
    //And clear read interrupt
    writeRegBit(CANINTF,RX0IF,0);
  }

  return gotMessage;
}

boolean CANCtrl::tx(CANMSG &msg, unsigned long timeout)
{
  unsigned long startTime, endTime;
  boolean sentMessage;
  unsigned short val;
  int i;
  unsigned short standardID = 0;

  standardID = short(msg.adrsValue);
  startTime = millis();                                                                                                                                               
  endTime = startTime + timeout;
  sentMessage = false;
  if(!msg.isExtendedAdrs)
  {
    //Write standard ID registers
    val = standardID >> 3;
    writeReg(TXB0SIDH,val);
    val = standardID << 5;
    writeReg(TXB0SIDL,val);
  }
  else
  {
    //Write extended ID registers, which use the standard ID registers
    val = msg.adrsValue >> 21;
    writeReg(TXB0SIDH,val);
    val = msg.adrsValue >> 16;
    val = val & 0b00000011;
    val = val | (msg.adrsValue >> 13 & 0b11100000);
    val |= 1 << EXIDE;
    writeReg(TXB0SIDL,val);
    val = msg.adrsValue >> 8;
    writeReg(TXB0EID8,val);
    val = msg.adrsValue;
    writeReg(TXB0EID0,val);
  }

  val = msg.dataLength & 0x0f;
  if(msg.rtr)
    bitWrite(val,TXRTR,1);
  writeReg(TXB0DLC,val);

  //Message bytes
  power_spi_enable();
  digitalWrite(SLAVESELECT,LOW);
  SPI.transfer(WRITE); 
  SPI.transfer(TXB0D0);
  for(i = 0; i < msg.dataLength; i++)
  {
    SPI.transfer(msg.data[i]);
  }
  digitalWrite(SLAVESELECT,HIGH);
  power_spi_disable();

  //Transmit the message
  writeRegBit(TXB0CTRL,TXREQ,1);

  sentMessage = false;
  while(millis() < endTime)
  {
    val = readReg(CANINTF);
    if(bitRead(val,TX0IF) == 1)
    {
      sentMessage = true;
      break;
    }
  }

  //Abort the send if failed
  writeRegBit(TXB0CTRL,TXREQ,0);

  //And clear write interrupt
  writeRegBit(CANINTF,TX0IF,0);

  return sentMessage;
}

byte CANCtrl::getTxErrCnt()
{
  return(readReg(TEC));
}

byte CANCtrl::getRxErrCnt()
{
  return(readReg(REC));
}

void CANCtrl::command(byte cmd)
{
  power_spi_enable();
  digitalWrite(SLAVESELECT,LOW);
  SPI.transfer(cmd); 
  digitalWrite(SLAVESELECT,HIGH);
  power_spi_disable();
}

void CANCtrl::writeReg(byte regno, byte val)
{
  power_spi_enable();
  digitalWrite(SLAVESELECT,LOW);
  SPI.transfer(WRITE); 
  SPI.transfer(regno);
  SPI.transfer(val);
  digitalWrite(SLAVESELECT,HIGH);  
  power_spi_disable();
}

void CANCtrl::writeRegBit(byte regno, byte bitno, byte val)
{
  power_spi_enable();
  digitalWrite(SLAVESELECT,LOW);
  SPI.transfer(BIT_MODIFY); 
  SPI.transfer(regno);
  SPI.transfer(1 << bitno);
  if(val != 0)
    SPI.transfer(0xff);
  else
    SPI.transfer(0x00);
  digitalWrite(SLAVESELECT,HIGH);
  power_spi_disable();
}

byte CANCtrl::readReg(byte regno)
{
  power_spi_enable();
  byte val;

  digitalWrite(SLAVESELECT,LOW);
  SPI.transfer(READ); 
  SPI.transfer(regno);
  val = SPI.transfer(0);
  digitalWrite(SLAVESELECT,HIGH);
  power_spi_disable();

  return val;  
}