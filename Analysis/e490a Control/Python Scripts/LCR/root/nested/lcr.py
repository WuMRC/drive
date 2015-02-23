'''
Created on Feb 22, 2015

@author: Shemanigans
'''
import usbtmc

# Declare USBTMC instrument LCR
lcr = usbtmc.Instrument(2391, 2313)

lcr.write(":TRIG:IMM")
print(lcr.ask("FETC:IMP:FORM?"))
lcr.write(":TRIG:IMM")
print(lcr.ask("FETC:IMP:FORM?"))
lcr.write(":TRIG:IMM")
print(lcr.ask("FETC:IMP:FORM?"))