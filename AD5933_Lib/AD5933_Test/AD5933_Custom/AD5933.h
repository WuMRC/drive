// Header File for AD5933 Library
#ifndef AD5933_Head // Pre-Processing code to prevent from duplicate declaration.
#define AD5933_Head

#include<math.h> // for math functions
#include<Wire.h> // for I2C communications

#define LOGGING1 0 // Basic Log, Error
#define LOGGING2 0 // Related to Development Phase
#define LOGGING3 0 // Detailed Log for Debugging
// Caution! Currently, verbose logging feature makes problem. Do not use them before proper debugging.

#define AD5933_ADR 0x0D // Device Serial Bus Address

#define INIT_START_FREQ 1	// defined values for Control Register
#define START_FREQ_SWEEP 2
#define INCR_FREQ 3
#define REPEAT_FREQ 4
#define POWER_DOWN 10
#define STAND_BY 11
#define TEMP_MEASURE 9

typedef uint8_t byte; // For the compatibility for Arduino Type Definitions

class AD5933_Class
{
public: // The detailed instruction will be on Wiki or ".cpp" file
	int delayTimeInit; // for setting delay time.
	double getTemperature();
	double getMagOnce();
	bool setStartFreq(long);
	bool setIncrement(long);
	bool setIncrementinHex(long);
	bool setNumofIncrement(int);
	bool setSettlingCycles(int, byte);
	bool resetAD5933();
	bool setExtClock(bool);
	bool setVolPGA(byte, byte);
	bool setCtrMode(byte);
	bool setCtrMode(byte, int);
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
	int getByte(int);
	bool setByte(int, int);

private:
	
	static const byte Address_Ptr = 0xB0; // Address Pointer to read register values.
	double opClock;
	int getRealComp();
	int getImagComp();
	double getMagValue();
	inline byte getStatusReg()
	{
		return (getByte(0x8F) & 0x07);
		//return getByte(0x8F);
	}
	
};

extern AD5933_Class AD5933;


#endif