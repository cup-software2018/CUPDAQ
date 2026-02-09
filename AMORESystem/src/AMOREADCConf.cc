#include <iostream>

#include "AMORESystem/AMOREADCConf.hh"

ClassImp(AMOREADCConf)

AMOREADCConf::AMOREADCConf(int sid)
  : AbsConf(sid, ADC::AMOREADC)
{
  SetNameTitle("AMOREADC", "ADC for AMoRE Experiment");

  for (int i = 0; i < fNCH; i++) {
    fCID[i] = i + 1;
    fPID[i] = 0;
  }

  fMID = sid + 128;
}

void AMOREADCConf::PrintConf() const
{
  std::cout << Form(" ++ AMOREADC config: SID(%d) MID(%d) NCH(%d) SR(%d) RL(%d) DLY(%d) ZSU(%d)", fSID, fMID, fNCH, fSR,
                    fRL, fDLY, fZSU)
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
  }
  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << std::endl;
}
