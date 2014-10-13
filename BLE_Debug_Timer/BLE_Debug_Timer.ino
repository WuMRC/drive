//Accelerometer demo sketch for TinyDuino
//Based onBluegiga BGLib Arduino interface library slave device stub sketch
//and accelerometer demo from TinyCitcuits 
//2013-06-30 by Ken Burns, TinyCircuits http://Tiny-Circuits.com
//2014-02-12 by Jeff Rowberg <jeff@rowberg.net>
//2014-07-13 modified by Adetunji Dahunsi <tunjid.com>
//Updates should (hopefully) always be available at https://github.com/jrowberg/bglib
//

//Changelog:
//   2014-06-13 - Initial release

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

//#include <SoftwareSerial.h> // software serial library for input and output to the serial mnitor.
#include <Wire.h>
#include <Math.h>
#include "BGLib.h" // BGLib C library for BGAPI communication.
#include <AltSoftSerial.h> // software serial library for input and output to the serial mnitor.
#include <MsTimer2.h> // timer function for notifications
#include "AD5933.h"

//uncomment the following line for debug serial output
#define DEBUG

volatile double x = 0;
volatile boolean writer = false;
boolean notifier = false; // variable to manage notification settings
uint8_t A[6] = {
  1, 2, 3, 42, 15, 66}; // integer array to carry accelerometer values
long sampleRate = 0; // Android app sample Rate
uint8_t upperFreq = 0; // Upper value for frequency sweep, or single value of frequency
// when frequency sweep is off.
uint8_t stepSize = 0;  // frequency step size between upp and lower values.
uint8_t lowerFreq = 0;  // Lower value of frequency sweep.


//================================================================
//BLE STATE TRACKING (UNIVERSAL TO JUST ABOUT ANY BLE PROJECT)
//================================================================

//BLE state machine definitions
#define BLE_STATE_STANDBY           0
#define BLE_STATE_SCANNING          1
#define BLE_STATE_ADVERTISING       2
#define BLE_STATE_CONNECTING        3
#define BLE_STATE_CONNECTED_MASTER  4
#define BLE_STATE_CONNECTED_SLAVE   5

//BLE state/link status tracker
uint8_t ble_state = BLE_STATE_STANDBY;
uint8_t ble_encrypted = 0;  // 0 = not encrypted, otherwise = encrypted
uint8_t ble_bonding = 0xFF; // 0xFF = no bonding, otherwise = bonding handle

//================================================================
//HARDWARE CONNECTIONS AND GATT STRUCTURE SETUP
//================================================================

//NOTE: this assumes you are using one of the following firmwares:
//- BGLib_U1A1P_38400_noflow
//- BGLib_U1A1P_38400_noflow_wake16
//- BGLib_U1A1P_38400_noflow_wake16_hwake15
//If not, then you may need to change the pin assignments and/or
//GATT handles to match your firmware.

#define LED_PIN         13  // Arduino Uno LED pin

#define GATT_HANDLE_C_BIOIMPEDANCE_DATA   17  // 0x11, supports "read", "notify" and "indicate" operations
#define GATT_HANDLE_C_SAMPLE_RATE   21  // 0x15, supports "write" operation
#define GATT_HANDLE_C_AC_FREQ   24  // 0x18, supports "write" operation

//#define BLE_WAKEUP_PIN 5 // BLE Wake up pin

//use SoftwareSerial on pins D3/D4 for RX/TX (Arduino side)
AltSoftSerial bleSerialPort(8, 9);

//create BGLib object:
//- use SoftwareSerial por for module comms
//- use nothing for passthrough comms (0 = null pointer)
//- enable packet mode on API protocol since flow control is unavailable

BGLib ble112((HardwareSerial *)&bleSerialPort, 0, 1);

#define BGAPI_GET_RESPONSE(v, dType) dType *v = (dType *)ble112.getLastRXPayload()

// AD5933 Specific Declaration Starts
#define TWI_FREQ 400000L // Setting TWI/I2C Frequency to 400MHz to speed up.

#define cycles_base 300      //First term to set a number of cycles to ignore
                             //to dissipate transients before a measurement is
                             //taken. The max value for this is 511.


#define cycles_multiplier 1  //Set a multiple for the cycles_base which
                             //is used to calculate the desired number
                             //of settling cycles. Values can be 1, 2, or 4.
                             

#define start_frequency 50000 //Set the start frequency, the only one of
                              //interest here(50 kHz).

