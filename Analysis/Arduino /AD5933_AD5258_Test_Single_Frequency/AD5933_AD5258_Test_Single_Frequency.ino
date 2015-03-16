#include "Wire.h"
#include "Math.h"
#include "AD5933.h" //Library for AD5933 functions (must be installed)
#include "AD5258.h" //Library for AD5258 functions (must be installed)


#define TWI_FREQ 400000L      // Set TWI/I2C Frequency to 400MHz.

#define cycles_base 15       // Cycles to ignore before a measurement is taken. Max is 511.

#define cycles_multiplier 1    // Multiple for cycles_base. Can be 1, 2, or 4.

#define cal_resistance 353  // Calibration resistance for the gain factor. 

#define cal_samples 10         // Number of measurements to take of the calibration resistance.

#define nOfLevels 100 // 10 levels, with 3 factors. Frequency has 99 levels though.

#define fIncrements 98
#define indicator_LED 12


// Define bit clearing and setting variables

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int ctrReg = 0; // Initialize control register variable.

double startFreqHz = 50000; // AC Start frequency (Hz).

double stepSizeHz = 1000; // AC frequency step size between consecutive values (Hz).

double Z_Value = 0;          // Initialize impedance magnitude.

double phaseAngle = 0;       // Initialize phase angle value.

double temp = 0; // Used to update AD5933's temperature.

double gainFactor = 0;      // Initialize Gain factor.

double systemPhaseShift = 0;       // Initialize system phase shift value.


AD5258 r1; // rheostat r1

AD5258 r2; // rheostat r2

void setup() {
  Serial.begin(38400);
  Wire.begin();

  Serial.println();
  Serial.println();
  Serial.println("Starting...");
  pinMode(indicator_LED, OUTPUT);

  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1); // Clear bits in port

  AD5933.setExtClock(false);
  AD5933.resetAD5933();
  AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
  AD5933.setStartFreq(startFreqHz);
  AD5933.setVolPGA(0, 1);
  AD5933.getGainFactorC(cal_resistance, cal_samples, gainFactor, systemPhaseShift, false);
  AD5933.getComplex(gainFactor, systemPhaseShift, Z_Value, phaseAngle);
  ctrReg = AD5933.getByte(0x80);
  
  r1.begin(1);
  r2.begin(2);
  
  Serial.println();
  
  Serial.print("Gain factor: ");
  Serial.print(gainFactor);
  Serial.println();
  Serial.print("System Phase Shift: ");
  Serial.print(systemPhaseShift);
  Serial.println();
  Serial.print("Measured RCAL: ");
  Serial.print(Z_Value);
  Serial.println();
  Serial.println();
  Serial.println();
}

void loop() {

  digitalWrite(indicator_LED, LOW); // Indication to switch capacitors.

  while (Serial.available() < 1) {
    delay(15);
  } // Wait for user to move electrodes before starting

  Serial.read(); // Read the key entered and continue the program.

  digitalWrite(indicator_LED, HIGH);  // Indication program is running.

  for(int i = 0; i < nOfLevels; i++) { // repetition loop
    //Serial.println();

    // Only use data within the linear range of AD5933. Take 50 points.
    for(int R1 = 14; R1 < 64; R1++) {  // r1 loop

      r1.writeRDAC(R1);

      AD5933.setCtrMode(REPEAT_FREQ);
      AD5933.getComplex(gainFactor, systemPhaseShift, Z_Value, phaseAngle);

      Serial.print(R1);
      Serial.print(",");
      Serial.print(startFreqHz);
      Serial.print(",");
      Serial.print(Z_Value);
      Serial.print(",");
      Serial.print(Z_Value * cos(phaseAngle));
      Serial.print(",");
      Serial.print(-1 * Z_Value * sin(phaseAngle));
      Serial.println();

    } // end r1 loop
  } // end repetition loop
} // End main loop


