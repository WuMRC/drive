// AD5933 library implementation via Arduino serial monitor by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// This sketch prints out constituents for a MATLAB Map object "impedanceMap".
// Values of [f,r1,r2,c,key] are mapped to [z,r,x].

#include "Wire.h"
#include "AD5258.h" //Library for AD5258 functions (must be installed)

#define SSR 11
#define indicator_LED 12


AD5258 r1; // rheostat r1

AD5258 r2; // rheostat r2

void setup() {
  Serial.begin(38400);
  Wire.begin();

  pinMode(SSR, OUTPUT);
  pinMode(indicator_LED, OUTPUT);

  r1.begin(1);
  r2.begin(2);
  Serial.println();
}

void loop() {

  digitalWrite(indicator_LED, LOW); // Indication to switch capacitors.

  while (Serial.available() < 1) {
    delay(15);
  } // Wait for user to swap capacitors befor triggering

  Serial.read(); // Read the key entered and continue the program.

  digitalWrite(indicator_LED, HIGH);  // Indication program is running.

  for(int i = 0; i < 4; i++) { // repetition loop

    Serial.print("Current iteration is ");
    Serial.print(i + 1);
    Serial.println(".");

    for(int R1 = 0; R1 < 64; R1++) {  // r1 loop

      r1.writeRDAC(R1);

      Serial.print("Changed rheostat. Wiper position is ");
      Serial.print(R1 + 1);
      Serial.println(".");

      for(int f = 0; f < 99; f++) { // LCR frequency loop (201 steps).

        triggerLCR();

        Serial.print("Triggered. Frequency is ");
        Serial.print(f + 2);
        Serial.println(" KHz.");

      } // end LCR frequency loop
    } // end r1 loop
  } // end repetition loop
} // End main loop

void triggerLCR() {
  digitalWrite(SSR, HIGH);
  delay(100);
  digitalWrite(SSR, LOW);
  delay(100);
}























