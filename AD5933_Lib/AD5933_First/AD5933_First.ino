//Pin for I2C: Uno, Ethernet	A4 (SDA), A5 (SCL)
//Acknowledgement: Some codes are inspried by https://github.com/open-ephys/autoimpedance

#include <Wire.h>
#include <math.h>

#define LOGGING1 1
#define LOGGING2 1
#define LOGGING3 1

const byte Address_Ptr = 0xB0;
#define AD5933_ADR 0x0D
const double opClock = 16000000;

int getByte(int);
boolean setByte(int, int);
double getTemperature();
int getRealComp();
int getImagComp();
double getMag();
byte getStatusReg();
boolean setStartFreq(long);
boolean setIncrement(long);
boolean setIncrementinHex(long);
boolean setNumofIncrement(int);

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  setByte(0x81, 0x18); // Reset & Use Ext. Clock - 0001 1000
}

void loop()
{
  Serial.println("Loop Begin!");
  setStartFreq(50000);
  setIncrementHex(1);
  setNumofIncrement(2);
  
  
  Serial.println("Loop End!");
  Serial.println();
  delay(5000);
}

boolean setSettlingCycles(int cycles, byte mult)
{
  if(cycles > 0x1FF || !(mult == 0 || mult == 1 || mult == 3) )
  {
#if LOGGING1
    Serial.println("setSettlingCycles - Invalid Parameter");
#endif
    return false;
  }
  int lowerHex = cycles % 256;
  int upperHex =   
}

boolean setNumofIncrement(int num)
{
  if(num > 0x1FF)
  {
#if LOGGING1
    Serial.print("setNumofIncrement - Freqeuncy Overflow!");
#endif
    return false;
  }
  
  int lowerHex = num % 256;
  int upperHex = (num >> 8) % 2;
  
  boolean t2, t4;
  t2 = setByte(0x88, upperHex);
  t4 = setByte(0x89, lowerHex);
  if(t2 && t4)
    return true;
  else
  {
#if LOGGING1
    Serial.println("setNumofIncrement - Data Transmission Failed!");
#endif
    return false;
  }
}

boolean setIncrement(long increment)
{
  long freqHex = increment / (opClock / pow(2, 29));
  return setIncrementHex(freqHex);
}

boolean setIncrementHex(long freqHex)
{
  if(freqHex > 0xFFFFFF)
  {
#if LOGGING1
    Serial.print("setIncrementHex - Freqeuncy Overflow!");
#endif
    return false;
  }
  
  int lowerHex = freqHex % 256;
  int midHex = ((freqHex - (long)lowerHex) >> 8) % 256;
  int upperHex = freqHex >> 16;
  
  boolean t2, t3, t4;
  t2 = setByte(0x85, upperHex);
  t3 = setByte(0x86, midHex);
  t4 = setByte(0x87, lowerHex);
  if(t2 && t3 && t4)
    return true;
  else
  {
#if LOGGING1
    Serial.println("setIncrementHex - Data Transmission Failed!");
#endif
    return false;
  }
  
}

boolean setStartFreq(long startFreq)
{
#if LOGGING3
  //double t1 = opClock / pow(2,29);
  //Serial.println(t1);
#endif
  long freqHex = startFreq / (opClock / pow(2, 29));
  if(freqHex > 0xFFFFFF)
  {
#if LOGGING1
    Serial.print("setStartFreq - Freqeuncy Overflow!");
#endif
    return false;
  }
  
  int lowerHex = freqHex % 256;
  int midHex = ((freqHex - (long)lowerHex) >> 8) % 256;
  int upperHex = freqHex >> 16;
#if LOGGING3  
  Serial.print("setStartFreq - freqHex: ");
  Serial.print(freqHex, HEX);
  Serial.print("\t");
  Serial.print("lower: ");
  Serial.print(lowerHex, HEX);
  Serial.print("\t");
  Serial.print("mid: ");
  Serial.print(midHex, HEX);
  Serial.print("\t");
  Serial.print("upper: ");
  Serial.print(upperHex, HEX);
  Serial.println();
#endif
  boolean t2, t3, t4;
  t2 = setByte(0x82, upperHex);
  t3 = setByte(0x83, midHex);
  t4 = setByte(0x84, lowerHex);
  if(t2 && t3 && t4)
    return true;
  else
  {
#if LOGGING1
    Serial.println("setStartFreq - Data Transmission Failed!");
#endif
    return false;
  }
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
 #if LOGGING1
  Serial.print("getTemperature - Current Temp. is ");
  Serial.print(cTemp);
  Serial.print("\n");
 #endif
  
  return cTemp;
}


