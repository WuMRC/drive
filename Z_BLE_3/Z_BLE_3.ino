// Accelerometer demo sketch for TinyDuino
// Based onBluegiga BGLib Arduino interface library slave device stub sketch
// and accelerometer demo from TinyCitcuits 
// 2013-06-30 by Ken Burns, TinyCircuits http://Tiny-Circuits.com
// 2014-02-12 by Jeff Rowberg <jeff@rowberg.net>
// 2014-07-13 modified by Adetunji Dahunsi <tunjid.com>
// Updates should (hopefully) always be available at https://github.com/jrowberg/bglib
// 

// Changelog:
//      2014-06-13 - Initial release

/* ============================================
 !!!!!!!!!!!!!!!!!
 !!! IMPORTANT !!!
 !!!!!!!!!!!!!!!!!
 
 THIS SCRIPT WILL NOT COMMUNICATE PROPERLY IF YOU DO NOT ENSURE ONE OF THE
 FOLLOWING IS TRUE:
 
 1. You enable the <wakeup_pin> functionality in your firmware
 
 2. You COMMENT OUT two lines below which depend on wake-up
 funcitonality to work properly (they will BLOCK otherwise):
 
 ble112.onBeforeTXCommand = onBeforeTXCommand;
 ble112.onTXCommandComplete = onTXCommandComplete;
 
/* ============================================
 BGLib Arduino interface library code is placed under the MIT license
 Copyright (c) 2014 Jeff Rowberg
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ===============================================
 */

//#include <SoftwareSerial.h> // software serial library for input and output to the serial monitor.
#include <Wire.h>
#include <Math.h>
#include "BGLib.h" // BGLib C library for BGAPI communication.
#include "AD5933.h" //Library for AD5933 functions (must be installed)
#include <AltSoftSerial.h>
#include <MsTimer2.h> // Timer function for notifications

// uncomment the following line for debug serial output
#define DEBUG

// ================================================================
// For Z_Logger
// ================================================================

double gain_factor = 0;

#define TWI_FREQ 400000L     // Setting TWI/I2C Frequency to 400MHz.

#define cycles_base 300      //First term to set a number of cycles to ignore
                             //to dissipate transients before a measurement is
                             //taken. The max value for this is 511.   
                             
#define cycles_multiplier 1  //Set a multiple for the cycles_base which
                             //is used to calculate the desired number
                             //of settling cycles. Values can be 1, 2, or 4.
                             
#define start_frequency 50000 //Set the initial AC current frequency (KHz).

#define cal_resistance 554.72 //Set a calibration resistance for the gain
                              //factor. This will have to be measured before any
                              //other measurements are performed.  
                              
#define cal_samples 10       //Set a number of measurements to take of the calibration
                             //resistance. These are used to get an average gain
                             //factor.
                             
double Z_value = 0;          // Initialize impedance magnitude

uint8_t upperFreq = 0;       // Upper value for frequency sweep, or single value of frequency
                             // when frequency sweep is off.
                       
uint8_t stepSize = 0;        // frequency step size between upp and lower values.

uint8_t lowerFreq = 0;       // Lower value of frequency sweep.

long rComp;

<<<<<<< HEAD
// ================================================================
=======
boolean dataRead = false;


>>>>>>> FETCH_HEAD
// General variables
// ================================================================

volatile double pseudo = 0; // Pseudo signal to replace cellerometer data.

volatile boolean writer = false;  // Variable that toggles notifications to phone.

boolean notifier = false; // Variable to manage sample rate. Managed from interrupt context.

uint8_t A[6] = {1, 2, 3, 4, 5, 6}; // Unsigned integer array to carry data to phone

long sampleRate = 0; // Android app sample Rate

// ================================================================
// BLE STATE TRACKING (UNIVERSAL TO JUST ABOUT ANY BLE PROJECT)
// ================================================================

// BLE state machine definitions
#define BLE_STATE_STANDBY           0
#define BLE_STATE_SCANNING          1
#define BLE_STATE_ADVERTISING       2
#define BLE_STATE_CONNECTING        3
#define BLE_STATE_CONNECTED_MASTER  4
#define BLE_STATE_CONNECTED_SLAVE   5

// BLE state/link status tracker
uint8_t ble_state = BLE_STATE_STANDBY;
uint8_t ble_encrypted = 0;  // 0 = not encrypted, otherwise = encrypted
uint8_t ble_bonding = 0xFF; // 0xFF = no bonding, otherwise = bonding handle

// ================================================================
// HARDWARE CONNECTIONS AND GATT STRUCTURE SETUP
// ================================================================

// NOTE: this assumes you are using one of the following firmwares:
//  - BGLib_U1A1P_38400_noflow
//  - BGLib_U1A1P_38400_noflow_wake16
//  - BGLib_U1A1P_38400_noflow_wake16_hwake15
// If not, then you may need to change the pin assignments and/or
// GATT handles to match your firmware.

