#include <iostream>

#include "DAQConfig/MADCSConf.hh"

ClassImp(MADCSConf)

MADCSConf::MADCSConf()
  : FADCTConf()
{
}

MADCSConf::MADCSConf(int sid)
  : FADCTConf(sid)
{
  SetNameTitle("MADCS", "Standalone MADC 125MHz");
  fIsLinked = true;
  fTRGON = 1;
  fPTRG = 0;
  fPSC = 1;
  fNCH = kNCHMADC;
  fADCType = ADC::MADCS;
}

void MADCSConf::PrintConf() const
{
  std::cout << Form(" ++ %s config: SID(%d) MID(%1d) NCH(%1d) RL(%d) DSR(%d) TLT(%X) TRGON(%d) PTRG(%d) PSC(%d)", GetName(),
               fSID, fMID, fNCH, fRL, fDSR, fTLT, fTRGON, fPTRG, fPSC)
       << std::endl;
  if (fIsEnabled) std::cout << Form(" ++ This %s is enabled", GetName()) << std::endl;
  else std::cout << Form(" ++ This %s is disabled", GetName()) << std::endl;

  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << "        CID : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fCID[i]);
  std::cout << std::endl;
  std::cout << "        PID : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fPID[i]);
  std::cout << std::endl;
  std::cout << "        POL : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fPOL[i]);
  std::cout << std::endl;
  std::cout << "     DACOFF : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fDACOFF[i]);
  std::cout << std::endl;
  std::cout << "        DLY : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fDLY[i]);
  std::cout << std::endl;
  std::cout << "      DTIME : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fDT[i]);
  std::cout << std::endl;
  std::cout << "         CW : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fCW[i]);
  std::cout << std::endl;
  std::cout << "         TM : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fTM[i]);
  std::cout << std::endl;
  std::cout << "        THR : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fTHR[i]);
  std::cout << std::endl;
  std::cout << "        PCT : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fPCT[i]);
  std::cout << std::endl;
  std::cout << "        PCI : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fPCI[i]);
  std::cout << std::endl;
  std::cout << "        PWT : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fPWT[i]);
  std::cout << std::endl;
  std::cout << "        PSW : ";
  for (int i = 0; i < fNCH; i++)
    std::cout << Form("%8d", fPSW[i]);
  std::cout << std::endl;
  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << std::endl;
}