int getByte(int address) {

  int rxByte;
#if LOGGING3  
  Serial.print("getByte - Initiating I2C Transmission. Address: ");
  Serial.print(address, HEX);
  Serial.print('\n');
#endif
  
  Wire.beginTransmission(AD5933_ADR);
  Wire.write(Address_Ptr);
  Wire.write(address);
  int i2cReturn = Wire.endTransmission();

#if LOGGING3
  Serial.print("getByte - Transmission Complete. i2cReturn: ");
  Serial.print(i2cReturn);
  Serial.print("\n");
#endif  
  
  Wire.requestFrom(AD5933_ADR, 1);

  if (1 <= Wire.available()) {
    rxByte = Wire.read();
#if LOGGING3   
    Serial.print("getByte - Message Received: ");
    Serial.print(rxByte,BIN);
    Serial.print(" or ");
    Serial.print(rxByte,HEX);    
    Serial.print("\n");
#endif
  } 
  else {
    rxByte = -1;
#if LOGGING1
    Serial.println("getByte - Failed to receive Message");
#endif
  }

  return rxByte;

}
boolean setByte(int address, int value) {
#if LOGGING3   
  Serial.print("setByte - Initiating I2C Transmission. Address: ");
  Serial.print(address, HEX);
  Serial.print(" , Value: ");
  Serial.print(value, HEX);
  Serial.print('\n');
#endif  
  Wire.beginTransmission(AD5933_ADR);
  Wire.write(address);
  Wire.write(value);
  int i2cReturn = Wire.endTransmission();

  if (i2cReturn)
  {
#if LOGGING1    
    Serial.println("setByte - Failed");
#endif
    return false;
  }
  else
  {
#if LOGGING3 
    Serial.println("setByte - Success");
#endif
    return true; 
  }
}

double getMag()
{
  int rComp, iComp;
  rComp = getRealComp();
  iComp = getImagComp();
  double result = sqrt( square((double)rComp) + square((double)iComp) );
#if LOGGING1 
  Serial.print("getMag - Resistance Magnitude is ");
  Serial.println(result);
#endif
  return result;
}

int getRealComp()
{
  int mReal, lReal;
  int result;
  mReal=getByte(0x94);
  lReal=getByte(0x95);
#if LOGGING3
  Serial.print("getRealComp - mReal: ");
  Serial.print(mReal, BIN);
  Serial.print('\t');
  Serial.print("lReal: ");
  Serial.println(lReal, BIN);
  Serial.print("getRealComp - Value: ");
#endif
  result = mReal*16*16+lReal;
#if LOGGING3
  Serial.println(result);
#endif
  return result;
}

int getImagComp()
{
  int mImag, lImag;
  int result;
  mImag=getByte(0x96);
  lImag=getByte(0x97);
#if LOGGING3
  Serial.print("getImagComp - mImag: ");
  Serial.print(mImag, BIN);
  Serial.print('\t');
  Serial.print("lImag: ");
  Serial.println(lImag, BIN);
  Serial.print("getImagComp - Value: ");
#endif
  result = mImag*16*16+lImag;
#if LOGGING3
  Serial.println(result);
#endif
  return result;
}
