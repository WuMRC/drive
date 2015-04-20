// Library Code Section of AD5933

// Author: Il-Taek Kwon

// Modifications: Adetunji Dahunsi

#include "AD5933.h"
//#include <WProgram.h>
#include <Arduino.h> // For the compatibility with Arduino Conventions.
//#include <WConstants.h>

//extern HardwareSerial Serial;
//AD5933_Class AD5933(100, Serial);
AD5933_Class AD5933;

bool AD5933_Class::performFreqSweep(double gainFactor, double *arrSave)
// Function to perform frequency Sweep, Just call it once to do it. It automatically do all the step.
// double gainFactor - You need to call getGainFactor(double,int)
//
// double *arrSave - Just put the name of the array to save it. It should have right number of entries to save it.
// If not, hidden error will be occur.
{
	int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg
	if (setCtrMode(STAND_BY, ctrReg) == false)
	{
#if LOGGING1
		printer->println("performFreqSweep - Failed to setting Stand By Status!");
#endif
		return false;
	}
	if (setCtrMode(INIT_START_FREQ, ctrReg) == false)
	{
#if LOGGING1
		printer->println("performFreqSweep - Failed to setting initialization with starting frequency!");
#endif
		return false;
	}
	//delay(delayTimeInit);
	if (setCtrMode(START_FREQ_SWEEP, ctrReg) == false)
	{
#if LOGGING1
		printer->println("performFreqSweep - Failed to set to start frequency sweeping!");
#endif
		return false;
	}

	int t1 = 0;
	while ( (getStatusReg() & 0x04) != 0x04 ) // Loop while if the entire sweep in not complete
	{
		//delay(delayTimeInit);
		arrSave[t1] = gainFactor / getMagOnce(); // Calculated with Gain Factor
#if LOGGING1
		printer->print("performFreqSweep - arrSave[");
		printer->print(t1);
		printer->print("] = ");
		printer->println(arrSave[t1]);
#endif
		if (setCtrMode(INCR_FREQ, ctrReg) == false)
		{
#if LOGGING1
			printer->println("performFreqSweep - Failed to set for increasing frequency!");
#endif
			return false;
		}
		t1++;
		//getByte(0x80);
	}
	if (setCtrMode(POWER_DOWN, ctrReg) == false)
	{
#if LOGGING1
		printer->println("performFreqSweep - Completed sweep, but failed to power down");
#endif
		return false;
	}
	return true; // Succeed!
}


bool AD5933_Class::compFreqRawSweep(int *arrReal, int *arrImag)
{
	int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg
	if (setCtrMode(STAND_BY, ctrReg) == false)
	{
		return false;
	}
	if (setCtrMode(INIT_START_FREQ, ctrReg) == false)
	{
		return false;
	}
	if (setCtrMode(START_FREQ_SWEEP, ctrReg) == false)
	{
		return false;
	}

	int t1 = 0;
	while ( (getStatusReg() & 0x04) != 0x04) // Loop while if the entire sweep in not complete
	{
		getComplexRawOnce(arrReal[t1], arrImag[t1]); // Only for real and Imag components.
		if (setCtrMode(INCR_FREQ, ctrReg) == false)
		{
			return false;
		}
		t1++;
	}
	if (setCtrMode(POWER_DOWN, ctrReg) == false)
	{
		return false;
	}
	return true; // Succeed!
}

bool AD5933_Class::compFreqSweep(double *arrGainFactor, double *arrPShift, double *arrReal, double *arrImag)
{
	int ctrReg = getByte(0x80);
	setCtrMode(STAND_BY, ctrReg);
	setCtrMode(INIT_START_FREQ, ctrReg);
	setCtrMode(START_FREQ_SWEEP, ctrReg);

	int t1 = 0;
	//int cReal, cImag;
	double Z_Val, phase;
	while ( (getStatusReg() & 0x04) != 0x04) // Loop while if the entire sweep in not complete
	{
		/*
    getComplexRawOnce(cReal, cImag); // Only for real and Imag components.
    double magImp = arrGainFactor[t1]/getMag(cReal, cImag);
    double phase = atan2(cImag, cReal) - arrPShift[t1];
    arrReal[t1] = magImp * cos(phase);
    arrImag[t1] = magImp * sin(phase);
		 */
		getComplexOnce(arrGainFactor[t1], arrPShift[t1], arrReal[t1], arrImag[t1], Z_Val, phase);

		if (setCtrMode(INCR_FREQ, ctrReg) == false)
		{
			return false;
		}
		t1++;
	}
	setCtrMode(POWER_DOWN, ctrReg);
	return true;


}

