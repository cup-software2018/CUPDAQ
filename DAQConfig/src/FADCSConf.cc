#include <iostream>

#include "DAQConfig/FADCSConf.hh"

ClassImp(FADCSConf)

FADCSConf::FADCSConf()
  : FADCTConf()
{
  SetLink();
}

FADCSConf::FADCSConf(int sid)
  : FADCTConf(sid)
{
  SetNameTitle("FADCS", "Standalone fadc 500MHz");
  fIsLinked = true;
  fTRGON = 1;
  fPTRG = 0;
  fPSC = 1;
  fADCType = ADC::FADCS;
  
  SetLink();
}

void FADCSConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  cout << Form(" ++ %s config: SID(%d) MID(%1d) NCH(%1d) RL(%d) TLT(%X) "
               "DSR(%d) TRGON(%d) PTRG(%d) PSCALE(%d)",
               GetName(), fSID, fMID, fNCH, fRL, fTLT, fDSR, fTRGON, fPTRG, fPSC)
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