// Library Code Section of AD5933

// Author: Il-Taek Kwon

// Modifications: Adetunji Dahunsi

#include "AD5933.h"
//#include <WProgram.h>
#include <Arduino.h> // For the compatibility with Arduino Conventions.
//#include <WConstants.h>

AD5933_Class AD5933;

/**
 * Gets the temperature of the AD5933
 */

double AD5933_Class::getTemperature() {

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

/*
 * Updates temperature information without writing to a variable.
 */

bool AD5933_Class::tempUpdate() {

	if (setCtrMode(TEMP_MEASURE) == false)
	{
#if LOGGING1
		printer->println("tempUpdate - Failed to set the control bit");
#endif
		return false;
	}

	while ( (getStatusReg() & 0x01) != 0x01)
	{
		; // Wait Until Get Vaild Temp. Measurement.
	}

	return true;
}

/**
 * Sets the Reset Bit (D4) in control register 0x81.
 */

bool AD5933_Class::resetAD5933() {

	int temp = (getByte(0x81) & 0x08); // Read 0x81 with retrieving D3
	return setByte(0x81, (temp | 0x10)); // Set D4 as 1 (Reset Bit)
}

/**
 * A function to enable/disable external clock.
 *
 * This function also sets the environmental variable (double opClock)
 *
 * @Param swt True uses an external clook.
 */

bool AD5933_Class::setExtClock(bool swt) {
	byte t1;
	if ( swt )
	{
		t1 = 0x08; // Use Ext. Clock
		opClock = 16000000;
	}
	else
	{
		t1 = 0x00; // Use Int. Clock
		opClock = 16776000;
	}
	return setByte(0x81, t1); // Write register 0x81.
}

/**
 * Sets the number of settling cycles before a measurement is taken.
 *
 * @Param cycles Cycles to ignore before a measurement is taken. Max is 511.
 * @Param mult Multiplier for cycles. Can be 1, 2, or 4.
 */

bool AD5933_Class::setSettlingCycles(int cycles, byte mult) {

	if (cycles > 0x1FF || !(mult == 1 || mult == 2 || mult == 4) ){
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


/**
 * Sets the start frequency for a frequency sweep in hertz.
 *
 * @Param startFreq The start frequency
 */
bool AD5933_Class::setStartFreq(long startFreq) {

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

/**
 * Sets the step size for a frequency sweep in hertz.
 *
 * NOTE: because the increment frequency should be converted into unique hexadecimal number,
 * it approximately calculates the Hex value.
 *
 * @Param increment The stepSize in hertz
 */

bool AD5933_Class::setStepSize(long increment) {

	long freqHex = increment / (opClock / pow(2, 29)); // Based on the data sheet.
	return setStepSizeInHex(freqHex); // Call setIncrementinHex(long);
}

/**
 * Sets the step size for a frequency sweep in using hexadecimal values.
 *
 * The value is calculated based on the datasheet.
 *
 * @Param freqHex The converted hexadecimal value
 */

bool AD5933_Class::setStepSizeInHex(long freqHex) {

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

/**
 * Sets the number of frequency increments in a frequency sweep. The AD5933 is rated for 1 - 100 KHZ.
 * You may go higher at the risk of larger errors.
 *
 * NOTE: This is the number of increments, not the total number of points in the sweep. An Increment of 5
 * means a total of 6 frequency points.
 *
 * @Param num Number of increments
 */

bool AD5933_Class::setNumofIncrement(byte num) {

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

/**
 * Changes the value of the excitation volatage at the transmit (output) stage of the AD5933 to one of the 4 selectable ranges.
 *
 * The contents if the control register remain the same with the exception of the bits determining
 * the output range.
 *
 * @Param rangeToSet Values of 1 - 4, for each excitation range respectively.
 */

bool AD5933_Class::setRange(byte rangeToSet) {
	return setRange(rangeToSet, getByte(0x80));
}

/**
 * Changes the value of the excitation volatage at the transmit (output) stage of the AD5933 to one of the 4 selectable ranges.
 *
 * This function also lets you overwrite the contents of the remaining bits in the control register with
 * values of your choosing.
 *
 * @Param rangeToSet Values of 1 - 4, for each excitation range respectively.
 * @Param ctrReg The new contents of the control resister to overwrite prior to bit shifting.
 */

bool AD5933_Class::setRange(byte rangeToSet, int ctrReg) {

	ctrReg &= 0xF9; // Clear D9 & D10 to modify

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

/**
 * Changes the value of the gain at the recieve (input) stage of the AD5933 to one of the 2 selectable gains.
 *
 * The contents if the control register remain the same with the exception of the single bit determining
 * the gain.
 *
 * @Param pgaGain Values of 1 or 5, for each gain respectively.
 */

bool AD5933_Class::setPGA(byte pgaGain){
	return setPGA(pgaGain, getByte(0x80));
}

/**
 * Changes the value of the gain at the recieve (input) stage of the AD5933 to one of the 2 selectable gains.
 *
 * This function also lets you overwrite the contents of the remaining bits in the control register with
 * values of your choosing.
 *
 * @Param pgaGain Values of 1 or 5, for each gain respectively.
 * @Param ctrReg The new contents of the control resister to overwrite prior to bit shifting.
 */

bool AD5933_Class::setPGA(byte pgaGain, int ctrReg) {

	ctrReg &= 0xFE; // Clear D8 to modify

	switch (pgaGain) {

	case GAIN_1:
		ctrReg |= 0x01;
#if LOGGING1
		Serial.println("Changed to GAIN_1");
#endif
		break;

	case GAIN_5:
		ctrReg |= 0x00;
#if LOGGING1
		Serial.println("Changed to GAIN_5");
#endif
		break;

	default:
#if LOGGING1
		printer->println("setPGA - Invalid Parameter!");
#endif
		return false; // return the signal of fail if there is not valid parameter.
		break;
	}
	return setByte(0x80, ctrReg); // return signal depends on the result of setting control register.

}

/**
 * Calculates a single gain factor for a single frequency in bi-polar mode. The AD5933 for consecutive measurements must
 * set the control mode to repeat frequency for accurate measurements. (setCtrMode(REPEAT_FREQ)
 * The AD5933 will automatically eneter standby mode after this, for the alternative see:
 *
 * 	bool getGainFactor(double, int, double &, double &, bool);
 *
 *
 * @Param cResistance The calibration resistance
 * @Param avgNum The number of times each gain factor should be measured, and then averaged by.
 * @Param gainFactor The double into which the gain factor is saved
 * @Param pShift The array into which the system phase shift is saved
 */

bool AD5933_Class::getGainFactor(double cResistance, int avgNum, double &gainFactor, double &pShift) {
	return getGainFactor(cResistance, avgNum, gainFactor, pShift, true);
}

/**
 * Calculates a single gain factor for a single frequency in bi-polar mode. The AD5933 for consecutive measurements must
 * set the control mode to repeat frequency for accurate measurements. (setCtrMode(REPEAT_FREQ)
 *
 * @Param cResistance The calibration resistance
 * @Param avgNum The number of times each gain factor should be measured, and then averaged by.
 * @Param gainFactor The double into which the gain factor is saved
 * @Param pShift The array into which the system phase shift is saved
 * @Param retStandBy A boolean instructing the AD5933 to enter standby mode after calculation or not.
 */
bool AD5933_Class::getGainFactor(double cResistance, int avgNum, double &gainFactor, double &pShift, bool retStandBy) {

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

/**
 * Calculates a single gain factor for a single frequency in tetra-polar mode. The AD5933 for consecutive measurements must
 * set thr control mode to repeat frequency for accurate measurements. (setCtrMode(REPEAT_FREQ)
 * The AD5933 will automatically eneter standby mode after this, for the alternative see:
 *
 * 	bool getGainFactorTetra(double, int, double &, double &, double &, bool);
 *
 *
 * @Param cResistance The calibration resistance
 * @Param avgNum The number of times each gain factor should be measured, and then averaged by.
 * @Param gainFactor The double into which the gain factor is saved
 * @Param pShift The array into which the system phase shift is saved
 */

bool AD5933_Class::getGainFactorTetra(double calResistance, int avgNum, double &gainFactor, double &vPShift, double &iPShift) {
	return getGainFactorTetra(calResistance, avgNum, gainFactor, vPShift, iPShift, true);
}

/**
 * Calculates a single gain factor for a single frequency in tetra-polar mode. The AD5933 for consecutive measurements must
 * set thr control mode to repeat frequency for accurate measurements. (setCtrMode(REPEAT_FREQ)
 *
 * @Param cResistance The calibration resistance
 * @Param avgNum The number of times each gain factor should be measured, and then averaged by.
 * @Param gainFactor The double into which the gain factor is saved
 * @Param pShift The array into which the system phase shift is saved
 * @Param retStandBy A boolean instructing the AD5933 to enter standby mode after calculation or not.
 */

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

	digitalWrite(IV_MUX, LOW); //Toggle current / voltage multiplexer to voltage
	delay(10);

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

	digitalWrite(IV_MUX, HIGH); // Toggle current / voltage multiplexer to current
	delay(10); // delay for mux to settle

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

/**
 * Calculates the gain factors and system phase shift for each step in a predefined frequency sweep for the AD5933 in a bi-polar configuration
 *
 * @Param cResistance The calibration resistance
 * @Param avgNum The number of times each gain factor should be measured, and then averaged by.
 * @Param gainFactorArr The array into which the gain factors are saved
 * @Param pShiftArr The array into which the system phase shifts are saved
 */

bool AD5933_Class::getGainFactorsSweep(double cResistance, int avgNum, double *gainFactorArr, double *pShiftArr) {
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

/**
 * Calculates the gain factors and system phase shift for each step in a predrfined frequency sweep for the AD5933 in a tetra-polar configuration
 *
 * @Param cResistance The calibration resistance
 * @Param avgNum The number of times each gain factor should be measured, and then averaged by.
 * @Param gainFactorArr The array into which the gain factors are saved
 * @Param vShiftArr The array into which the system voltage phase shifts are saved
 * @Param cShiftArr The array into which the system current phase shifts are saved
 */

bool AD5933_Class::getGainFactorsTetraSweep(double cResistance, int avgNum, double *gainFactorArr, double *vShiftArr, double *cShiftArr){

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

	int arrayIndex = 0;

	while ( (getStatusReg() & 0x04) != 0x04) { // While the fequency sweep isn't complete

		//////

		int averageIndex = 0, iVoltage = 0, rVoltage = 0, iCurrent = 0, rCurrent = 0;
		double voltageSum = 0, currentSum = 0, voltagePhaseSum = 0, currentPhaseSum = 0;


		digitalWrite(IV_MUX, LOW); //Toggle current / voltage multiplexer to voltage
		delay(10);

		if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
			return false;
		}

		// Until reached pre-defined number for averaging.
		while (averageIndex < avgNum) {

			getComplexRawOnce(rVoltage, iVoltage);

			voltageSum += getMag(rVoltage, iVoltage);
			voltagePhaseSum += atan2(iVoltage, rVoltage);

			if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
				return false;
			}

			averageIndex++;
		}

		digitalWrite(IV_MUX, HIGH); // Toggle current / voltage multiplexer to current
		delay(10); // delay for mux to settle

		averageIndex = 0; // Reset index for current

		if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
			return false;
		}

		while (averageIndex < avgNum) {

			getComplexRawOnce(rCurrent, iCurrent);

			currentSum += getMag(rCurrent, iCurrent);
			currentPhaseSum += atan2(iCurrent, rCurrent);

			if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
				return false;
			}

			averageIndex++;
		}

		double voltageMag = voltageSum / ((double) avgNum);
		double currentMag = currentSum / ((double) avgNum);

		gainFactorArr[arrayIndex] = cResistance * (currentMag / voltageMag);
		vShiftArr[arrayIndex] = voltagePhaseSum / ((double) avgNum);
		cShiftArr[arrayIndex] = currentPhaseSum / ((double) avgNum);

		//////

		if (setCtrMode(INCR_FREQ, ctrReg) == false){
			return false;
		}

		arrayIndex++;
	}

	if (setCtrMode(POWER_DOWN, ctrReg) == false){
		return false;
	}
	return true; // Succeed!
}

/*
 * Gets the raw values of the DFT from the AD5933 after calculation
 *
 * @Param realComp The real component of the DFT
 * @Param imagComp The imaginary component of the DFT
 */

bool AD5933_Class::getComplexRawOnce(int &realComp, int &imagComp) {

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

/*
 * Gets the impedance magnitude and phase angle for the AD5933 in a bi-polar configuration.
 *
 * @Param gainFactor The gainfactor caluclulated at calibration
 * @Param pShift The system phase shift calculated at calibration
 * @Param impVal The magnitude of the impedance
 * @Param phase The phase angle of the impedance
 */

bool AD5933_Class::getComplex(double gainFactor, double pShift, double &impVal, double &phase) {
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

/*
 * Gets the impedance magnitude for the AD5933 in a bi-polar configuration.
 *
 * @Param gainFactor The gainfactor caluclulated at calibration
 * @Param impVal The magnitude of the impedance
 */

bool AD5933_Class::getImpedance(double gainFactor, double &impVal) {
	while ( (getStatusReg() & 0x02) != 0x02 ); // Wait until measurement is complete.

	int rComp, iComp;

	byte impData[4];
	blockRead(0x94, 4, impData);
	rComp = impData[0] * 16 * 16 + impData[1];
	iComp = impData[2] * 16 * 16 + impData[3];

	double magSq = getMag(rComp, iComp);
	impVal = gainFactor / magSq;
	return true;
}


/*
 * Gets the impedance magnitude and phase angle for the AD5933 in a tetra-polar configuration.
 *
 * @Param gainFactor The gainfactor caluclulated at calibration
 * @Param vPhaseShift The system voltage phase shift calculated at calibration
 * @Param iPhaseShift The system current phase shift calculated at calibration
 * @Param impVal The magnitude of the impedance
 * @Param phase The phase angle of the impedance
 */
bool AD5933_Class::getComplexTetra(int millisDelay, double gainFactor, double vPhaseShift, double iPhaseShift, double &impVal, double &phase) {

	int rVoltage, iVoltage, rCurrent, iCurrent;

	int ctrReg = getByte(0x80); // Get the content of Control Register and put it into ctrReg

	digitalWrite(IV_MUX, LOW); // Voltage measurement
	delay(millisDelay);

	if (setCtrMode(REPEAT_FREQ, ctrReg) == false) {
		return false;
	}
	getComplexRawOnce(rVoltage, iVoltage);

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

/**
 * Sends a command instruction to the control register without assumming the contents of the
 * control register.
 *
 * @Param modetoSet One of the available commands to send to control register of the AD5933
 */

bool AD5933_Class::setCtrMode(byte modetoSet) {
	return setCtrMode(modetoSet, getByte(0x80));
}

/**
 * Sends a command instruction to the control register with an explicit integer detailing the contents
 * of the control register. If this integer doesn't match what the control register already contains,
 * the previous bits in the register will be overwritten with this one.
 *
 * @Param modetoSet One of the available commands to send to control register of the AD5933
 * @Param ctrReg The new contents of the control resister to overwrite prior to bit shifting.
 */
bool AD5933_Class::setCtrMode(byte modetoSet, int ctrReg) {

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

/*
 * Hidden Function to get the contents of a register address via I2C.
 * Returns retrived byte data.
 *
 * @Param address The I2C register address to query
 */

int AD5933_Class::getByte(int address) {

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

/*
 * Hidden Function to write values to an I2C address.
 *
 * @Param address The I2C register address to write to
 * @Param value The value to write
 */

bool AD5933_Class::setByte(int address, int value) {

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

/*
 * Hidden Function to get magnitude value of impedance measurement. (It does not wait.)
 * Returns the magnitude of impedance value
 */
double AD5933_Class::getMagValue() {
	
	int rComp, iComp;

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

/*
 * Wrapper Function of getMagValue. It waits until the ADC completes the conversion.
 *
 */
double AD5933_Class::getMagOnce()
{
	while (!isValueReady()) // wait until ADC conversion is complete.
	{
		//delay(delayTimeInit);
		;
	}
	return getMagValue();
}

/*
 * Reads a block of data from the AD5933
 * Implemented for Block Access with reading multiple bytes in once as optimization
 *
 * Returns false when failed. Returns true when succeed.
 *
 * @Param address The address of the AD5933
 * @Param num2Read The number of values to read
 * @Param toSave The byte array the values read are saved to
 */

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

/*
 * Checks if the AD5933 has completed it's DFT algorithm and the results are present in the registers.
 * It is private function. It should not be used outside of the program.
 */

bool AD5933_Class::isValueReady()
{
	if ( (getStatusReg() & 0x02) == 0x02 )
		return true;
	else
		return false;
}

/*
 * Used to set I/O pins for custom AD5933 PCB (Tetrapolar, made by Henway)
 * Programmed by Adetunji Dahunsi
 * Designed for Henway Tetrapolar Custom AD5933 PCB Circuit
 *
 * @Param state Configure the AD5933 in bi-polar or tetra polar mode (LOW - TETRA, HIGH - BI)
 *
 */

bool AD5933_Class::setupDevicePins(int state) {

	pinMode(LED5, OUTPUT);
	pinMode(LED6, OUTPUT);
	pinMode(LED7_R, OUTPUT);
	pinMode(LED7_G, OUTPUT);
	pinMode(LED7_B, OUTPUT);
	pinMode(LED8, OUTPUT);

	pinMode(BOOST, OUTPUT);
	pinMode(IV_MUX, OUTPUT);
	pinMode(BI_TETRA_MUX, OUTPUT);

	pinMode(PUSH1, INPUT);
	pinMode(PUSH2, INPUT);

	pinMode(25, INPUT);
	pinMode(24, INPUT);

	int leds[6] = {LED5, LED6, LED7_R, LED7_G, LED7_B, LED8};

	//All leds off
	for(int i = 0; i < 6; i++) {
		digitalWrite(leds[i], HIGH);
	}

	digitalWrite(BOOST, HIGH);

	switch(state) {

	case LOW:
		digitalWrite(BI_TETRA_MUX, LOW); // Set AD5933 to tetra-polar mode
		return true;
	case HIGH:
		digitalWrite(BI_TETRA_MUX, HIGH); // Set AD5933 to tetra-polar mode
		return true;
	default:
		return false;
	}
}
