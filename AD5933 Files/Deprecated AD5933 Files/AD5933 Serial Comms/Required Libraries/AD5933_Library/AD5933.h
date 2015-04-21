// Header File for AD5933 Library

// Author: Il-Taek Kwon

// Modifications: Adetunji Dahunsi

#ifndef AD5933_Head // Pre-Processing code to prevent from duplicate declaration.
#define AD5933_Head

#include<math.h> // for math functions
#include<Wire.h> // for I2C communications
#include<Arduino.h> // For access to hardware pins
#include<HardwareSerial.h> // for Serial Logging

const int LOGGING1 = 0; // Basic Log, Error
const int LOGGING2 = 0; // Related to Development Phase
const int LOGGING3 = 0; // Detailed Log for Debugging
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

const int RANGE_1 = 1;
const int RANGE_2 = 2;
const int RANGE_3 = 3;
const int RANGE_4 = 4;

const int GAIN_1 = 1;
const int GAIN_5 = 5;

const int BI_TETRA_MUX = 29;
const int IV_MUX = 23;

//const double M_PI = 3.14159265358979323846;	// pi
//const double M_PI_2 = 1.57079632679489661923;	// pi/2

typedef uint8_t byte; // For the compatibility for Arduino Type Definitions

class AD5933_Class
{
public: // The detailed instruction will be on Wiki or ".cpp" file
	int delayTimeInit; // for setting delay time.
	double getTemperature();
	bool tempUpdate();
	double getMagOnce();
	bool setStartFreq(long);
	bool setIncrement(long);
	bool setIncrementinHex(long);
	bool setNumofIncrement(byte);
	bool setSettlingCycles(int, byte);
	bool resetAD5933();
	bool setExtClock(bool);
	bool setCtrMode(byte);
	bool setCtrMode(byte, int);
	bool setPGA(byte);
	bool setPGA(byte, int);
	bool setRange(byte);
	bool setRange(byte, int);
	double getGainFactor(double);
	double getGainFactor(double, int);
	double getGainFactor(double, int, bool);
	bool performFreqSweep(double, double *);
	AD5933_Class()
	{
		delayTimeInit=100;
		opClock = 16776000;
	}
	AD5933_Class(int delayTime) // Another option of constructor
	{
		delayTimeInit=delayTime;
		opClock = 16776000;
	}
	AD5933_Class(int delayTime, HardwareSerial &print)
	{
		delayTimeInit=delayTime;
		opClock = 16776000;
		printer = &print;
	}
	int getByte(int);
	bool setByte(int, int);
	bool isValueReady();
	//int getRealComp();
	//int getImagComp();
	bool getComplexRawOnce(int &, int &);
	bool getComplexOnce(double, double, double &, double &, double &, double &);
	bool compFreqRawSweep(int *, int *);
	bool compFreqSweep(double *, double *, double *, double *);
	bool getGainFactorC(double, int, double &, double &);
	bool getGainFactorC(double, int, double &, double &, bool);
	bool getGainFactorTetra(double, int, double &, double &, double &);
	bool getGainFactorTetra(double, int, double &, double &, double &, bool);
	bool getGainFactorS_Set(double , int, double *, double *);
	bool getGainFactorS_TP(double, int, double, double, double &, double &, double &, double &);
	bool compCbrArray(double, double, double, double, double *, double *);
	bool getArraysLI(double&, double&, double&, uint8_t&, double&, double&, double*, double*);
	bool getGainFactors_LI(double, int, double, double, double &, double &, double &, double &);
	bool getComplex(double, double, double &, double &);
	bool getComplexTetra(int, double, double, double, double &, double &);


	

private:
	
	static const byte Address_Ptr = 0xB0; // Address Pointer to read register values.
	double opClock;
	long incrHex;
	//int getRealCompP();
	//int getImagCompP();
	byte numIncrement;
	double getMagValue();
	double returnStandardPhaseAngle(double angle);
	inline byte getStatusReg()
	{
		return (getByte(0x8F) & 0x07);
	}
	HardwareSerial *printer;
	bool blockRead(int, int, byte *);
	inline double getMag(int cReal, int cImag)
	{
		return sqrt( ( square(cReal) + square(cImag)) );
		//return sqrt( (cReal * cReal) + (cImag * cImag) );
		//return sqrt( ( sq((double)cReal) + sq((double)cImag)) );
	}
	
};

extern AD5933_Class AD5933;


#endif
