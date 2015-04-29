#include <avr/power.h>
#include <avr/sleep.h>

#include <Wire.h>
#include <I2C.h>
#include <SPI.h>

#include <SOMO.h>
#include <MIRP.h>
#include <CAN.h>
#include <RGB.h>
#include <Accel.h>

//Modes
#define WHIP	 0x01
//#define RICKROLL 0x02
//#define STARWARS 0x03

//States
#define IDLE     0x00
#define FAIL	 0x01
#define DAMAGE	 0x02
#define STUN	 0x03
#define HEAL	 0x04
#define READY    0x05
#define STARTUP  0x06
#define COOLDOWN 0x07
#define WAITING  0x08
#define ANNIHILATION 0x09

//#define DEBUGACCEL

bool button_pressed = false;
bool CAN_BYPASS = false;
byte state = STARTUP;
byte level = 0;
byte mode = WHIP;
unsigned long cooldowntime = 0;

byte accelstate = IDLE;
byte down = IDLE;
byte heal = IDLE;

void setup()
{
  cli(); //Disable interrupts
  ADCSRA = 0; //Disable ADC conversion
  power_adc_disable(); //Turn off ADC power
  DDRB &= ~0x01; //Set button pin as input
  PORTB |= 0x01; //Enable pullup resistor
  //PCICR |= 0x01;  //Enable PORTB PCINT
  //PCMSK0 |= 0x01; //Enable PCINT0
  sei(); //Enable interrupts

  SOMO.init();
  MIRP.init();
  CAN_BYPASS = !(PINB & 0x01);
  if (!CAN_BYPASS) CAN.init();
  else delay(100);
  RGB.init();
#ifndef DEBUGACCEL
  Accel.init();
#endif

  light();
  RGB.start();
  audio(); //startup sound
  if (!CAN_BYPASS)
  {
    state = WAITING;
    light();
    can();
  }
  else
  {
    level = 1;
    delay(500);
    state = READY;
    light();
    audio();
  }
}

void loop()
{
  if (state == READY)
  {
    state = IDLE;
    light();
  }
  if (state == COOLDOWN)
  {
    if (millis() - cooldowntime >= 4000)
    {
      state = READY;
      light();
      audio();
    }
  }
  else
  {

    if (!button_pressed) delay(100);
    button_pressed = !(PINB & 0x01);
    if (button_pressed)
    {
      accel();
    }
    else
    {
#ifdef DEBUGACCEL
      state = DAMAGE;
#else
      if (down < 3 && heal < 3) accelstate = IDLE;
      else if (down >= 30 && heal <= down) accelstate = ANNIHILATION;
      else if (down >= 12 && heal <= 5) accelstate = DAMAGE;
      else if (down >= 3 && heal <= 3) accelstate = STUN;
      else if (down < heal && heal >= 5) accelstate = HEAL;
      else accelstate = FAIL;
      state = accelstate;
#endif
      accelstate = heal = down = IDLE;
      light();
      if (state == DAMAGE || state == STUN || state == HEAL || state == FAIL || state == ANNIHILATION) cast();
      else ;//goto_sleep();
    }
  }
}

void cast()
{
  if (state != FAIL)
  {
    mirp();
    audio();
    state = COOLDOWN;
    light();
    cooldowntime = millis();
  }
  if (state == FAIL)
  {
    audio();
    state = READY;
    light();
    audio();
  }
}

void accel()
{
#ifndef DEBUGACCEL
  RGB.stop();
  Accel.read();
  RGB.start();

  if (Accel.cz < -1 || Accel.cz > 2)
  {
    down++;
    delay(100);
  }
  else if (abs(Accel.cy) > 1.75 && Accel.cz > -1)
  {
    heal++;
    delay(50);
  }

#else
  //RGB.stop();
  delay(10);
  //RGB.start();
  delay(100);
#endif
}

void audio()
{
  SOMO.play(mode, state);
}

void mirp()
{
  switch (state)
  {
    case STUN: 	 MIRP.send(m_stun, level*10);   break;
    case DAMAGE: MIRP.send(m_damage, level*10); break;
    case HEAL: 	 MIRP.send(m_heal, level*10);   break;
    case ANNIHILATION: MIRP.send(m_damage, 100); break;
  }
}

void can()
{
  CANMSG sendmsg, replymsg;
  bool sent, received;

  sendmsg.adrsValue = 0x02 << 3;
  sendmsg.dataLength = 2;
  sendmsg.data[0] = 0x01;
  sendmsg.data[1] = 0x01;
  while (true)
  {
    sent = false; received = false;
    sent = CAN.tx(sendmsg);
    if (sent) received = CAN.rx(replymsg);
    if (received)
    {
      if (replymsg.data[0] == 0xFF && replymsg.data[1] == 0x01 && replymsg.data[2] == 0x50)
      {
        level = replymsg.data[3];
        break;
      }
    }
  }
  CAN.setSleepMode();
  state = READY;
  light();
  audio();
}

void light()
{
  switch (state)
  {
    case IDLE: RGB.setColor(BLUE, OFF, 0.5f, true); break;
    case FAIL: RGB.setColor(RED, OFF, 5.0f); break;
    case STUN: RGB.setColor(BLUE, OFF, 40.0f); break;
    case HEAL:  RGB.setColor(GREEN);  break;
    case DAMAGE: RGB.setColor(RED); break;
    case READY:  RGB.setColor(GREEN, OFF, 10.0f); break;
    case STARTUP: RGB.setColor(CYAN, OFF, 15.0f); break;
    case WAITING: RGB.setColor(YELLOW, DIMYELLOW, 0.2f, true); break;
    case COOLDOWN: RGB.setColor(RED, OFF, 0.8f, true); break;
    case ANNIHILATION: RGB.setColor(WHITE, true); break;
  }
}
/*
void goto_sleep()
{
  PCMSK0 |= 0x01; //Enable PCINT0
  set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
  cli();
  sleep_enable();
  sei();
  sleep_cpu();
}

ISR(PCINT0_vect)
{
  PCMSK0 &= ~0x01; //Disable PCINT0
}
*/