bool AD5933_Class::compCbrArray(double GF_Init, double GF_Incr, double PS_Init, double PS_Incr, double *arrGainFactor, double *arrPShift)
{
	int t1 = 0;
	arrGainFactor[0] = GF_Init;
	arrPShift[0] = PS_Init;
	for (t1 = 1; t1 < numIncrement; t1++)
	{
		arrGainFactor[t1] = arrGainFactor[t1 - 1] + GF_Incr;
		arrPShift[t1] = arrPShift[t1 - 1] + PS_Incr;
	}
	return true;
}

bool AD5933_Class::getArraysLI(
		double &deltaGF, double &deltaPS,
		double &stepSize, uint8_t &numOfIncrements,
		double &GF_Initial, double &PS_Initial,
		double *arrGainFactor, double *arrPShift)
{
	int t1 = 0;

	for (t1 = 1; t1 <= numOfIncrements; t1++)
	{
		arrGainFactor[t1] = (deltaGF * t1 / numOfIncrements) + GF_Initial;
		arrPShift[t1] = (deltaPS * t1 / numOfIncrements) + PS_Initial;
	}
	return true;
}

bool AD5933_Class::getGainFactors_LI(double cResistance, int avgNum, double startFreq, double endFreq, double &GF_Initial, double &GF_Final, double &PS_Initial, double &PS_Final)
{
	int ctrReg = getByte(0x80);
	byte numIncrementBackup = numIncrement;
	long incrHexBackup = incrHex;
	setNumofIncrement(1);
	setIncrement(endFreq - startFreq);
	if (setCtrMode(STAND_BY, ctrReg) == false) {
		return false;
	}
	if (setCtrMode(INIT_START_FREQ, ctrReg) == false) {
		return false;
	}
	if (setCtrMode(START_FREQ_SWEEP, ctrReg) == false) {
		return false;
	}

	int t1 = 0;
	int rImag, rReal;
	double sumMag = 0, sumPhase = 0;
	for (t1 = 0; t1 < avgNum; t1++)
	{
		getComplexRawOnce(rReal, rImag); // Only for real and Imag components.
		sumMag += getMag(rReal, rImag);
		sumPhase += atan2(rImag, rReal);

		if (setCtrMode(REPEAT_FREQ, ctrReg) == false)
		{
			return false;
		}
	}
	GF_Initial = (sumMag / avgNum) * cResistance;
	PS_Initial = sumPhase / avgNum;

	setCtrMode(INCR_FREQ, ctrReg);

	sumMag = 0;
	sumPhase = 0;

	for (t1 = 0; t1 < avgNum; t1++)
	{
		getComplexRawOnce(rReal, rImag); // Only for real and Imag components.
		sumMag += getMag(rReal, rImag);
		sumPhase += atan2(rImag, rReal);

		if (setCtrMode(REPEAT_FREQ, ctrReg) == false)
		{
			return false;
		}
	}
	GF_Final = (sumMag / avgNum) * cResistance;
	PS_Final = (sumPhase / avgNum);


	if (setCtrMode(POWER_DOWN, ctrReg) == false)
	{
		return false;
	}
	setNumofIncrement(numIncrementBackup);
	setIncrementinHex(incrHexBackup);
	return true; // Succeed!
}


bool AD5933_Class::getGainFactorS_TP(double cResistance, int avgNum, double startFreq, double endFreq, double &GF_Init, double &GF_Incr, double &PS_Init, double &PS_Incr)
{
	int ctrReg = getByte(0x80);
	byte numIncrementBackup = numIncrement;
	long incrHexBackup = incrHex;
	setNumofIncrement(1);
	setIncrement(endFreq - startFreq);
	if (setCtrMode(STAND_BY, ctrReg) == false)
	{
		return false;
	}
	if (setCtrMode(INIT_START_FREQ, ctrReg) == false)
	{
		return false;
	}
	if (setCtrMode(START_FREQ_SWEEP, ctrReg) == false)
	{
		return false;
	}

	int t1 = 0;
	int rImag, rReal;
	double sumMag = 0, sumPhase = 0;
	for (t1 = 0; t1 < avgNum; t1++)
	{
		getComplexRawOnce(rReal, rImag); // Only for real and Imag components.
		sumMag += getMag(rReal, rImag);
		sumPhase += atan2(rImag, rReal);

		if (setCtrMode(REPEAT_FREQ, ctrReg) == false)
		{
			return false;
		}
	}
	GF_Init = (sumMag / avgNum) * cResistance;
	PS_Init = sumPhase / avgNum;

	setCtrMode(INCR_FREQ, ctrReg);
	sumMag = 0;
	sumPhase = 0;

	for (t1 = 0; t1 < avgNum; t1++)
	{
		getComplexRawOnce(rReal, rImag); // Only for real and Imag components.
		sumMag += getMag(rReal, rImag);
		sumPhase += atan2(rImag, rReal);

		if (setCtrMode(REPEAT_FREQ, ctrReg) == false)
		{
			return false;
		}
	}
	GF_Incr = ((sumMag / avgNum) * cResistance - GF_Init) / numIncrementBackup;
	PS_Incr = ((sumPhase / avgNum) - PS_Init) / numIncrementBackup;


	if (setCtrMode(POWER_DOWN, ctrReg) == false)
	{
		return false;
	}
	setNumofIncrement(numIncrementBackup);
	setIncrementinHex(incrHexBackup);
	return true; // Succeed!
}

