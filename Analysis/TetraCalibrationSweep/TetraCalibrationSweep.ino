
#include "Wire.h"
#include "Math.h"
#include "AD5258.h" //Library for AD5258 functions (must be installed)

#include "AD5933.h" //Library for AD5933 functions (must be installed)

// ================================================================
// Constants
// ================================================================

#define TWI_FREQ 400000L      // Set TWI/I2C Frequency to 400MHz.

const int CYCLES_BASE = 15;       // Cycles to ignore before a measurement is taken. Max is 511.

const int CYCLES_MULTIPLIER = 1;    // Multiple for CYCLES_BASE. Can be 1, 2, or 4.

const int CAL_RESISTANCE = 332;  // Calibration resistance for the gain factor.

const int CAL_SAMPLES = 10;         // Number of measurements to take of the calibration resistance.

const int DELAY = 200; // Delay between toggling the multiplexer


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

int ctrReg = 0;

long startFreq = 6; // AC start frequency in kilohertz.

long stepSize = 1;  // Step size of AC frequency sweep in kilohertz.

const byte numOfIncrements = 94; // Number of increments in frequency sweep.

double GAIN_FACTOR [numOfIncrements + 1];

double VOLTAGE_PHASE [numOfIncrements + 1];

double CURRENT_PHASE [numOfIncrements + 1];

double zMagnitude = 0;

double phaseAngle = 0;

double temp = 0;

int count = 0;
int currentStep = 0;

void setup() {

  // ================================================================
  // For AD5933
  // ================================================================

  //Serial.println();
  //Serial.println();s

  // put your setup code here, to run once:
  //Set IOs mode

  AD5933.setupDevicePins(LOW);

  Wire.begin(); // Start Arduino I2C library
  Serial.begin(38400); // Open serial port
  //Serial.println();

  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1); // Clear bits in port

  for (int i = 0; i < numOfIncrements + 1; i++) {
    GAIN_FACTOR[i] = 0;
    VOLTAGE_PHASE[i] = 0;
    CURRENT_PHASE[i] = 0;
  }

  AD5933.setExtClock(false);
  AD5933.resetAD5933();
  AD5933.setSettlingCycles(CYCLES_BASE, CYCLES_MULTIPLIER);
  AD5933.setStartFreq(startFreq * 1000);
  AD5933.setStepSize(stepSize * 1000);
  AD5933.setNumofIncrement(numOfIncrements);
  AD5933.setRange(1);
  AD5933.setPGA(1);
  temp = AD5933.getTemperature();

  AD5933.getGainFactorsTetraSweep(CAL_RESISTANCE, CAL_SAMPLES, GAIN_FACTOR, VOLTAGE_PHASE, CURRENT_PHASE);

  Serial.print("Current phase is: ");
  Serial.print(CURRENT_PHASE[0]);
  Serial.print(", Voltage phase is: ");
  Serial.print(VOLTAGE_PHASE[0]);
  Serial.print(", Gain factor is: ");
  Serial.println(GAIN_FACTOR[0]);
  //Serial.println();
  Serial.println("Program start: ");
}

void loop() {

  if (Serial.available()) {
    delay(15);
    uint8_t ch = Serial.read();

    if (ch == '1') {

      if (currentStep == numOfIncrements + 1) {
        currentStep = 0;
      }

      switch (currentStep) {

        case 0: // First step in frequency sweep; initialize, measure, then increase frequency

          ctrReg = AD5933.getByte(0x80);
          AD5933.setCtrMode(STAND_BY, ctrReg);
          AD5933.setCtrMode(INIT_START_FREQ, ctrReg);
          AD5933.setCtrMode(START_FREQ_SWEEP, ctrReg);
          
          AD5933.getComplexTetra(DELAY,
                                 GAIN_FACTOR[currentStep],
                                 VOLTAGE_PHASE[currentStep],
                                 CURRENT_PHASE[currentStep],
                                 zMagnitude,
                                 phaseAngle);

          AD5933.setCtrMode(INCR_FREQ);
          break;

        default: // for intermediate frequencies, measure then increase frequency

          AD5933.getComplexTetra(DELAY,
                                 GAIN_FACTOR[currentStep],
                                 VOLTAGE_PHASE[currentStep],
                                 CURRENT_PHASE[currentStep],
                                 zMagnitude,
                                 phaseAngle);

          AD5933.setCtrMode(INCR_FREQ);
          break;

        case numOfIncrements: // Last frequency in sweep; measure, power down, then reset values after printing

          AD5933.getComplexTetra(DELAY,
                                 GAIN_FACTOR[currentStep],
                                 VOLTAGE_PHASE[currentStep],
                                 CURRENT_PHASE[currentStep],
                                 zMagnitude,
                                 phaseAngle);

          AD5933.setCtrMode(POWER_DOWN, ctrReg);
          break;
      }
      
      Serial.print(startFreq + (currentStep * stepSize));
      Serial.print(",");
      Serial.print(zMagnitude, 4);
      Serial.print(",");
      Serial.print(phaseAngle, 4);
      Serial.print(",");     
      Serial.print(GAIN_FACTOR[currentStep]);
      Serial.print(",");     
      Serial.print(VOLTAGE_PHASE[currentStep]);
      Serial.print(",");     
      Serial.print(CURRENT_PHASE[currentStep]);      
      Serial.println();

      count++;
      currentStep++;

    }
  } // end if serial available
}































