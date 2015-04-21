// Companion to Agilent e4980 Python control by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// This sketch is triggered by a remote controller running a Python script
// to control Agilent's e4980a via USBTMC

#include "Wire.h"
#include "AD5258.h" //Library for AD5258 functions (must be installed)

#define SSR 11
#define indicator_LED 12


AD5258 R1; // rheostat r1
AD5258 R2; // rheostat r1

uint8_t r1 = 0;
uint8_t r2 = 0;

void setup() {
  Serial.begin(38400);
  Wire.begin();

  pinMode(indicator_LED, OUTPUT);

  R1.begin(1); // Specify i2c address for r1
  R2.begin(2); // Specify i2c address for r2
  Serial.println();
}

void loop() {

  if (Serial.available()) {
    uint8_t ch = Serial.read();
    uint8_t status;

    if (ch == '0') {
      digitalWrite(indicator_LED, LOW);
      Serial.println("Program done.");
    }

    if (ch == '1') {
      if(r1 == 64) {
        r1 = 0;
      }
      digitalWrite(indicator_LED, HIGH);
      R1.writeRDAC(r1);
      Serial.print("Triggered R1. wiper is at ");
      Serial.println(r1);
      r1++;
    }

    if (ch == '2') {
      if(r2 == 64) {
        r2 = 0;
      }
      digitalWrite(indicator_LED, HIGH);
      R2.writeRDAC(r2);
      Serial.print("Triggered R2. wiper is at ");
      Serial.println(r2);
      r2++;
    }
    

  } // end if serial available
} // end main loop




