bool AD5933_Class::getGainFactorS_Set(double cResistance, int avgNum, double *gainFactorArr, double *pShiftArr)
{
	int ctrReg = getByte(0x80);
	if (setCtrMode(STAND_BY, ctrReg) == false)
	{
		return false;
	}
	if (setCtrMode(INIT_START_FREQ, ctrReg) == false)
	{
		return false;
	}
	if (setCtrMode(START_FREQ_SWEEP, ctrReg) == false)
	{
		return false;
	}

	int t1 = 0;
	int rImag, rReal;
	double mag;
	while ( (getStatusReg() & 0x04) != 0x04) // Loop while if the entire sweep in not complete
	{
		double tSumMag = 0, tSumPhase = 0;
		for (int t2 = 0; t2 < avgNum; t2++)
		{
			getComplexRawOnce(rReal, rImag); // Only for real and Imag components.
			tSumMag += getMag(rReal, rImag);
			tSumPhase += atan2(rImag, rReal);
			if (setCtrMode(REPEAT_FREQ, ctrReg) == false)
			{
				return false;
			}
		}
		//Serial.println(t1);
		gainFactorArr[t1] = (tSumMag / avgNum) * cResistance;
		pShiftArr[t1] = tSumPhase / avgNum;

		if (setCtrMode(INCR_FREQ, ctrReg) == false)
		{
			return false;
		}
		t1++;
	}
	if (setCtrMode(POWER_DOWN, ctrReg) == false)
	{
		return false;
	}
	return true; // Succeed!
}

bool AD5933_Class::getGainFactorC(double cResistance, int avgNum, double &gainFactor, double &pShift)
{
	return getGainFactorC(cResistance, avgNum, gainFactor, pShift, true);
}


bool AD5933_Class::getGainFactorC(double cResistance, int avgNum, double &gainFactor, double &pShift, bool retStandBy)
{
	int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg
	if (setCtrMode(STAND_BY, ctrReg) == false)
	{
		return false;
	}
	if (setCtrMode(INIT_START_FREQ, ctrReg) == false)
	{
		return false;
	}
	//delay(delayTimeInit);
	if (setCtrMode(START_FREQ_SWEEP, ctrReg) == false)
	{
		return false;
	}

	int t1 = 0, rImag, rReal;
	double tSum = 0, tSumP = 0;
	while (t1 < avgNum) // Until reached pre-defined number for averaging.
	{
		getComplexRawOnce(rReal, rImag);
		tSum += getMag(rReal, rImag);
		tSumP += atan2(rImag, rReal);
		if (setCtrMode(REPEAT_FREQ, ctrReg) == false)
		{
			return false;
		}
		t1++;
	}
	double mag = tSum / (double)avgNum;
	pShift = tSumP / (double)avgNum;

	if (retStandBy == false)
	{
		gainFactor = mag * cResistance;
		return true;
	}

	if ( setCtrMode(STAND_BY, ctrReg) == false)
	{
		return false;
	}
	resetAD5933();
	// Gain Factor is different from one of the datasheet in this program. Reciprocal Value.
	gainFactor = mag * cResistance;
	return true;
}

bool AD5933_Class::getGainFactorTetra(double calResistance, int avgNum, double &gainFactor, double &vPShift, double &iPShift) {
	return getGainFactorTetra(calResistance, avgNum, gainFactor, vPShift, iPShift, true);
}