#define cal_resistance 553.91 //Set a calibration resistance for the gain
                            //factor. This will have to be measured before any
                            //other measurements are performed.
                           
#define cal_samples 10 //Set a number of measurements to take of the calibration
                       //resistance. These are used to get an average gain
                       //factor.
double gain_factor = 0, Z_Value = 99999;
// AD5933 Specific Declaration Ends

//================================================================
//ARDUINO APPLICATION SETUP AND LOOP FUNCTIONS
//================================================================

//initialization sequence
void setup() {

  //Initialize accelerometer
  Wire.begin();
  Serial.begin(38400);

  // initialize status LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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

  // Initialize AD5933
  AD5933.setExtClock(false);
  AD5933.resetAD5933();
  AD5933.setStartFreq(start_frequency);
  AD5933.setSettlingCycles(cycles_base, cycles_multiplier);
  AD5933.setIncrementinHex(1);
  AD5933.setNumofIncrement(2);
  AD5933.setVolPGA(0,1);
  AD5933.tempUpdate();  
  gain_factor = AD5933.getGainFactor(cal_resistance, cal_samples, false);
  // End of Initialization of AD5933

  // open BLE software serial port
  bleSerialPort.begin(38400);

  my_ble_evt_system_boot( NULL);

  MsTimer2::set(20, notify);
  MsTimer2::start();

}

//================================================================
//MAIN APPLICATION LOOP 
//================================================================
void loop() {  
  // keep polling for new data from BLE
  ble112.checkActivity();

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
  }   
  // Check if GATT Client (Smartphone) is subscribed to notifications.

  // Put Z value here. Place holder pseudo signal.
  x = x + 0.02;
  if(x >= 180) {
    x = -180;
  }

  // Add for AD5933
  if( AD5933::isValueReady() == true )
  {
  	Z_value = gain_factor/AD5933.getMagOnce();
  }
  // End of AD5933 Part

  if (writer == true) { 
    //Write notification to characteristic on ble112. Causes notification to be sent.   
    A[0] = (100 * sin((x*3.14)/180));
    A[1] = (100 * cos((x*3.14)/180));
    A[2] = (0);
    //A[3] = fabs((x * 0.66));
    ble112.ble_cmd_attributes_write(GATT_HANDLE_C_BIOIMPEDANCE_DATA, 0, 6 , A);
    //Serial.println("Attributes written.");
    writer = false;
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

//================================================================
//INTERNAL BGLIB CLASS CALLBACK FUNCTIONS
//================================================================

//called when the module begins sending a command
void onBusy() {
  // turn LED on when we're busy
  //digitalWrite(LED_PIN, HIGH);

}

//called when the module receives a complete response or "system_boot" event
void onIdle() {
  // turn LED off when we're no longer busy
  //    digitalWrite(LED_PIN, LOW);
}

//called when the parser does not read the expected response in the specified time limit
void onTimeout() {
  // reset module (might be a bit drastic for a timeout condition though)
  Serial.println("Timed out.");
  ble112.ble_cmd_system_reset(0);

}

//================================================================
//APPLICATION EVENT HANDLER FUNCTIONS
//================================================================

void notify() {
  if (notifier == true) {
    writer = true; 
  }  
}

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
    'C', 'h', 'i', 'p', ' ', 'D', 'e', 'b', 'u', 'g',' ', '0', '0', ':', '0', '0', ':', '0', '0'
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

  // reset frequency to default
  MsTimer2::set(20, notify);
  MsTimer2::start();
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

  // check for data written to "c_sample_rate" handle
  if (msg -> handle == GATT_HANDLE_C_SAMPLE_RATE && msg -> value.len > 0) {
    // set ping 8, 9, and 10 to three lower-most bits of first byte of RX data
    // (nice for controlling RGB LED or something)
    //digitalWrite(8, msg -> value.data[0] & 0x01);
    //digitalWrite(9, msg -> value.data[0] & 0x02);
    //digitalWrite(10, msg -> value.data[0] & 0x04);
    sampleRate = (long) (1000 / (msg -> value.data[0])); 
    Serial.print("Sucessful write attempt; new frequency / period: ");
    Serial.print(msg -> value.data[0]);
    Serial.print(" hertz");
    Serial.print(" / ");         
    Serial.print(sampleRate);
    Serial.print(" milliseconds.");    
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
  else if (msg -> flags == 2) {
    notifier = true;
  }
  else {
    notifier = false;
  }
}  


