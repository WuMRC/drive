// AD5933 library implementation via Arduino serial monitor by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// This sketch prints out constituents for a MATLAB Map object "impedanceMap".
// Values of [f,r1,r2,c,key] are mapped to [z,r,x].

#include "Wire.h"
#include "Math.h"
#include "AD5933.h" //Library for AD5933 functions (must be installed)
#include "AD5258.h" //Library for AD5258 functions (must be installed)


#define TWI_FREQ 400000L      // Set TWI/I2C Frequency to 400MHz.

#define cycles_base 15       // Cycles to ignore before a measurement is taken. Max is 511.

#define cycles_multiplier 1    // Multiple for cycles_base. Can be 1, 2, or 4.

#define cal_resistance 473.37  // Calibration resistance for the gain factor. 

#define cal_samples 10         // Number of measurements to take of the calibration resistance.

#define nOfLevels 4 // 10 levels, with 3 factors. Frequency has 99 levels though.

#define fIncrements 98

#define indicator_LED 12


// Define bit clearing and setting variables

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// ================================================================
// Dynamic variables
// ================================================================

int ctrReg = 0; // Initialize control register variable.

uint8_t currentStep = 0; // Used to loop frequency sweeps.

double startFreqHz = 2000; // AC Start frequency (Hz).

double stepSizeHz = 1000; // AC frequency step size between consecutive values (Hz).

double Z_Value = 0;          // Initialize impedance magnitude.

double phaseAngle = 0;       // Initialize phase angle value.

double temp = 0; // Used to update AD5933's temperature.

double GF_Array[fIncrements + 1]; // gain factor array.

double PS_Array[fIncrements + 1]; // phase shift array.

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
  AD5933.setRange(RANGE_4);
  AD5933.setStartFreq(startFreqHz);
  AD5933.setIncrement(stepSizeHz);
  AD5933.setNumofIncrement(fIncrements);
  AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
  temp = AD5933.getTemperature();
  AD5933.setVolPGA(0, 1);
  AD5933.getGainFactorS_Set(cal_resistance, cal_samples, GF_Array, PS_Array);
  ctrReg = AD5933.getByte(0x80);

  /*  Serial.println();
   
   for(int i = 0; i <= fIncrements; i++) { // print and set CR filter array.
   
   if(i == 0) {
   ctrReg = AD5933.getByte(0x80);
   AD5933.setCtrMode(STAND_BY, ctrReg);
   AD5933.setCtrMode(INIT_START_FREQ, ctrReg);
   AD5933.setCtrMode(START_FREQ_SWEEP, ctrReg);
   AD5933.getComplex(GF_Array[i], PS_Array[i], Z_Value, phaseAngle);
   }
   
   else if(i > 0 &&  i < fIncrements) {
   AD5933.getComplex(GF_Array[i], PS_Array[i], Z_Value, phaseAngle);
   AD5933.setCtrMode(INCR_FREQ, ctrReg);
   }
   
   else if(i = fIncrements) {
   AD5933.getComplex(GF_Array[i], PS_Array[i], Z_Value, phaseAngle);
   AD5933.setCtrMode(POWER_DOWN, ctrReg);
   }
   
   Serial.print("Frequency: ");
   Serial.print("\t");
   Serial.print(startFreqHz + (stepSizeHz * i));
   Serial.print("\t");        
   Serial.print("Gainfactor term: ");
   Serial.print(i);
   Serial.print("\t");
   Serial.print(GF_Array[i]);
   Serial.print("\t");
   Serial.print("SystemPS term: ");
   Serial.print(i);
   Serial.print("\t");
   Serial.print(PS_Array[i], 4);
   Serial.print("\t");        
   Serial.print("Z_Value: ");
   Serial.print(i);
   Serial.print("\t");
   Serial.print(Z_Value);        
   Serial.println(); 
   }  */

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
    for(int R1 = 0; R1 < 64; R1++) {  // r1 loop

      r2.writeRDAC(R1);

      for(int currentStep = 0; currentStep <= fIncrements; currentStep++) { // frequency loop

        if(currentStep == 0) {
          AD5933.setCtrMode(STAND_BY, ctrReg);
          AD5933.setCtrMode(INIT_START_FREQ, ctrReg);
          AD5933.setCtrMode(START_FREQ_SWEEP, ctrReg);
        }

        else if(currentStep > 0 &&  currentStep < fIncrements) {
          AD5933.setCtrMode(INCR_FREQ, ctrReg);
        }

        else if(currentStep == fIncrements) {
          AD5933.setCtrMode(POWER_DOWN, ctrReg);
        }

        AD5933.getComplex(GF_Array[currentStep], PS_Array[currentStep], Z_Value, phaseAngle);

        // Print

        Serial.print(R1);
        Serial.print(",");
        Serial.print(startFreqHz + (stepSizeHz * currentStep));
        Serial.print(",");
        Serial.print(Z_Value);
        Serial.print(",");
        Serial.print(Z_Value * cos(phaseAngle));
        Serial.print(",");
        Serial.print(-1 * Z_Value * sin(phaseAngle));
        Serial.println();

      } // end frequency loop
    } // end r1 loop
  } // end repetition loop
} // End main loop


