bool AD5933_Class::getGainFactorTetra(double calResistance, int avgNum, double &gainFactor, double &vPShift, double &iPShift, bool retStandBy){

	int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg

	if (setCtrMode(STAND_BY, ctrReg) == false){
		return false;
	}

	if (setCtrMode(INIT_START_FREQ, ctrReg) == false){
		return false;
	}

	if (setCtrMode(START_FREQ_SWEEP, ctrReg) == false){
		return false;
	}

	int index = 0, iVoltage = 0, rVoltage = 0, iCurrent = 0, rCurrent = 0;
	double voltageSum = 0, currentSum = 0, voltagePhaseSum = 0, currentPhaseSum = 0;

	delay(100);
	digitalWrite(IV_MUX, LOW); //Toggle current / voltage multiplexer to voltage
	delay(100);

	if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
		return false;
	}
	// Until reached pre-defined number for averaging.
	while (index < avgNum) {

		getComplexRawOnce(rVoltage, iVoltage);

		voltageSum += getMag(rVoltage, iVoltage);
		voltagePhaseSum += atan2(iVoltage, rVoltage);

		if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
			return false;
		}

		index++;
	}

	delay(100);
	digitalWrite(IV_MUX, HIGH); // Toggle current / voltage multiplexer to current
	delay(100); // delay for mux to settle

	index = 0; // Reset index for current

	if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
		return false;
	}

	while (index < avgNum) {

		getComplexRawOnce(rCurrent, iCurrent);

		currentSum += getMag(rCurrent, iCurrent);
		currentPhaseSum += atan2(iCurrent, rCurrent);

		if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
			return false;
		}

		index++;
	}

	double voltageMag = voltageSum / ((double) avgNum);
	double currentMag = currentSum / ((double) avgNum);

	vPShift = voltagePhaseSum / ((double) avgNum);
	iPShift = currentPhaseSum / ((double) avgNum);

	if (retStandBy == false){
		gainFactor = calResistance * (currentMag / voltageMag);
		return true;
	}

	if ( setCtrMode(STAND_BY, ctrReg) == false){
		return false;
	}

	resetAD5933();

	gainFactor = calResistance * (currentMag / voltageMag);

	return true;
}


double AD5933_Class::getGainFactor(double cResistance, int avgNum, bool retStandBy)
// A function to get Gain Factor. It performs one impedance measurement in start frequency.
// double cResistance - Calibration Resistor Value
// avgNum - number of measurement for averaging.
// Returns -1 if error occurs.
{
	int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg
	if (setCtrMode(STAND_BY, ctrReg) == false)
	{
#if LOGGING1
		printer->println("getGainFactor - Failed to setting Stand By Status!");
#endif
		return -1;
	}
	if (setCtrMode(INIT_START_FREQ, ctrReg) == false)
	{
#if LOGGING1
		printer->println("getGainFactor  - Failed to setting initialization with starting frequency!");
#endif
		return -1;
	}
	//delay(delayTimeInit);
	if (setCtrMode(START_FREQ_SWEEP, ctrReg) == false)
	{
#if LOGGING1
		printer->println("getGainFactor - Failed to set to start frequency sweeping!");
#endif
		return -1;
	}

	int t1 = 0;
	double tSum = 0;
	while (t1 < avgNum) // Until reached pre-defined number for averaging.
	{
		tSum += getMagOnce();
		if (setCtrMode(REPEAT_FREQ, ctrReg) == false)
		{
#if LOGGING1
			printer->println("getGainFactor - Failed to set to repeat this frequency!");
#endif
			return -1;
		}
		t1++;
	}
	double mag = tSum / (double)avgNum;
#if LOGGING2
	printer->print("getGainFactor - Gain Factor: ");
	printer->println(mag * cResistance);

#endif

	if (retStandBy == false)
	{
#if LOGGING3
		printer->println("getGainFactor - terminate the function without going into Stand By");
#endif
		return mag * cResistance;
	}

	if ( setCtrMode(STAND_BY, ctrReg) == false)
	{
#if LOGGING1
		printer->println("getGainFactor - Failed to set into Stand-By Status");
#endif
		return -1;
	}
	resetAD5933();
	// Gain Factor is different from one of the datasheet in this program. Reciprocal Value.
	return mag * cResistance;

}

double AD5933_Class::getGainFactor(double cResistance, int avgNum)
// Calculate Gain Factor with measuring and averaging multiple times.
{
	return getGainFactor(cResistance, avgNum, true);
}

double AD5933_Class::getGainFactor(double cResistance)
// Calculate Gain Factor with measuring once.
{
	return getGainFactor(cResistance, 1, true);
}

bool AD5933_Class::setCtrMode(byte modetoSet)
// setting Control Register to change control mode without assuming control register. (0x80)
{
	return setCtrMode(modetoSet, getByte(0x80));
}

bool AD5933_Class::setCtrMode(byte modetoSet, int ctrReg)
// setting Control Register to change control mode.
//
{
	ctrReg &= 0x0F; // Get the last 4 digits.
	switch (modetoSet)
	{
	case INIT_START_FREQ:
		ctrReg |= 0x10;
		break;
	case START_FREQ_SWEEP:
		ctrReg |= 0x20;
		break;
	case INCR_FREQ:
		ctrReg |= 0x30;
		break;
	case REPEAT_FREQ:
		ctrReg |= 0x40;
		break;
	case POWER_DOWN:
		ctrReg |= 0xA0;
		break;
	case STAND_BY:
		ctrReg |= 0xB0;
		break;
	case TEMP_MEASURE:
		ctrReg |= 0x90;
		break;
	default:
#if LOGGING1
		printer->println("setCtrMode - Invalid Parameter!");
#endif
		return false; // return the signal of fail if there is not valid parameter.
		break;
	}
	return setByte(0x80, ctrReg); // return signal depends on the result of setting control register.

}

