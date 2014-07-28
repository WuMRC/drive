// Header File for AD5933
#ifndef AD5933_Head
#define AD5933_Head

#include<math.h>
#include<Wire.h>

#define LOGGING1 1 // Basic Log, Error
#define LOGGING2 0 // Related to Development Phase
#define LOGGING3 0 // Detailed Log for Debugging

#define AD5933_ADR 0x0D

#define INIT_START_FREQ 1
#define START_FREQ_SWEEP 2
#define INCR_FREQ 3
#define REPEAT_FREQ 4
#define POWER_DOWN 10
#define STAND_BY 11

typedef uint8_t byte;

class AD5933_Class
{
public:
	int delayTimeInit;
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
	bool performFreqSweep(double, double *);
	AD5933_Class()
	{
		delayTimeInit=100;
		opClock = 16776000;
	}
	AD5933_Class(int delayTime)
	{
		delayTimeInit=delayTime;
		opClock = 16776000;
	}

private:
	int getByte(int);
	bool setByte(int, int);
	
	static const byte Address_Ptr = 0xB0;
	double opClock;
	int getRealComp();
	int getImagComp();
	double getMagValue();
	byte getStatusReg();
	
};

extern AD5933_Class AD5933;


#endif