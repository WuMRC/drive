/***************************************************************************
This is a library for the AD5258 Potentiometer
Based on the library of Adafruit's LSM303 by Kevin Townsend
Initial Draft By Adetunji Dahunsi <tunjid.com>
Modified by Il-Taek Kwon
Updates should always be available at https://github.com/WuMRC
***************************************************************************/

#include <AD5258.h>

/***************************************************************************
 Initialization Function
 ***************************************************************************/
// addressIndex is defined like the datasheet.
// However, the number for addressIndex is shifted. You can see the comments of header file.
// e.g. GND on AD0 & AD1 == (addressIndex = 1)
bool AD5258::begin(uint8_t addressIndex, bool exeBegin)
{
  if(exeBegin)
  	Wire.begin();

  switch (addressIndex) {
    case 1:
#if VERBOSE
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println(" is online.");
#endif
  deviceAddress = AD5258_ADR1;
  return true;

    case 2:
#if VERBOSE
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println(" is online.");
#endif
  deviceAddress = AD5258_ADR2;
  return true;

    case 3:
#if VERBOSE
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println(" is online.");
#endif
  deviceAddress = AD5258_ADR3;
  return true;

    case 4:
#if VERBOSE
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println(" is online.");
#endif
  deviceAddress = AD5258_ADR4;
  return true;

  default:
#if VERBOSE
  Serial.print("AD5258 initialization failed.");
#endif
  return false;
  }

}

bool AD5258::begin(uint8_t addressIndex)
{
	return begin(addressIndex, true);
}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/
// Used to write RDAC register - General Register to manipulate the wiper
void AD5258::writeRDAC(uint8_t value)
{
  write8(deviceAddress, RDAC_ADDRESS, value);
}

// Used to write EEPROM register - register saving custom "default" value
void AD5258::writeEEPROM(uint8_t value)
{
  write8(deviceAddress, EEPROM_ADDRESS, value);
}

uint8_t AD5258::readRDAC()
{
	read8(deviceAddress, RDAC_ADDRESS);
}

uint8_t AD5258::readEEPROM()
{
	read8(deviceAddress, EEPROM_ADDRESS);
}

void AD5258::toggleSoftWriteProtect(bool state)
{
  uint8_t value = 0;
  if(state) {
    value = SOFT_WRITE_PROTECT_ON;
    write8(deviceAddress, SOFT_WRITE_PROTECT_ADDRESS, value);
  }

  else {
    value = SOFT_WRITE_PROTECT_OFF;
    write8(deviceAddress, SOFT_WRITE_PROTECT_ADDRESS, value);
  }
}

void AD5258::write8(uint8_t address, uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Returns the Fraction of Tolerance
double AD5258::readTolerance() 
{
	uint8_t integer = read8(deviceAddress, TOLERANCE_ADDRESS_A);
	uint8_t decimal  = read8(deviceAddress, TOLERANCE_ADDRESS_B);
	double answer = 0;

	if((integer & 0x80) == 0) { // Value is positive
		answer = (double) integer;
		answer += ((double) decimal) / 256;
	}
	else { // value is negative
		//integer = integer << 1;
		//integer = integer >> 1; // Clear MSB describing sign.
		integer = integer & 0x7F;
		answer = (double) integer;
		answer += -1 * (((double) decimal) / 256);
	}
 return answer;
}

uint8_t AD5258::read8(uint8_t address, uint8_t reg)
{
  uint8_t value;

  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(address, (uint8_t)1);
  value = Wire.read();
  Wire.endTransmission();

  return value;
}