bool AD5933_Class::setRange(byte rangeToSet)
// setting Control Register to change control mode without assuming control register. (0x80)
{
	return setByte(rangeToSet, getByte(0x80));
}

bool AD5933_Class::setRange(byte rangeToSet, int ctrReg)
// setting Control Register to change control mode.
//
{

	ctrReg &= 0xF9; // Get D9 & D10.

	switch (rangeToSet) {

	case RANGE_1:
		ctrReg |= 0x00;
		//Serial.println("Changed to RANGE_1");
		break;

	case RANGE_2:
		ctrReg |= 0x06;
		//Serial.println("Changed to RANGE_2");
		break;

	case RANGE_3:
		ctrReg |= 0x04;
		//Serial.println("Changed to RANGE_3");
		break;

	case RANGE_4:
		ctrReg |= 0x02;
		//Serial.println("Changed to RANGE_4");
		break;
	default:
#if LOGGING1
		printer->println("setRange - Invalid Parameter!");
#endif
		return false; // return the signal of fail if there is not valid parameter.
		break;
	}
	return setByte(0x80, ctrReg); // return signal depends on the result of setting control register.

}

bool AD5933_Class::setPGA(byte pgaGain)
// setting Control Register to change control mode without assuming control register. (0x80)
{
	return setByte(pgaGain, getByte(0x80));
}

bool AD5933_Class::setPGA(byte pgaGain, int ctrReg) {

	ctrReg &= 0xFE; // Get D8.

	switch (pgaGain) {

	case GAIN_1:
		ctrReg |= 0x00;
		Serial.println("Changed to GAIN_1");
		break;

	case GAIN_5:
		ctrReg |= 0x01;
		Serial.println("Changed to GAIN_5");
		break;

	default:
#if LOGGING1
		printer->println("setRange - Invalid Parameter!");
#endif
		return false; // return the signal of fail if there is not valid parameter.
		break;
	}
	return setByte(0x80, ctrReg); // return signal depends on the result of setting control register.

}

bool AD5933_Class::setExtClock(bool swt)
// A function to enable/disable external clock.
// This function also sets environmental variable. (double opClock)
{
	byte t1;
	if ( swt )
	{
		t1 = 0x04; // Use Ext. Clock
		opClock = 16000000;
	}
	else
	{
		t1 = 0x00; // Use Int. Clock
		opClock = 16776000;
	}
	return setByte(0x81, t1); // Write register 0x81.
}

bool AD5933_Class::resetAD5933()
// Set Reset Bit(D4) in control register 0x81.
{
	int temp = (getByte(0x81) & 0x04); // Read 0x81 with retrieving D3
	return setByte(0x81, (temp | 0x10)); // Set D4 as 1 (Reset Bit)
}

bool AD5933_Class::setSettlingCycles(int cycles, byte mult)
{
	if (cycles > 0x1FF || !(mult == 1 || mult == 2 || mult == 4) )
	{
#if LOGGING1
		printer->println("setSettlingCycles - Invalid Parameter");
#endif
		return false;
	}
	int lowerHex = cycles % 256;
	int upperHex = ((cycles - (long)lowerHex) >> 8) % 2;
	byte t1; // Parsing upper and lower bits.
	/*switch(mult)
    {
      case 1:
        t1 = 0;
        break;
      case 2:
        t1 = 1;
        break;
      case 4:
        t1 = 3;
        break;
      default:
  #if LOGGING1
      printer->println("setSettlingCycles - Invalid Mult Parameter");
  #endif
      return false;
      break;
    }*/
	t1--; // Enhanced Code for setting t1.
	upperHex |= (t1 << 1);  // t1 is for D9, D10. The upperHex just accounts for D8. Thus, the value after left-shifting t1 accounts for D9, D10.
	// Thus, this above writes bits for D9, D10.
#if LOGGING2
	printer->print("setSettlingCycles - upper: ");
	printer->println(upperHex, BIN);
#endif
	bool t2, t3;
	t2 = setByte(0x8A, upperHex);
	t3 = setByte(0x8B, lowerHex);
	if ( t2 && t3 ) // Checking if successful.
		return true;  // Succeed!
	else
	{
#if LOGGING1
		printer->println("setSettingCycles - Data Write Fail");
#endif
		return false;
	}
}

