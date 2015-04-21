#define TWI_FREQ 400000L
#define cycles_base 100
#define cycles_multiplier 1
#define start_frequency 50000
#define cal_samples 10

#include "AD5933.h"
#include <Wire.h>

const int numofIncrement = 10;
const double calResistance = 553.88;
//double arrGFactor[numofIncrement];
double gainFactor, pShift;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  //setByte(0x81, 0x18); // Reset & Use Ext. Clock - 0001 1000
  AD5933.setExtClock(false);
  AD5933.resetAD5933();
  AD5933.setStartFreq(start_frequency);
  AD5933.setIncrement(5000);
  AD5933.setNumofIncrement(numofIncrement);
  AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
  AD5933.getTemperature();
  AD5933.setVolPGA(0,1);
  Serial.println("Please setup for calibration. If completed, press p and Enter>");
  while( Serial.read() != 'p')
    ;
  //gainFactor = AD5933.getGainFactor(calResistance,1);
  AD5933.getGainFactorC(calResistance, 10, gainFactor, pShift, false);
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
  AD5933.getComplexOnce(gainFactor, pShift, cReal, cImag, Z_Val, phase);
  
  Serial.print(millis() / 1000.0); 
  Serial.print("\t"); 
  Serial.print(cReal);
  Serial.print("\t"); 
  Serial.print(cImag);
  Serial.print("\t"); 
  Serial.print(Z_Val);
  Serial.print("\t"); 
  Serial.print(phase);
  Serial.println();
}
