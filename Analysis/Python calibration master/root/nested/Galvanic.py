'''
Created on Feb 22, 2015
@author: Adetunji Dahunsi


'''
from time import sleep
import serial


Arduino = serial.Serial('/dev/tty.usbmodem1421', 38400, timeout=10)
PCB = serial.Serial('/dev/tty.usbserial-AI02B04W', 38400, timeout=10)



for R1 in range(14, 64):
    # Instruct Arduino to change rheostat 1 wiper
    Arduino.write(b'1')
    sleep(0.1)
    PCB.write(b'1')
    sleep(0.1)
    print(str(R1))
Arduino.write(b'0')

    
   
