'''
Created on Feb 22, 2015
@author: Adetunji Dahunsi


'''
from time import sleep
import usbtmc
import serial

lcr = usbtmc.Instrument(2391, 2313)

Arduino = serial.Serial('/dev/tty.usbmodem1421', 38400, timeout=10)


for n in range(0, 1):
    for C in range(0, 4):
        # Wait for user input (switch capacitors)
        for R2 in range(13, 63):
            # Instruct Arduino to change rheostat 2 wiper
            Arduino.write(b'2')
            for R1 in range(13, 63):
                # Instruct Arduino to change rheostat 1 wiper
                Arduino.write(b'1')
                sleep(0.1)
                for f in range(0, 99):
                    lcr.write(":TRIG:IMM")
                    lcr.last_btag = 0
                    sleep(0.01)
                    data = lcr.ask("FETC:IMP:FORM?")
                    sleep(0.01)
                    print(str(R1) + "," + str(f) + "," + data)
Arduino.write(b'0')
    
   
