//-------------------------------------------------------------------------------
//  TinyCircuits Accelerometer TinyShield Example Sketch
//  Using Bosch BMA250 in I2C mode
//
//  Created 6/30/2013
//  by Ken Burns, TinyCircuits http://Tiny-Circuits.com
//
//  This example code is in the public domain.
//
//-------------------------------------------------------------------------------

#include <Wire.h>

#define BMA250_I2CADDR      0x18
#define BMA250_RANGE        0x03   // 0x03 = 2g, 0x05 = 4g, 0x08 = 8g, 0x0C = 16g
#define BMA250_BW           0x08   // 7.81Hz (update time of 64ms)

int AccelX;
int AccelY;
int AccelZ;
float AccelTemperature;

void setup()
{
  Wire.begin();
  Serial.begin(38400);
  BMA250Init();
}


void loop()
{
  BMA250ReadAccel();

  // Print out the accelerometer data
  Serial.print("x: ");
  Serial.print(AccelX);
  Serial.print(", y: ");
  Serial.print(AccelY);
  Serial.print(", z:");
  Serial.print(AccelZ);
  Serial.print(",  t: ");   
  Serial.print(AccelTemperature);
  Serial.println("degC");    
  
  delay(1000);
}


void BMA250Init()
{
  // Setup the range measurement setting
  Wire.beginTransmission(BMA250_I2CADDR);
  Wire.write(0x0F); 
  Wire.write(BMA250_RANGE);
  Wire.endTransmission();
  
  // Setup the bandwidth
  Wire.beginTransmission(BMA250_I2CADDR);
  Wire.write(0x10);
  Wire.write(BMA250_BW);
  Wire.endTransmission();
}


int BMA250ReadAccel()
{
  uint8_t ReadBuff[8];
  
  // Read the 7 data bytes from the BMA250
  Wire.beginTransmission(BMA250_I2CADDR);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.requestFrom(BMA250_I2CADDR,7);
  
  for(int i = 0; i < 7;i++)
  {
    ReadBuff[i] = Wire.read();
  }
  
  AccelX = ReadBuff[1] << 8;
  AccelX |= ReadBuff[0];
  AccelX >>= 6;
  
  AccelY = ReadBuff[3] << 8;
  AccelY |= ReadBuff[2];
  AccelY >>= 6;
  
  AccelZ = ReadBuff[5] << 8;
  AccelZ |= ReadBuff[4];
  AccelZ >>= 6;  

  AccelTemperature = (ReadBuff[6] * 0.5) + 24.0;
}



