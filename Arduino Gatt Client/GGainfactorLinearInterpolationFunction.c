// Used to get the gain factor and phase shifts
// at the first and last frquency points in the sweep

bool AD5933_Class::getGainFactors_LI(double cResistance, int avgNum, double startFreq, double endFreq, double &GF_Initial, double &GF_Final, double &PS_Initial, double &PS_Final)
{
  int ctrReg = getByte(0x80);
  byte numIncrementBackup = numIncrement;
  long incrHexBackup = incrHex;
  setNumofIncrement(2);
  setIncrement(endFreq-startFreq);
  if(setCtrMode(STAND_BY, ctrReg) == false)
    {
      return false;
  }
    if(setCtrMode(INIT_START_FREQ, ctrReg) == false)
    {
      return false;
    }
    if(setCtrMode(START_FREQ_SWEEP, ctrReg) == false)
    {
      return false;
    }
    
  int t1=0;
  int rImag, rReal;
  double sumMag=0, sumPhase=0;
    for(t1=0;t1<avgNum;t1++)
    {
      getComplexRawOnce(rReal, rImag); // Only for real and Imag components. 
      sumMag += getMag(rReal, rImag);
      sumPhase += atan2(rImag, rReal);
      
      if(setCtrMode(REPEAT_FREQ, ctrReg) == false)
      {
          return false;
      }
    }
    GF_Initial = (sumMag / avgNum) * cResistance;
    PS_Initial = sumPhase / avgNum;
    
    setCtrMode(INCR_FREQ, ctrReg);
    sumMag = 0;
    sumPhase = 0;
    
    for(t1=0;t1<avgNum;t1++)
    {
      getComplexRawOnce(rReal, rImag); // Only for real and Imag components. 
      sumMag += getMag(rReal, rImag);
      sumPhase += atan2(rImag, rReal);
      
      if(setCtrMode(REPEAT_FREQ, ctrReg) == false)
      {
          return false;
      }
    }
    GF_Final = (sumMag / avgNum) * cResistance;
    PS_Final = (sumPhase / avgNum);
    
    
    if(setCtrMode(POWER_DOWN, ctrReg) == false)
    {
      return false;
    }
    setNumofIncrement(numIncrementBackup);
    setIncrementinHex(incrHexBackup);
    return true; // Succeed!  
}

// Uses linear interpolation to find GF and PS
// Values for each increment in the sweep.

/*
(GFfinal - GF_Initial) / (GFunknown - GF_intial) 
= 
(FreqFinal - FreqInitial) / (FreqUnknown - FreqInitial)

therefore

GFUnkown
 = 
[((GFfinal - GF_Initial) * (FreqUnknown - FreqInitial) / (FreqFinal - FreqInitial)] + GFinitial
 = 
[ΔGF * (FreqUnknown - FreqInitial) / ΔFreq] + GFinitial

but ΔFreq = stepSize * number of increments
and FreqUnknown - FreqInitial =  stepSize * current iteration

therefore step size cancels out and the GainFactor
at the current iteration reduces to

[ΔGF * (currentIteration) / numOfIncrements] + GFinitial

same goes for phase shift.
*/

bool AD5933_Class::getArraysLI(
  double &deltaGF, double &deltaPS, 
  double &stepSize, uint8_t &numOfIncrements,
  double &GF_Initial, double &PS_Initial,
  double *arrGainFactor, double *arrPShift)
{
  int t1 = 0;

  for(t1 = 1; t1 < numOfIncrements; t1++)
  {
    arrGainFactor[t1] = (deltaGF * t1 / numOfIncrements) + GF_Initial;
    arrPShift[t1] = (deltaPS * t1 / numOfIncrements) + PS_Initial;
  }
  return true;
}