bool AD5933_Class::setNumofIncrement(byte num)
// Function to set the number of incrementing.
// byte num - the number of incrementing.
{
	if (num > 0x1FF + 1)
	{
#if LOGGING1
		printer->print("setNumofIncrement - Frequency Overflow!");
#endif
		return false;
	}

	int lowerHex = num % 256;
	int upperHex = (num >> 8) % 2; // Parsing number for register input.

	bool t2, t4;
	t2 = setByte(0x88, upperHex);
	t4 = setByte(0x89, lowerHex);
	if (t2 && t4)
	{
		numIncrement = num + 1;
		return true; // Succeed!
	}
	else
	{
#if LOGGING1
		printer->println("setNumofIncrement - Data Transmission Failed!");
#endif
		return false;
	}
}

bool AD5933_Class::setIncrement(long increment)
// Function to set increment frequency.
// Because the increment frequency should be converted into unique Hexadecimal number, it approximately calculates the Hex value.
// long increment - increment frequency in Hz.
{
	long freqHex = increment / (opClock / pow(2, 29)); // Based on the data sheet.
	return setIncrementinHex(freqHex); // Call setIncrementinHex(long);
}

bool AD5933_Class::setIncrementinHex(long freqHex)
// Function to set increment frequency in converted Hex value. (calculated based on the datasheet.)
// long freqHex - converted hexadecimal value
{
	if (freqHex > 0xFFFFFF)
	{
#if LOGGING1
		printer->print("setIncrementHex - Freqeuncy Overflow!");
#endif
		return false;
	}

	int lowerHex = freqHex % 256;
	int midHex = ((freqHex - (long)lowerHex) >> 8) % 256;
	int upperHex = freqHex >> 16;

	bool t2, t3, t4;
	t2 = setByte(0x85, upperHex);
	t3 = setByte(0x86, midHex);
	t4 = setByte(0x87, lowerHex);
	if (t2 && t3 && t4)
	{
		incrHex = freqHex;
		return true; // Succeed!
	}
	else
	{
#if LOGGING1
		printer->println("setIncrementHex - Data Transmission Failed!");
#endif
		return false;
	}

}

bool AD5933_Class::setStartFreq(long startFreq) // long startFreq in Hz
{
#if LOGGING3
	//double t1 = opClock / pow(2,29);
	//printer->println(t1);
#endif
	long freqHex = startFreq / (opClock / pow(2, 29)); // based on datasheet
	if (freqHex > 0xFFFFFF)
	{
#if LOGGING1
		printer->print("setStartFreq - Freqeuncy Overflow!");
#endif
		return false;
	}

	int lowerHex = freqHex % 256;
	int midHex = ((freqHex - (long)lowerHex) >> 8) % 256;
	int upperHex = freqHex >> 16;
#if LOGGING3
	printer->print("setStartFreq - freqHex: ");
	printer->print(freqHex, HEX);
	printer->print("\t");
	printer->print("lower: ");
	printer->print(lowerHex, HEX);
	printer->print("\t");
	printer->print("mid: ");
	printer->print(midHex, HEX);
	printer->print("\t");
	printer->print("upper: ");
	printer->print(upperHex, HEX);
	printer->println();
#endif
	bool t2, t3, t4;
	t2 = setByte(0x82, upperHex);
	t3 = setByte(0x83, midHex);
	t4 = setByte(0x84, lowerHex);
	if (t2 && t3 && t4)
		return true; // succeed!
	else
	{
#if LOGGING1
		printer->println("setStartFreq - Data Transmission Failed!");
#endif
		return false;
	}
}


double AD5933_Class::getTemperature()
// Function to get temperature measurement.
{

	if ( tempUpdate() == false )
		return -1;

	int tTemp[2];
	long tTempVal;
	double cTemp;

	tTemp[0] = getByte(0x92);
	tTemp[1] = getByte(0x93);
	tTempVal = (tTemp[0] % (12 * 16)) * 16 * 16 + tTemp[1];
	if (bitRead(tTemp[0], 5) == 0)
	{
		// Positive Formula
		cTemp = (double)tTempVal / 32;
	}
	else
	{
		// Negative Formula
		cTemp = (tTempVal - 16384.0) / 32;
	}
#if LOGGING1
	printer->print("getTemperature - Current Temp. is ");
	printer->print(cTemp);
	printer->print("\n");
#endif

	return cTemp;
}

bool AD5933_Class::tempUpdate()
// Function to update temperature information without reading.
{
	if (setCtrMode(TEMP_MEASURE) == false)
	{
#if LOGGING1
		printer->println("getTemperature - Failed to set the control bit");
#endif
		return false;
	}

	while ( getStatusReg() & 0x01 != 0x01)
	{
		; // Wait Until Get Vaild Temp. Measurement.
	}

	return true;
}


