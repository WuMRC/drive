// Header File for AD5933 Library
// Author: Il-Taek Kwon
// Modifications: Adetunji Dahunsi

#ifndef AD5933_Head // Pre-Processing code to prevent from duplicate declaration.
#define AD5933_Head

#include<math.h> // for math functions
#include<Wire.h> // for I2C communications
#include<Arduino.h> // For access to hardware pins
#include<HardwareSerial.h> // for Serial Logging

#define LOGGING1 0 // Basic Log, Error
#define LOGGING2 0 // Related to Development Phase
#define LOGGING3 0 // Detailed Log for Debugging
// Caution! Currently, verbose logging feature makes problem. Do not use them before proper debugging.

const int AD5933_ADR = 0x0D; // Device Serial Bus Address
const int BLOCK_READ_CODE = 0xA1; // Command Code for block read.

const int INIT_START_FREQ = 1;	// defined values for Control Register
const int START_FREQ_SWEEP = 2;
const int INCR_FREQ = 3;
const int REPEAT_FREQ = 4;
const int POWER_DOWN = 10;
const int STAND_BY = 11;
const int TEMP_MEASURE = 9;

const int RANGE_1 = 1; // constants for setting "range" (voltage amplitude to inject into subject)
const int RANGE_2 = 2;
const int RANGE_3 = 3;
const int RANGE_4 = 4;

const int GAIN_1 = 1; // constants for PGA gain inside
const int GAIN_5 = 5;

/*
The constants below is for Henway circuits and designed by Adetunji Dahunsi.
*/
const int LED7_R = 3;
const int LED7_G = 4;
const int LED7_B = 5;
const int LED5 = 13;
const int LED6 = 12;
const int LED8 = 11;
const int PUSH1 = 27; // Push button 1:  Switch 2
const int PUSH2 = 28; // Push button 2:  Switch 3
const int BOOST = 22; // 5V On

const int BI_TETRA_MUX = 29; // Bi-Polar / Tetra-polar multiplexer switch
const int IV_MUX = 23; // Current / Voltage multiplexer switch

// End of Henway Constants

//const double M_PI = 3.14159265358979323846;	// pi
//const double M_PI_2 = 1.57079632679489661923;	// pi/2

typedef uint8_t byte; // For the compatibility for Arduino Type Definitions

class AD5933_Class
{
public: // The detailed instruction will be on ".cpp" file comments.
		// Wiki on Github includes out-of-dated instructions, but it could be a good reference. (Will be updated if time allows.)
		
	int delayTimeInit; // for setting delay time.

	// Common functions used within a sketch

	double getTemperature();

	bool tempUpdate();
	bool resetAD5933();

	bool setExtClock(bool);
	bool setSettlingCycles(int, byte);

	bool setStartFreq(long);
	bool setStepSize(long);
	bool setStepSizeInHex(long);
	bool setNumofIncrement(byte);

	bool setCtrMode(byte);
	bool setCtrMode(byte, int);

	bool setPGA(byte);
	bool setPGA(byte, int);

	bool setRange(byte);
	bool setRange(byte, int);

	bool getGainFactor(double, int, double &, double &);
	bool getGainFactor(double, int, double &, double &, bool);
	bool getGainFactorsSweep(double, int, double *, double *);
	bool getGainFactorTetra(double, int, double &, double &, double &);
	bool getGainFactorTetra(double, int, double &, double &, double &, bool);
	bool getGainFactorsTetraSweep(double, int, double *, double *, double *);

	bool getComplexRawOnce(int &, int &);

	bool getComplex(double, double, double &, double &);
	bool getComplexTetra(int, double, double, double, double &, double &);
	bool getImpedance(double, double &);

	bool setupDevicePins(int);

	double getMagOnce();

	// Constructors (Actually not frequently used)
	AD5933_Class() {
		delayTimeInit=100;
		opClock = 16776000;
	}
	AD5933_Class(int delayTime) {// Another option of constructor
		delayTimeInit=delayTime;
		opClock = 16776000;
	}
	AD5933_Class(int delayTime, HardwareSerial &print){
		delayTimeInit=delayTime;
		opClock = 16776000;
		printer = &print;
	}

	
private:	// Internal fucntions and variables used within the library

	static const byte Address_Ptr = 0xB0; 	// Address Pointer to read register values.
	double opClock; 						// Internal Variable for operating clock
	long incrHex;							// Internal Variable for hex value of increment
	//int getRealCompP();
	//int getImagCompP();
	byte numIncrement;						// Internal Variable for value of # of increment
	double getMagValue();
	
	/*
	*	Inline Function for getting status register.
	*/
	inline byte getStatusReg()
	{
		return (getByte(0x8F) & 0x07);
	}
		
	HardwareSerial *printer; // Decleared for Logging Feature (Not actually used)
	
	bool blockRead(int, int, byte *);
	
	/*
	* 	Inline Function for getting root squared magnitude from two integer value
	*/
	inline double getMag(int cReal, int cImag)
	{
		return sqrt( ( square(cReal) + square(cImag)) );
		//return sqrt( (cReal * cReal) + (cImag * cImag) );
		//return sqrt( ( sq((double)cReal) + sq((double)cImag)) );
	}
	
	bool isValueReady();

	int getByte(int);
	bool setByte(int, int);

};

extern AD5933_Class AD5933;

#endif
