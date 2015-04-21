//Pin for I2C: Uno, Ethernet	A4 (SDA), A5 (SCL)
//Acknowledgement: Some codes are inspried by https://github.com/open-ephys/autoimpedance
// This file is for the step before making header file.
// These codes will be a class.

#include <Wire.h>
#include <math.h>

#define LOGGING1 1 // Basic, Error
#define LOGGING2 1 // in Dev
#define LOGGING3 0 // Detailed

const byte Address_Ptr = 0xB0;
#define AD5933_ADR 0x0D
const double opClock = 16000000;

int getByte(int);
boolean setByte(int, int);
double getTemperature();
int getRealComp();
int getImagComp();
<<<<<<< HEAD
double getMagValue();
double getMagOnce();
=======
double getMag();
>>>>>>> FETCH_HEAD
byte getStatusReg();
boolean setStartFreq(long);
boolean setIncrement(long);
boolean setIncrementinHex(long);
boolean setNumofIncrement(int);
boolean setSettlingCycles(int, byte);
boolean resetAD5933();
boolean setExtClock(boolean);
boolean setVolPGA(byte, byte);

boolean setCtrMode(byte);
boolean setCtrMode(byte, int);
#define INIT_START_FREQ 1
#define START_FREQ_SWEEP 2
#define INCR_FREQ 3
#define REPEAT_FREQ 4
<<<<<<< HEAD
#define POWER_DOWN 10
#define STAND_BY 11

double getGainFactor(double);
=======
#define POWER_DOWN A
#define STAND_BY B
>>>>>>> FETCH_HEAD

// (Voltage, PGA Gain)
// Voltage: 0-2Vpp / 1-0.2Vpp / 2-0.4Vpp / 3-1Vpp

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  
  //setByte(0x81, 0x18); // Reset & Use Ext. Clock - 0001 1000
  setExtClock(true);
  resetAD5933();
}

void loop()
{
  Serial.println("Loop Begin!");
  setStartFreq(50000);
  setIncrementHex(1);
  setNumofIncrement(2);
  setSettlingCycles(0x1FF, 4);
  getTemperature();
  setVolPGA(0,1);
<<<<<<< HEAD
  Serial.println("Please setup for calibration. If completed, press p and Enter>");
  while( Serial.read() != 'p')
    ;
  double gainFactor = getGainFactor(4700);
  Serial.println("Change resistor to measure! If completed, press p and Enter>");
  while( Serial.read() != 'p')
    ;
=======
  
>>>>>>> FETCH_HEAD
  
  
  Serial.println("Loop End!");
  Serial.println();
  delay(5000);
}

<<<<<<< HEAD
double getGainFactor(double cResistance)
{
  int ctrReg = getByte(0x80);
  setCtrMode(STAND_BY);
  setCtrMode(INIT_START_FREQ);
  delay(100);
  setCtrMode(START_FREQ_SWEEP);
  
  double mag = getMagOnce();
  resetAD5933();
  // Gain Factor is different from one of the datasheet in this program. Reciprocal Value.
#if LOGGING2
  Serial.print("getGainFactor - Gain Factor: ");
  Serial.println(mag*cResistance);

#endif
  return mag * cResistance;
 
}

=======
>>>>>>> FETCH_HEAD
boolean setCtrMode(byte modetoSet)
{
  return setCtrMode(modetoSet, getByte(0x80));
}

boolean setCtrMode(byte modetoSet, int ctrReg)
{
<<<<<<< HEAD
  ctrReg &= 0x0F;
  switch(modetoSet)
  {
    case INIT_START_FREQ:
      ctrReg |= 0x10;
      break;
    case START_FREQ_SWEEP:
      ctrReg |= 0x20;
      break;
    case INCR_FREQ:
      ctrReg |= 0x30;
      break;
    case REPEAT_FREQ:
      ctrReg |= 0x40;
      break;
    case POWER_DOWN:
      ctrReg |= 0xA0;
      break;
    case STAND_BY:
      ctrReg |= 0xB0;
      break;
    default:
#if LOGGING1
      Serial.println("setCtrMode - Invalid Parameter!");
#endif
      return false;
      break;
  } 
  return setByte(0x80, ctrReg);
 
=======
   
>>>>>>> FETCH_HEAD
}

boolean setVolPGA(byte voltageNum, byte pgaGain)
{
  if( (voltageNum < 0 || voltageNum > 3) || !(pgaGain == 1 || pgaGain == 5) )  
  {
#if LOGGING1
    Serial.println("setVolPGA - invaild parameter");
#endif
    return false;
  } 
  int temp = getByte(0x80);
  temp &= 0xF0;
  temp |= voltageNum << 1;
  if(pgaGain == 1)
    temp |= 0x01;
  else
    temp &= 0xFE;
#if LOGGING2
  Serial.print("setVolPGA - Final Value to Set: ");
  Serial.println(temp, BIN);
#endif
  return setByte(0x80,temp);
}

boolean setExtClock(boolean swt)
{
  byte t1;
  if( swt )
    t1 = 0x04;
  else
    t1 = 0x00;
  return setByte(0x81, t1);
}

boolean resetAD5933()
{
  int temp = (getByte(0x81) & 0x04);
  return setByte(0x81, (temp | 0x10));
}

boolean setSettlingCycles(int cycles, byte mult)
{
  if(cycles > 0x1FF || !(mult == 1 || mult == 2 || mult == 4) )
  {
#if LOGGING1
    Serial.println("setSettlingCycles - Invalid Parameter");
#endif
    return false;
  }
  int lowerHex = cycles % 256;
  int upperHex = ((cycles - (long)lowerHex) >> 8) % 2;
  byte t1;
  switch(mult)
  {
    case 1:
      t1 = 0;
      break;
    case 2:
      t1 = 1;
      break;
    case 4:
      t1 = 3;
      break;
    default:
#if LOGGING1
    Serial.println("setSettlingCycles - Invalid Mult Parameter");
#endif
    return false;
    break;    
  }
  upperHex |= (t1 << 1);
#if LOGGING2
  Serial.print("setSettlingCycles - upper: ");
  Serial.println(upperHex,BIN);
#endif
  boolean t2, t3;
  t2=setByte(0x8A, upperHex);
  t3=setByte(0x8B, lowerHex);
  if( t2 && t3 )
    return true;
  else
  {
#if LOGGING1
    Serial.println("setSettingCycles - Data Write Fail");
#endif
    return false;
  }
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

<<<<<<< HEAD
double getMagValue()
=======
double getMag()
>>>>>>> FETCH_HEAD
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

<<<<<<< HEAD
double getMagOnce()
{
  while((getByte(0x8F) & 0x02) != 0x02)
  {
    delay(100);
  }
  return getMagValue();  
}

=======
>>>>>>> FETCH_HEAD
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
