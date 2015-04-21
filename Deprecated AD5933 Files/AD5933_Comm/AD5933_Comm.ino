/*
AD5933_Comm

Initially, this code is to simply test the I2C bus communications with the AD5933.
This includes the "wire.h" library for I2C commmunications and assumes a setup with
SDA as A4 (yellow wire) and SCL as A5 (green wire). The goal for this program is to
take a temperature reading, with the results reported through the serial port.

I'll be expanding the functionality and this intro as I go.
References will be fully cited at the end (specific code snippets will be cited in line).

For reference:
Command Codes
-------------
0xA0 = Block Write
0xA1 = Block Read
0xB0 = Address Pointer

Register Map
------------
0x92 Temperature
0x93

*/

//Include the Wire.h library for I2C communication
#include <Wire.h>

void setup(){
  //Initialize the I2C communications
  Wire.begin();
  //Initialize the serial port communications
  Serial.begin(9600);
}

void loop() {
  
  //[Part 1]: Tell the AD5933 where we want to look in memory
  //
  //Initialize communication at the default address for the AD5933.
  Wire.beginTransmission(0x0D);
  //Send the command that we want to move a pointer.
  Wire.write(0xB0);
  //Send the address we want the pointer set to (temperature).
  Wire.write(0x92);
  //End the communication.
  Wire.endTransmission();
  
  //[Part 2]: Read the status register
  //Request 2 bytes from the AD5933
  Wire.requestFrom(0x0D, 2);
  //Declare the variables to hold the temperature register data
  byte t1,t2;
  //Store the first byte
  t1 = Wire.read();
  //Store the second byte
  t2 = Wire.read();
  ;
 
 //Report the temperature data through the serial port.
 Serial.print(t1);
 Serial.print("\t");
 Serial.println(t2);
 
 delay(1000);
 
  
}


/* 
References 
----------
[1] http://tronixstuff.com/2010/10/20/tutorial-arduino-and-the-i2c-bus/

[2]

[3]
*/


