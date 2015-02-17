/***************************************************************************
This is a library for the AD5258 Potentiometer
It is based on the library of Adafruit's LSM303 by Kevin Townsend
By Adetunji Dahunsi <tunjid.com>
Updates should always be available at https://github.com/WuMRC
***************************************************************************/

#include <AD5258.h>

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/
bool AD5258::begin(int addressIndex)
{
  Wire.begin();

  switch (addressIndex) {
    case 1:
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println(" is online.");
  deviceAddress = AD5258_ADR1;
  return true;

    case 2:
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println(" is online.");
  deviceAddress = AD5258_ADR2;
  return true;

    case 3:
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println(" is online.");
  deviceAddress = AD5258_ADR3;
  return true;

    case 4:
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println(" is online.");
  deviceAddress = AD5258_ADR4;
  return true;

  default:
  Serial.print("AD5258 initialization failed.");
  return false;
  }

}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/
void AD5258::writeRDAC(uint8_t value)
{
  write8(deviceAddress, RDAC_ADDRESS, value);
}

void AD5258::writeEEPROM(uint8_t value)
{
  write8(deviceAddress, EEPROM_ADDRESS, value);
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
integer = integer << 1;
integer = integer >> 1; // Clear MSB describing sign.
answer = (double) integer;
answer += -1 * (((double) decimal) / 256);
}
 return answer;
}

/*void AD5258::read()
{
  // Read the accelerometer
  Wire.beginTransmission((uint8_t)LSM303_ADDRESS_ACCEL);
  Wire.write(LSM303_REGISTER_ACCEL_OUT_X_L_A | 0x80);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)LSM303_ADDRESS_ACCEL, (uint8_t)6);

  // Wait around until enough data is available
  while (Wire.available() < 6);

  uint8_t xlo = Wire.read();
}
*/


uint8_t AD5258::read8(uint8_t address, uint8_t reg)
{
  uint8_t value;

  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(address, (uint8_t)1);
  value = Wire.read();
  Wire.endTransmission();

  return value;
}
