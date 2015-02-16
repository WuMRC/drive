// AD5933 library implementation via Arduino serial monitor by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// This sketch prints out constituents for a MATLAB Map object "impedanceMap".
// Values of [f,r1,r2,c,key] are mapped to [z,r,x].

#include "Wire.h"
#include "Math.h"
#include "BGLib.h" // BGLib C library for BGAPI communication.
#include "AD5933.h" //Library for AD5933 functions (must be installed)
#include "AD5258.h" //Library for AD5933 functions (must be installed)


#define TWI_FREQ 400000L      // Set TWI/I2C Frequency to 400MHz.

#define cycles_base 15       // Cycles to ignore before a measurement is taken. Max is 511.

#define cycles_multiplier 1    // Multiple for cycles_base. Can be 1, 2, or 4.

#define cal_resistance 461  // Calibration resistance for the gain factor. 

#define cal_samples 10         // Number of measurements to take of the calibration resistance.

#define nOfLevels 10 // 10 levels, with 3 factors. Frequency has 99 levesls though.

#define fIncrements 98

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

int n = 0; // Number of samples to take of each measurement. Dependent on STDEV.

long sampleRatePeriod = 0; // Android app sample rate period (microseconds).

uint8_t currentStep = 0; // Used to loop frequency sweeps.

uint8_t sampleRate = 50; // Android app sample rate (Hz).

uint8_t startFreq = 2;       // AC Start frequency (KHz).

uint8_t stepSize = 0;        // AC frequency step size between consecutive values (KHz).

uint8_t numOfIncrements = 0;       // Number of frequency increments.

double startFreqHz = ((long)(startFreq)) * 1000; // AC Start frequency (Hz).

double stepSizeHz = 0; // AC frequency step size between consecutive values (Hz).

double endFreqHz = 0; // End frequency for 2 point calibration.

double gain_factor = 0;      // Initialize Gain factor.

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

double cArray[nOfLevels]; // capacitor values. 

double r1Array[nOfLevels]; // r1 values. 

double r2Array[nOfLevels]; // r2 values.

double fArray[fIncrements]; // gain factor array.

AD5258 r1; // rheostat r1

AD5258 r2; // rheostat r2

void setup() {

  Wire.begin(); // Start Arduino I2C library

  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1); // Clear bits in port

  AD5933.setExtClock(false); 
  AD5933.resetAD5933(); 
  AD5933.setSettlingCycles(cycles_base,cycles_multiplier); 
  AD5933.setStartFreq(startFreqHz); 
  AD5933.setVolPGA(0, 1); 
  temp = AD5933.getTemperature(); 
  AD5933.getGainFactorC(cal_resistance, cal_samples, gain_factor, systemPhaseShift, false);

  ctrReg = AD5933.getByte(0x80);

  r1.begin(1);

  r2.begin(2);

  Serial.print("Start freq: ");
  Serial.print(startFreqHz);
  Serial.println();

  Serial.print("System Phase Shift: ");
  Serial.print(systemPhaseShift, 4);
  Serial.println();

}

void loop() {

  for(int i = 0; i < nOfLevels; i++) { // Capacitor loop

    Serial.println("Insert capacitor");

    while (Serial.available() == 0) { // Wait for user to swap capacitors befor triggering
    } // End capacitor while

    for(int j = 0; j < nOfLevels; j++) {  // r2 loop

      for(int k = 0; k < nOfLevels; k++) {  // r1 loop

        //adjustAD5933();

        for(int currentStep = 0; currentStep <= fIncrements; currentStep++) { // frequency loop

          if(currentStep == 0) {
            AD5933.setCtrMode(STAND_BY, ctrReg);
            AD5933.setCtrMode(INIT_START_FREQ, ctrReg);
            AD5933.setCtrMode(START_FREQ_SWEEP, ctrReg);
          }

          AD5933.getComplex(GF_Array[currentStep], PS_Array[currentStep], Z_Value, phaseAngle);

          if(currentStep == fIncrements) {
            currentStep = 0;
            AD5933.setCtrMode(POWER_DOWN, ctrReg);
          }
          else {
            AD5933.setCtrMode(INCR_FREQ, ctrReg);
            currentStep++;
          }

          for(int m = 1; m < n; m++) {  // number of samples loop

            AD5933.setCtrMode(REPEAT_FREQ); // Repeat measurement
            AD5933.getComplex(gain_factor, systemPhaseShift, Z_Value, phaseAngle);

            // Print

          } // end number of samples loop

        } // end frequency loop

      } // end r1 loop

    } // end r2 loop

  } // End capacitor loop

}

void adjustAD5933(int startFreqC, int stepSizeC, int numOfIncrementsC) {

  startFreq = startFreqC; 
  stepSize = stepSizeC;  
  numOfIncrements = numOfIncrementsC;

  startFreqHz = (double)startFreq * 1000;  

  stepSizeHz = (double)stepSize * 1000;
  endFreqHz = startFreqHz + ((double)stepSize * ((double) numOfIncrements) * 1000);  

  AD5933.setExtClock(false);
  AD5933.resetAD5933();
  AD5933.setStartFreq(startFreqHz);
  AD5933.setIncrement(stepSizeHz);
  AD5933.setNumofIncrement(numOfIncrements);      
  AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
  AD5933.getTemperature();
  AD5933.setVolPGA(0, 1);

  AD5933.getGainFactorS_Set(cal_resistance, cal_samples, GF_Array, PS_Array); 
}

String generateMapKey(int fIndex, int r1Index, int r2Index, int cIndex, int level) {
  String key = "";
  key += "c";
  key += cIndex;
  key += level;
  key += "r";
  key += r1Index;
  key += level;
  key += "r";
  key += r2Index;
  key += level;
  key += "f";
  key += fIndex;
  key += level; 

  return key;
}
















