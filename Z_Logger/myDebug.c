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

#define cal_resistance 4600 //Set a calibration resistance for the gain
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
  delay(100);
  #endif
  
  Wire.begin();
  delay(100);
  
  AD5933.setExtClock(false);
  //[A.X] Send a reset command to the AD5933.
  if(AD5933.resetAD5933() == true)
  {
    #if VERBOSE
    Serial.println("Reset command sent.");
    delay(100);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.println("Error sending reset command!");
    delay(1000);
    #endif
  }
  //End [A.X]

  
  //--- A. Initialization and Calibration Meassurement ---
  
  //[A.0] Set the clock for internal/external frequency
  //Set the operational clock to internal
  //AD5933.setExtClock(0);
  
  //[A.1] Set the measurement frequency
  if (AD5933.setStartFreq(start_frequency) == true)
  {
    #if VERBOSE
    Serial.print("Start frequency set to: ");
    Serial.print(start_frequency);
    Serial.println (" Hz.");
    delay(1000);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error setting start frequency!");
    delay(1000);
    #endif
  }
  //End [A.1]

  //[A.2] Set a number of settling time cycles
  if (AD5933.setSettlingCycles(cycles_base, cycles_multiplier) == true)
  {
    #if VERBOSE
    Serial.print("Settling cycles set to: ");
    Serial.print(cycles_base*cycles_multiplier);
    Serial.println(" cycles.");
    delay(1000);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error setting settling cycles!");
    delay(1000);
    #endif
  }
  //End [A.2]

  //[A.3] Calculate the gain factor (needs cal resistance, # of measurements)
  //Note: The gain factor finding function returns the INVERSE of the factor
  //as defined on the datasheet!
  long gain_factor = AD5933.getGainFactor(cal_resistance, cal_samples, false);
  if (gain_factor != -1)
  {
    #if VERBOSE
    Serial.print("Gain factor (");
    Serial.print(cal_samples);
    Serial.print(" samples) is: ");
    Serial.println(gain_factor);
    delay(1000);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error calculating gain factor!");
    delay(1000);
    #endif
  } 
  //End [A.3]

  //--- End A ---

}


void loop()
{
  
  //--- B. Repeated single measurement ---
  //Gain factor calibration already sets the frequency, so just send 
  //repeat single magnitude capture command.
  
  //[B.1] Issue a "repeat frequency" command.
  if (AD5933.setCtrMode(REPEAT_FREQ) == true)
  {
    #if VERBOSE
    Serial.println("Repeat_Frequency command sent.");
    delay(1000);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.println("Error sending Repeat_Frequency command!");
    delay(1000);
    #endif
  }
  //End [B.1]
  
  //[B.2] Check the status register to ensure measurement is complete (i.e.
  // the 7th bit is 1). This may not be necessary given wait built into library.
  //Wire.beginTransmission(0x0D); //Open communications to to AD5933.
  //Wire.write(B10110000); //Send the "pointer" commmand.
  //Wire.write(0x8F); //Send the address for the pointer (status register).
  //Wire.endTransmission(); //Close communications.
  //Wire.requestFrom(0x0D, 1); //Request the data byte stored in the register 
                             //the pointer is at. 
  //byte register_0x8F =Wire.read(); //Store the data in a byte variable.
  byte register_0x8F = AD5933.getByte(0x8F);
  byte measurement_status = register_0x8F &= B00000010; //Isolate the register
                                                        //bit showing status of
                                                        //real and imaginary
                                                        //registers.
  //End [B.2]
  
  //[B.3] If a successful measurement was taken...
  if (measurement_status = B00000010)
  {
    #if VERBOSE
    Serial.println("Valid measurement data present.");
    delay(1000);
    #endif
    
    //[B.2.1] Capture the magnitude from real & imaginary registers.
    double Z_magnitude = AD5933.getMagOnce();
    delay(1000);
    #if VERBOSE
    Serial.print("Magnitude found to be: ");
    Serial.println(Z_magnitude);
    delay(1000);
    #endif
    //End [B.2.1]
  
    /*
    //[B.2.2] Calculate the impedance using the magnitude and gain factor.
    double Z_value = gain_factor/Z_magnitude;
    #if VERBOSE
    Serial.print("Impedance found to be: ");
    Serial.print(Z_value);
    Serial.println(" Ohms.");
    delay(1000);
    #endif
    //End [B.2.2]
    
    //[B.2.3] Output the impedance value (serial, array, etc.)
    Serial.print("Impedance = ");
    Serial.print(Z_value);
    Serial.println(" Ohms.");
    */
  }
  
  //... else, there is an error with the current measurement.
  else
  {
    #if VERBOSE
    Serial.print("Error acquiring measurement!");
    delay(1000);
    #endif
  }
  //End [B.3]
  
  // --- End B ---    




  Serial.println("Looping!");
  delay(500);
}
