/******************************************************************************
Accel.cpp
Accel Library Source File

Jim Lindblom @ SparkFun Electronics
Original Creation Date: June 3, 2014
https://github.com/sparkfun/MMA8452_Accelerometer

Modified by CJ Vaughter & Dylan Rogers: March 11, 2015
******************************************************************************/

#include "Accel.h"

MMA8452Q Accel;

// INITIALIZATION
//	This function initializes the MMA8452Q. It sets up the scale (either 2, 4,
//	or 8g), output data rate, portrait/landscape detection and tap detection.
//	It also checks the WHO_AM_I register to make sure we can communicate with
//	the sensor. Returns a 0 if communication failed, 1 if successful.
byte MMA8452Q::init(byte addr, byte fsr, byte odr)
{
	I2C.begin();
	
	address = addr; // Store address into private variable
	scale = fsr; // Haul fsr into our class variable, scale
	
	byte c = I2C.readReg(address, WHO_AM_I);  // Read WHO_AM_I register
	if (c != 0x2A) return 0; // WHO_AM_I should always be 0x2A
	
	standby();  // Must be in standby to change registers
	setScale(scale);  // Set up accelerometer scale
	setODR(odr);  // Set up output data rate
	setupPL();  // Set up portrait/landscape detection
	// Multiply parameter by 0.0625g to calculate threshold.
	setupTap(0x80, 0x80, 0x08); // Disable x, y, set z to 0.5g
	active();  // Set to active to start reading

	return 1;
}

// READ ACCELERATION DATA
//  This function will read the acceleration values from the MMA8452Q. After
//	reading, it will update two triplets of variables:
//		* int's x, y, and z will store the signed 12-bit values read out
//		  of the acceleromter.
//		* floats cx, cy, and cz will store the calculated acceleration from
//		  those 12-bit values. These variables are in units of g's.
void MMA8452Q::read()
{
	byte rawData[6];  // x/y/z accel register data stored here

	I2C.readRegs(address, OUT_X_MSB, (byte*)&rawData, (byte)6);  // Read the six raw data registers into data array
	
	x = (rawData[0]<<8 | rawData[1]) >> 4;
	y = (rawData[2]<<8 | rawData[3]) >> 4;
	z = (rawData[4]<<8 | rawData[5]) >> 4;
	cx = (float) x / (float)(1<<11) * (float)(scale);
	cy = (float) y / (float)(1<<11) * (float)(scale);
	cz = (float) z / (float)(1<<11) * (float)(scale);
}

// CHECK IF NEW DATA IS AVAILABLE
//	This function checks the status of the MMA8452Q to see if new data is availble.
//	returns 0 if no new data is present, or a 1 if new data is available.
byte MMA8452Q::available()
{
	return (I2C.readReg(address, STATUS) & 0x08) >> 3;
}

// SET FULL-SCALE RANGE
//	This function sets the full-scale range of the x, y, and z axis accelerometers.
//	Possible values for the fsr variable are SCALE_2G, SCALE_4G, or SCALE_8G.
void MMA8452Q::setScale(byte fsr)
{
	// Must be in standby mode to make changes!!!
	byte cfg = I2C.readReg(address, XYZ_DATA_CFG);
	cfg &= 0xFC; // Mask out scale bits
	cfg |= (fsr >> 2);  // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
	I2C.writeReg(address, XYZ_DATA_CFG, cfg);
}

// SET THE OUTPUT DATA RATE
//	This function sets the output data rate of the MMA8452Q.
//	Possible values for the odr parameter are: ODR_800, ODR_400, ODR_200, 
//	ODR_100, ODR_50, ODR_12, ODR_6, or ODR_1
void MMA8452Q::setODR(byte odr)
{
	// Must be in standby mode to make changes!!!
	byte ctrl = I2C.readReg(address, CTRL_REG1);
	ctrl &= 0xCF; // Mask out data rate bits
	ctrl |= (odr << 3);
	I2C.writeReg(address, CTRL_REG1, ctrl);
}

