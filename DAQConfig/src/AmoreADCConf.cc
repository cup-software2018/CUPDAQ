#include <iostream>

#include "DAQConfig/AmoreADCConf.hh"

ClassImp(AmoreADCConf)

AmoreADCConf::AmoreADCConf()
  : AbsConf()
{
}

AmoreADCConf::AmoreADCConf(int sid)
  : AbsConf(sid, ADC::AMOREADC)
{
  SetNameTitle("AMOREADC", "ADC for AMoRE Experiment");

  fNCH = kNCHAMOREADC;
  fSR = 10;
  fRL = 0;
  fDLY = 1000;
  fCW = 0;
  fSKBIN = 0;
  fZEROSUP = 0;

  for (int i = 0; i < kNCHAMOREADC; i++) {
    fCID[i] = i + 1;
    fPID[i] = 0;
    fTRGON[i] = 0;
    fORDER[i] = 1;
    fLOWER[i] = 100;
    fUPPER[i] = 1000;
    fTHR[i] = 10;
    fDT[i] = 0;
  }
}

void AmoreADCConf::PrintConf() const
{
  std::cout << Form(" ++ AMOREADC config: SID(%d) MID(%d) NCH(%d) SR(%d) RL(%d) "
               "DLY(%d) CW(%d) SKBIN(%d) ZEORESUP(%d)",
               fSID, fMID, fNCH, fSR, fRL, fDLY, fCW, fSKBIN, fZEROSUP)
       << std::endl;
  if (fIsEnabled) std::cout << " ++ This AMOREADC is enabled" << std::endl;
  else std::cout << " ++ This AMOREADC is disabled" << std::endl;

  std::cout << " -----------------------------------------------" << std::endl;
  for (int j = 0; j < 2; j++) {
    if (j > 0) std::cout << std::endl;
    std::cout << "        CID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      std::cout << Form("%6d", fCID[i]);
    std::cout << std::endl;
    std::cout << "        PID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      std::cout << Form("%6d", fPID[i]);
    std::cout << std::endl;
    std::cout << "      TRGON : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      std::cout << Form("%6d", fTRGON[i]);
    std::cout << std::endl;
    std::cout << "      ORDER : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      std::cout << Form("%6d", fORDER[i]);
    std::cout << std::endl;
    std::cout << "      LOWER : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      std::cout << Form("%6d", fLOWER[i]);
    std::cout << std::endl;
    std::cout << "      UPPER : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      std::cout << Form("%6d", fUPPER[i]);
    std::cout << std::endl;
    std::cout << "        THR : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      std::cout << Form("%6d", fTHR[i]);
    std::cout << std::endl;
    std::cout << "         DT : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      std::cout << Form("%6d", fDT[i]);
    std::cout << std::endl;
  }
  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << std::endl;
}
