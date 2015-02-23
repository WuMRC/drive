// Based onBluegiga BGLib Arduino interface library slave device stub sketch
// 2014-02-12 by Jeff Rowberg <jeff@rowberg.net>
// 2014-11-02 modified by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/WuMRC

#include "Wire.h"
#include "Math.h"
#include "BGLib.h" // BGLib C library for BGAPI communication.
#include "AD5933.h" //Library for AD5933 functions (must be installed)
#include "Micro40Timer.h" // Timer function for notifications
#include "Adafruit_LSM303.h" // For accelerometer data

// uncomment the following line for debug serial output
#define DEBUG

// ================================================================
// Constants
// ================================================================

#define TWI_FREQ 400000L      // Set TWI/I2C Frequency to 400MHz.

#define cycles_base 15       // Cycles to ignore before a measurement is taken. Max is 511.

#define cycles_multiplier 1    // Multiple for cycles_base. Can be 1, 2, or 4.

#define cal_resistance 461  // Calibration resistance for the gain factor. 

#define cal_samples 10         // Number of measurements to take of the calibration resistance.

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

long sampleRatePeriod = 0; // Android app sample rate period (microseconds).

uint8_t currentStep = 0; // Used to loop frequency sweeps.

uint8_t sampleRate = 50; // Android app sample rate (Hz).

uint8_t startFreq = 50;       // AC Start frequency (KHz).

uint8_t stepSize = 0;        // AC frequency step size between consecutive values (KHz).

uint8_t numOfIncrements = 0;       // Number of frequency increments.

uint8_t bioImpData[11]; // Unsigned integer array to carry data to phone.

uint8_t sampleRateArray[2] = {
  sampleRate, 0}; // Initialize default sample rate value holder.

uint8_t freqSweepArray[4] = {
  startFreq, stepSize , numOfIncrements, 0}; // Initialize default frquency sweep value holder.

double startFreqHz = ((long)(startFreq)) * 1000; // AC Start frequency (Hz).

double stepSizeHz = 0; // AC frequency step size between consecutive values (Hz).

double endFreqHz = 0; // End frequency for 2 point calibration.

double gain_factor = 0;      // Initialize Gain factor.

double Z_Value = 0;          // Initialize impedance magnitude.

double rComp = 0;            // Initialize real component value.

double iComp = 0;            // Initialize imaginary component value.

double systemPhaseShift = 0;       // Initialize system phase shift value.

double phaseAngle = 0;       // Initialize phase angle value.

double deltaGF = 0; // Used for 2 point calibration.

double deltaPS = 0; // Used for 2 point calibration.

double pseudo = 0; // Pseudo signal to replace accelerometer data.

double temp = 0; // Used to update AD5933's temperature.

double* GF_Array = NULL; // Pointer for dynamic gain factor array size.

double* PS_Array = NULL; // Pointer for dynamic phase shift array size.

double* CR_Array = NULL; // Pointer for calibration impedance at a certain frequency. Used for filtering.

boolean NOTIFICATIONS_FLAG = false; // Variable that toggles notifications to phone subscription state. 

boolean TIMED_OUT_FLAG = false; // Used to identify if the module timed out.

boolean FREQ_SWEEP_FLAG = false; // Used to toggle frequency sweeps.

volatile 
boolean SAMPLE_RATE_FLAG = false;  // Variable to manage sample rate. Managed from interrupt context.

Adafruit_LSM303 lsm;

// ================================================================
// BLE STATE TRACKING 
// ================================================================

// BLE state machine definitions

#define BLE_STATE_STANDBY           0
#define BLE_STATE_SCANNING          1
#define BLE_STATE_ADVERTISING       2
#define BLE_STATE_CONNECTING        3
#define BLE_STATE_CONNECTED_MASTER  4
#define BLE_STATE_CONNECTED_SLAVE   5
#define BLE_STATE_TIMED_OUT         6

// BLE state/link status tracker

uint8_t ble_state = BLE_STATE_STANDBY;
uint8_t ble_encrypted = 0;  // 0 = not encrypted, otherwise = encrypted
uint8_t ble_bonding = 0xFF; // 0xFF = no bonding, otherwise = bonding handle