#define LED_PIN         13  // Arduino Uno LED pin
#define BLE_RESET_PIN   6   // BLE reset pin (active-low)

#define GATT_HANDLE_C_BIOIMPEDANCE_DATA   17  // 0x11, supports "read", "notify" and "indicate" operations
#define GATT_HANDLE_C_SAMPLE_RATE   21  // 0x15, supports "write" operation
#define GATT_HANDLE_C_AC_FREQ   24  // 0x18, supports "write" operation

// use SoftwareSerial on pins D8/D8 for RX/TX (Arduino side)

AltSoftSerial bleSerialPort(8, 9); // change this to 3, 4 when using TinyDuino as pins cannot be changed to use
                                   // AltSoftSerial

// create BGLib object:
//  - use SoftwareSerial por for module comms
//  - use nothing for passthrough comms (0 = null pointer)
//  - enable packet mode on API protocol since flow control is unavailable

BGLib ble112((HardwareSerial *)&bleSerialPort, 0, 1);

#define BGAPI_GET_RESPONSE(v, dType) dType *v = (dType *)ble112.getLastRXPayload()

// ================================================================
// ARDUINO APPLICATION SETUP AND LOOP FUNCTIONS
// ================================================================

// initialization sequence
void setup() {
<<<<<<< HEAD
  
  // ================================================================
=======

  TWBR = 1;
  Wire.begin();

>>>>>>> FETCH_HEAD
  // For Z_Logger
  // ================================================================
  
  TWBR = 1;
  Wire.begin();
  AD5933.setExtClock(false);
  AD5933.resetAD5933();
  AD5933.setStartFreq(start_frequency);
  AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
  AD5933.setIncrementinHex(1);
  AD5933.setNumofIncrement(2);
  AD5933.setVolPGA(0,1);
  AD5933.tempUpdate();  
  gain_factor = AD5933.getGainFactor(cal_resistance, cal_samples, false);
  
  // ================================================================
  // For General elements
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

  // ONLY enable these if you are using the <wakeup_pin> parameter in your firmware's hardware.xml file
  // BLE module must be woken up before sending any UART data
  //ble112.onBeforeTXCommand = onBeforeTXCommand;
  //ble112.onTXCommandComplete = onTXCommandComplete;

  // set up BGLib event handlers
  ble112.ble_evt_system_boot = my_ble_evt_system_boot;
  ble112.ble_evt_connection_status = my_ble_evt_connection_status;
  ble112.ble_evt_connection_disconnected = my_ble_evt_connection_disconnect;
  ble112.ble_evt_attributes_value = my_ble_evt_attributes_value;
  ble112.ble_evt_attclient_indicated = my_ble_evt_attclient_indicated;
  ble112.ble_evt_attributes_status = my_ble_evt_attributes_status;

  // open Arduino USB serial (and wait, if we're using Leonardo)
  // use 38400 since it works at 8MHz as well as 16MHz
  Serial.begin(38400);
  while (!Serial);

  // open BLE software serial port
  bleSerialPort.begin(38400);

  my_ble_evt_system_boot(NULL);
  
  // reset module (maybe not necessary for your application)
  digitalWrite(BLE_RESET_PIN, LOW);
  delay(5); // wait 5ms
  digitalWrite(BLE_RESET_PIN, HIGH);
  
  // Start with sampling rate of 50 hertz
  MsTimer2::set(20, notify); // 20 millisecond period -> 50 hertz frequency
  MsTimer2::start();

}