int AD5933_Class::getByte(int address) {
	// Hidden Function to get register value via I2C Transmission.

	int rxByte;
#if LOGGING3
	printer->print("getByte - Initiating I2C Transmission. Address: ");
	printer->print(address, HEX);
	printer->print('\n');
#endif

	//Wire.beginTransmission(AD5933_ADR); // Begin I2C Transmission with AD5933 Chip.
	//Wire.write(Address_Ptr); // Send Address Pointer to write the target address
	//Wire.write(address); // Write address to read.
	//int i2cReturn = Wire.endTransmission(); // End Transmission.

	if ( !setByte(Address_Ptr, address))
		return false;

	//#if LOGGING3
	//  printer->print("getByte - Transmission Complete. i2cReturn: ");
	//  printer->print(i2cReturn);
	//  printer->print("\n");
	//#endif

	Wire.requestFrom(AD5933_ADR, 1); // Request the value of the written address.

	if (1 <= Wire.available()) { // If the MCU get the value,
		rxByte = Wire.read(); // Read the value.
#if LOGGING3
		printer->print("getByte - Message Received: ");
		printer->print(rxByte, BIN);
		printer->print(" or ");
		printer->print(rxByte, HEX);
		printer->print("\n");
#endif
	}
	else {
		rxByte = -1; // Returns -1 if fails.
#if LOGGING1
		printer->println("getByte - Failed to receive Message");
#endif
	}

	return rxByte;

}

bool AD5933_Class::setByte(int address, int value) {
	// Hidden Function to transmit the value to write.
#if LOGGING3
	printer->print("setByte - Initiating I2C Transmission. Address: ");
	printer->print(address, HEX);
	printer->print(" , Value: ");
	printer->print(value, HEX);
	printer->print('\n');
#endif
	Wire.beginTransmission(AD5933_ADR); // Begin I2C Transmission.
	Wire.write(address); // Write Address
	Wire.write(value); // Write Value
	int i2cReturn = Wire.endTransmission(); // Terminate the transmission.

	if (i2cReturn)
	{
#if LOGGING1
		printer->println("setByte - Failed");
#endif
		return false;
	}
	else
	{
#if LOGGING3
		printer->println("setByte - Success");
#endif
		return true;
	}
}

double AD5933_Class::getMagValue() {
	// Hidden Function to get magnitude value of impedance measurement. (It does not wait.)
	// TODO: Rewrite this function with using block read function.

	int rComp, iComp;
	//rComp = getRealComp(); // Getting Real Component
	//iComp = getImagComp(); // Getting Imaginary Component

	byte impData[4];
	blockRead(0x94, 4, impData);
	rComp = impData[0] * 16 * 16 + impData[1];
	iComp = impData[2] * 16 * 16 + impData[3];

	double result = getMag(rComp, iComp); // Calculating magnitude.
#if LOGGING3
	printer->print("getMagValue - Resistance Magnitude is ");
	printer->println(result);
#endif
	return result;
}

double AD5933_Class::getMagOnce()
// Wrapper Function of getMagValue. It waits until the ADC completes the conversion.
{
	while (!isValueReady()) // wait until ADC conversion is complete.
	{
		//delay(delayTimeInit);
		;
	}
	return getMagValue();
}

bool AD5933_Class::blockRead(int address, int num2Read, byte *toSave)
{
	if ( !AD5933.setByte(Address_Ptr, address) )
		return false;
	if ( !AD5933.setByte(BLOCK_READ_CODE, num2Read) )
		return false;

	for (byte t1 = 0; t1 < num2Read; t1++)
	{
		Wire.requestFrom(AD5933_ADR, 1); // Request the value of the written address.

		if (1 <= Wire.available()) { // If the MCU get the value,
			toSave[t1] = Wire.read(); // Read the value.
#if LOGGING3
			printer->print(address + t1, HEX);
			printer->print(" ");
			printer->println(toSave[t1], HEX);
#endif
		}
		else {
			toSave[t1] = -1; // Returns -1 if fails.
#if LOGGING1
			printer->println("blockRead - Failed to receive Message");
#endif
			return false;

		}
	}

	return true;
}

bool AD5933_Class::isValueReady()
{
	if ( (getStatusReg() & 0x02) == 0x02 )
		return true;
	else
		return false;
}

bool AD5933_Class::getComplexRawOnce(int &realComp, int &imagComp)
{
	while ( (getStatusReg() & 0x02) != 0x02 ); // Wait until measurement is complete.

	int rComp, iComp;

	byte impData[4];
	blockRead(0x94, 4, impData);
	rComp = impData[0] * 16 * 16 + impData[1];
	iComp = impData[2] * 16 * 16 + impData[3];

	//double magSq = square((double)rComp) + square((double)iComp);
	//double td1=gainFactor/magSq;
	//realComp = abs(rComp)*td1;
	//imagComp = abs(iComp)*td1;
	realComp = rComp;
	imagComp = iComp;

	return true;
}

