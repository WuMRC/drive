// Companion to Agilent e4980 Python control by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// This sketch is triggered by a remote controller running a Python script
// to control Agilent's e4980a via USBTMC

#include "Wire.h"
#include "AD5258.h" //Library for AD5258 functions (must be installed)

#define SSR 11
#define indicator_LED 12


AD5258 digipot; // rheostat r1
uint8_t wiper = 0;

void setup() {
  Serial.begin(38400);
  Wire.begin();

  pinMode(indicator_LED, OUTPUT);

  digipot.begin(2); // Specify i2c address for digipot
  Serial.println();
}

void loop() {

  if (Serial.available()) {
    uint8_t ch = Serial.read();
    uint8_t status;

    if (ch == '0') {
      digitalWrite(indicator_LED, LOW);
      Serial.println("Switch off.");
    }

    if (ch == '1') {
      if(wiper == 64) {
        wiper = 0;
      }
      digitalWrite(indicator_LED, HIGH);
      digipot.writeRDAC(wiper);
      Serial.print("Triggered. wiper is at ");
      Serial.println(wiper);
      wiper++;
    }

  } // end if serial available
} // end main loop



























