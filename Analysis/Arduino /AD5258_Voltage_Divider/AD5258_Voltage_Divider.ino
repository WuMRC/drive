// AD5258 voltage divider by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// This sketch creates a simple voltage divider used to calibrate the AD5933.

#include "Wire.h"
#include "Math.h"
#include "AD5258.h" //Library for AD5933 functions (must be installed)

#define nOfLevels 1000 // number of repetitons

#define REFERENCE_VOLTAGE_PIN 8 // This pin is connected to a analog potentiometer to andjust vIN.

#define RAB_1 1235 // End to end resistance for potentiometer 1
#define RAB_2 1213 // End to end resistance for potentiometer 2

#define vIn 0.201 // Supply voltage to AD5258. Must be 0.2 V or less, else risk over current to AD5258.

int rawAnalogR1 = 0;  // Raw value read from arduino analog pin A1
int rawAnalogR2 = 0; // Raw value read from arduino analog pin A2

float vOutR1 = 0; // Voltage read from analog pin A1
float vOutR2 = 0; // Voltage read from analog pin A2

float RWB_1 = 0; // Resistance between point B and the wiper for potentiometer 1
float RWB_2 = 0; // Resistance between point B and the wiper for potentiometer 2

AD5258 pot1; // potentiometer 2
AD5258 pot2; // potentiometer 2

void setup() {
  Serial.println();
  Wire.begin(); // Start Arduino I2C library
  Serial.begin(38400);

  // MUST BE DONE BEFORE analogRead(pin) IS CALLED!
  // Set analog reference voltage to 0.2 volts so AD5258 and arduino are not damaged.
  analogReference(EXTERNAL); 

  pinMode(REFERENCE_VOLTAGE_PIN, OUTPUT);
  digitalWrite(REFERENCE_VOLTAGE_PIN, HIGH); 

  pot1.begin(1);
  pot2.begin(2);

  Serial.println();
  Serial.println();
}

void loop() {

  while (Serial.available() < 1) {
    delay(15);
  } // Wait for user to move electrodes before starting
  
  Serial.read(); // Read data input in the serial monitor to clear it.

  for(int i = 0; i < nOfLevels; i++) { // repetition loop

    // Only collect data within the linear range of AD5933. Take 50 points.
    for(int potStep = 14; potStep < 64; potStep++) {

      pot1.writeRDAC(potStep);
      pot2.writeRDAC(potStep);

      rawAnalogR1 = analogRead(A1);
      rawAnalogR2 = analogRead(A2);

      vOutR1 = rawAnalogR1 * (vIn / 1023.0);    
      vOutR2 = rawAnalogR2 * (vIn / 1023.0);

      RWB_1 = RAB_1 * (1 - (vOutR1 / vIn));
      RWB_2 = RAB_2 * (1 - (vOutR2 / vIn));

      Serial.print(potStep);
      Serial.print(",");
      Serial.print(RWB_1, 6);
      Serial.print(",");
      Serial.println(RWB_2, 6);
      
    } // end potentiometer step loop
  } // end repetition loop
} // end main loop

