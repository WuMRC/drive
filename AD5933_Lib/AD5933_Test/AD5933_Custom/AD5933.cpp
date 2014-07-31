// Library Code Section of AD5933

#include "AD5933.h"
//#include <WProgram.h>
#include <Arduino.h> // For the compatibility with Arduino Conventions.
#include <WConstants.h>

AD5933_Class AD5933;

bool AD5933_Class::performFreqSweep(double gainFactor, double *arrSave)
// Function to perform frequency Sweep, Just call it once to do it. It automatically do all the step.
// double gainFactor - You need to call getGainFactor(double,int)
//
// double *arrSave - Just put the name of the array to save it. It should have right number of entries to save it. 
// If not, hidden error will be occur.
{
  int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg
  if(setCtrMode(STAND_BY) == false)
  {
#if LOGGING1
    Serial.println("performFreqSweep - Failed to setting Stand By Status!");
#endif
    return false;
  }
  if(setCtrMode(INIT_START_FREQ) == false)
  {
#if LOGGING1
    Serial.println("performFreqSweep - Failed to setting initialization with starting frequency!");
#endif
    return false;
  }
  delay(delayTimeInit);
  if(setCtrMode(START_FREQ_SWEEP) == false)
  {
#if LOGGING1
    Serial.println("performFreqSweep - Failed to set to start frequency sweeping!");
#endif
    return false;
  }
  
  int t1=0;
  while( (getByte(0x8F) & 0x04) != 0x04 ) // Loop while if the entire sweep in not complete
  {
    delay(delayTimeInit);
    arrSave[t1]=gainFactor/getMagOnce(); // Calculated with Gain Factor
#if LOGGING1
    Serial.print("performFreqSweep - arrSave[");
    Serial.print(t1);
    Serial.print("] = ");
    Serial.println(arrSave[t1]);  
#endif
    if(setCtrMode(INCR_FREQ) == false)
    {
#if LOGGING1
      Serial.println("performFreqSweep - Failed to set for increasing frequency!");
#endif
      return false;
    }
    t1++;
    //getByte(0x80);
  }
  if(setCtrMode(POWER_DOWN) == false)
  {
#if LOGGING1
    Serial.println("performFreqSweep - Completed sweep, but failed to power down");
#endif
    return false;
  }
  return true; // Succeed!
}

double AD5933_Class::getGainFactor(double cResistance, int avgNum)
// A function to get Gain Factor. It performs one impedance measurement in start frequency.
// double cResistance - Calibration Resistor Value
// avgNum - number of measurement for averaging.
// Returns -1 if error occurs.
{
  int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg
  if(setCtrMode(STAND_BY) == false)
  {
#if LOGGING1
    Serial.println("getGainFactor - Failed to setting Stand By Status!");
#endif
    return -1;
  }
  if(setCtrMode(INIT_START_FREQ) == false)
  {
#if LOGGING1
    Serial.println("getGainFactor  - Failed to setting initialization with starting frequency!");
#endif
    return -1;
  }
  delay(delayTimeInit);
  if(setCtrMode(START_FREQ_SWEEP) == false)
  {
#if LOGGING1
    Serial.println("getGainFactor - Failed to set to start frequency sweeping!");
#endif
    return -1;
  }
  
  int t1 = 0;
  long tSum = 0;
  while(t1 < avgNum) // Until reached pre-defined number for averaging.
  {
    tSum += getMagOnce();
    if(setCtrMode(REPEAT_FREQ) == false)
    {
#if LOGGING1
    	Serial.println("getGainFactor - Failed to set to repeat this frequency!");
#endif
    	return -1;
    }
    t1++;  
  }
  double mag = tSum/(double)avgNum;
  if( setCtrMode(STAND_BY) == false)
  {
#if LOGGING1
	Serial.println("getGainFactor - Failed to set into Stand-By Status");
#endif  	
  	return -1;
  }
  resetAD5933();
    // Gain Factor is different from one of the datasheet in this program. Reciprocal Value.
#if LOGGING2
  Serial.print("getGainFactor - Gain Factor: ");
  Serial.println(mag*cResistance);

#endif
  return mag * cResistance;
 
}

double AD5933_Class::getGainFactor(double cResistance)
// Calculate Gain Factor with measuring once.
{
  return getGainFactor(cResistance, 1);
}

bool AD5933_Class::setCtrMode(byte modetoSet)
// setting Control Register to change control mode without assuming control register. (0x80)
{
  return setCtrMode(modetoSet, getByte(0x80));
}

bool AD5933_Class::setCtrMode(byte modetoSet, int ctrReg)
// setting Control Register to change control mode.
//
{
  ctrReg &= 0x0F; // Get the last 4 digits.
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
    case TEMP_MEASURE:
      ctrReg |= 0x90;
      break;
    default:
#if LOGGING1
      Serial.println("setCtrMode - Invalid Parameter!");
#endif
      return false; // return the signal of fail if there is not valid parameter.
      break;
  } 
  return setByte(0x80, ctrReg); // return signal depends on the result of setting control register.
 
}

