// AD5258 library implementation via Arduino serial monitor by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

// This sketch changes the wiper position for a two AD5258s; "r1" and "r2" in a rheostat configuration.

#include "Wire.h"
#include "Math.h"
#include "AD5258.h" //Library for AD5933 functions (must be installed)

#define R1 49
#define R2 50

int numberOfCommas = 0;
int increment = 0;
uint8_t firstByte = 0;
uint8_t bytesAvailable = 0;
uint8_t wiperPosition = 0;
uint8_t incomingByte = 0;
uint8_t wiperPositionHolder = 0;
boolean inputSucess = false;

AD5258 r1; // rheostat r1
AD5258 r2; // rheostat r1

void setup() {
  Serial.println();
  Wire.begin(); // Start Arduino I2C library
  Serial.begin(38400);
  r1.begin(1);
  r2.begin(2);
  Serial.println();
  Serial.println("Welcome! Please note that wiper positions go from 1 - 63.");
  Serial.println();
  Serial.println("Use the following syntax to input commands:");
  Serial.println();
  Serial.print(1, DEC);
  Serial.println("\t1,wiperPosition - change Rheostat 1.");
  Serial.print(2, DEC);
  Serial.println("\t2,wiperPosition - change Rheostat 2.");
  Serial.println();
}

void loop() {

  if (Serial.available() > 0) {
    delay(15); // Delay because Arduino serial buffer will return 1 if queried too quickly.
    bytesAvailable = Serial.available();
    firstByte = Serial.read();

    while (Serial.available() > 0) {
      incomingByte = Serial.read();

      switch(firstByte) {
      case R1:
      case R2:
        increment++;
        if(incomingByte == 44) {
          numberOfCommas++;
        }
        else if (incomingByte > 47 && incomingByte < 58){
          //Parse ASCII char value to int and cocatenate to form WiperPosition
          wiperPositionHolder += ((pow(10, ((double) (bytesAvailable - increment - 1)))) * (incomingByte - 48)) + 0.5; 
          inputSucess = true;
        }

        else {
          inputSucess = false;
        }

        if(wiperPositionHolder > 64) {
          inputSucess = false;
        }
        break;
      }
    } // while (Serial.available() > 0) {

    if(numberOfCommas > 1) {
      inputSucess = false;
    }

    if(inputSucess) {
      wiperPosition = wiperPositionHolder;
      Serial.print("wiper position: ");
      Serial.print(wiperPosition);

      if(wiperPosition < 64) {
        switch(firstByte) {
        case R1:
          r1.writeRDAC(wiperPosition);
          Serial.println(" for rheostat 1.");
          break;
        case R2:
          r2.writeRDAC(wiperPosition);
          Serial.println(" for rheostat 2.");
          break;
        }
      }
      else {
        switch(firstByte) {
        case R1:
          Serial.println();
          Serial.print("Tolerance for R1 is: ");
          Serial.println(r1.readTolerance());
          break;
        case R2:
          Serial.println();
          Serial.print("Tolerance for R2 is: ");
          Serial.println(r2.readTolerance());
          break;
        }
      }
      wiperPositionHolder = 0;
      numberOfCommas = 0;
      increment = 0;
    }
    else {
      Serial.println("Error, bad value.");
    }
  } // if (Serial.available() > 0) { 
} // void loop() {















































 
