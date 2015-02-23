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

#define cal_resistance 461  // Calibration resistance for the gain factor. 

#define cal_samples 10         // Number of measurements to take of the calibration resistance.

#define nOfLevels 4 // 10 levels, with 3 factors. Frequency has 99 levels though.

#define fIncrements 98

#define nOfSamples 1

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

double systemPhaseShift = 0;       // Initialize system phase shift value.

double Z_Value = 0;          // Initialize impedance magnitude.

double rComp = 0;            // Initialize real component value.

double xComp = 0;            // Initialize imaginary component value.

double phaseAngle = 0;       // Initialize phase angle value.

double temp = 0; // Used to update AD5933's temperature.

double rw1 = 0; // Rheostat 1's wiper resistance

double rw2 = 0; // Rheostat 2's wiper resistance

double GF_Array[fIncrements + 1]; // gain factor array.

double PS_Array[fIncrements + 1]; // phase shift array.

double RX_Array[fIncrements + 1][2]; // array of array containing R & X values for fitting.

double cArray[nOfLevels] = {
  2.256, 3.26, 4.72, 7.125}; // capacitor values. 

double r1Array[nOfLevels] = {
  614.54, 631.98, 649.37, 666.43}; // r1 values. 

double r2Array[nOfLevels] = {
  995.35, 1012.27, 1029.55, 1046.71}; // r2 values.

AD5258 r1; // rheostat r1

AD5258 r2; // rheostat r2

void setup() {

  Wire.begin(); // Start Arduino I2C library
  Serial.begin(38400);

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

  Serial.println();

  /*for(int i = 0; i <= fIncrements; i++) { // print and set CR filter array.
   
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
  Serial.println("F,R1,R2,C,Key,Z,R,X");
}

void loop() {

  for(int cap = 0; cap < nOfLevels; cap++) { // Capacitor loop

    digitalWrite(indicator_LED, LOW); // Indication to switch capacitors.

    while (Serial.available() < 1) {
    } // Wait for user to swap capacitors befor triggering

    Serial.read(); // Read the key entered and continue the program.

    digitalWrite(indicator_LED, HIGH);  // Indication program is running.

    for(int R2 = 0; R2 < nOfLevels; R2++) {  // r2 loop
      // Serial.println("r2 loop");
      r2.writeRDAC(56 + R2);

      for(int R1 = 0; R1 < nOfLevels; R1++) {  // r1 loop
        // Serial.println("r1 loop");
        r1.writeRDAC(33 + R1);

        for(int currentStep = 0; currentStep <= fIncrements; currentStep++) { // frequency loop
          // Serial.print("currentStep: ");
          // Serial.println(currentStep);
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

          for(int N = 0; N < nOfSamples; N++) {  // number of samples loop
            // Serial.println("number of samples loop");
            AD5933.setCtrMode(REPEAT_FREQ); // Repeat measurement
            AD5933.getComplex(GF_Array[currentStep], PS_Array[currentStep], Z_Value, phaseAngle);

            RX_Array[currentStep][0] = Z_Value * cos(phaseAngle); // Resistance value
            RX_Array[currentStep][1] = -1 * Z_Value * sin(phaseAngle); // Reactance Value

              if(currentStep == fIncrements) {

            }

            // Print

            Serial.print(startFreqHz + (stepSizeHz * currentStep));
            Serial.print(",");
            Serial.print(r1Array[R1]);
            Serial.print(",");
            Serial.print(r2Array[R2]);
            Serial.print(",");
            Serial.print(cArray[cap]);
            Serial.print(",");
            Serial.print(generateMapKey(currentStep,R1,R2,cap,N));
            Serial.print(",");
            Serial.print(Z_Value, 4);
            Serial.print(",");
            Serial.print(RX_Array[currentStep][0]);
            Serial.print(",");
            Serial.print(RX_Array[currentStep][1]);
            Serial.println();

          } // end number of samples loop
        } // end frequency loop
      } // end r1 loop
    } // end r2 loop
  } // End capacitor loop
} // End main loop

String generateMapKey(int fIndex, int r1Index, int r2Index, int cIndex, int level) {
  String key = "";
  key += "C";
  key += cIndex;
  key += "R";
  key += r1Index;
  key += "r";
  key += r2Index;
  key += "F";
  key += fIndex;
  key += "L";
  key += level; 

  return key;
}



