// ================================================================
// MAIN APPLICATION LOOP 
// ================================================================
void loop() {
  // keep polling for new data from BLE  
  ble112.checkActivity();
  Serial.println(millis());
  
  // For Z_Logger
  // =================================================== 

  AD5933.tempUpdate();
  AD5933.setCtrMode(REPEAT_FREQ);
  if( dataRead == false && AD5933.isValueReady() == true )
  {
    Z_value = gain_factor/AD5933.getMagOnce();
    dataRead = true;
    // to add codes for calculating complex components  
  }
  // For BLE
  // =================================================== 
  
  rComp = (long)((Z_value * 1000) + 0.5);
  /*
  // check for input from the user
  if (Serial.available()) {
    uint8_t ch = Serial.read();
    uint8_t status;
    if (ch == '0') {
      // Reset BLE112 module
      Serial.println("-->\tsystem_reset: { boot_in_dfu: 0 }");
      ble112.ble_cmd_system_reset(0);
      while ((status = ble112.checkActivity(1000)));
      // system_reset doesn't have a response, but this BGLib
      // implementation allows the system_boot event specially to
      // set the "busy" flag to false for this particular case
    }
  } */

  pseudo = pseudo + 1;
  if(pseudo >= 180) {
    pseudo = -180;
  }  
  // Check if GATT Client (Smartphone) is subscribed to notifications.
  if (writer == true) {  
    //Simple way of changinging frequency of notifications. see documentation on WuMRC Github or tunji.com/blog for more details on this.
    A[0] = (100 * sin((pseudo*3.14)/180));
    A[1] = (100 * cos((pseudo*3.14)/180));
    A[2] = (0);

      if(((Z_value - 554.72) > -3 && (Z_value - 554.72) < 3) || ((Z_value - 554.72) > 500)) {
        A[3] = 0;
        A[4] = 0;
        A[5] = 0;
      }
      else {
        changeVal(rComp, A);
      }      

      //Write notification to characteristic on ble112. Causes notification to be sent.
      ble112.ble_cmd_attributes_write(GATT_HANDLE_C_BIOIMPEDANCE_DATA, 0, 6 , A);
      dataRead = false; // To prevent duplicate data reading to take cycles.
      writer = false;     
  }   
  else {
    // Do zilch, zip, nada, nothing if notifications are not enabled.
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
  // reset module (might be a bit drastic for a timeout condition though)
  Serial.println(F("Timed out."));

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
    'Z', '_', 'B', 'L', 'E',' ', 'D', '3', 'm', 'o', ' ', '0', '0', ':', '0', '0', ':', '0', '0'
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

  // set state to ADVERTISING
  ble_state = BLE_STATE_ADVERTISING;
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

  // check for data written to "c_rx_data" handle
  if (msg -> handle == GATT_HANDLE_C_SAMPLE_RATE && msg -> value.len > 0) {
    sampleRate = (long) (1000 / (msg -> value.data[0])); 
    Serial.print("Sucessful write attempt; new frequency / period: ");
    Serial.print(msg -> value.data[0]);
    Serial.print(" hertz");
    Serial.print(" / ");         
    Serial.print(sampleRate);
    Serial.print(" milliseconds.");   
   
   // Starting to change the settings of AD5933
    MsTimer2::stop();
    AD5933.resetAD5933();
    int cycleBase = (int)(477.84 * pow(2.718,-0.017*sampleRate) );
    
    AD5933.setSettlingCycles(cycleBase, cycles_multiplier);
    AD5933.tempUpdate();
    AD5933.setCtrMode(INIT_START_FREQ);
    AD5933.setCtrMode(START_FREQ_SWEEP);
    //gain_factor = AD5933.getGainFactor(cal_resistance, cal_samples, false);
    //AD5933.setCtrMode(REPEAT_FREQ);
    
    // End changing process.
    
    MsTimer2::set(sampleRate, notify);
    MsTimer2::start();
    Serial.println();
  }
  // check for data written to "c_ac_freq" handle  
  if (msg -> handle == GATT_HANDLE_C_AC_FREQ && msg -> value.len > 0) {
    upperFreq = msg -> value.data[0];
    stepSize = msg -> value.data[1];  
    lowerFreq = msg -> value.data[2];  
    Serial.print("Sucessful write attempt to c_ac_freq.");
    Serial.println();      
    Serial.print("Upper Frequency (KHz): ");  
    Serial.print(upperFreq);
    Serial.println();    
    Serial.print("Step size (KHz): ");    
    Serial.print(stepSize);
    Serial.println();    
    Serial.print("Lower Frequency (KHz): ");    
    Serial.print(lowerFreq);  
    Serial.println();    
    //TODO: add initialization code for AD5933
  }  
}
void my_ble_evt_attclient_indicated(const struct ble_msg_attclient_indicated_evt_t *msg) {
#ifdef DEBUG
  Serial.print("###\tattclient_indicate: { ");
  Serial.print("Indication received.");
  Serial.println(" }");
#endif
}

void my_ble_evt_attributes_status (const struct ble_msg_attributes_status_evt_t *msg) {
#ifdef DEBUG
  Serial.print("###\tattributes_status: { ");
  Serial.print("nSubscription changed");
  Serial.print(", flags: "); 
  Serial.print(msg -> flags, HEX);

  Serial.println(" }");
#endif

  if (msg -> flags == 1) {
    notifier = true;
  }        
  else if (msg -> flags == 0){
    notifier = false;
  }
  else {
    notifier = false;          
  }  
}

void notify() {
  if (notifier == true) {
    writer = true; 
  }  
}


void changeVal(long value, uint8_t *fVals)
{
  fVals[5] = value % 100;
  value /= 100;
  fVals[4] = value % 100;
  value /= 100;
  fVals[3] = value % 100;
}

/*
void changeVal(long val, uint8_t *values) {
  values[3] = ((getNthDigit(val, 10, 6) * 10) + getNthDigit(val, 10, 5));
  values[4] = ((getNthDigit(val, 10, 4) * 10) + getNthDigit(val, 10, 3));
  values[5] = ((getNthDigit(val, 10, 2) * 10) + getNthDigit(val, 10, 1));
}

long getNthDigit(long number, int base, int n) {
  long answer = 0;
  answer = (long) (number / pow(base, n - 1));
  answer = answer % base;
  return answer;
}

*/