bool AD5933_Class::setVolPGA(byte voltageNum, byte pgaGain)
// Function to set sweep voltage and PGA Gain
// byte voltageNum - Refer to Table 10 in datasheet
// 1 - 2.0Vpp 	2 - 200mVpp		3- 400mVpp		4- 1.0Vpp
// (They are typical values with 3.3V power supply. See Figure 4-10 in datasheet)
// byte pgaGain - determines the gain of ADC signal. (to convert the current into the value)
//
{
  if( (voltageNum < 0 || voltageNum > 3) || !(pgaGain == 1 || pgaGain == 5) )  
  {
#if LOGGING1
    Serial.println("setVolPGA - invaild parameter");
#endif
    return false;
  } 
  int temp = getByte(0x80); // Get the content of Control Register and put it into temp
  temp &= 0xF0; // discard the last 4 digits.
  temp |= voltageNum << 1; // Shift one digits to fit in D9 and D10. (p 23-24/40 in datasheet)
  if(pgaGain == 1)
    temp |= 0x01; // if PGA Gain is x1, then write 1 at D8.
  else
    temp &= 0xFE; // if PGA Gain is x5, then write 0 at D8.
#if LOGGING2
  Serial.print("setVolPGA - Final Value to Set: ");
  Serial.println(temp, BIN);
#endif
  return setByte(0x80,temp); // Write value at 0x80 Register.
}

bool AD5933_Class::setExtClock(bool swt)
// A function to enable/disable external clock.
// This function also sets environmental variable. (double opClock)
{
  byte t1;
  if( swt )
  {
    t1 = 0x04; // Use Ext. Clock
    opClock = 16000000;
  }
  else
  {
    t1 = 0x00; // Use Int. Clock
    opClock = 16776000;
  }  
  return setByte(0x81, t1); // Write register 0x81.
}

bool AD5933_Class::resetAD5933()
// Set Reset Bit(D4) in control register 0x81.
{
  int temp = (getByte(0x81) & 0x04); // Read 0x81 with retrieving D3
  return setByte(0x81, (temp | 0x10)); // Set D4 as 1 (Reset Bit)
}

bool AD5933_Class::setSettlingCycles(int cycles, byte mult)
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
  byte t1; // Parsing upper and lower bits.
/*switch(mult)
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
  }*/
  t1--; // Enhanced Code for setting t1.
  upperHex |= (t1 << 1); 	// t1 is for D9, D10. The upperHex just accounts for D8. Thus, the value after left-shifting t1 accounts for D9, D10.
  							// Thus, this above writes bits for D9, D10.
#if LOGGING2
  Serial.print("setSettlingCycles - upper: ");
  Serial.println(upperHex,BIN);
#endif
  bool t2, t3;
  t2=setByte(0x8A, upperHex);
  t3=setByte(0x8B, lowerHex);
  if( t2 && t3 ) // Checking if successful.
    return true;  // Succeed! 
  else
  {
#if LOGGING1
    Serial.println("setSettingCycles - Data Write Fail");
#endif
    return false;
  }
}

bool AD5933_Class::setNumofIncrement(int num)
// Function to set the number of incrementing.
// int num - the number of incrementing.
{
  if(num > 0x1FF + 1)
  {
#if LOGGING1
    Serial.print("setNumofIncrement - Frequency Overflow!");
#endif
    return false;
  }
  
  num--; 	// Decrement due to the internal interpretation.
	  		// Example: If the value is 2, it performs 3 times.
  int lowerHex = num % 256;
  int upperHex = (num >> 8) % 2; // Parsing number for register input.
  
  bool t2, t4;
  t2 = setByte(0x88, upperHex);
  t4 = setByte(0x89, lowerHex);
  if(t2 && t4)
    return true; // Succeed!
  else
  {
#if LOGGING1
    Serial.println("setNumofIncrement - Data Transmission Failed!");
#endif
    return false;
  }
}

bool AD5933_Class::setIncrement(long increment)
// Function to set increment frequency.
// Because the increment frequency should be converted into unique Hexadecimal number, it approximately calculates the Hex value.
// long increment - increment frequency in Hz.
{
  long freqHex = increment / (opClock / pow(2, 29)); // Based on the data sheet.
  return setIncrementinHex(freqHex); // Call setIncrementinHex(long);
}

bool AD5933_Class::setIncrementinHex(long freqHex)
// Function to set increment frequency in converted Hex value. (calculated based on the datasheet.)
// long freqHex - converted hexadecimal value
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
  
  bool t2, t3, t4;
  t2 = setByte(0x85, upperHex);
  t3 = setByte(0x86, midHex);
  t4 = setByte(0x87, lowerHex);
  if(t2 && t3 && t4)
    return true; // Succeed!
  else
  {
#if LOGGING1
    Serial.println("setIncrementHex - Data Transmission Failed!");
#endif
    return false;
  }
  
}

