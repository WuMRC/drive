
// AD5933 library implementation via Arduino serial monitor by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// NOTE: Even though the sampling rate may be set to a certain value, printing to the serial monitior takes some time...
// therefore the elapsed time shown will not correspond closely to the sample rate.
// Should you save the data to a SD card however, the sample rate is better reflected.

#include "Wire.h"
#include "Math.h"
#include "Micro40Timer.h" // Timer function for notifications
#include "AD5933.h" //Library for AD5933 functions (must be installed)

// ================================================================
// Constants
// ================================================================

#define TWI_FREQ 400000L      // Set TWI/I2C Frequency to 400MHz.

#define cycles_base 15       // Cycles to ignore before a measurement is taken. Max is 511.

#define cycles_multiplier 1    // Multiple for cycles_base. Can be 1, 2, or 4.

#define cal_resistance 461  // Calibration resistance for the gain factor. 

#define cal_samples 10         // Number of measurements to take of the calibration resistance.

#define B 98

#define S 115

#define R 114

#define FR 102

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

int incomingByte = 0; // Bytes read from the serial monitor.

int firstByte = 0; // First byte read from the serial monitor. Used to set AD5933 mode.

int bytesAvailable = 0; // Bytes available to read.

int rIncrement = 0; // Increments in sample rate serial reading. For parsing.

int fIncrement = 0; // Increments in frequency sweep serial reading. For parsing.

int sampleRate = 50; // App sample rate in hertz.

int startFreq = 50; // AC start frequency in kilohertz.

int stepSize = 0;  // Step size of AC frequency sweep in kilohertz.

int numOfIncrements = 0;  // Number of increments in frequency sweep.

int sampleRateHolder = 0; // Holds value of input prior to check of input syntax.

int startFreqHolder = 0; // Holds value of input prior to check of input syntax.

int stepSizeHolder = 0; // Holds value of input prior to check of input syntax.    

int numOfIncrementsHolder = 0; // Holds value of input prior to check of input syntax.

int numberOfCommas = 0; // Number of commas. Used to parse frequency sweeps.

long sampleRatePeriod = 0; // Sample rate period (microseconds).

uint8_t currentStep = 0; // Used to loop frequency sweeps.

double startFreqHz = ((long)(startFreq)) * 1000; // AC Start frequency (Hz).

double stepSizeHz = 0; // AC frequency step size between consecutive values (Hz).

double endFreqHz = 0; // End frequency for 2 point calibration.

double gain_factor = 0;      // Initialize Gain factor.

double Z_Value = 0;          // Initialize impedance magnitude.

double systemPhaseShift = 0;       // Initialize system phase shift value.

double phaseAngle = 0;       // Initialize phase angle value.

double temp = 0; // Used to update AD5933's temperature.

double* GF_Array = NULL; // Pointer for dynamic gain factor array size.

double* PS_Array = NULL; // Pointer for dynamic phase shift array size.

double* CR_Array = NULL; // Pointer for calibration impedance at a certain frequency. Used for filtering.

boolean inputSucess = false;

boolean NOTIFICATIONS_FLAG = false; // Variable that toggles notifications to phone subscription state. 

boolean FREQ_SWEEP_FLAG = false; // Used to toggle frequency sweeps.

volatile 
boolean SAMPLE_RATE_FLAG = false;  // Variable to manage sample rate. Managed from interrupt context.

