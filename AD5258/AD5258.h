/***************************************************************************
This is a library for the AD5258 Potentiometer
By Adetunji Dahunsi <tunjid.com>
Updates should always be available at https://github.com/WuMRC
***************************************************************************/

#ifndef __AD5258_H__
#define __AD5258_H__

#include "Wire.h"

#define AD5258_ADR1 					0x18 								
#define AD5258_ADR2 					0x1A 								
#define AD5258_ADR3 					0x4C 								
#define AD5258_ADR4 					0x4E
#define RDAC_ADDRESS 					0x00
#define EEPROM_ADDRESS  				0x20 								
#define SOFT_WRITE_PROTECT_ADDRESS 	 	0x40
#define SOFT_WRITE_PROTECT_OFF 			0x00 
#define SOFT_WRITE_PROTECT_ON 			0x01  								




class AD5258
{
  public:
  	int deviceAddress;
    bool begin(int);
    void read(void);
    void writeRDAC(byte address, byte value);
    void writeEEPROM(byte address, byte value);
    void toggleSoftWriteProtect(byte address, bool state);
    //byte readRDAC(byte address, byte reg);
    //byte readEEPROM(byte address, byte reg);    

  private:
};

#endif
