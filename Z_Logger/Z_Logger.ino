/*
--------------------------------------------------------------------------------
Z_Logger

Goal for this version:
Configure and execute single impedance measurement command, log the result to
a text file, and repeat, at a sample rate of 200 times/sec.
*/

//--- Hard-coded inputs for the sketch:

#define VERBOSE 1 //Toggles verbose debugging output via the serial monitor.
                  //1 = On, 0 = Off.

#define start_frequency 50000 //Define the single frequency of interest (50 kHz).

#define cal_resistance ### //Define a calibration resistance for the gain
                           //factor.
//---

#include <Wire.h> //Library for I2C communications
#include <AD5933.h> //Library for AD5933 functions (must be installed)

void setup()
{
  #if VERBOSE
  Serial.begin(9600); //Initialize serial communication for debugging.
  Serial.println("Program start!");
  #endif
  Wire.begin();
  
  //--- A. Initialization and Calibration Meassurement ---
  
  //[A.1] Set the measurement frequency
  if (setStartFreq(start_frequency) == true)
  {
    #if VERBOSE
    Serial.print("Start frequency set.");
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error setting start frequency!");
    #endif
  }
  //End [A.1]
  
  //[A.2] Set a number of settling time cycles
  //
  //End [A.2]

  //[A.3] Calculate the gain factor (needs cal resistance, # of measurements)
  //
  //[End A.3]
  
  //--- End A ---

}

void loop()
{
  //--- B. Repeated single measurement ---
  //Gain factor calibration already sets the frequency, so just send 
  //repeat single magnitude capture command.
  
  //[B.1] Issue a "repeat frequency" command.
  
  //[B.2] Check the status register to ensure measurement is complete (i.e.
  // the 7th bit is 1). If TRUE...
    
    //[B.2.1] Capture the magnitude from real & imaginary registers.
  
    //[B.2.2] Calculate the impedance using the magnitude and gain factor.
    
    //[B.2.3] Output the impedance value (serial, array, etc.)
  
  //[B.3] ELSE:
    
    //Wait (delay time? until complete?) or iterate loop.

  // --- End B ---    
}
  

