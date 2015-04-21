/*
AD5933_Single_Temp
-----------------------------------------------------------------------------------------------------------------------
This code is meant as an initial test of communications (via I2C) with the AD5933. Each time it's run, the Arduino
initiates comunications with the AD5933, sends a command to take a single temerature measurement, and then reads the
result back through through the serial monitor. Status checks are included at all steps for debugging.

Yet-to-Do List
--------------
1. Make sub-functions for repeated steps
2. Input temp conversion step using AD5933 datasheet (to translate results from binary).

 
NOTES:
------

Read commands need a pointer to be set first, but single byte write commands do not. 

Command Codes
-------------
0xA0 (1010 0000) = Block Write
0xA1 (1010 0001) = Block Read
0xB0 (1011 0000) = Address Pointer
 
Registers
---------
0x80  Control Command (Input)
0x81
 
0x92  Temperature Data
0x93
-----------------------------------------------------------------------------------------------------------------------
*/
 
#include <Wire.h> //Include the Wire.h library for I2C communication

byte register_0x80 = B00000000; //Initializing variables for the registers we will be reading
byte register_0x8F = B00000000;
byte register_0x92 = B00000000;
byte register_0x93 = B00000000;

void setup(){
  Wire.begin(); //Initialize the I2C communications.
  Serial.begin(9600); //Initialize the serial port communications.
  Serial.println("Program start!"); //Send status message to the serial monitor.
  delay(1000);
  
  //[1]-- Verify that the control register is set to "Standby Mode".
  Serial.println("Checking control register...");//Send status message to the serial monitor.
  Wire.beginTransmission(0x0D); //Open communications to to AD5933.
  Wire.write(B10110000); //Send the "pointer" commmand.
  Wire.write(0x80); //Send the address for the pointer (first control register).
  Wire.endTransmission(); //Close communications.
  Wire.requestFrom(0x0D, 1); //Request the data byte stored in the register the pointer is at. 
  register_0x80 =Wire.read(); //Store the data in a byte variable.
  Serial.print("Control register 0x80 set to: "); //Display the data via the serial monitor.
  Serial.println(register_0x80, BIN);
  //--[1]
  
  delay(1000);
  
  //[2]-- Send command to take a temperature measurement.
  Serial.println("Sending temperature measurement command...");
  Wire.beginTransmission(0x0D); //Open communications to to AD5933.
  Wire.write(0x80); //Send the register we want to look at (first control register).
  Wire.write(B10010000); //Send byte (take temperature measurement command).
  Wire.endTransmission(); //Close communications.
  //--[2]
  
  //Note: No delay here since AD5933 automatically returns to "Standby Mode" after executing
  //a "Measure Temperature" command.
  
  //[3]-- Verify that the control register is set to "Measure Temperature".
  Serial.println("Checking control register...");//Send status message to the serial monitor.
  Wire.beginTransmission(0x0D); //Open communications to to AD5933.
  Wire.write(B10110000); //Send the "pointer" commmand.
  Wire.write(0x80); //Send the address for the pointer (first control register).
  Wire.endTransmission(); //Close communications.
  Wire.requestFrom(0x0D, 1); //Request the data byte stored in the register the pointer is at. 
  register_0x80 =Wire.read(); //Store the data in a byte variable.
  Serial.print("Control register 0x80 set to: "); //Display the data via the serial monitor.
  Serial.println(register_0x80, BIN);
  //--[3]
  
  delay(1000);
  
  //[4]--Check the status register to verify a valid temperature reading was taken and stored.
  Serial.println("Checking status..."); //Send status message to the serial monitor.
  Wire.beginTransmission(0x0D); //Open communications to to AD5933.
  Wire.write(B10110000); //Send the "pointer" commmand.
  Wire.write(0x8F); //Send the address for the pointer (status register).
  Wire.endTransmission(); //Close communications.
  Wire.requestFrom(0x0D, 1); //Request the data byte stored in the register the pointer is at. 
  register_0x8F =Wire.read(); //Store the data in a byte variable.
  Serial.print("Status register 0x8F set to: "); //Display the data via the serial monitor.
  Serial.println(register_0x8F, BIN);
  //--[4]
  
  //[5]-- Read the stored temperature data.
  Serial.println("Checking temperature data registers...");//Send status message to the serial monitor.
  Wire.beginTransmission(0x0D); //Open communications to to AD5933.
  Wire.write(B10110000); //Send the "pointer" commmand.
  Wire.write(0x92); //Send the address for the pointer (first temperature data register).
  Wire.endTransmission(); //Close communications.
  Wire.requestFrom(0x0D, 2); //Request the data byte stored in the register the pointer is at.
  register_0x92 = Wire.read(); //Store the first byte in a variable.
  register_0x93 = Wire.read(); //Store the second byte in a variable.
  Serial.print("Temperature register 0x92 set to: "); //Display the data via the serial monitor.
  Serial.println(register_0x92, BIN);
  Serial.print("Temperature register 0x93 set to: "); //Display the data via the serial monitor.
  Serial.println(register_0x93, BIN);
  //--[3]
  
  delay(1000);
  Serial.println("End Program!"); //Send status message to the serial monitor.
}

void loop() {
  delay(1000);
}


/* 

References 
----------

[1] I2C basics
http://tronixstuff.com/2010/10/20/tutorial-arduino-and-the-i2c-bus/
 
[2] AD5933 register mapping comments (Jon Newman)
https://github.com/open-ephys/autoimpedance/blob/master/code/ardunio/registers.ino

[3]  Temperaure reading function, incomplete? (Jon Newman)
https://github.com/open-ephys/autoimpedance/blob/master/code/ardunio/registers.ino
 
[4] "setByte" and "setRegister" functions, incomplete? (Jon Newman)
https://github.com/open-ephys/autoimpedance/blob/2f756a0137bc48c78484359fb0e21d89360ed67c/code/ardunio/transactions.ino
 
*/