void setup() {

  // ================================================================
  // For AD5933
  // ================================================================

  Wire.begin(); // Start Arduino I2C library
  Serial.begin(38400); // Open serial port

  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1); // Clear bits in port

  AD5933.setExtClock(false); 
  AD5933.resetAD5933(); 
  AD5933.setSettlingCycles(cycles_base,cycles_multiplier); 
  AD5933.setStartFreq(startFreqHz); 
  AD5933.setVolPGA(0, 1); 
  temp = AD5933.getTemperature(); 
  AD5933.getGainFactorC(cal_resistance, cal_samples, gain_factor, systemPhaseShift, false);

  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("Welcome! Please use the following syntax to input commands:");
  Serial.println("All AC frequencies are in kilohertz.");
  Serial.println("Sample rate frequency is in hertz.");
  Serial.println("For frequency sweeps, it is necessary to pad all values with zero to make up 3 bytes each.");
  Serial.println("Input is comma delimited with no spaces.");
  Serial.println("b - begin sampling.");
  Serial.println("s - stop sampling");
  Serial.println("r,samplerate  - change sample rate.");
  Serial.println("f,frequency,stepSize,numberOfIncrements - set frequency sweep. values must be between 1 and 100");
  Serial.println();

  // ================================================================
  // For Timer interrupts
  // ================================================================

  // Start with sampling rate of 50 hertz
  sampleRatePeriod = 20000;
  Micro40Timer::set(sampleRatePeriod, notify); // 20000 microsecond period -> 50 hertz frequency
  Micro40Timer::start();
}

void loop() {


  // For Serial Monitor input
  // ================================================================

  if (Serial.available() > 0) {
    Micro40Timer::stop(); 
    delay(15); // Delay because Arduino serial buffer will return 1 if queried too quickly.
    bytesAvailable = Serial.available();
    firstByte = Serial.read();
    rIncrement = 0;
    fIncrement = 0;
    sampleRateHolder = 0;
    startFreqHolder = 0;       
    stepSizeHolder = 0;      
    numOfIncrementsHolder = 0;
    numberOfCommas = 0;
    inputSucess = false; 

    switch(firstByte) {

    case B:
      NOTIFICATIONS_FLAG = true;
      Micro40Timer::set(sampleRatePeriod, notify);
      Micro40Timer::start(); 
      break;

    case S:
      NOTIFICATIONS_FLAG = false; 
      Micro40Timer::set(sampleRatePeriod, notify);
      Micro40Timer::start(); 
      break;

    default:
      if(Serial.available() < 1) {
        Serial.println("Error. Please check your syntax.");
      }
      break;
    }

    while (Serial.available() > 0) {

      incomingByte = Serial.read();

      switch(firstByte) {

      case R:
        rIncrement++;

        if(incomingByte == 44) {
          numberOfCommas++;
        }

        else if (incomingByte > 47 && incomingByte < 58){
          //Parse ASCII char value to int and cocatenate to form sample Rate
          sampleRateHolder += ((pow(10, ((double) (bytesAvailable - rIncrement - 1)))) * (incomingByte - 48)) + 0.5; 
          inputSucess = true;
        }

        else {
          inputSucess = false;
        }

        if(numberOfCommas > 1) {
          inputSucess = false;
        }

        break;

      case FR:
        fIncrement++;

        if(bytesAvailable != 13) {
          Serial.println("Error. Please check your syntax (Number of bytes).");
        }

        else {

          if(incomingByte == 44) {
            numberOfCommas++;
            fIncrement = 0;
          }

          else if (numberOfCommas == 1 && incomingByte > 47 && incomingByte < 58){
            //Parse ASCII char value to int and cocatenate to form start frequency
            startFreqHolder += ((pow(10, ((double) (bytesAvailable - fIncrement - 10)))) * (incomingByte - 48)) + 0.5;
            inputSucess = true; 
          }

          else if (numberOfCommas == 2 && incomingByte > 47 && incomingByte < 58){
            //Parse ASCII char value to int and cocatenate to form step size
            stepSizeHolder += ((pow(10, ((double) (bytesAvailable - fIncrement - 10)))) * (incomingByte - 48)) + 0.5;
            inputSucess = true; 
          }

          else if (numberOfCommas == 3 && incomingByte > 47 && incomingByte < 58){
            //Parse ASCII char value to int and cocatenate to form number of increments
            numOfIncrementsHolder += ((pow(10, ((double) (bytesAvailable - fIncrement - 10)))) * (incomingByte - 48)) + 0.5;
            inputSucess = true; 
          }

          else {
            inputSucess = false;
            break;
          }

          if(numberOfCommas != 3) {
            inputSucess = false;
          }
        }
        break;
      }
    }

    if(inputSucess == true) {
      switch(firstByte) {
      case R:
        sampleRate = sampleRateHolder;
        adjustAD5933(firstByte, sampleRate, 0, 0);
        break;
      case FR:
        startFreq = startFreqHolder;
        stepSize = stepSizeHolder;
        numOfIncrements = numOfIncrementsHolder;
        adjustAD5933(firstByte, startFreq, stepSize, numOfIncrements);
        break;
      }
    }

    else {
      Serial.println("Error. Please check your syntax.");
    }

  }

  // End Serial Monitor input
  // ================================================================

  // Program proper
  // ================================================================

  if (SAMPLE_RATE_FLAG) { // this flag is toggled from the timer's interrupt context. It controls the sample rate. 

    AD5933.tempUpdate();

    if(!FREQ_SWEEP_FLAG) { // Repeat frequency, don't sweep.
      AD5933.setCtrMode(REPEAT_FREQ);
      AD5933.getComplex(gain_factor, systemPhaseShift, Z_Value, phaseAngle);
    }

    else { // Perform frequency sweep.

      // Byte of ctrReg alredy gotten from changing mode to enable freq sweep.
      if(currentStep == 0) {
        AD5933.setCtrMode(STAND_BY, ctrReg);
        AD5933.setCtrMode(INIT_START_FREQ, ctrReg);
        AD5933.setCtrMode(START_FREQ_SWEEP, ctrReg);
      }

      AD5933.getComplex(GF_Array[currentStep], PS_Array[currentStep], Z_Value, phaseAngle);

      if(currentStep == numOfIncrements) {
        currentStep = 0;
        AD5933.setCtrMode(POWER_DOWN, ctrReg);
      }
      else {
        AD5933.setCtrMode(INCR_FREQ, ctrReg);
        currentStep++;
      }
    }

    Serial.print("Elapsed time (ms): ");
    Serial.print(millis());
    Serial.print("\tCurrent Frequency: ");
    Serial.print(startFreqHz + (stepSizeHz * currentStep));
    Serial.print("\tImpedance: ");
    Serial.print(Z_Value, 4);
    Serial.print("\tPhase Angle: "); 
    Serial.print(phaseAngle, 4);
    Serial.println();
    Serial.println();

    SAMPLE_RATE_FLAG = false; // Switch this flag back to false till timer interrupt switches it back on.     
  }   


  else {
    // Do zilch, zip, nada, nothing if notifications are not enabled.
  } 

}