// ================================================================
// HARDWARE CONNECTIONS AND GATT STRUCTURE SETUP
// ================================================================

#define LED_PIN         13  // Arduino Uno LED pin
#define BLE_RESET_PIN   6   // BLE reset pin (active-low)

#define GATT_HANDLE_C_BIOIMPEDANCE_DATA   17  // 0x11, supports "read", "notify" and "indicate" operations
#define GATT_HANDLE_C_SAMPLE_RATE   21  // 0x15, supports "read" and "write" operations
#define GATT_HANDLE_C_AC_FREQ   25  // 0x19, supports "read" and "write" operations

// use SoftwareSerial on pins D8/D8 for RX/TX (Arduino side)

//Serial1 bleSerialPort(19, 18); // change this to 3, 4 when using TinyDuino as pins cannot be changed to use
// AltSoftSerial

// create BGLib object:

//  - use nothing for passthrough comms (0 = null pointer)
//  - enable packet mode on API protocol since flow control is unavailable

BGLib ble112((HardwareSerial *)&Serial1, 0, 1);

#define BGAPI_GET_RESPONSE(v, dType) dType *v = (dType *)ble112.getLastRXPayload()

// ================================================================
// ARDUINO APPLICATION SETUP AND LOOP FUNCTIONS
// ================================================================

// initialization sequence

void setup() {

  // ================================================================
  // For AD5933
  // ================================================================

  Wire.begin(); // Start Arduino I2C library

  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1); // Clear bits in port

  AD5933.setExtClock(false); 
  AD5933.resetAD5933(); 
  AD5933.setSettlingCycles(cycles_base,cycles_multiplier); 
  AD5933.setStartFreq(startFreqHz); 
  AD5933.setVolPGA(0, 1); 
  temp = AD5933.getTemperature(); 
  AD5933.getGainFactorC(cal_resistance, cal_samples, gain_factor, systemPhaseShift, false);

  // ================================================================
  // For BLE, Serial Communication and Timer interrupts
  // ================================================================

  // initialize status LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // initialize BLE reset pin (active-low)
  pinMode(BLE_RESET_PIN, OUTPUT);
  digitalWrite(BLE_RESET_PIN, HIGH);

  // set up internal status handlers (these are technically optional)
  ble112.onBusy = onBusy;
  ble112.onIdle = onIdle;
  ble112.onTimeout = onTimeout;

  // set up BGLib event handolers
  ble112.ble_evt_system_boot = my_ble_evt_system_boot;
  ble112.ble_evt_connection_status = my_ble_evt_connection_status;
  ble112.ble_evt_connection_disconnected = my_ble_evt_connection_disconnect;
  ble112.ble_evt_attributes_value = my_ble_evt_attributes_value;
  ble112.ble_evt_attclient_indicated = my_ble_evt_attclient_indicated;
  ble112.ble_evt_attributes_status = my_ble_evt_attributes_status;
  ble112.ble_rsp_attributes_write = my_ble_rsp_attributes_write; 

  ctrReg = AD5933.getByte(0x80);

  // open Arduino USB serial
  // use 38400 since it works at 8MHz as well as 16MHz

  Serial.begin(38400);
  while (!Serial);

  // open BLE Hardware serial port
  Serial1.begin(38400);

  my_ble_evt_system_boot(NULL);

  // reset module
  digitalWrite(BLE_RESET_PIN, LOW);
  delay(5); // wait 5ms
  digitalWrite(BLE_RESET_PIN, HIGH);

  Serial.print("Start freq: ");
  Serial.print(startFreqHz);
  Serial.println();

  Serial.print("System Phase Shift: ");
  Serial.print(systemPhaseShift, 4);
  Serial.println();

  // Write default values for handles once. 
  ble112.ble_cmd_attributes_write(GATT_HANDLE_C_SAMPLE_RATE, 0, 1, sampleRateArray);

  delay(20); // Wait 20 ms so async callback isn't blocked.

  ble112.ble_cmd_attributes_write(GATT_HANDLE_C_AC_FREQ, 0, 3, freqSweepArray); 

  bioImpData[10] = startFreq; // initialize start frequency data.

  CR_Array = new double[1];
  CR_Array[0] = cal_resistance; // initialize CR array;

  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM303. Check your wiring!");
    while (1);
  }
  else {
    Serial.println("All good, starting program.");
  }

  // Start with sampling rate of 50 hertz
  sampleRatePeriod = 20000;
  Micro40Timer::set(sampleRatePeriod, notify); // 20000 microsecond period -> 50 hertz frequency
  Micro40Timer::start();
}

