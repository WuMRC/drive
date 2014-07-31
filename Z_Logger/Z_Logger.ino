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

#define cycles_base 500      //First term to set a number of cycles to ignore
                             //to dissipate transients before a measurement is
                             //taken. The max value for this is 511.

#define cycles_multiplier 2  //Set a multiple for the cycles_base which
                             //is used to calculate the desired number
                             //of settling cycles. Values can be 1, 2, or 4.
                             

#define start_frequency 50000 //Set the start frequency, the only one of
                              //interest here(50 kHz).

#define cal_resistance 5000 //Set a calibration resistance for the gain
                            //factor. This will have to be measured before any
                            //other measurements are performed.
                           
#define cal_samples 10 //Set a number of measurements to take of the calibration
                       //resistance. These are used to get an average gain
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
  delay(3000);
  Wire.begin();
  
  //--- A. Initialization and Calibration Meassurement ---
  
  //[A.1] Set the measurement frequency
  if (setStartFreq(start_frequency) == true)
  {
    #if VERBOSE
    Serial.print("Start frequency set to: ");
    Serial.print(start_frequency);
    Serial.println (" Hz.");
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
  if (setSettlingCycles(cyces_base, cyles_multiplier) == true)
  {
    #if VERBOSE
    Serial.print("Settling cycles set to: ");
    Serial.print(cycles_base*cycles_multiple);
    Serial.println(" cycles.");
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error setting settling cycles!");
    #endif
  }
  //End [A.2]

  //[A.3] Calculate the gain factor (needs cal resistance, # of measurements)
  //Note: The gain factor finding function returns the INVERSE of the factor
  //as defined on the datasheet!
  long gain_factor = getGainFactor(cal_resistance, cal_samples);
  if (gain_factor != -1)
  {
    #if VERBOSE
    Serial.print("Gain factor (");
    Serial.print(cal_samples);
    Serial.print(" samples) is: ");
    Serial.println(gain_factor);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error calculating gain factor!");
    #endif
  } 
  //End [A.3]
  
  //--- End A ---

//}

//void loop() <-- uncomment once single run through sucessful.
//{
  
  //--- B. Repeated single measurement ---
  //Gain factor calibration already sets the frequency, so just send 
  //repeat single magnitude capture command.
  
  //[B.1] Issue a "repeat frequency" command.
  if (setCtrMode(REPEAT_FREQ) == true)
  {
    #if VERBOSE
    Serial.print("Repeat_Frequency command sent.");
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error sending Repeat_Frequency command!");
    #endif
  }
  //End [B.1]
  
  //[B.2] Check the status register to ensure measurement is complete (i.e.
  // the 7th bit is 1). If TRUE...
    
    //[B.2.1] Capture the magnitude from real & imaginary registers.
  
    //[B.2.2] Calculate the impedance using the magnitude and gain factor.
    
    //[B.2.3] Output the impedance value (serial, array, etc.)
  
  //End [B.2]
  
  //[B.3] ELSE:
    
    //Wait (delay time? until complete?) or iterate loop.

  //End [B.3]
  
  // --- End B ---    
}

void loop()
{
  Serial.println("Looping!")
  delay(1000);
}

