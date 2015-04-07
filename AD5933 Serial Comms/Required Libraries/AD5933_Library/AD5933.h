// Header File for AD5933 Library
#ifndef AD5933_Head // Pre-Processing code to prevent from duplicate declaration.
#define AD5933_Head

#include<math.h> // for math functions
#include<Wire.h> // for I2C communications
#include<HardwareSerial.h> // for Serial Logging

#define LOGGING1 0 // Basic Log, Error
#define LOGGING2 0 // Related to Development Phase
#define LOGGING3 0 // Detailed Log for Debugging
// Caution! Currently, verbose logging feature makes problem. Do not use them before proper debugging.

#define AD5933_ADR 0x0D // Device Serial Bus Address
#define BLOCK_READ_CODE 0xA1 // Command Code for block read.

#define INIT_START_FREQ 1	// defined values for Control Register
#define START_FREQ_SWEEP 2
#define INCR_FREQ 3
#define REPEAT_FREQ 4
#define POWER_DOWN 10
#define STAND_BY 11
#define TEMP_MEASURE 9
#define RANGE_1 12
#define RANGE_2 13
#define RANGE_3 14
#define RANGE_4 15


#define M_PI 3.14159265358979323846	// pi
#define M_PI_2 1.57079632679489661923	// pi/2 

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
	bool setVolPGA(byte, byte);
	bool setCtrMode(byte);
	bool setCtrMode(byte, int);
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
	bool getGainFactorS_Set(double , int, double *, double *);
	bool getGainFactorS_TP(double, int, double, double, double &, double &, double &, double &);
	bool compCbrArray(double, double, double, double, double *, double *);
	bool getArraysLI(double&, double&, double&, uint8_t&, double&, double&, double*, double*);
	bool getGainFactors_LI(double, int, double, double, double &, double &, double &, double &);
	bool getComplex(double, double, double &, double &);


	

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