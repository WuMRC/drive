# Import USBTMC class
import usbtmc
# Set lcr to class instance {Intrument(VendorID, ProductID)}
lcr =  usbtmc.Instrument(2391, 2313)
# Print identification to make sure device is connected
print(lcr.ask("*IDN?"))
# Trigger LCR
lcr.write(":TRIG:IMM")
# Read data from LCR
print(lcr.ask("FETC:IMP:FORM?"))