bool AD5933_Class::getComplexOnce(double gainFactor, double pShift, double &vReal, double &vComp, double &impVal, double &phase)
{
	while ( (getStatusReg() & 0x02) != 0x02 ); // Wait until measurement is complete.

	int rComp, iComp;

	byte impData[4];
	blockRead(0x94, 4, impData);
	rComp = impData[0] * 16 * 16 + impData[1];
	iComp = impData[2] * 16 * 16 + impData[3];

	double magSq = getMag(rComp, iComp);
	impVal = gainFactor / magSq;
	phase = atan2(iComp, rComp) - pShift;
	vReal = impVal * cos(phase);
	vComp = impVal * sin(phase);

	return true;
}

bool AD5933_Class::getComplex(double gainFactor, double pShift, double &impVal, double &phase)
{
	while ( (getStatusReg() & 0x02) != 0x02 ); // Wait until measurement is complete.

	int rComp, iComp;

	byte impData[4];
	blockRead(0x94, 4, impData);
	rComp = impData[0] * 16 * 16 + impData[1];
	iComp = impData[2] * 16 * 16 + impData[3];

	double magSq = getMag(rComp, iComp);
	impVal = gainFactor / magSq;
	phase = atan2(iComp, rComp) - pShift;
	return true;
}

bool AD5933_Class::getComplexTetra(int millisDelay, double gainFactor, double vPhaseShift, double iPhaseShift, double &impVal, double &phase) {

	int rVoltage, iVoltage, rCurrent, iCurrent;

	int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg

	delay(millisDelay);
	digitalWrite(IV_MUX, LOW); // Voltage measurement
	delay(millisDelay);

	if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
		return false;
	}
	getComplexRawOnce(rVoltage, iVoltage);

	delay(millisDelay);
	digitalWrite(IV_MUX, HIGH); // Current measurement
	delay(millisDelay);

	if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
		return false;
	}

	getComplexRawOnce(rCurrent, iCurrent);

	impVal = gainFactor * (getMag(rVoltage, iVoltage) / getMag(rCurrent, iCurrent));
	phase = (atan2(iVoltage, rVoltage) - vPhaseShift) - (atan2(iCurrent, rCurrent) - iPhaseShift);
	return true;
}

double AD5933_Class::returnStandardPhaseAngle(double angle) {
	return angle;
}

/*

  if(angle >= 0 && angle < M_PI_2) {    // 1st quadrant
    angle = angle;
  }
  else if(angle >= M_PI_2 && angle < M_PI) {      // 2nd quadrant
    angle = M_PI - angle;
  }
  else if(angle <= -1 * M_PI_2 && angle > -1 * M_PI) {      // 3rd quadrant
    angle = angle + M_PI;
  }
  else if(angle <= 0 && angle > -1 * M_PI_2) {         // 4th quadrant
    angle *=  -1;
  } */

/*
int AD5933_Class::getRealComp()
{
  while(!isValueReady()) // wait until ADC conversion is complete.
  {
    //delay(delayTimeInit);
    ;
  }
  return getRealCompP();
}

int AD5933_Class::getImagComp()
{
  while(!isValueReady()) // wait until ADC conversion is complete.
  {
    //delay(delayTimeInit);
    ;
  }
  return getImagCompP();
}

int AD5933_Class::getRealCompP()
// Function to get real component.
{
  int mReal, lReal;
  int result;
  mReal=getByte(0x94);
  lReal=getByte(0x95);
#if LOGGING3
  printer->print("getRealComp - mReal: ");
  printer->print(mReal, BIN);
  printer->print('\t');
  printer->print("lReal: ");
  printer->println(lReal, BIN);
  printer->print("getRealComp - Value: ");
#endif
  result = (int16_t)(mReal*16*16+lReal);
#if LOGGING3
  printer->println(result);
#endif
  return result;
}

int AD5933_Class::getImagCompP()
// Function to get imaginary component.
{
  int mImag, lImag;
  int result;
  mImag=getByte(0x96);
  lImag=getByte(0x97);
#if LOGGING3
  printer->print("getImagComp - mImag: ");
  printer->print(mImag, BIN);
  printer->print('\t');
  printer->print("lImag: ");
  printer->println(lImag, BIN);
  printer->print("getImagComp - Value: ");
#endif
  result = (int16_t)(mImag*16*16+lImag);
#if LOGGING3
  printer->println(result);
#endif
  return result;
}
 */
