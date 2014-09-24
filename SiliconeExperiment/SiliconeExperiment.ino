/*************************************************** 
  This is an example for the Adafruit Thermocouple Sensor w/MAX31855K

  Designed specifically to work with the Adafruit Thermocouple Sensor
  ----> https://www.adafruit.com/products/269

  These displays use SPI to communicate, 3 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_MAX31855.h"


int SSRplate = 11;
boolean Switch = false;
int time = 20000;

  
void setup() {
  Serial.begin(38400);
  pinMode(SSRplate, OUTPUT);

  
  Serial.println("MAX31855 test");
  // wait for MAX chip to stabilize
  delay(500);
}


void loop() {
  
  if(Switch == false) {
    digitalWrite(SSRplate, LOW);
  }
      
  if(Switch == true) {
    digitalWrite(SSRplate, HIGH);
  }      
       
  if (Serial.available()) {
  uint8_t ch = Serial.read();
  uint8_t status;
  if (ch == '0') {
    Switch = false;
    Serial.println("Switch off.");
  }
  if (ch == '1') {
    Switch = true;
    Serial.println("Switch on.");
  }
  if (ch == '2') {
    Switch = true;
    Serial.print("Switch is on for ");
    Serial.print((time/1000));    
    Serial.print(" seconds");
    digitalWrite(SSRplate, HIGH);
    Serial.println();        
    delay(time);
    digitalWrite(SSRplate, LOW);
    Switch = false;
  }  
}

  
}
