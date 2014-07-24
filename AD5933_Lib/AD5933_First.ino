//Pin for I2C: Uno, Ethernet	A4 (SDA), A5 (SCL)
//Acknowledgement: Some codes are inspried by https://github.com/open-ephys/autoimpedance

#include <Wire.h>
#include <math.h>

const byte Address_Ptr = 0xB0;
#define AD5933_ADR 0x0D

int getByte(int);
boolean setByte(int, int);
double getTemperature();
int getRealComp();
int getImagComp();
double getMag();
byte getStatusReg();
boolean setStartFreq(double startFreq);

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  setByte(0x81, 0x18); // Reset & Use Ext. Clock - 0001 1000
}

void loop()
{
  
  
}

byte getStatusReg()
{
  return getByte(0x8F) & 0x07;
}

double getTemperature()
{
  setByte(0x80,0x90); // Read Temp.
  delay(100);
  
  int tTemp[2];
  long tTempVal;
  double cTemp;
  
  tTemp[0]=getByte(0x92);
  tTemp[1]=getByte(0x93);
  tTempVal = (tTemp[0] % (12 * 16))*16*16 + tTemp[1];
  if(bitRead(tTemp[0],5) == 0)
  {
    // Positive
    cTemp = (double)tTempVal/32;
  }
  else
  {
    // Negative
    cTemp = (tTempVal-16384.0) / 32;
  }
  Serial.print("getTemperature - Current Temp. is ");
  Serial.print(cTemp);
  Serial.print("\n");
  
  return cTemp;
}


int getByte(int address) {

  int rxByte;
  
 //Serial.print("getByte - Initiating I2C Transmission. Address: ");
  //Serial.print(address, HEX);
  //Serial.print('\n');
  
  Wire.beginTransmission(AD5933_ADR);
  Wire.write(Address_Ptr);
  Wire.write(address);
  int i2cReturn = Wire.endTransmission();

  //Serial.print("getByte - Transmission Complete. i2cReturn: ");
  //Serial.print(i2cReturn);
  //Serial.print("\n");
  
  Wire.requestFrom(AD5933_ADR, 1);

  if (1 <= Wire.available()) {
    rxByte = Wire.read();
    //Serial.print("getByte - Message Received: ");
    //Serial.print(rxByte,BIN);
    //Serial.print(" or ");
    //Serial.print(rxByte,HEX);    
    //Serial.print("\n");
  } 
  else {
    rxByte = -1;
    Serial.println("getByte - Failed to receive Message");
  }

  return rxByte;

}
boolean setByte(int address, int value) {
  
  //Serial.print("setByte - Initiating I2C Transmission. Address: ");
  //Serial.print(address, HEX);
  //Serial.print(" , Value: ");
  //Serial.print(value, HEX);
  //Serial.print('\n');
  Wire.beginTransmission(AD5933_ADR);
  Wire.write(address);
  Wire.write(value);
  int i2cReturn = Wire.endTransmission();

  if (i2cReturn)
  {
    Serial.println("setByte - Failed");
    return false;
  }
  else
  {
    //Serial.println("setByte - Success");
    return true; 
  }
}

double getMag()
{
  int rComp, iComp;
  rComp = getRealComp();
  iComp = getImagComp();
  double result = sqrt( square((double)rComp) + square((double)iComp) );
  Serial.print("getMag - Resistance Magnitude is ");
  Serial.println(result);
  return result;
}

int getRealComp()
{
  int mReal, lReal;
  int result;
  mReal=getByte(0x94);
  lReal=getByte(0x95);
  //Serial.print("getRealComp - mReal: ");
  //Serial.print(mReal, BIN);
  //Serial.print('\t');
  //Serial.print("lReal: ");
  //Serial.println(lReal, BIN);
  Serial.print("getRealComp - Value: ");
  result = mReal*16*16+lReal;
  Serial.println(result);
  return result;
}

int getImagComp()
{
  int mImag, lImag;
  int result;
  mImag=getByte(0x96);
  lImag=getByte(0x97);
  //Serial.print("getImagComp - mImag: ");
  //Serial.print(mImag, BIN);
  //Serial.print('\t');
  //Serial.print("lImag: ");
  //Serial.println(lImag, BIN);
  Serial.print("getImagComp - Value: ");
  result = mImag*16*16+lImag;
  Serial.println(result);
  return result;
}
