
#include "Wire.h"
#include "Math.h"
#include "AD5933.h" //Library for AD5933 functions (must be installed)

// ================================================================
// Constants
// ================================================================

#define TWI_FREQ 400000L      // Set TWI/I2C Frequency to 400MHz.

const int CYCLES_BASE = 15;       // Cycles to ignore before a measurement is taken. Max is 511.

const int CYCLES_MULTIPLIER = 1;    // Multiple for CYCLES_BASE. Can be 1, 2, or 4.

const int CAL_RESISTANCE = 353;  // Calibration resistance for the gain factor. 

const int CAL_SAMPLES = 10;         // Number of measurements to take of the calibration resistance.

const int LED7_R = 3;

const int LED7_G = 4;

const int LED7_B = 5;

const int LED5 = 13;

const int LED6 = 12;

const int LED8 = 11;

const int PUSH1 = 27; // Push button 1:  Switch 2

const int PUSH2 = 28; // Push button 2:  Switch 3

const int BOOST = 22; // 5V On

const int BI_TETRA = 29; // Selects between Bi-Polar & Tetra-Polar modes

const int SW_VI = 23; // Voltage current multiplexer

const int DELAY = 10; // Voltage current multiplexer

int leds[6] = {
  LED5, LED6, LED7_R, LED7_G, LED7_B, LED8};

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

int startFreq = 50; // AC start frequency in kilohertz.

int stepSize = 0;  // Step size of AC frequency sweep in kilohertz.

int numOfIncrements = 0;  // Number of increments in frequency sweep.

int R_VOLTAGE = 0;

int I_VOLTAGE = 0;

int R_CURRENT = 0;

int I_CURRENT = 0;

double GAIN_FACTOR = 0;

double Z_VALUE = 0;

double temp = 0;

double gain_factor = 0;

double systemPhaseShift = 0;

void setup() {

  // ================================================================
  // For AD5933
  // ================================================================

  // put your setup code here, to run once:
  //Set IOs mode 

  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);  
  pinMode(LED7_R, OUTPUT);
  pinMode(LED7_G, OUTPUT);
  pinMode(LED7_B, OUTPUT);
  pinMode(LED8, OUTPUT);

  pinMode(BI_TETRA, OUTPUT);
  pinMode(SW_VI, OUTPUT);


  pinMode(BOOST, OUTPUT);

  pinMode(PUSH1, INPUT);
  pinMode(PUSH2, INPUT);

  pinMode(25, INPUT);
  pinMode(24, INPUT);

  //All leds off
  for(int i=0;i<6;i++) {
    digitalWrite(leds[i],HIGH);
  }

  digitalWrite(BOOST, HIGH);
  digitalWrite(BI_TETRA, LOW); // Set AD5933 to tetra-polar mode

  Wire.begin(); // Start Arduino I2C library
  Serial.begin(38400); // Open serial port
  Serial.println();

  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1); // Clear bits in port

  AD5933.setExtClock(false); 
  AD5933.resetAD5933(); 
  AD5933.setSettlingCycles(CYCLES_BASE, CYCLES_MULTIPLIER); 
  AD5933.setStartFreq(50000);
  AD5933.setRange(1);

  delay(DELAY);
  AD5933.setPGA(1);
  delay(DELAY); 
  temp = AD5933.getTemperature(); 

  AD5933.getGainFactorC(CAL_RESISTANCE, CAL_SAMPLES, gain_factor, systemPhaseShift, false);

  AD5933.setCtrMode(REPEAT_FREQ);

  delay(DELAY);
  digitalWrite(SW_VI, HIGH); // Voltage measurement
  delay(DELAY);
  AD5933.getComplexRawOnce(R_VOLTAGE, I_VOLTAGE);

  AD5933.setCtrMode(REPEAT_FREQ);

  delay(DELAY);
  digitalWrite(SW_VI, LOW); // Current measurement
  delay(DELAY);
  AD5933.getComplexRawOnce(R_CURRENT, I_CURRENT);

  AD5933.setCtrMode(REPEAT_FREQ);

  GAIN_FACTOR = CAL_RESISTANCE * (getMag(R_CURRENT, I_CURRENT) / getMag(R_VOLTAGE, I_VOLTAGE)); 

  Serial.println();
  Serial.print("R_CURRENT is: ");
  Serial.println(R_CURRENT);
  Serial.print("I_CURRENT is: ");
  Serial.println(I_CURRENT);
  Serial.print("R_VOLTAGE is: ");
  Serial.println(R_VOLTAGE); 
  Serial.print("I_VOLTAGE is: ");
  Serial.println(I_VOLTAGE);   
  Serial.print("Gain factor is: ");
  Serial.println(GAIN_FACTOR);
}

void loop() {

  if (Serial.available()) {
    delay(15);
    uint8_t ch = Serial.read();
    uint8_t status;

    if (ch == '0') {
      Serial.println("Switch off.");
    }

    if (ch == '1') {

      AD5933.setCtrMode(REPEAT_FREQ);

      delay(DELAY);
      digitalWrite(SW_VI, HIGH); // Voltage measurement
      delay(DELAY);
      AD5933.getComplexRawOnce(R_VOLTAGE, I_VOLTAGE);

      AD5933.setCtrMode(REPEAT_FREQ);

      delay(DELAY);
      digitalWrite(SW_VI, LOW); // Current measurement
      delay(DELAY);
      AD5933.getComplexRawOnce(R_CURRENT, I_CURRENT);

      Z_VALUE = GAIN_FACTOR * (getMag(R_VOLTAGE, I_VOLTAGE) / getMag(R_CURRENT, I_CURRENT));

      //Serial.println();
      //Serial.print("Impedance is: ");
      Serial.println(Z_VALUE);
    }

    if (ch == '2') {

    }

  } // end if serial available

}

double getMag(int cReal, int cImag) {
  return sqrt( ( square(cReal) + square(cImag)) );
}






