// ================================================================
// MAIN APPLICATION LOOP 
// ================================================================

void loop() {

  // keep polling for new data from BLE

  ble112.checkActivity();

  pseudo = pseudo + 0.1; // change psuedo signal independent of sample rate.

  if(pseudo >= 180) {
    pseudo = -180;
  }  

  // Check if GATT Client (Smartphone) is subscribed to notifications.

  if (SAMPLE_RATE_FLAG) { // this flag is toggled from the timer's interrupt context. It controls the sample rate. 

    // For AD5933
    // =================================================== 

    // Serial.println(millis());

    bioImpData[10] = startFreq + (currentStep * stepSize);
    AD5933.tempUpdate();

    if(!FREQ_SWEEP_FLAG) { // Repeat frequency, don't sweep.
      AD5933.setCtrMode(REPEAT_FREQ);
      AD5933.getComplex(gain_factor, systemPhaseShift, Z_Value, phaseAngle);
    }

    else { // Perform frequency sweep at the speed of the loop determined by SAMPLE_RATE_FLAG.

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

    // For BLE
    // =================================================== 

    /*bioImpData[0] = (35 * sin((pseudo*3.14)/180));
     bioImpData[1] = (35 * cos((pseudo*3.14)/180));
     bioImpData[2] = (0);*/
    lsm.read();
    bioImpData[0] = (int) (lsm.accelData.x / 10);
    bioImpData[1] = (int) (lsm.accelData.y / 10);
    bioImpData[2] = (int) (lsm.accelData.z / 10);  

    if(Z_Value > 7000) {

      for(int i = 3; i <= 7; i++) {    // Transmit zero if filter is not satisfied.    
        bioImpData[i] = 0;
      }

    }
    else {
      Z_Value += 0.00005;
      Z_Value *= 10000;
      phaseAngle += 0.00005;
      phaseAngle *= 10000;
      updateData((long) Z_Value, (long) phaseAngle, bioImpData);
    }     

    //Write notification values to characteristic on ble112. Causes notification to be sent.
    ble112.ble_cmd_attributes_write(GATT_HANDLE_C_BIOIMPEDANCE_DATA, 0, 11, bioImpData);
    SAMPLE_RATE_FLAG = false; // Switch this flag back to false till timer interrupt switches it back on.     
  }   
  else {
    // Do zilch, zip, nada, nothing if notifications are not enabled.
    //AD5933.setCtrMode(POWER_DOWN, ctrReg);
  } 

  // blink Arduino LED based on state:
  //  - solid = STANDBY
  //  - 1 pulse per second = ADVERTISING
  //  - 2 pulses per second = CONNECTED_SLAVE
  //  - 3 pulses per second = CONNECTED_SLAVE with encryption

  uint16_t slice = millis() % 1000;

  if (ble_state == BLE_STATE_STANDBY) {
    digitalWrite(LED_PIN, HIGH);
  } 
  if (ble_state == BLE_STATE_ADVERTISING) {
    digitalWrite(LED_PIN, slice < 100);
  } 
  if (ble_state == BLE_STATE_TIMED_OUT) {
    digitalWrite(LED_PIN, LOW);
  } 
  if (ble_state == BLE_STATE_CONNECTED_SLAVE) {    
    if (!ble_encrypted) {
      digitalWrite(LED_PIN, slice < 100 || (slice > 200 && slice < 300));   
    }       
    else {
      digitalWrite(LED_PIN, slice < 100 || (slice > 200 && slice < 300) || (slice > 400 && slice < 500));
    }
  }
}

// ================================================================
// INTERNAL BGLIB CLASS CALLBACK FUNCTIONS
// ================================================================

// called when the module begins sending a command
void onBusy() {
  // turn LED on when we're busy
  //digitalWrite(LED_PIN, HIGH);

}

// called when the module receives a complete response or "system_boot" event
void onIdle() {
  // turn LED off when we're no longer busy
  //    digitalWrite(LED_PIN, LOW);
}

// called when the parser does not read the expected response in the specified time limit
void onTimeout() {
  ble_state == BLE_STATE_TIMED_OUT;
  TIMED_OUT_FLAG = true;
  // reset module (might be a bit drastic for a timeout condition though)
  Serial.println(F("Timed out."));
  resetBLE();

}

// ================================================================
// APPLICATION EVENT HANDLER FUNCTIONS
// ================================================================

void my_ble_evt_system_boot(const ble_msg_system_boot_evt_t *msg) {
#ifdef DEBUG
  Serial.print("###\tsystem_boot: { ");
  Serial.print("major: "); 
  Serial.print(msg -> major, HEX);
  Serial.print(", minor: "); 
  Serial.print(msg -> minor, HEX);
  Serial.print(", patch: "); 
  Serial.print(msg -> patch, HEX);
  Serial.print(", build: "); 
  Serial.print(msg -> build, HEX);
  Serial.print(", ll_version: "); 
  Serial.print(msg -> ll_version, HEX);
  Serial.print(", protocol_version: "); 
  Serial.print(msg -> protocol_version, HEX);
  Serial.print(", hw: "); 
  Serial.print(msg -> hw, HEX);
  Serial.println(" }");
#endif

  // system boot means module is in standby state
  //ble_state = BLE_STATE_STANDBY;
  // ^^^ skip above since we're going right back into advertising below

  TIMED_OUT_FLAG = false; // reset time out boolean

  // set advertisement interval to 200-300ms, use all advertisement channels
  // (note min/max parameters are in units of 625 uSec)
  ble112.ble_cmd_gap_set_adv_parameters(320, 480, 7);
  while (ble112.checkActivity(1000));

  // USE THE FOLLOWING TO LET THE BLE STACK HANDLE YOUR ADVERTISEMENT PACKETS
  // ========================================================================
  // start advertising general discoverable / undirected connectable
  //ble112.ble_cmd_gap_set_mode(BGLIB_GAP_GENERAL_DISCOVERABLE, BGLIB_GAP_UNDIRECTED_CONNECTABLE);
  //while (ble112.checkActivity(1000));

  // USE THE FOLLOWING TO HANDLE YOUR OWN CUSTOM ADVERTISEMENT PACKETS
  // =================================================================

  // build custom advertisement data
  // default BLE stack value: 0201061107e4ba94c3c9b7cdb09b487a438ae55a19
  uint8 adv_data[] = {
    0x02, // field length
    BGLIB_GAP_AD_TYPE_FLAGS, // field type (0x01)
    0x06, // data (0x02 | 0x04 = 0x06, general discoverable + BLE only, no BR+EDR)
    0x11, // field length
    BGLIB_GAP_AD_TYPE_SERVICES_128BIT_ALL, // field type (0x07)
    0xe4, 0xba, 0x94, 0xc3, 0xc9, 0xb7, 0xcd, 0xb0, 0x9b, 0x48, 0x7a, 0x43, 0x8a, 0xe5, 0x5a, 0x19
  };

  // set custom advertisement data
  ble112.ble_cmd_gap_set_adv_data(0, 0x15, adv_data);
  while (ble112.checkActivity(1000));

  // build custom scan response data (i.e. the Device Name value)
  // default BLE stack value: 140942474c69622055314131502033382e344e4657
  uint8 sr_data[] = {
    0x14, // field length
    BGLIB_GAP_AD_TYPE_LOCALNAME_COMPLETE, // field type
    'Z', '_', 'B', 'L', 'E',' ', 'D', '3', 'm', 'o', ' ',
    //'M', 'E', 'G', 'A', ' ', 
    '0', '0', ':', '0', '0', ':', '0', '0'
  };

  // get BLE MAC address
  ble112.ble_cmd_system_address_get();
  while (ble112.checkActivity(1000));
  BGAPI_GET_RESPONSE(r0, ble_msg_system_address_get_rsp_t);

  // assign last three bytes of MAC address to ad packet friendly name (instead of 00:00:00 above)
  sr_data[13] = (r0 -> address.addr[2] / 0x10) + 48 + ((r0 -> address.addr[2] / 0x10) / 10 * 7); // MAC byte 4 10's digit
  sr_data[14] = (r0 -> address.addr[2] & 0xF)  + 48 + ((r0 -> address.addr[2] & 0xF ) / 10 * 7); // MAC byte 4 1's digit
  sr_data[16] = (r0 -> address.addr[1] / 0x10) + 48 + ((r0 -> address.addr[1] / 0x10) / 10 * 7); // MAC byte 5 10's digit
  sr_data[17] = (r0 -> address.addr[1] & 0xF)  + 48 + ((r0 -> address.addr[1] & 0xF ) / 10 * 7); // MAC byte 5 1's digit
  sr_data[19] = (r0 -> address.addr[0] / 0x10) + 48 + ((r0 -> address.addr[0] / 0x10) / 10 * 7); // MAC byte 6 10's digit
  sr_data[20] = (r0 -> address.addr[0] & 0xF)  + 48 + ((r0 -> address.addr[0] & 0xF ) / 10 * 7); // MAC byte 6 1's digit

  // set custom scan response data (i.e. the Device Name value)
  ble112.ble_cmd_gap_set_adv_data(1, 0x15, sr_data);
  while (ble112.checkActivity(1000));

  // put module into discoverable/connectable mode (with user-defined advertisement data)
  ble112.ble_cmd_gap_set_mode(BGLIB_GAP_USER_DATA, BGLIB_GAP_UNDIRECTED_CONNECTABLE);
  while (ble112.checkActivity(1000));

  // set state to ADVERTISING depending on timed out condition
  if(TIMED_OUT_FLAG == false) {
    ble_state = BLE_STATE_ADVERTISING;
  }
  else {
    ble_state = BLE_STATE_TIMED_OUT;
  }
}

void my_ble_evt_connection_status(const ble_msg_connection_status_evt_t *msg) {
#ifdef DEBUG
  Serial.print("###\tconnection_status: { ");
  Serial.print("connection: "); 
  Serial.print(msg -> connection, HEX);
  Serial.print(", flags: "); 
  Serial.print(msg -> flags, HEX);
  Serial.print(", address: ");
  // this is a "bd_addr" data type, which is a 6-byte uint8_t array
  for (uint8_t i = 0; i < 6; i++) {
    if (msg -> address.addr[i] < 16) Serial.write('0');
    Serial.print(msg -> address.addr[i], HEX);
  }
  Serial.print(", address_type: "); 
  Serial.print(msg -> address_type, HEX);
  Serial.print(", conn_interval: "); 
  Serial.print(msg -> conn_interval, HEX);
  Serial.print(", timeout: "); 
  Serial.print(msg -> timeout, HEX);
  Serial.print(", latency: "); 
  Serial.print(msg -> latency, HEX);
  Serial.print(", bonding: "); 
  Serial.print(msg -> bonding, HEX);
  Serial.println(" }");
#endif

  // "flags" bit description:
  //  - bit 0: connection_connected
  //           Indicates the connection exists to a remote device.
  //  - bit 1: connection_encrypted
  //           Indicates the connection is encrypted.
  //  - bit 2: connection_completed
  //           Indicates that a new connection has been created.
  //  - bit 3; connection_parameters_change
  //           Indicates that connection parameters have changed, and is set
  //           when parameters change due to a link layer operation.

  // check for new connection established
  if ((msg -> flags & 0x05) == 0x05) {
    // track state change based on last known state, since we can connect two ways
    if (ble_state == BLE_STATE_ADVERTISING) {
      ble_state = BLE_STATE_CONNECTED_SLAVE;
    } 
    else {
      ble_state = BLE_STATE_CONNECTED_MASTER;
    }
  }

  // update "encrypted" status
  ble_encrypted = msg -> flags & 0x02;

  // update "bonded" status
  ble_bonding = msg -> bonding;
}

void my_ble_evt_connection_disconnect(const struct ble_msg_connection_disconnected_evt_t *msg) {
#ifdef DEBUG
  Serial.print("###\tconnection_disconnect: { ");
  Serial.print("connection: "); 
  Serial.print(msg -> connection, HEX);
  Serial.print(", reason: "); 
  Serial.print(msg -> reason, HEX);
  Serial.println(" }");
#endif

  // set state to DISCONNECTED
  //ble_state = BLE_STATE_DISCONNECTED;
  // ^^^ skip above since we're going right back into advertising below

  // after disconnection, resume advertising as discoverable/connectable
  //ble112.ble_cmd_gap_set_mode(BGLIB_GAP_GENERAL_DISCOVERABLE, BGLIB_GAP_UNDIRECTED_CONNECTABLE);
  //while (ble112.checkActivity(1000));

  // after disconnection, resume advertising as discoverable/connectable (with user-defined advertisement data)
  ble112.ble_cmd_gap_set_mode(BGLIB_GAP_USER_DATA, BGLIB_GAP_UNDIRECTED_CONNECTABLE);
  while (ble112.checkActivity(1000));

  // set state to ADVERTISING
  ble_state = BLE_STATE_ADVERTISING;

  // clear "encrypted" and "bonding" info
  ble_encrypted = 0;
  ble_bonding = 0xFF;
}

void my_ble_evt_attributes_value(const struct ble_msg_attributes_value_evt_t *msg) {
  Micro40Timer::stop(); // stop data being written to BLE (Timer interrupts) as soon as this callback is called.
#ifdef DEBUG
  Serial.print("###\tattributes_value: { ");
  Serial.print("connection: "); 
  Serial.print(msg -> connection, HEX);
  Serial.print(", reason: "); 
  Serial.print(msg -> reason, HEX);
  Serial.print(", handle: "); 
  Serial.print(msg -> handle, HEX);
  Serial.print(", offset: "); 
  Serial.print(msg -> offset, HEX);
  Serial.print(", value_len: "); 
  Serial.print(msg -> value.len, HEX);
  Serial.print(", value_data: ");
  // this is a "uint8array" data type, which is a length byte and a uint8_t* pointer
  for (uint8_t i = 0; i < msg -> value.len; i++) {
    if (msg -> value.data[i] < 16) Serial.write('0');
    Serial.print(msg -> value.data[i], HEX);
  }
  Serial.println(" }");
#endif

  // check for data written to "c_sample_rate" handle
  if (msg -> handle == GATT_HANDLE_C_SAMPLE_RATE && msg -> value.len > 0) {

    sampleRate = msg -> value.data[0];
    sampleRatePeriod = 1000000 / ((long) sampleRate);
    
    sampleRateArray[0] = sampleRate;
    sampleRateArray[1] = 1;

    Serial.println();
    Serial.print("Sucessful write attempt; new frequency / period: ");
    Serial.print(sampleRate);
    Serial.print(" hertz");
    Serial.print(" / ");         
    Serial.print(sampleRatePeriod);
    Serial.print(" microseconds.");
    Serial.println();

    ble112.ble_cmd_attributes_write(GATT_HANDLE_C_SAMPLE_RATE, 0, 2, sampleRateArray); 

    Micro40Timer::set(sampleRatePeriod, notify); 
    Micro40Timer::start();
  }
  // check for data written to "c_ac_freq" handle  
  if (msg -> handle == GATT_HANDLE_C_AC_FREQ && msg -> value.len > 0) {

    delete [] GF_Array; // Free memory from previous GF array.
    delete [] PS_Array; // Free memory from previous PS array.
    delete [] CR_Array; // Free memory from previous CR array.

    Serial.println("Old Arrays Deleted.");

    startFreq = msg -> value.data[0]; 
    stepSize = msg -> value.data[1];  
    numOfIncrements = msg -> value.data[2];
    freqSweepArray[0] = startFreq;
    freqSweepArray[1] = stepSize;
    freqSweepArray[2] = numOfIncrements;
    freqSweepArray[3] = 1;

    bioImpData[10] = startFreq; // update AC freq value
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

      // generate gain factor array using two point calibration.

      GF_Array = new double[numOfIncrements + 1];
      PS_Array = new double[numOfIncrements + 1];
      CR_Array = new double[numOfIncrements + 1];

      deltaGF = 0;
      deltaPS = 0;

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

      /*AD5933.getGainFactors_LI(
       cal_resistance, cal_samples, startFreqHz, endFreqHz,
       GF_Array[0], GF_Array[numOfIncrements], 
       PS_Array[0], PS_Array[numOfIncrements]);
       
       deltaGF = GF_Array[numOfIncrements] - GF_Array[0];
       deltaPS = PS_Array[numOfIncrements] - PS_Array[0];
       
       AD5933.getArraysLI(
       deltaGF, deltaPS,
       stepSizeHz, numOfIncrements,
       GF_Array[0], PS_Array[0],
       GF_Array, PS_Array);*/

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
    
    ble112.ble_cmd_attributes_write(GATT_HANDLE_C_AC_FREQ, 0, 4, freqSweepArray); 

    Micro40Timer::set(sampleRatePeriod, notify);
    Micro40Timer::start();    
  }  
}
void my_ble_evt_attclient_indicated(const struct ble_msg_attclient_indicated_evt_t *msg) {
#ifdef DEBUG
  Serial.print("###\tattclient_indicate: { ");
  Serial.print("Indication received.");
  Serial.println(" }");
#endif
}

void my_ble_rsp_attributes_write(const struct ble_msg_attributes_write_rsp_t * msg) {
#ifdef DEBUG
  if(msg -> result == 0) {
  }
  else {
    Serial.print("###\trsp_attributes_write: {");
    Serial.print("result: "); 
    Serial.print(msg -> result, DEC);
    Serial.println("}");
  }
#endif  
}

void my_ble_evt_attributes_status (const struct ble_msg_attributes_status_evt_t *msg) {
#ifdef DEBUG
  Serial.print("###\tattributes_status: { ");
  Serial.print("nSubscription changed");
  Serial.print(", flags: "); 
  Serial.print(msg -> flags, HEX);
  Serial.print(", Handle: "); 
  Serial.print(msg -> handle, DEC);
  Serial.println(" }");
#endif

  if (msg -> flags == 1) {
    NOTIFICATIONS_FLAG = true;
  }        
  else if (msg -> flags == 0){
    NOTIFICATIONS_FLAG = false;
  }
  else {
    NOTIFICATIONS_FLAG = false;
  }  
}

void notify() {
  if (NOTIFICATIONS_FLAG) {
    SAMPLE_RATE_FLAG = true; 
  }
}

void updateData(long magnitude, long phaseAng, uint8_t *values) {

  if(phaseAng > 0) {
    values[9] = phaseAng % 100;
    phaseAng /= 100;
    values[8] = phaseAng % 100;
    phaseAng /= 100;
    values[7] = phaseAng;
  }
  else {
    phaseAng *= -1;
    values[9] =  -1 * (phaseAng % 100);
    phaseAng /= 100;
    values[8] = phaseAng % 100;
    phaseAng /= 100;
    values[7] = phaseAng;  
  }  
  values[6] = magnitude % 100;
  magnitude /= 100;
  values[5] = magnitude % 100;
  magnitude /= 100;
  values[4] = magnitude % 100;
  magnitude /= 100;
  values[3] = magnitude % 100;
}

void resetBLE() {
  digitalWrite(BLE_RESET_PIN, LOW);
  delay(50); // wait 5ms
  digitalWrite(BLE_RESET_PIN, HIGH);
  Serial.println("Reset attempt.");
}