void adjustAD5933(int purpose, int v1, int v2, int v3) {

  Micro40Timer::stop(); // stop Timer interrupts as soon as this is called.

  // check for data written to "c_sample_rate" handle
  if (purpose == R) {

    sampleRate = v1;
    sampleRatePeriod = 1000000 / ((long) sampleRate);

    Serial.println();
    Serial.print("Sucessful write attempt; new frequency / period: ");
    Serial.print(sampleRate);
    Serial.print(" hertz");
    Serial.print(" / ");         
    Serial.print(sampleRatePeriod);
    Serial.print(" microseconds.");
    Serial.println();

    Micro40Timer::set(sampleRatePeriod, notify); 
    Micro40Timer::start();
  }

  // check for data written to "c_ac_freq" handle  
  if (purpose == FR) {

    delete [] GF_Array; // Free memory from previous GF array.
    delete [] PS_Array; // Free memory from previous PS array.
    delete [] CR_Array; // Free memory from previous CR array.

    Serial.println("Old Arrays Deleted.");

    startFreq = v1; 
    stepSize = v2;  
    numOfIncrements = v3;

    currentStep = 0; // reset currentStep for filtering in loop.

    startFreqHz = (double)startFreq * 1000;  

    if(stepSize == 0) { // frequency sweep is disabled

      FREQ_SWEEP_FLAG = false;

      CR_Array = new double[1]; // size of one for single impedance at single frequency.

      Serial.println("New Array created.");

      AD5933.setExtClock(false);
      AD5933.resetAD5933();
      AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
      AD5933.setStartFreq(startFreqHz);
      AD5933.setVolPGA(0, 1);
      AD5933.getGainFactorC(cal_resistance, cal_samples, gain_factor, systemPhaseShift, false);
      AD5933.getComplex(gain_factor, systemPhaseShift, CR_Array[0], phaseAngle);

      Serial.println("Gain factors gotten.");

      Serial.println();
      Serial.print("Frequency:");
      Serial.print("\t");
      Serial.print(startFreqHz);
      Serial.print("\t");
      Serial.print("Gain factor:");
      Serial.print("\t");
      Serial.print(gain_factor);
      Serial.print("\t");
      Serial.print("SystemPS:");
      Serial.print("\t");
      Serial.print(systemPhaseShift, 4);
      Serial.print("\t");
      Serial.print("CR: ");
      Serial.print("\t");
      Serial.print(CR_Array[0]); 
      Serial.println();      
    }
    else { // frequency sweep is enabled

      FREQ_SWEEP_FLAG = true;

      stepSizeHz = (double)stepSize * 1000;
      endFreqHz = startFreqHz + ((double)stepSize * ((double) numOfIncrements) * 1000);
      Serial.println(startFreqHz);
      Serial.println(endFreqHz);
      Serial.println(stepSizeHz);
      Serial.println(numOfIncrements);   

      // generate gain factor array using two point calibration.

      GF_Array = new double[numOfIncrements + 1];
      PS_Array = new double[numOfIncrements + 1];
      CR_Array = new double[numOfIncrements + 1];

      for (int i = 0; i < numOfIncrements; i++) {
        GF_Array[i] = 0;    // Initialize all elements to zero.
        PS_Array[i] = 0;    // Initialize all elements to zero.        
      }

      Serial.println("New Arrays created.");

      AD5933.setExtClock(false);
      AD5933.resetAD5933();
      AD5933.setStartFreq(startFreqHz);
      AD5933.setIncrement(stepSizeHz);
      AD5933.setNumofIncrement(numOfIncrements);      
      AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
      AD5933.getTemperature();
      AD5933.setVolPGA(0, 1);

      AD5933.getGainFactorS_Set(cal_resistance, cal_samples, GF_Array, PS_Array);

      Serial.println("Gain factors gotten.");

      Serial.println();

      for(int i = 0; i <= numOfIncrements; i++) { // print and set CR filter array.

        if(i == 0) {
          ctrReg = AD5933.getByte(0x80);
          AD5933.setCtrMode(STAND_BY, ctrReg);
          AD5933.setCtrMode(INIT_START_FREQ, ctrReg);
          AD5933.setCtrMode(START_FREQ_SWEEP, ctrReg);
          AD5933.getComplex(GF_Array[i], PS_Array[i], CR_Array[i], phaseAngle);
        }

        else if(i > 0 &&  i < numOfIncrements) {
          AD5933.getComplex(GF_Array[i], PS_Array[i], CR_Array[i], phaseAngle);
          AD5933.setCtrMode(INCR_FREQ, ctrReg);
        }

        else if(i = numOfIncrements) {
          AD5933.getComplex(GF_Array[i], PS_Array[i], CR_Array[i], phaseAngle);
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
        Serial.print("CR term: ");
        Serial.print(i);
        Serial.print("\t");
        Serial.print(CR_Array[i]);        
        Serial.println(); 
      }   
    }
    Serial.println();
    Serial.print("Sucessful write attempt to c_ac_freq.");
    Serial.println();      
    Serial.print("Start Frequency (KHz): ");  
    Serial.print(startFreq);
    Serial.println();    
    Serial.print("Step size (KHz): ");    
    Serial.print(stepSize);
    Serial.println();    
    Serial.print("Number of increments: ");    
    Serial.print(numOfIncrements);
    Serial.println();
    Serial.println();

    Micro40Timer::set(sampleRatePeriod, notify);
    Micro40Timer::start();    
  }  
}


void notify() {
  if (NOTIFICATIONS_FLAG) {
    SAMPLE_RATE_FLAG = true; 
  }
}



















