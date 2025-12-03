#include <iostream>

#include "DAQConfig/GADCSConf.hh"

ClassImp(GADCSConf)

GADCSConf::GADCSConf()
  : AbsConf()
{
}

GADCSConf::GADCSConf(int sid)
  : AbsConf(sid)
{
  SetNameTitle("GADCS", "Standalone GADC 125MHz");

  fNCH = kNCHGADC;
  fRL = 8;
  fTLT = 1;
  fDSR = 1;

  unsigned long pct = 1;
  unsigned long pwt = 0;
  unsigned long pst = 0;
  unsigned long pstor = 0;
  unsigned long tm = (pstor << 3) | (pst << 2) | (pwt << 1) | pct;

  for (int i = 0; i < kNCHGADC; i++) {
    fCID[i] = i + 1;
    fPID[i] = 0;
    fPOL[i] = 0;
    fDACOFF[i] = 4000;
    fAMD[i] = 1;
    fDLY[i] = 0;
    fDT[i] = 0;
    fCW[i] = 1000;
    fTM[i] = tm;
    fTHR[i] = 10;
    fPCT[i] = 1;
    fPCI[i] = 1000;
    fPWT[i] = 0;
    fPSW[i] = 0;
  }

  fIsLinked = true;
  fTRGON = 1;
  fPTRG = 0;
  fPSC = 1;
  fADCType = ADC::GADCS;
}

void GADCSConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  cout << Form(" ++ %s config: SID(%d) MID(%1d) NCH(%1d) RL(%d) DSR(%d) TLT(%X) TRGON(%d) PTRG(%d) PSC(%d)", GetName(),
               fSID, fMID, fNCH, fRL, fDSR, fTLT, fTRGON, fPTRG, fPSC)
       << endl;

  if (fIsEnabled) cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else cout << Form(" ++ This %s is disabled", GetName()) << endl;

  cout << " -----------------------------------------------" << endl;

  auto print = [&](const char * label, const int * arr) {
    cout << " " << label << " : ";
    for (int i = 0; i < fNCH; i++) {
      cout << Form("%8d", arr[i]);
    }
    cout << endl;
  };

  print("CID", fCID);
  print("PID", fPID);
  print("POL", fPOL);
  print("DACOFF", fDACOFF);
  print("DLY", fDLY);
  print("DTIME", fDT);
  print("CW", fCW);
  print("TM", fTM);
  print("THR", fTHR);
  print("PCT", fPCT);
  print("PCI", fPCI);
  print("PWT", fPWT);
  print("PSW", fPSW);

  cout << " -----------------------------------------------" << endl;
  cout << endl;
}
