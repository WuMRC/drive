// AD5258 library implementation via Arduino serial monitor by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// This sketch changes the wiper position for a AD5258 "r1" in a rheostat configuration.

#include "Wire.h"
#include "Math.h"
#include "AD5258.h" //Library for AD5933 functions (must be installed)

// Define bit clearing and setting variables

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define R 114

uint8_t bytesAvailable = 0;
uint8_t wiperPosition = 0;
uint8_t incomingByte = 0;
uint8_t wiperPositionHolder = 0;
boolean inputSucess = false;

AD5258 r1; // rheostat r1

void setup() {
  Wire.begin(); // Start Arduino I2C library
  Serial.begin(38400);
  Serial.println("Started");

  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1); // Clear bits in port
  r1.begin(1);
}

void loop() {

  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      delay(15); // Delay because Arduino serial buffer will return 1 if queried too quickly.
      bytesAvailable = Serial.available();
      incomingByte = Serial.read();
        if (incomingByte > 47 && incomingByte < 58){
        //Parse ASCII char value to int and cocatenate to form sample Rate
        wiperPositionHolder += ((pow(10, ((double) (bytesAvailable - 1)))) * (incomingByte - 48)) + 0.5; 
        inputSucess = true;
      }

      else {
        inputSucess = false;
      }

      if(wiperPositionHolder > 63) {
        inputSucess = false;
      }
    }
    if(inputSucess) {
      wiperPosition = wiperPositionHolder;
      Serial.print("wiper position: ");
      Serial.println(wiperPosition);
      r1.writeRDAC(wiperPosition);
      wiperPositionHolder = 0;
    }
    else {
      Serial.println("Error, bad value.");
    }
  }
}






























