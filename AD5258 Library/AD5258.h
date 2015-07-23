/***************************************************************************
This is a library for the AD5258 Potentiometer
Based on the library of Adafruit's LSM303 by Kevin Townsend
Initial Draft By Adetunji Dahunsi <tunjid.com>
Modified by Il-Taek Kwon
Updates should always be available at https://github.com/WuMRC
***************************************************************************/

#ifndef __AD5258_H__
#define __AD5258_H__

#include<math.h> // for math functions
#include<Wire.h> // for I2C communications

typedef uint8_t byte; // For the compatibility for Arduino Type Definitions

#include "Wire.h"

#define AD5258_ADR1 					        0x18 // AD1 - GND, AD0 - GND								
#define AD5258_ADR2 					        0x1A // AD1 - VDD, AD0 - GND								
#define AD5258_ADR3 					        0x4C // AD1 - GND, AD0 - VDD								
#define AD5258_ADR4 					        0x4E // AD1 - VDD, AD0 - VDD
#define RDAC_ADDRESS 					        0x00
#define EEPROM_ADDRESS  				      0x20 
#define TOLERANCE_ADDRESS_A           0x3E 
#define TOLERANCE_ADDRESS_B           0x3F                								
#define SOFT_WRITE_PROTECT_ADDRESS 	 	0x40
#define SOFT_WRITE_PROTECT_OFF 			  0x00 
#define SOFT_WRITE_PROTECT_ON 			  0x01  								

class AD5258
{
  public:

    bool begin(uint8_t);
    bool begin(uint8_t, bool);
    void writeRDAC(uint8_t value);
    void writeEEPROM(uint8_t value);
    void toggleSoftWriteProtect(bool state);
    double readTolerance();
    uint8_t readRDAC();
    uint8_t readEEPROM();    

  private:
	uint8_t deviceAddress;
	uint8_t read8(uint8_t address, uint8_t reg);
    void write8(uint8_t address, uint8_t reg, uint8_t value);
};

#endif
