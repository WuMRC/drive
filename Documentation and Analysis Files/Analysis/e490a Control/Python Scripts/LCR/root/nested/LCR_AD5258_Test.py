'''
Created on Feb 22, 2015
@author: Adetunji Dahunsi


'''
from time import sleep
import usbtmc
import serial

lcr = usbtmc.Instrument(2391, 2313)

Arduino = serial.Serial('/dev/tty.usbmodem1421', 38400, timeout=10)


for i in range(0, 4):
    for R1 in range(0, 64):
        # Instruct Arduino to change rheostat wiper
        Arduino.write(b'1')
        sleep(0.1)
        for f in range(0, 99):
            lcr.write(":TRIG:IMM")
            lcr.last_btag = 0
            sleep(0.01)
            data = lcr.ask("FETC:IMP:FORM?")
            sleep(0.01)
            print(str(R1) + "," + str(f) + "," + data)
       
   
