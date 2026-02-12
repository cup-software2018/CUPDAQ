#include <iostream>

#include "DAQConfig/FADCTConf.hh"

ClassImp(FADCTConf)

FADCTConf::FADCTConf()
  : AbsConf()
{
}

FADCTConf::FADCTConf(int sid)
  : AbsConf(sid, ADC::FADCT)
{
  SetNameTitle("FADCT", "TCB controlled fadc 500MHz");

  fNCH = kNCHFADC;
  fRL = 8;
  fTLT = 0xFFFE;
  fDSR = 1;
  fTRGON = 0;

  unsigned long pct = 1;
  unsigned long pwt = 0;
  unsigned long pst = 0;
  unsigned long pstor = 0;
  unsigned long tm = (pstor << 3) | (pst << 2) | (pwt << 1) | pct;

  for (int i = 0; i < kNCHFADC; i++) {
    fCID[i] = i + 1;
    fPID[i] = 0;
    fPOL[i] = 0;
    fDACOFF[i] = 4000;
    fAMD[i] = 0;
    fDLY[i] = 0;
    fDT[i] = 0;
    fCW[i] = 1000;
    fTM[i] = tm;
    fTHR[i] = 10;
    fPCT[i] = 1;
    fPCI[i] = 1000;
    fPWT[i] = 0;
    fPSW[i] = 0;
    fPedRMS[i] = 0;
    fPMTGain[i] = 0;
    fPMTGainScale[i] = 0;
    fPMTFallTime[i] = 0;
    fPMTTransitTime[i] = 0;
    fPMTTTS[i] = 0;
    fPMTQEff[i] = 0;
  }
}

void FADCTConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  cout << Form(" ++ %s config: SID(%d) MID(%1d) NCH(%1d) TRGON(%d) RL(%d) "
               "TLT(%X) DSR(%d) DAQID(%d)",
               GetName(), fSID, fMID, fNCH, fTRGON, fRL, fTLT, fDSR, fDAQID)
       << endl;

  if (fIsEnabled) cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else cout << Form(" ++ This %s is disabled", GetName()) << endl;

  cout << " -----------------------------------------------" << endl;

  auto print = [&](const char * label, const int * arr) {
    cout << Form("%8s", label);
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
