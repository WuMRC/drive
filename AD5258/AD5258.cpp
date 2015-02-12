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
  Serial.println("coming online...");
  deviceAddress = AD5258_ADR1;
  return true;

    case 2:
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println("coming online...");
  deviceAddress = AD5258_ADR3;
  return true;

    case 3:
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println("coming online...");
  deviceAddress = AD5258_ADR2;
  return true;

    case 4:
  Serial.print("AD5258 of index ");
  Serial.print(addressIndex);
  Serial.println("coming online...");
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
void AD5258::writeRDAC(byte value)
{
  write8(deviceAddress, RDAC_ADDRESS, value);
}

void AD5258::writeEEPROM(byte value)
{
  write8(deviceAddress, EEPROM_ADDRESS, value);
}

void AD5258::toggleSoftWriteProtect(bool state)
{
  byte value = 0;
  if(state) {
    value = SOFT_WRITE_PROTECT_ON;
    write8(deviceAddress, SOFT_WRITE_PROTECT_ADDRESS, value);
  }

  else {
    value = SOFT_WRITE_PROTECT_OFF;
    write8(deviceAddress, SOFT_WRITE_PROTECT_ADDRESS, value);
  }
}

void AD5258::write8(byte address, byte reg, byte value)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

/*void AD5258::read()
{
  // Read the accelerometer
  Wire.beginTransmission((byte)LSM303_ADDRESS_ACCEL);
  Wire.write(LSM303_REGISTER_ACCEL_OUT_X_L_A | 0x80);
  Wire.endTransmission();
  Wire.requestFrom((byte)LSM303_ADDRESS_ACCEL, (byte)6);

  // Wait around until enough data is available
  while (Wire.available() < 6);

  uint8_t xlo = Wire.read();
}



byte AD5258::read8(byte address, byte reg)
{
  byte value;

  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)1);
  value = Wire.read();
  Wire.endTransmission();

  return value;
}*/
