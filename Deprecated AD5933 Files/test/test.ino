#include "Math.h"

#define B 98

#define S 115

#define R 114

#define FR 102

int incomingByte = 0;
int firstByte = 0;
int bytesAvailable = 0;
int sIncrement = 0;
int fIncrement = 0;
int sampleRate = 50;
int startFreq = 0;       
int stepSize = 0;      
int numOfIncrements = 0; 
int sampleRateHolder = 0;
int startFreqHolder = 0;       
int stepSizeHolder = 0;      
int numOfIncrementsHolder = 0; 
int castHolder = 0;
int numberOfCommas = 0;
boolean inputSucess = false;


void setup() {
  Serial.begin(38400);
  Serial.println();
  Serial.println();
  Serial.println("Welcome! Please use the following syntax to input commands:");
  Serial.println("All frequencies are in kilohertz.");
  Serial.println("b: begin sampling.");
  Serial.println("s: stop sampling");
  Serial.println("r,samplerate: change sample rate. samplerate must be between 1 and 100, pad value with zero where necessary. No spaces.");
  Serial.println("f,frequency: set single frequency sweep. frequency must be between 1 and 100, pad value with zero where necessary. No spaces.");
  Serial.println("f,frequency,stepSize,numberOfIncrements: set multiple frequency sweep. values must be between 1 and 100, pad values with zero where necessary. No spaces.");
  Serial.println();
}

void loop() {

  if (Serial.available() > 0) {

    delay(15);
    bytesAvailable = Serial.available();
    firstByte = Serial.read();
    sIncrement = 0;
    fIncrement = 0;
    sampleRateHolder = 0;
    startFreqHolder = 0;       
    stepSizeHolder = 0;      
    numOfIncrementsHolder = 0;
    numberOfCommas = 0;
    inputSucess = false; 

    switch(firstByte) {
    case B:
      Serial.println("Sample rate: true");
      break;

    case S:
      Serial.println("Sample rate: false");
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
        sIncrement++;

        if(incomingByte == 44) {
          Serial.println("Comma.");
        }

        else if (incomingByte > 47 && incomingByte < 58){
          //Parse ASCII char value to int and cocatenate to form sample Rate
          castHolder = (char) incomingByte;
          sampleRateHolder += ((pow(10, ((double) (bytesAvailable - sIncrement - 1)))) * (castHolder - 48)) + 0.5; 
          inputSucess = true;
        }

        else {
          Serial.println("Error. Please check your syntax.");
          inputSucess = false;
        }

        if(inputSucess = true) {
          sampleRate = sampleRateHolder;
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
            castHolder = (char) incomingByte;
            startFreqHolder += ((pow(10, ((double) (bytesAvailable - fIncrement - 10)))) * (castHolder - 48)) + 0.5;
            inputSucess = true; 
          }

          else if (numberOfCommas == 2 && incomingByte > 47 && incomingByte < 58){
            //Parse ASCII char value to int and cocatenate to form step seiz
            castHolder = (char) incomingByte;
            stepSizeHolder += ((pow(10, ((double) (bytesAvailable - fIncrement - 10)))) * (castHolder - 48)) + 0.5;
            inputSucess = true; 
          }

          else if (numberOfCommas == 3 && incomingByte > 47 && incomingByte < 58){
            //Parse ASCII char value to int and cocatenate to form number of increments
            castHolder = (char) incomingByte;
            numOfIncrementsHolder += ((pow(10, ((double) (bytesAvailable - fIncrement - 10)))) * (castHolder - 48)) + 0.5;
            inputSucess = true; 
          }

          else {
            Serial.println("Error. Please check your syntax (Parse error).");
            inputSucess = false;
          }

          if(inputSucess = true) {
            startFreq = startFreqHolder;
            stepSize = stepSizeHolder;
            numOfIncrements = numOfIncrementsHolder;
          }
        }

        break;
      }

    }
    Serial.println();
    Serial.print("Sample  Rate: ");
    Serial.println(sampleRate, DEC);
    Serial.print("Start Frequency: ");
    Serial.println(startFreq, DEC);
    Serial.print("Step  Size: ");
    Serial.println(stepSize, DEC);
    Serial.print("Number of increments: ");
    Serial.println(numOfIncrements, DEC);
    Serial.println();
  }
}





























