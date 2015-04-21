'''
Created on Feb 22, 2015

@author: Shemanigans
'''
import serial

ser = serial.Serial('/dev/tty.usbmodem1421', 38400, timeout=10)

ser.write(b'1')

c = ser.read(1)

print(c)
print('done')
