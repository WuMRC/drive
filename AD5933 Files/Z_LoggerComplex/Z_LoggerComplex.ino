#define TWI_FREQ 400000L
#define cycles_base 511
#define cycles_multiplier 4
#define start_frequency 50000
#define cal_samples 10

#include "AD5933.h"
#include <Wire.h>

const int numofIncrement = 10;
const double calResistance = 1840;
double gainFactor, pShift;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  //setByte(0x81, 0x18); // Reset & Use Ext. Clock - 0001 1000
  AD5933.setExtClock(false);
  AD5933.resetAD5933();
  AD5933.setStartFreq(start_frequency);
  AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
  AD5933.setStepSizeInHex(1);
  AD5933.setNumofIncrement(2);
  AD5933.setPGA(GAIN_1);
  AD5933.setRange(RANGE_1);
  
  double temp = AD5933.getTemperature();
#if VERBOSE
  Serial.print("Temperature is ");
  Serial.print(temp);
  Serial.println(" degree celcius.");
#endif

  Serial.println("Please setup for calibration. If completed, press p and Enter>");
  while( Serial.read() != 'p')
    ;
  //gainFactor = AD5933.getGainFactor(calResistance,1);
  AD5933.getGainFactor(calResistance, cal_samples, gainFactor, pShift, false);
  Serial.print("Gain Factor: ");
  Serial.println(gainFactor);
  Serial.print("System Phase Shift: ");
  Serial.println(pShift);
  Serial.println("Change resistor to measure! If completed, press p and Enter>");
  while( Serial.read() != 'p')
    ;
}

void loop()
{
  AD5933.tempUpdate();
  AD5933.setCtrMode(REPEAT_FREQ);
  
  /*
  int cReal, cImag;
  AD5933.getComplexRawOnce(cReal, cImag);
  double mag = sqrt( square(cReal) + square(cImag) );
  double Z_Val = gainFactor / mag;
  double phase = atan2(cImag, cReal)-pShift;
  */
  double cReal, cImag;
  double Z_Val, phase;
  AD5933.getComplex(gainFactor, pShift, Z_Val, phase); 
  
  Serial.print(millis() / 1000.0); 
  
  Serial.print("\t"); 
  Serial.print(Z_Val);
  Serial.print("\t"); 
  Serial.print(phase);
  Serial.println();
}