// SET UP TAP DETECTION
//	This function can set up tap detection on the x, y, and/or z axes.
//	The xThs, yThs, and zThs parameters serve two functions:
//		1. Enable tap detection on an axis. If the 7th bit is SET (0x80)
//			tap detection on that axis will be DISABLED.
//		2. Set tap g's threshold. The lower 7 bits will set the tap threshold
//			on that axis.
void MMA8452Q::setupTap(byte xThs, byte yThs, byte zThs)
{
	// Set up single and double tap - 5 steps:
	// for more info check out this app note:
	// http://cache.freescale.com/files/sensors/doc/app_note/AN4072.pdf
	// Set the threshold - minimum required acceleration to cause a tap.
	byte temp = 0;
	if (!(xThs & 0x80)) // If top bit ISN'T set
	{
		temp |= 0x3; // Enable taps on x
		I2C.writeReg(address, PULSE_THSX, xThs);  // x thresh
	}
	if (!(yThs & 0x80))
	{
		temp |= 0xC; // Enable taps on y
		I2C.writeReg(address, PULSE_THSY, yThs);  // y thresh
	}
	if (!(zThs & 0x80))
	{
		temp |= 0x30; // Enable taps on z
		I2C.writeReg(address, PULSE_THSZ, zThs);  // z thresh
	}
	// Set up single and/or double tap detection on each axis individually.
	I2C.writeReg(address, PULSE_CFG, temp | 0x40);
	// Set the time limit - the maximum time that a tap can be above the thresh
	I2C.writeReg(address, PULSE_TMLT, 0x30);  // 30ms time limit at 800Hz odr
	// Set the pulse latency - the minimum required time between pulses
	I2C.writeReg(address, PULSE_LTCY, 0xA0);  // 200ms (at 800Hz odr) between taps min
	// Set the second pulse window - maximum allowed time between end of
	//	latency and start of second pulse
	I2C.writeReg(address, PULSE_WIND, 0xFF);  // 5. 318ms (max value) between taps max
}

// READ TAP STATUS
//	This function returns any taps read by the MMA8452Q. If the function 
//	returns no new taps were detected. Otherwise the function will return the
//	lower 7 bits of the PULSE_SRC register.
byte MMA8452Q::readTap()
{
	byte tapStat = I2C.readReg(address, PULSE_SRC);
	if (tapStat & 0x80) return tapStat & 0x7F; // Read EA bit to check if a interrupt was generated
	else return 0;
}

// SET UP PORTRAIT/LANDSCAPE DETECTION
//	This function sets up portrait and landscape detection.
void MMA8452Q::setupPL()
{
	// Must be in standby mode to make changes!!!
	// For more info check out this app note:
	//	http://cache.freescale.com/files/sensors/doc/app_note/AN4068.pdf
	// 1. Enable P/L
	I2C.writeReg(address, PL_CFG, I2C.readReg(address, PL_CFG) | 0x40); // Set PL_EN (enable)
	// 2. Set the debounce rate
	I2C.writeReg(address, PL_COUNT, 0x50);  // Debounce counter at 100ms (at 800 hz)
}

// READ PORTRAIT/LANDSCAPE STATUS
//	This function reads the portrait/landscape status register of the MMA8452Q.
//	It will return either PORTRAIT_U, PORTRAIT_D, LANDSCAPE_R, LANDSCAPE_L,
//	or LOCKOUT. LOCKOUT indicates that the sensor is in neither p or ls.
byte MMA8452Q::readPL()
{
	byte plStat = I2C.readReg(address, PL_STATUS);
	if (plStat & 0x40) return LOCKOUT; // Z-tilt lockout
	else return (plStat & 0x6) >> 1;// Otherwise return LAPO status
}

// SET STANDBY MODE
//	Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Q::standby()
{
	byte c = I2C.readReg(address, CTRL_REG1);
	I2C.writeReg(address, CTRL_REG1, c & ~(0x01)); //Clear the active bit to go into standby
}

// SET ACTIVE MODE
//	Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Q::active()
{
	byte c = I2C.readReg(address, CTRL_REG1);
	I2C.writeReg(address, CTRL_REG1, c | 0x01); //Set the active bit to begin detection
}