bool AD5933_Class::setStartFreq(long startFreq) // long startFreq in Hz
{
#if LOGGING3
  //double t1 = opClock / pow(2,29);
  //Serial.println(t1);
#endif
  long freqHex = startFreq / (opClock / pow(2, 29)); // based on datasheet
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
  bool t2, t3, t4;
  t2 = setByte(0x82, upperHex);
  t3 = setByte(0x83, midHex);
  t4 = setByte(0x84, lowerHex);
  if(t2 && t3 && t4)
    return true; // succeed!
  else
  {
#if LOGGING1
    Serial.println("setStartFreq - Data Transmission Failed!");
#endif
    return false;
  }
}

byte AD5933_Class::getStatusReg() // TODO: I might try to change into inline function. / Make functions use this.
{
  return getByte(0x8F) & 0x07;
}

double AD5933_Class::getTemperature()
// Function to get temperature measurement.
{
  //setByte(0x80,0x90); // Read Temp. TODO: use function.
  if(setCtrMode(TEMP_MEASURE) == false)
  {
#if LOGGING1
	Serial.println("getTemperature - Failed to set the control bit");
#endif
  	return false;
  }
  delay(delayTimeInit);
  
  int tTemp[2];
  long tTempVal;
  double cTemp;
  
  while( getByte(0x8F) & 0x01 != 0x01)
  {
  	; // Wait Until Get Vaild Temp. Measurement.
  }
  
  tTemp[0] = getByte(0x92);
  tTemp[1] = getByte(0x93);
  tTempVal = (tTemp[0] % (12 * 16))*16*16 + tTemp[1];
  if(bitRead(tTemp[0],5) == 0)
  {
    // Positive Formula
    cTemp = (double)tTempVal/32;
  }
  else
  {
    // Negative Formula
    cTemp = (tTempVal-16384.0) / 32;
  }
 #if LOGGING1
  Serial.print("getTemperature - Current Temp. is ");
  Serial.print(cTemp);
  Serial.print("\n");
 #endif
  
  return cTemp;
}


int AD5933_Class::getByte(int address) {
// Hidden Function to get register value via I2C Transmission.

  int rxByte;
#if LOGGING3  
  Serial.print("getByte - Initiating I2C Transmission. Address: ");
  Serial.print(address, HEX);
  Serial.print('\n');
#endif
  
  Wire.beginTransmission(AD5933_ADR); // Begin I2C Transmission with AD5933 Chip.
  Wire.write(Address_Ptr); // Send Address Pointer to write the target address
  Wire.write(address); // Write address to read.
  int i2cReturn = Wire.endTransmission(); // End Transmission.

#if LOGGING3
  Serial.print("getByte - Transmission Complete. i2cReturn: ");
  Serial.print(i2cReturn);
  Serial.print("\n");
#endif  
  
  Wire.requestFrom(AD5933_ADR, 1); // Request the value of the written address.

  if (1 <= Wire.available()) { // If the MCU get the value,
    rxByte = Wire.read(); // Read the value.
#if LOGGING3   
    Serial.print("getByte - Message Received: ");
    Serial.print(rxByte,BIN);
    Serial.print(" or ");
    Serial.print(rxByte,HEX);    
    Serial.print("\n");
#endif
  } 
  else {
    rxByte = -1; // Returns -1 if fails.
#if LOGGING1
    Serial.println("getByte - Failed to receive Message");
#endif
  }

  return rxByte;

}

bool AD5933_Class::setByte(int address, int value) {
// Hidden Function to transmit the value to write.
#if LOGGING3   
  Serial.print("setByte - Initiating I2C Transmission. Address: ");
  Serial.print(address, HEX);
  Serial.print(" , Value: ");
  Serial.print(value, HEX);
  Serial.print('\n');
#endif  
  Wire.beginTransmission(AD5933_ADR); // Begin I2C Transmission.
  Wire.write(address); // Write Address
  Wire.write(value); // Write Value
  int i2cReturn = Wire.endTransmission(); // Terminate the transmission.

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

double AD5933_Class::getMagValue()
// Hidden Function to get magnitude value of impedance measurement. (It does not wait.)
{
  int rComp, iComp;
  rComp = getRealComp(); // Getting Real Component
  iComp = getImagComp(); // Getting Imaginary Component
  double result = sqrt( square((double)rComp) + square((double)iComp) ); // Calculating magnitude.
#if LOGGING3 
  Serial.print("getMagValue - Resistance Magnitude is ");
  Serial.println(result);
#endif
  return result;
}

double AD5933_Class::getMagOnce()
// Wrapper Function of getMagValue. It waits until the ADC completes the conversion.
{
  while((getByte(0x8F) & 0x02) != 0x02) // wait until ADC conversion is complete.
  {
    delay(delayTimeInit);
  }
  return getMagValue();  
}

int AD5933_Class::getRealComp()
// Function to get real component.
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

int AD5933_Class::getImagComp()
// Function to get imaginary component.
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