#include "AD5933.h"
#include <Wire.h>

//AD5933_Class AD5933;

const int numofIncrement = 2;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  //setByte(0x81, 0x18); // Reset & Use Ext. Clock - 0001 1000
  AD5933.setExtClock(false);
  AD5933.resetAD5933();
}

void loop()
{
  Serial.println("Loop Begin!");
  AD5933.setStartFreq(50000);
  AD5933.setIncrementinHex(1);
  AD5933.setNumofIncrement(numofIncrement);
  AD5933.setSettlingCycles(0x1FF, 4);
  AD5933.getTemperature();
  AD5933.setVolPGA(0,1);
  Serial.println("Please setup for calibration. If completed, press p and Enter>");
  while( Serial.read() != 'p')
    ;
  double gainFactor = AD5933.getGainFactor(4700);
  Serial.println("Change resistor to measure! If completed, press p and Enter>");
  while( Serial.read() != 'p')
    ;
  double rValue[numofIncrement];
  AD5933.performFreqSweep(gainFactor, rValue);
  
  
  Serial.println("Loop End!");
  Serial.println();
  delay(5000);
}

